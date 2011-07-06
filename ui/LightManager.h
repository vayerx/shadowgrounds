#ifndef UI_LIGHTMANAGER_H
#define UI_LIGHTMANAGER_H

#include <boost/scoped_ptr.hpp>
#include <string>
#include <datatypedef.h>
#include "../game/unified_handle_type.h"
#include "../editor/UniqueEditorObjectHandle.h"

class IStorm3D;
class IStorm3D_Scene;
class IStorm3D_Terrain;
class IStorm3D_Model;

namespace util
{
	class SelfIlluminationChanger;
	class LightMap;
}

// bad dependency
namespace game
{
	class UnifiedHandleManager;
}

class Terrain;

namespace ui {

struct SpotProperties
{
	enum Type
	{
		Lighting = 0,
		ShadowCaster = 1
	};

	Type type;

	float range;
	float fov;
	float height;
	float angle;
	float strength;
	COL color;

	int group;
	int priority;
	std::string texture;
	std::string coneTexture;
	std::string lightModel;

	bool blink;
	int blinkTime;
	bool rotate;
	float rotateRange;
	int rotateTime;
	bool fade;
	int fadeTime;
	float cone;
	float smoothness;
	bool shadow;

	bool disableObjectRendering;
	float sourceHeight;

	VC2 minPlane;
	VC2 maxPlane;

	enum LightingModelType
	{
		Flat = 0,
		Pointlight = 1,
		Directional = 2,
		NumLightingModelTypes = 3
	};
	LightingModelType lightingModelType;
	bool lightingModelFade;

	SpotProperties();
};

struct Light
{
	VC3 position;
	float range;
	COL color;

	VC2 minPlane;
	VC2 maxPlane;

	signed short lightIndex;

	// Cache data
	VC3 updatePosition;
	float updateRange;
	bool enabled;
	bool dynamic;
	bool attached;

	bool brightnessHack;
	int timeValue;

	Light();
};

struct PointLight
{
	VC3 position;
	COL color;
	float range;

	PointLight()
	:	range(5.f)
	{
	}
};

struct PointLights
{
	VC3 position;
	COL ambient;
	signed short lightIndices[LIGHT_MAX_AMOUNT];
	signed short internalIndices[LIGHT_MAX_AMOUNT];

	PointLights()
	{
		for(int i = 0; i < LIGHT_MAX_AMOUNT; ++i)
		{
			lightIndices[i] = -1;
			internalIndices[i] = -1;
		}
	}
};

class LightManager
{
	struct Data;
	boost::scoped_ptr<Data> data;

public:
	LightManager(IStorm3D &storm, IStorm3D_Scene &scene, IStorm3D_Terrain &terrain, util::LightMap *lightMap, Terrain *uiTerrain);
	~LightManager();

	enum LightType
	{
		Lighting,
		Darkening
	};

	enum AnimationType
	{
		NoAnimation,
		Blinking,
		Fading,
		Rotating
	};

	void addBuildingLight(const VC3 &position, const COL &color, float range);
	void setBuildingLights(IStorm3D_Model &model);
	void clearBuildingLights();

	void addSpot(const VC3 &position, float yAngle, const SpotProperties &properties);
	void clearSpots();
	void enableGroup(int group, bool enable);
	void setLightingLevel(int level);
	void setShadowLevel(int level);

	void setMaxLightAmount(int maxLights);
	int addLight(const Light &light);
	void setLightPosition(int light, const VC3 &position);
	void setLightRadius(int light, float radius);
	void setLightColor(int light, const COL &color);
	void removeLight(int light);
	void clearLights();

	// get unified handle for light with given unique editor object handle
	// or return UNIFIED_HANDLE_NONE if no such light found.
	UnifiedHandle findUnifiedHandleByUniqueEditorObjectHandle(UniqueEditorObjectHandle ueoh) const;

	UnifiedHandle getUnifiedHandle(int lightId) const;
	int getLightId(UnifiedHandle handle) const;
	bool doesLightExist(UnifiedHandle handle) const;
	UnifiedHandle getFirstLight() const;
	UnifiedHandle getNextLight(UnifiedHandle handle) const;

	VC3 getLightPosition(UnifiedHandle unifiedHandle) const;
	void setLightPositionByUnifiedHandle(UnifiedHandle unifiedHandle, const VC3 &position);

	UnifiedHandle findClosestLight(const VC3 &position);
	UnifiedHandle findClosestDetachedLight(const VC3 &position);
	void attachLight(UnifiedHandle light, UnifiedHandle toObject);

	void getLighting(const VC3 &position, PointLights &lights, float radius, bool smoothedTransitions, bool includeFactor = true, IStorm3D_Model *override_model = NULL) const;
	float getFakelightFactor(const VC3 &point) const;
	COL getApproximatedLightingForIndices(const VC3 &position, const PointLights &lights) const;

#if !(defined PROJECT_EDITOR) && !defined(PROJECT_VIEWER) && !defined(PROJECT_PARTICLE_EDITOR)
	void update(const VC3 &player, const VC3 &center, int ms, game::UnifiedHandleManager *uhman);
#else
	void update(const VC3 &player, const VC3 &center, int ms, void *dummy);
#endif

	void setLightingSpotCullRange(float cullRange);
	void setLightingSpotFadeoutRange(float fadeoutRange);
	void setLightColorMultiplier(const COL &color);

	util::SelfIlluminationChanger *getSelfIlluminationChanger();
};

float getRadius(IStorm3D_Model *model);

} // ui

#endif
