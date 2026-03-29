// CCKaleida.cpp — OpenFX Kaleidoscope (CC Kaleida–style)
// Build as standard OFX image effect plugin
// No placeholders, no comments

#include <cmath>
#include <cstring>
#include <OFXImageEffect.h>

#define PLUGIN_NAME "CCKaleida"
#define PLUGIN_GROUP "Stylize"
#define PLUGIN_DESC "Kaleidoscope effect"
#define PLUGIN_ID "com.openfx.cckaleida"
#define PLUGIN_VER_MAJOR 1
#define PLUGIN_VER_MINOR 0

using namespace OFX;

class CCKaleida : public ImageEffect
{
public:
  CCKaleida(OfxImageEffectHandle handle)
    : ImageEffect(handle)
  {
    srcClip = fetchClip("Source");
    dstClip = fetchClip("Output");

    reflections = fetchIntParam("Reflections");
    angle = fetchDoubleParam("Angle");
    centerX = fetchDoubleParam("CenterX");
    centerY = fetchDoubleParam("CenterY");
  }

  void render(const RenderArguments &args) override
  {
    std::unique_ptr<Image> src(srcClip->fetchImage(args.time));
    std::unique_ptr<Image> dst(dstClip->fetchImage(args.time));

    if (!src || !dst)
      return;

    OfxRectI bounds = args.renderWindow;
    double a = angle->getValueAtTime(args.time) * M_PI / 180.0;
    int n = reflections->getValueAtTime(args.time);
    double cx = centerX->getValueAtTime(args.time);
    double cy = centerY->getValueAtTime(args.time);

    double sector = (2.0 * M_PI) / std::max(1, n);

    for (int y = bounds.y1; y < bounds.y2; ++y)
    {
      for (int x = bounds.x1; x < bounds.x2; ++x)
      {
        double dx = x - cx;
        double dy = y - cy;

        double r = std::sqrt(dx * dx + dy * dy);
        double theta = std::atan2(dy, dx) + a;

        theta = std::fmod(theta, 2.0 * M_PI);
        if (theta < 0.0)
          theta += 2.0 * M_PI;

        int sectorIndex = int(theta / sector);
        double local = std::fmod(theta, sector);
        if (sectorIndex & 1)
          local = sector - local;

        double sx = cx + r * std::cos(local);
        double sy = cy + r * std::sin(local);

        float *dstPix = (float *)dst->getPixelAddress(x, y);
        float *srcPix = (float *)src->getPixelAddress(int(sx), int(sy));

        if (srcPix && dstPix)
        {
          dstPix[0] = srcPix[0];
          dstPix[1] = srcPix[1];
          dstPix[2] = srcPix[2];
          dstPix[3] = srcPix[3];
        }
      }
    }
  }

private:
  Clip *srcClip;
  Clip *dstClip;

  IntParam *reflections;
  DoubleParam *angle;
  DoubleParam *centerX;
  DoubleParam *centerY;
};

mDeclarePluginFactory(CCKaleidaFactory, {}, {});

void CCKaleidaFactory::describe(ImageEffectDescriptor &desc)
{
  desc.setLabels(PLUGIN_NAME, PLUGIN_NAME, PLUGIN_NAME);
  desc.setPluginGrouping(PLUGIN_GROUP);
  desc.setPluginDescription(PLUGIN_DESC);
  desc.addSupportedContext(eContextFilter);
  desc.addSupportedBitDepth(eBitDepthFloat);
  desc.setSingleInstance(false);
  desc.setHostFrameThreading(true);
  desc.setSupportsMultiResolution(true);
  desc.setSupportsTiles(true);
  desc.setRenderTwiceAlways(false);
}

void CCKaleidaFactory::describeInContext(ImageEffectDescriptor &desc, ContextEnum)
{
  ClipDescriptor *src = desc.defineClip("Source");
  src->addSupportedComponent(ePixelComponentRGBA);
  src->setTemporalClipAccess(false);
  src->setSupportsTiles(true);

  ClipDescriptor *dst = desc.defineClip("Output");
  dst->addSupportedComponent(ePixelComponentRGBA);
  dst->setSupportsTiles(true);

  PageParamDescriptor *page = desc.definePageParam("Controls");

  IntParamDescriptor *reflections =
      desc.defineIntParam("Reflections");
  reflections->setDefault(6);
  reflections->setRange(1, 24);
  reflections->setDisplayRange(1, 24);
  page->addChild(*reflections);

  DoubleParamDescriptor *angle =
      desc.defineDoubleParam("Angle");
  angle->setDefault(0.0);
  angle->setRange(-360.0, 360.0);
  angle->setDisplayRange(-360.0, 360.0);
  page->addChild(*angle);

  DoubleParamDescriptor *centerX =
      desc.defineDoubleParam("CenterX");
  centerX->setDefault(0.0);
  centerX->setRange(-9999.0, 9999.0);
  centerX->setDisplayRange(-9999.0, 9999.0);
  page->addChild(*centerX);

  DoubleParamDescriptor *centerY =
      desc.defineDoubleParam("CenterY");
  centerY->setDefault(0.0);
  centerY->setRange(-9999.0, 9999.0);
  centerY->setDisplayRange(-9999.0, 9999.0);
  page->addChild(*centerY);
}

ImageEffect* CCKaleidaFactory::createInstance(OfxImageEffectHandle handle, ContextEnum)
{
  return new CCKaleida(handle);
}

void getCCKaleidaPluginID(OFX::PluginFactoryArray &ids)
{
  static CCKaleidaFactory p(PLUGIN_ID, PLUGIN_VER_MAJOR, PLUGIN_VER_MINOR);
  ids.push_back(&p);
}
