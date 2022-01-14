#include <Gfx/Graph/VideoNodeRenderer.hpp>

#include <ossia/detail/flicks.hpp>
#include <ossia/detail/algorithms.hpp>

#include <QElapsedTimer>

namespace score::gfx
{


VideoNodeRenderer::VideoNodeRenderer(const VideoNode& node) noexcept
    : NodeRenderer{}
    , node{node}
{
}

VideoNodeRenderer::~VideoNodeRenderer()
{

}

TextureRenderTarget VideoNodeRenderer::renderTargetForInput(const Port& input)
{
  return {};
}

void VideoNodeRenderer::createPipelines(RenderList& r)
{
  auto& gpu = this->node.m_resources.at(&r);
  if (gpu.m_gpu)
  {
    auto& shaders = gpu.shaders;
    SCORE_ASSERT(m_p.empty());
    for(Edge* edge : this->node.output[0]->edges)
    {
      auto rt = r.renderTargetForOutput(*edge);
      if(rt.renderTarget)
      {
        auto& mesh = TexturedTriangle::instance();
        m_p.emplace_back(edge, score::gfx::buildPipeline(
              r,
              mesh,
              shaders.first,
              shaders.second,
              rt,
              m_processUBO,
              m_materialUBO,
              gpu.m_gpu->samplers));
      }
    }
  }
}

void VideoNodeRenderer::releasePipelines()
{
  for(auto& p : m_p)
  {
    p.second.release();
  }
  m_p.clear();
}

void VideoNodeRenderer::init(RenderList& renderer)
{
  auto& rhi = *renderer.state.rhi;

  auto& mesh = TexturedQuad::instance();
  if (!m_meshBuffer)
  {
    auto [mbuffer, ibuffer] = renderer.initMeshBuffer(mesh);
    m_meshBuffer = mbuffer;
    m_idxBuffer = ibuffer;
  }

  #include <Gfx/Qt5CompatPush> // clang-format: keep
  m_processUBO = rhi.newBuffer(
      QRhiBuffer::Dynamic, QRhiBuffer::UniformBuffer, sizeof(ProcessUBO));
  m_processUBO->create();

  m_materialUBO = rhi.newBuffer(
  QRhiBuffer::Dynamic, QRhiBuffer::UniformBuffer, sizeof(Material));
  m_materialUBO->create();

  #include <Gfx/Qt5CompatPop> // clang-format: keep

  createPipelines(renderer);
  m_recomputeScale = true;
}


// TODO if we have multiple renderers for the same video, we must always keep
// a frame because rendered may have different rates, so we cannot know "when"
// all renderers have rendered, thue the pattern in the following function
// is not enough
void VideoNodeRenderer::update(RenderList& renderer, QRhiResourceUpdateBatch& res)
{
  res.updateDynamicBuffer(
      m_processUBO, 0, sizeof(ProcessUBO), &this->node.standardUBO);

  auto& gpu = const_cast<VideoNode&>(this->node).m_resources.at(&renderer);
  if(gpu.decoderChanged)
  {
    gpu.decoderChanged = false;
    m_recomputeScale = true;

    releasePipelines();
    createPipelines(renderer);
  }

  if(m_recomputeScale || m_currentScaleMode != this->node.m_scaleMode)
  {
    auto& n = this->node.m_feeder;
    m_currentScaleMode = this->node.m_scaleMode;
    auto sz = computeScale(m_currentScaleMode, renderer.state.size, QSizeF(n.m_currentWidth, n.m_currentHeight));
    Material mat;
    mat.scale_w = sz.width();
    mat.scale_h = sz.height();

    res.updateDynamicBuffer(m_materialUBO, 0, sizeof(Material), &mat);
    m_recomputeScale = false;
  }
}


void VideoNodeRenderer::runRenderPass(RenderList& renderer, QRhiCommandBuffer& cb, Edge& edge)
{
  auto it = ossia::find_if(m_p, [ptr=&edge] (const auto& p){ return p.first == ptr; });
  SCORE_ASSERT(it != m_p.end());
  {
    const auto sz = renderer.state.size;
    cb.setGraphicsPipeline(it->second.pipeline);
    cb.setShaderResources(it->second.srb);
    cb.setViewport(QRhiViewport(0, 0, sz.width(), sz.height()));

    assert(this->m_meshBuffer);
    assert(this->m_meshBuffer->usage().testFlag(QRhiBuffer::VertexBuffer));
    auto& mesh = TexturedQuad::instance();

    mesh.setupBindings(*this->m_meshBuffer, this->m_idxBuffer, cb);

    cb.draw(mesh.vertexCount);
  }
}


void VideoNodeRenderer::release(RenderList& r)
{
  delete m_processUBO;
  m_processUBO = nullptr;

  delete m_materialUBO;
  m_materialUBO = nullptr;

  for(auto& p : m_p)
    p.second.release();
  m_p.clear();

  m_meshBuffer = nullptr;
}

}
