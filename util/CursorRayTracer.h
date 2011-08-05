
#ifndef CURSORRAYTRACER_H
#define CURSORRAYTRACER_H

/**
 *
 * For picking an object from 3d world based on cursor screen coordinates.
 *
 */

#include <Storm3D_Datatypes.h>

class IStorm3D;
class IStorm3D_Scene;
class IStorm3D_Terrain;


class CursorRayTracer
{
public:
  CursorRayTracer(IStorm3D *s3d, IStorm3D_Scene *scene, IStorm3D_Terrain *terrain, int screenSizeX, int screenSizeY);
  ~CursorRayTracer();

  void setCameraVector(VC3 camera);

  void rayTrace(int x, int y, Storm3D_CollisionInfo &cinfo, bool sideways, bool terrainOnly, bool accurate = false);

private:
  int screenSizeX;
  int screenSizeY;
  IStorm3D *s3d;
  IStorm3D_Scene *scene;
  IStorm3D_Terrain *terrain;
  VC3 cameraVector;

};


#endif
