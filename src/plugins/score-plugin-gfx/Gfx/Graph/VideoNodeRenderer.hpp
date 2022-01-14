#pragma once
#include <Gfx/Graph/VideoNode.hpp>
#include <Gfx/Graph/NodeRenderer.hpp>
#include <Video/VideoInterface.hpp>

namespace score::gfx
{
class GPUVideoDecoder;


class VideoNodeRenderer
    : public NodeRenderer
{
public:
  explicit VideoNodeRenderer(const VideoNode& node) noexcept;
  ~VideoNodeRenderer();

  VideoNodeRenderer() = delete;
  VideoNodeRenderer(const VideoNodeRenderer&) = delete;
  VideoNodeRenderer(VideoNodeRenderer&&) = delete;
  VideoNodeRenderer& operator=(const VideoNodeRenderer&) = delete;
  VideoNodeRenderer& operator=(VideoNodeRenderer&&) = delete;

  TextureRenderTarget renderTargetForInput(const Port& input) override;

  void init(RenderList& renderer) override;
  void runRenderPass(
      RenderList&,
      QRhiCommandBuffer& commands,
      Edge& edge) override;

  void update(RenderList& renderer, QRhiResourceUpdateBatch& res) override;
  void release(RenderList& r) override;

private:
  void createPipelines(RenderList& r);
  const VideoNode& node;

  std::vector<std::pair<Edge*, Pipeline>> m_p;
  QRhiBuffer* m_meshBuffer{};
  QRhiBuffer* m_idxBuffer{};
  QRhiBuffer* m_processUBO{};
  QRhiBuffer* m_materialUBO{};

  struct Material {
    float scale_w{}, scale_h{};
  };

  score::gfx::ScaleMode m_currentScaleMode{};
  bool m_recomputeScale{};
  void releasePipelines();
};

}
