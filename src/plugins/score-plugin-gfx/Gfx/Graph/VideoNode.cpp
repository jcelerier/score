#include <Gfx/Graph/VideoNode.hpp>
#include <Gfx/Graph/VideoNodeRenderer.hpp>

#include <Gfx/Graph/decoders/HAP.hpp>
#include <Gfx/Graph/decoders/RGBA.hpp>
#include <Gfx/Graph/decoders/YUV420.hpp>
#include <Gfx/Graph/decoders/YUV422.hpp>
#include <Gfx/Graph/decoders/YUYV422.hpp>

#include <QTimer>
QTimer* tm{};
namespace score::gfx
{
VideoNode::VideoNode(
    std::shared_ptr<Video::VideoInterface> dec,
    std::optional<double> nativeTempo,
    QString f)
    : m_decoder{std::move(dec)}
    , m_filter{f}
    , m_feeder{*this}
    , m_nativeTempo{nativeTempo}
{
  output.push_back(new Port{this, {}, Types::Image, {}});
  if(!tm)
    tm = new QTimer;
}

void VideoNode::initResources(RenderList& r)
{
  m_feeder.init(*this, r);

}

void VideoNode::releaseResources(RenderList& r)
{
  m_feeder.release(*this, r);
}

VideoNode::~VideoNode() { }

score::gfx::NodeRenderer*
VideoNode::createRenderer(RenderList& r) const noexcept
{
  return new VideoNodeRenderer{*this};
}

void VideoNode::seeked()
{
  SCORE_TODO;
}

void VideoNode::setScaleMode(ScaleMode s)
{
  m_scaleMode = s;
}


GPUVideoFeeder::GPUVideoFeeder(const VideoNode& node) noexcept
  : node{node}
{
  auto& dec = decoder();
  m_currentFormat = dec.pixel_format;
  m_currentWidth = dec.width;
  m_currentHeight = dec.height;
}

GPUVideoFeeder::~GPUVideoFeeder()
{
  auto& dec = decoder();
  for (auto frame : m_framesToFree)
    dec.release_frame(frame);
}

std::unique_ptr<GPUVideoDecoder> GPUVideoFeeder::createGpuDecoder(VideoNode& model)
{
  auto& dec = decoder();
  auto& filter = model.m_filter;
  switch (m_currentFormat)
  {
    case AV_PIX_FMT_YUV420P:
    case AV_PIX_FMT_YUVJ420P:
      return std::make_unique<YUV420Decoder>(dec);
    case AV_PIX_FMT_YUVJ422P:
    case AV_PIX_FMT_YUV422P:
      return std::make_unique<YUV422Decoder>(dec);
    case AV_PIX_FMT_UYVY422:
      return std::make_unique<UYVY422Decoder>(dec);
    case AV_PIX_FMT_YUYV422:
      return std::make_unique<YUYV422Decoder>(dec);
    case AV_PIX_FMT_RGB0:
      return std::make_unique<PackedDecoder>(
          QRhiTexture::RGBA8, 4, dec, "processed.a = 1.0; " + filter);
    case AV_PIX_FMT_RGBA:
      return std::make_unique<PackedDecoder>(
          QRhiTexture::RGBA8, 4, dec, filter);
    case AV_PIX_FMT_BGR0:
      return std::make_unique<PackedDecoder>(
          QRhiTexture::BGRA8, 4, dec, "processed.a = 1.0; " + filter);
    case AV_PIX_FMT_BGRA:
      return std::make_unique<PackedDecoder>(
          QRhiTexture::BGRA8, 4, dec, filter);
    case AV_PIX_FMT_ARGB:
      // Go from ARGB  xyzw
      //      to RGBA  yzwx
      return std::make_unique<PackedDecoder>(
            QRhiTexture::RGBA8, 4, dec, "processed.rgba = tex.yzwx; " + filter);
    case AV_PIX_FMT_ABGR:
      // Go from ABGR  xyzw
      //      to BGRA  yzwx
      return std::make_unique<PackedDecoder>(
            QRhiTexture::BGRA8, 4, dec, "processed.bgra = tex.yzwx; " + filter);
#if LIBAVUTIL_VERSION_INT >= AV_VERSION_INT(56, 19, 100)
    case AV_PIX_FMT_GRAYF32LE:
    case AV_PIX_FMT_GRAYF32BE:
      return std::make_unique<PackedDecoder>(
          QRhiTexture::R32F, 4, dec, filter);
#endif
    case AV_PIX_FMT_GRAY8:
      return std::make_unique<PackedDecoder>(
          QRhiTexture::R8, 1, dec, "processed.rgba = vec4(tex.r, tex.r, tex.r, 1.0);" + filter);
    case AV_PIX_FMT_GRAY16:
      return std::make_unique<PackedDecoder>(
            QRhiTexture::R16, 2, dec, "processed.rgba = vec4(tex.r, tex.r, tex.r, 1.0);" + filter);
    default:
    {
      // try to read format as a 4cc
      std::string_view fourcc{(const char*)&m_currentFormat, 4};

      if (fourcc == "Hap1")
        return std::make_unique<HAPDefaultDecoder>(
            QRhiTexture::BC1, dec, filter);
      else if (fourcc == "Hap5")
        return std::make_unique<HAPDefaultDecoder>(
            QRhiTexture::BC3, dec, filter);
      else if (fourcc == "HapY")
        return std::make_unique<HAPDefaultDecoder>(
            QRhiTexture::BC3,
            dec,
            HAPDefaultDecoder::ycocg_filter + filter);
      else if (fourcc == "HapM")
        return std::make_unique<HAPMDecoder>(dec, filter);
      else if (fourcc == "HapA")
        return std::make_unique<HAPDefaultDecoder>(
            QRhiTexture::BC4, dec, filter);
      else if (fourcc == "Hap7")
        return std::make_unique<HAPDefaultDecoder>(
            QRhiTexture::BC7, dec, filter);

      break;
    }
  }

  qDebug() << "Unhandled pixel format: "
           << av_get_pix_fmt_name(m_currentFormat);
  return std::make_unique<EmptyDecoder>();

  // FIXME m_recomputeScale = true;
}
/*
void GPUVideoFeeder::setupGpuDecoder(VideoNode& node, RenderList& r)
{
  if (m_gpu)
  {
    m_gpu->release(r);

    for(auto& p : m_p)
    {
      p.second.release();
    }
    m_p.clear();
  }


  createGpuDecoder(node);

  createPipelines(r);
}
*/
void GPUVideoFeeder::checkFormat(VideoNode& node, RenderList& r, AVPixelFormat fmt, int w, int h)
{
  auto& gpu = node.m_resources.at(&r);

  // TODO won't work if VK is threaded and there are multiple windows
  if (!gpu.m_gpu || fmt != m_currentFormat || w != m_currentWidth
      || h != m_currentHeight)
  {
    m_currentFormat = fmt;
    m_currentWidth = w;
    m_currentHeight = h;

    if(gpu.m_gpu)
      gpu.m_gpu->release(r);

    gpu.m_gpu = createGpuDecoder(node);
    if(gpu.m_gpu)
      gpu.shaders = gpu.m_gpu->init(r);
    gpu.decoderChanged = true;
  }
  else
  {
    m_currentFormat = fmt;
    m_currentWidth = w;
    m_currentHeight = h;
  }
}

GPUVideoResources GPUVideoFeeder::init(VideoNode& node, RenderList& renderer)
{
  if(auto m_gpu = createGpuDecoder(node))
  {
    auto shaders = m_gpu->init(renderer);
    return GPUVideoResources{std::move(m_gpu), std::move(shaders), true};
  }
  return {};
}

void GPUVideoFeeder::update(VideoNode& node, RenderList& renderer, QRhiResourceUpdateBatch& res)
{
  auto& decoder = this->decoder();

  // Release frames from the previous update (which have necessarily been uploaded)
  for (auto frame : m_framesToFree)
    decoder.release_frame(frame);
  m_framesToFree.clear();

  // Camera and other sources where we always want the latest frame
  if(decoder.realTime)
  {
    if (auto frame = decoder.dequeue_frame())
    {
      displayRealTimeFrame(node, *frame, renderer, res);
    }
  }
  else if (mustReadVideoFrame(node))
  {
    // Video files which require more precise timing handling
    if (auto frame = nextFrame(node))
    {
      displayVideoFrame(node, *frame, renderer, res);
    }
  }
}

void GPUVideoFeeder::release(VideoNode& node, RenderList& r)
{
  auto& rr = this->node.m_resources;
  if(auto it = rr.find(&r); it != rr.end())
  {
    auto& gpu = it->second;
    if (gpu.m_gpu)
      gpu.m_gpu->release(r);
  }
}

AVFrame* GPUVideoFeeder::nextFrame(VideoNode& nodem)
{
  auto& decoder = this->decoder();

  //double expected_frame = nodem.standardUBO.time / decoder.fps;
  double current_flicks = nodem.standardUBO.time * ossia::flicks_per_second<double>;
  double flicks_per_frame = ossia::flicks_per_second<double> / 25.;

  ossia::small_vector<AVFrame*, 8> prev{};

  if(auto frame = m_nextFrame)
  {
    auto drift_in_frames = (current_flicks - decoder.flicks_per_dts * frame->pts) / flicks_per_frame;

    if (abs(drift_in_frames) <= 1.)
    {
      // we can finally show this frame
      m_nextFrame = nullptr;
      return frame;
    }
    else if(abs(drift_in_frames) > 5.)
    {
      // we likely seeked, move on to the dequeue
      prev.push_back(frame);
      m_nextFrame = nullptr;
    }
    else if(drift_in_frames < -1.)
    {
      // we early, keep showing the current frame (e.g. do nothing)
      return nullptr;
    }
    else if(drift_in_frames > 1.)
    {
      // we late, move on to the dequeue
      prev.push_back(frame);
      m_nextFrame = nullptr;
    }
  }

  while (auto frame = decoder.dequeue_frame())
  {
    auto drift_in_frames = (current_flicks - decoder.flicks_per_dts * frame->pts) / flicks_per_frame;

    if (abs(drift_in_frames) <= 1.)
    {
      m_framesToFree.insert(m_framesToFree.end(), prev.begin(), prev.end());
      return frame;
    }
    else if(abs(drift_in_frames) > 5.)
    {
      // we likely seeked, dequeue
      prev.push_back(frame);
      if(prev.size() >= 8)
        break;

      continue;
    }
    else if(drift_in_frames < -1.)
    {
      //current_time < frame_time: we are in advance by more than one frame, keep showing the current frame
      m_nextFrame = frame;
      m_framesToFree.insert(m_framesToFree.end(), prev.begin(), prev.end());
      return nullptr;
    }
    else if(drift_in_frames > 1.)
    {
      //current_time > frame_time: we are late by more than one frame, fetch new frames
      prev.push_back(frame);
      if(prev.size() >= 8)
        break;

      continue;
    }

    m_framesToFree.insert(m_framesToFree.end(), prev.begin(), prev.end());
    return frame;
  }

  switch(prev.size())
  {
    case 0:
    {
      return nullptr;
    }
    case 1:
    {
      return prev.back(); // display the closest frame we got
    }
    default:
    {
      for(std::size_t i = 0; i < prev.size() - 1; ++i) {
        m_framesToFree.push_back(prev[i]);
      }
      return prev.back();
    }
  }
  return nullptr;
}

Video::VideoInterface& GPUVideoFeeder::decoder() const noexcept
{
  return *node.m_decoder;
}

void GPUVideoFeeder::displayRealTimeFrame(VideoNode& node, AVFrame& frame, RenderList& renderer, QRhiResourceUpdateBatch& res)
{
  checkFormat(node,
      renderer,
      static_cast<AVPixelFormat>(frame.format),
      frame.width,
      frame.height);

  for(auto& [p, gpu] : node.m_resources)
  {
    if (gpu.m_gpu)
    {
      gpu.m_gpu->exec(renderer, res, frame);
    }
  }

  m_framesToFree.push_back(&frame);
}

void GPUVideoFeeder::displayVideoFrame(VideoNode& node, AVFrame& frame, RenderList& renderer, QRhiResourceUpdateBatch& res)
{
  auto& decoder = this->decoder();

  checkFormat(node,
      renderer,
      static_cast<AVPixelFormat>(frame.format),
      frame.width,
      frame.height);


  for(auto& [p, gpu] : node.m_resources)
  {
    if (gpu.m_gpu)
    {
      gpu.m_gpu->exec(renderer, res, frame);
    }
  }

  m_lastFrameTime
      = (decoder.flicks_per_dts * frame.pts) / ossia::flicks_per_second<double>;

  m_framesToFree.push_back(&frame);
  m_timer.restart();
}

bool GPUVideoFeeder::mustReadVideoFrame(VideoNode& nodem)
{
  if(!std::exchange(m_readFrame, true))
    return true;

  //auto& nodem = const_cast<VideoNode&>(static_cast<const VideoNode&>(node));
  auto& decoder = this->decoder();

  double tempoRatio = 1.;
  if (nodem.m_nativeTempo)
    tempoRatio = (*nodem.m_nativeTempo) / 120.;

  auto current_time = nodem.standardUBO.time * tempoRatio; // In seconds
  auto next_frame_time = m_lastFrameTime;

  // pause
  if (nodem.standardUBO.time == m_lastPlaybackTime)
  {
    return false;
  }
  m_lastPlaybackTime = nodem.standardUBO.time;

  // what more can we do ?
  const double inv_fps
      = decoder.fps > 0
      ? (1. / (tempoRatio * decoder.fps))
      : (1. / 24.);
  next_frame_time += inv_fps;

  // If we are late
  if(current_time > next_frame_time)
    return true;

  // If we must display the next frame
  if(abs(m_timer.elapsed() - (1000. * inv_fps)) > 0.)
    return true;

  return false;
}
}
#include <hap/source/hap.c>
