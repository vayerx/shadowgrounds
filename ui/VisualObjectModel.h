
#ifndef VISUALOBJECTMODEL_H
#define VISUALOBJECTMODEL_H

#include "VisualObject.h"


#define VISUALOBJECTMODEL_EFFECT_ADDITIVE 1
#define VISUALOBJECTMODEL_EFFECT_TRANSPARENCY 2
#define VISUALOBJECTMODEL_EFFECT_TRANSPARENT_80 4
#define VISUALOBJECTMODEL_EFFECT_TRANSPARENT_50 8
#define VISUALOBJECTMODEL_EFFECT_TRANSPARENT_30 16
#define VISUALOBJECTMODEL_EFFECT_LESS_DIFFUSE 32
#define VISUALOBJECTMODEL_EFFECT_MULTIPLY 64
#define VISUALOBJECTMODEL_EFFECT_ELECTRIC 128
#define VISUALOBJECTMODEL_EFFECT_BURNING 256
#define VISUALOBJECTMODEL_EFFECT_SLIME 512
#define VISUALOBJECTMODEL_EFFECT_CLOAK 1024
#define VISUALOBJECTMODEL_EFFECT_CLOAKHIT 2048
#define VISUALOBJECTMODEL_EFFECT_BURNED_CRISPY 4096
#define VISUALOBJECTMODEL_EFFECT_PROTECTIVESKIN 8192
#define VISUALOBJECTMODEL_EFFECT_CLOAKRED 16384


class IStorm3D;
class IStorm3D_Scene;
class IStorm3D_Model;
class OdeStormModelCooker;

namespace game
{
  // bad!!!
  class Projectile;
  class GameScene;
}


namespace ui
{
  // quick hack for friend operator...
  class GamePointers;

  class VisualObjectModel
  {
  public:
    static void setVisualStorm(IStorm3D *s3d, IStorm3D_Scene *scene);

    VisualObjectModel(const char *filename);
    ~VisualObjectModel();

    void freeObjectInstance();
    VisualObject *getNewObjectInstance();

		inline const char *getFilename() { return filename; }

  private:
    static IStorm3D *visualStorm;
    static IStorm3D_Scene *visualStormScene;

    IStorm3D_Model *sharedModel;
    int refCount;
    char *filename;

    // quick hack to access storm...
    friend class GamePointers;
		friend class game::GameScene;
		friend class OdeStormModelCooker;
  };

}

#endif
