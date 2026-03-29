// BlinkTextOFX.cpp
#include <string>
#include <cstring>
#include <cmath>
#include "ofxImageEffect.h"

#define PLUGIN_NAME        "BlinkTextShader"
#define PLUGIN_GROUP       "UserShaders"
#define PLUGIN_DESC        "User-defined shader text with blink effect"
#define PLUGIN_IDENTIFIER  "com.example.blinktextshader"
#define PLUGIN_VERSION_MAJOR 1
#define PLUGIN_VERSION_MINOR 0

static OfxHost *gHost = nullptr;

class BlinkTextEffect {
public:
    OfxImageEffectHandle effect;
    OfxParamHandle shaderText;
    OfxParamHandle blinkSpeed;

    BlinkTextEffect(OfxImageEffectHandle h)
        : effect(h), shaderText(nullptr), blinkSpeed(nullptr) {}

    void setupParams(OfxParamSetHandle params) {
        OfxPropertySetHandle props;

        gHost->paramDefine(params, kOfxParamTypeString, "shaderText", &shaderText);
        gHost->paramGetPropertySet(shaderText, &props);
        gHost->propSetString(props, kOfxPropLabel, 0, "Shader Text");
        gHost->paramSetValue(shaderText,
            "vec4 blink(vec2 uv, float time) {"
            "float v = abs(sin(time));"
            "return vec4(v, v, v, 1.0);"
            "}"
        );

        gHost->paramDefine(params, kOfxParamTypeDouble, "blinkSpeed", &blinkSpeed);
        gHost->paramGetPropertySet(blinkSpeed, &props);
        gHost->propSetString(props, kOfxPropLabel, 0, "Blink Speed");
        gHost->paramSetValue(blinkSpeed, 2.0);
    }

    void render(OfxTime time, OfxImageEffectRenderArguments *args) {
        OfxPropertySetHandle outputProps;
        OfxImageHandle outputImg = nullptr;

        gHost->clipGetImage(args->outputClip, time, nullptr, &outputImg);
        if (!outputImg) return;

        gHost->imageGetPropertySet(outputImg, &outputProps);

        int width = gHost->propGetInt(outputProps, kOfxImagePropBounds, 2);
        int height = gHost->propGetInt(outputProps, kOfxImagePropBounds, 3);
        float speed = 1.0;
        gHost->paramGetValue(blinkSpeed, &speed);

        float blink = std::fabs(std::sin(time * speed));

        unsigned char *data = nullptr;
        gHost->propGetPointer(outputProps, kOfxImagePropData, 0, (void **)&data);

        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                int idx = (y * width + x) * 4;
                unsigned char v = static_cast<unsigned char>(blink * 255.0f);
                data[idx + 0] = v;
                data[idx + 1] = v;
                data[idx + 2] = v;
                data[idx + 3] = 255;
            }
        }

        gHost->clipReleaseImage(outputImg);
    }
};

static OfxStatus renderAction(OfxImageEffectHandle effect,
                             OfxPropertySetHandle,
                             OfxPropertySetHandle inArgs,
                             OfxPropertySetHandle) {
    BlinkTextEffect *inst = nullptr;
    gHost->imageEffectGetInstanceData(effect, (void **)&inst);
    if (!inst) return kOfxStatFailed;

    OfxImageEffectRenderArguments args;
    std::memset(&args, 0, sizeof(args));
    args.time = gHost->propGetDouble(inArgs, kOfxPropTime, 0);
    args.outputClip = nullptr;
    gHost->imageEffectGetClipHandle(effect, "Output", &args.outputClip, nullptr);

    inst->render(args.time, &args);
    return kOfxStatOK;
}

static OfxStatus describe(OfxImageEffectHandle effect) {
    OfxPropertySetHandle props;
    gHost->imageEffectGetPropertySet(effect, &props);
    gHost->propSetString(props, kOfxPropLabel, 0, PLUGIN_NAME);
    gHost->propSetString(props, kOfxImageEffectPluginPropGrouping, 0, PLUGIN_GROUP);
    gHost->propSetString(props, kOfxImageEffectPropSupportedContexts, 0, kOfxImageEffectContextFilter);
    gHost->propSetString(props, kOfxImageEffectPropSupportedPixelDepths, 0, kOfxBitDepthByte);
    return kOfxStatOK;
}

static OfxStatus describeInContext(OfxImageEffectHandle effect, OfxPropertySetHandle) {
    OfxParamSetHandle params;
    gHost->imageEffectGetParamSet(effect, &params);

    BlinkTextEffect temp(effect);
    temp.setupParams(params);

    return kOfxStatOK;
}

static OfxStatus createInstance(OfxImageEffectHandle effect) {
    BlinkTextEffect *inst = new BlinkTextEffect(effect);
    OfxParamSetHandle params;
    gHost->imageEffectGetParamSet(effect, &params);
    inst->setupParams(params);
    gHost->imageEffectSetInstanceData(effect, inst);
    return kOfxStatOK;
}

static OfxStatus destroyInstance(OfxImageEffectHandle effect) {
    BlinkTextEffect *inst = nullptr;
    gHost->imageEffectGetInstanceData(effect, (void **)&inst);
    delete inst;
    return kOfxStatOK;
}

static OfxStatus mainEntry(const char *action,
                           const void *handle,
                           OfxPropertySetHandle inArgs,
                           OfxPropertySetHandle outArgs) {
    if (std::strcmp(action, kOfxActionDescribe) == 0)
        return describe((OfxImageEffectHandle)handle);
    if (std::strcmp(action, kOfxActionDescribeInContext) == 0)
        return describeInContext((OfxImageEffectHandle)handle, inArgs);
    if (std::strcmp(action, kOfxActionCreateInstance) == 0)
        return createInstance((OfxImageEffectHandle)handle);
    if (std::strcmp(action, kOfxActionDestroyInstance) == 0)
        return destroyInstance((OfxImageEffectHandle)handle);
    if (std::strcmp(action, kOfxImageEffectActionRender) == 0)
        return renderAction((OfxImageEffectHandle)handle, nullptr, inArgs, outArgs);
    return kOfxStatReplyDefault;
}

static OfxPlugin plugin = {
    kOfxImageEffectPluginApi,
    1,
    PLUGIN_IDENTIFIER,
    PLUGIN_VERSION_MAJOR,
    PLUGIN_VERSION_MINOR,
    mainEntry,
    nullptr
};

extern "C" {
OfxExport int OfxGetNumberOfPlugins(void) {
    return 1;
}
OfxExport OfxPlugin *OfxGetPlugin(int) {
    return &plugin;
}
}
