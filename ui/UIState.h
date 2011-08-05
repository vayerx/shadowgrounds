
#ifndef UISTATE_H
#define UISTATE_H

namespace ui
{
  class UIState
  {
  public:
    bool guiVisible;
    int camera;
    //GameCamera::CAMERA_MODE cameraModes[GAMEUI_CAMERAS_AMOUNT];
    float cameraTimeFactor;
		float cameraFOV;
		float cameraTargDist;

		int id;

		UIState()
		{
			id = 0;
			guiVisible = false;
			camera = 0;
			cameraTimeFactor = 1.0f;
			cameraFOV = 120.0f;
			cameraTargDist = 1.0f;
		}
  };
}

#endif

