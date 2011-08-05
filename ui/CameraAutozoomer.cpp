
#include "precompiled.h"

#include "CameraAutozoomer.h"

#include "GameCamera.h"
#include "../game/SimpleOptions.h"
#include "../game/options/options_camera.h"


namespace ui
{
	static float caz_savedIndoorZoom = 0.0f;
	static float caz_savedOutdoorZoom = 0.0f;

	static float caz_indoorZoom = 0.0f;
	static float caz_outdoorZoom = 0.0f;

	//static CameraAutozoomer::CAMERA_AUTOZOOMER_AREA caz_lastArea = CameraAutozoomer::CAMERA_AUTOZOOMER_AREA_OUTDOOR;
	static CameraAutozoomer::CAMERA_AUTOZOOMER_AREA caz_currentArea = CameraAutozoomer::CAMERA_AUTOZOOMER_AREA_OUTDOOR;
	static CameraAutozoomer::CAMERA_AUTOZOOMER_AREA caz_savedArea = CameraAutozoomer::CAMERA_AUTOZOOMER_AREA_OUTDOOR;

	void CameraAutozoomer::setAreaZoom(CAMERA_AUTOZOOMER_AREA area, float areaZoom)
	{
		if (area == CAMERA_AUTOZOOMER_AREA_INDOOR)
		{
			caz_indoorZoom = areaZoom;
		}
		if (area == CAMERA_AUTOZOOMER_AREA_OUTDOOR)
		{
			caz_outdoorZoom = areaZoom;
		}
	}

	void CameraAutozoomer::setCurrentArea(CAMERA_AUTOZOOMER_AREA area)
	{
		caz_currentArea = area;
	}

	void CameraAutozoomer::run(GameCamera *camera)
	{
		assert(caz_indoorZoom != 0.0f);
		assert(caz_outdoorZoom != 0.0f);

		if (caz_currentArea == CAMERA_AUTOZOOMER_AREA_INDOOR)
		{
			if (caz_indoorZoom != camera->getZoom())
			{
				camera->zoomTo(caz_indoorZoom);
			}
		}
		else if (caz_currentArea == CAMERA_AUTOZOOMER_AREA_OUTDOOR)
		{
			if (caz_outdoorZoom != camera->getZoom())
			{
				camera->zoomTo(caz_outdoorZoom);
			}
		}
	}

	float CameraAutozoomer::getZoom(GameCamera *camera)
	{
		if (caz_currentArea == CAMERA_AUTOZOOMER_AREA_INDOOR)
		{
			return caz_indoorZoom;
		}
		else if (caz_currentArea == CAMERA_AUTOZOOMER_AREA_OUTDOOR)
		{
			return caz_outdoorZoom;
		}
		return caz_outdoorZoom;
	}

	void CameraAutozoomer::resetForNewMission()
	{
		caz_indoorZoom = game::SimpleOptions::getFloat(DH_OPT_F_CAMERA_AUTOZOOM_INDOOR);
		caz_outdoorZoom = game::SimpleOptions::getFloat(DH_OPT_F_CAMERA_AUTOZOOM_OUTDOOR);
		caz_currentArea = CAMERA_AUTOZOOMER_AREA_OUTDOOR;

		saveCheckpointState();
	}

	void CameraAutozoomer::saveCheckpointState()
	{
		caz_savedIndoorZoom = caz_indoorZoom;
		caz_savedOutdoorZoom = caz_outdoorZoom;
		caz_savedArea = caz_currentArea;
	}

	void CameraAutozoomer::loadCheckpointState()
	{
		caz_indoorZoom = caz_savedIndoorZoom;
		caz_outdoorZoom = caz_savedOutdoorZoom;
		caz_currentArea = caz_savedArea;
	}


}

