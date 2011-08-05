
#ifndef CAMERAAUTOZOOMER_H
#define CAMERAAUTOZOOMER_H

namespace ui
{
	//class CameraAutozoomerImpl;
	class GameCamera;

	class CameraAutozoomer
	{
		public:
			enum CAMERA_AUTOZOOMER_AREA
			{
				CAMERA_AUTOZOOMER_AREA_INDOOR = 1,
				CAMERA_AUTOZOOMER_AREA_OUTDOOR = 2
			};

			static void setAreaZoom(CAMERA_AUTOZOOMER_AREA area, float areaZoom);

			static void setCurrentArea(CAMERA_AUTOZOOMER_AREA area);

			static void run(GameCamera *camera);
			static float getZoom(GameCamera *camera);

			static void resetForNewMission();

			static void saveCheckpointState();
			static void loadCheckpointState();

		private:
			//CameraAutozoomerImpl *impl;
	};
}

#endif
