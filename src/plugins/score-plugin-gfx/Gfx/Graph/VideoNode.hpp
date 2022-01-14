#pragma once

#include <Gfx/Graph/Node.hpp>
#include <Video/VideoInterface.hpp>
#include <Gfx/Graph/decoders/GPUVideoDecoder.hpp>
namespace Video
{
struct VideoInterface;
}
namespace score::gfx
{
class GPUVideoDecoder;

struct GPUVideoResources {
  std::unique_ptr<GPUVideoDecoder> m_gpu;
  std::pair<QShader, QShader> shaders;
  bool decoderChanged{};
};
struct GPUVideoFeeder
{
  explicit GPUVideoFeeder(const VideoNode& node) noexcept;
  ~GPUVideoFeeder();

  std::unique_ptr<GPUVideoDecoder> createGpuDecoder(VideoNode& node);
  //void setupGpuDecoder(RenderList& r);
  void checkFormat(VideoNode& node, RenderList& r, AVPixelFormat fmt, int w, int h);

  GPUVideoResources init(VideoNode& node, RenderList& renderer);
  void update(VideoNode& node, RenderList& renderer, QRhiResourceUpdateBatch& res);
  void release(VideoNode& node, RenderList& r);

  void displayRealTimeFrame(VideoNode& node, AVFrame& frame, RenderList& renderer, QRhiResourceUpdateBatch& res);
  void displayVideoFrame(VideoNode& node, AVFrame& frame, RenderList& renderer, QRhiResourceUpdateBatch& res);
  bool mustReadVideoFrame(VideoNode& nodem);
  AVFrame* nextFrame(VideoNode& node);

  Video::VideoInterface& decoder() const noexcept;

  const VideoNode& node;
  std::vector<AVFrame*> m_framesToFree;

  AVPixelFormat m_currentFormat = AVPixelFormat(-1);
  int m_currentWidth = 0;
  int m_currentHeight = 0;

  QElapsedTimer m_timer;
  AVFrame* m_nextFrame{};

  double m_lastFrameTime{};
  double m_lastPlaybackTime{-1.};
  bool m_readFrame{};
};

class VideoNodeRenderer;
/**
 * @brief Model for rendering a video
 */
struct SCORE_PLUGIN_GFX_EXPORT VideoNode : ProcessNode
{
public:
  VideoNode(
      std::shared_ptr<Video::VideoInterface> dec,
      std::optional<double> nativeTempo,
      QString f = {});

  virtual ~VideoNode();

  score::gfx::NodeRenderer*
  createRenderer(RenderList& r) const noexcept override;

  void seeked();

  void setScaleMode(score::gfx::ScaleMode s);

  void initResources(RenderList& r) override;
  void releaseResources(RenderList& r) override;
private:
  friend VideoNodeRenderer;
  friend GPUVideoFeeder;

  std::shared_ptr<Video::VideoInterface> m_decoder;
  QString m_filter;

  GPUVideoFeeder m_feeder;

  std::optional<double> m_nativeTempo;
  score::gfx::ScaleMode m_scaleMode{};


  ossia::flat_map<RenderList*, GPUVideoResources> m_resources;
};

}
