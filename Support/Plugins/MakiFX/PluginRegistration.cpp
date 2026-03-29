#include <stdio.h>
#include "ofxsImageEffect.h"
#include "CCKaleida.h"
#include "UserDefinedShader.h"

namespace OFX 
{
  namespace Plugin 
  {
    void getPluginIDs(OFX::PluginFactoryArray &ids)
    {
      static CCKaleidaFactory p1;
      ids.push_back(&p1);
      static UserDefinedShaderEffectFactory p2;
      ids.push_back(&p2);
    }
  }
}
