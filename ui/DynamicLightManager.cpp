#include "precompiled.h"

#include "DynamicLightManager.h"
#include "LightManager.h"

#include "../game/unified_handle.h"

namespace ui {

struct LightInstance
{
	VC3 position;
	float radiusFactor;
	float brightness;

	// For simple 1-1 mapping
	int lightIndex;
	bool enabled;

	LightInstance()
	:	radiusFactor(0.f),
		brightness(0.f),
		lightIndex(-1),
		enabled(true)
	{
	}
};

struct LightType
{
	PointLight light;
	std::string name;

	std::vector<LightInstance> instances;
};

struct DynamicLightManager::Data
{
	LightManager *lightManager;
	std::vector<LightType> lights;

	Data(LightManager *lightManager_)
	:	lightManager(lightManager_)
	{
	}
};

DynamicLightManager::DynamicLightManager(LightManager *lightManager)
:	data(new Data(lightManager))
{
	PointLight light;
	light.color = COL(0.7f,0.7f,0.2f);
	light.position = VC3(0,0,0);
	light.range = 5;
	addLightType("dynamic_light_fire_medium", light);
}

DynamicLightManager::~DynamicLightManager()
{
}

int DynamicLightManager::addLightType(const std::string &name, const PointLight &light)
{
	assert(getLightType(name) == -1);

	LightType type;
	type.light = light;
	type.name = name;

	data->lights.push_back(type);
	return data->lights.size() - 1;
}

int DynamicLightManager::getLightType(const std::string &name)
{
	for(unsigned int i = 0; i < data->lights.size(); ++i)
	{
		if(data->lights[i].name == name)
			return i;
	}

	return -1;
}

int DynamicLightManager::addLightInstance(int lightType, const VC3 &position, float radiusFactor, float brightness)
{
	if(lightType < 0 || lightType >= int(data->lights.size()))
	{
		assert(!"Invalid light parameters");
		return - 1;
	}

	// For the time being just create a single light for each instance

	LightType &type = data->lights[lightType];
	LightInstance instance;
	instance.position = position;
	instance.brightness = brightness;
	instance.radiusFactor = radiusFactor;

	Light light;
	light.position = position;
	light.range = type.light.range * instance.radiusFactor;
	light.color = type.light.color * instance.brightness;
	light.minPlane.x = light.position.x - light.range;
	light.minPlane.y = light.position.z - light.range;
	light.maxPlane.x = light.position.x + light.range;
	light.maxPlane.y = light.position.z + light.range;
	light.dynamic = true;

	instance.lightIndex = data->lightManager->addLight(light);

	for(unsigned int i = 0; i < type.instances.size(); ++i)
	{
		if(!type.instances[i].enabled)
		{
			type.instances[i] = instance;
			return i;
		}
	}

	type.instances.push_back(instance);
	return type.instances.size() - 1;
}

void DynamicLightManager::setLightInstancePosition(int typeId, int instanceId, const VC3 &position)
{
	if(typeId < 0 || typeId >= int(data->lights.size()))
	{
		assert(!"Invalid light parameters");
		return;
	}

	LightType &type = data->lights[typeId];
	if(instanceId < 0 || instanceId >= int(type.instances.size()))
	{
		assert(!"Invalid light parameters");
		return;
	}

	LightInstance &instance = type.instances[instanceId];
	assert(instance.enabled);

	instance.position = position;
	data->lightManager->setLightPosition(instance.lightIndex, instance.position);
}

void DynamicLightManager::setLightInstanceRadiusFactor(int typeId, int instanceId, float radiusFactor)
{
	if(typeId < 0 || typeId >= int(data->lights.size()))
	{
		assert(!"Invalid light parameters");
		return;
	}

	LightType &type = data->lights[typeId];
	if(instanceId < 0 || instanceId >= int(type.instances.size()))
	{
		assert(!"Invalid light parameters");
		return;
	}

	LightInstance &instance = type.instances[instanceId];
	assert(instance.enabled);

	assert(radiusFactor >= 0 && radiusFactor <= 1.f);

	instance.radiusFactor = radiusFactor;
	data->lightManager->setLightRadius(instance.lightIndex, type.light.range * instance.radiusFactor);
}

void DynamicLightManager::setLightInstanceBrightness(int typeId, int instanceId, float brightness)
{
	if(typeId < 0 || typeId >= int(data->lights.size()))
	{
		assert(!"Invalid light parameters");
		return;
	}

	LightType &type = data->lights[typeId];
	if(instanceId < 0 || instanceId >= int(type.instances.size()))
	{
		assert(!"Invalid light parameters");
		return;
	}

	LightInstance &instance = type.instances[instanceId];
	assert(instance.enabled);

	assert(brightness >= 0 && brightness <= 1.f);

	instance.brightness = brightness;
	data->lightManager->setLightColor(instance.lightIndex, type.light.color * instance.brightness);
}

void DynamicLightManager::removeLightInstance(int typeId, int instanceId)
{
	if(typeId < 0 || typeId >= int(data->lights.size()))
	{
		assert(!"Invalid light parameters");
		return;
	}

	LightType &type = data->lights[typeId];
	if(instanceId < 0 || instanceId >= int(type.instances.size()))
	{
		assert(!"Invalid light parameters");
		return;
	}

	LightInstance &instance = type.instances[instanceId];
	assert(instance.enabled);

	instance.enabled = false;
	data->lightManager->removeLight(instance.lightIndex);
}

UnifiedHandle DynamicLightManager::getUnifiedHandle(int typeId, int instanceId) const
{
	assert(typeId >= 0 && typeId <= UNIFIED_HANDLE_DYNAMIC_LIGHT_TYPE_ID_LAST_VALUE);
	assert(instanceId >= 0 && instanceId <= UNIFIED_HANDLE_DYNAMIC_LIGHT_INSTANCE_ID_LAST_VALUE);

	// note, this is always true, assuming the masks, etc. are not totally wrong..
	assert(IS_UNIFIED_HANDLE_DYNAMIC_LIGHT((UNIFIED_HANDLE_BIT_DYNAMIC_LIGHT | typeId | (instanceId << UNIFIED_HANDLE_DYNAMIC_LIGHT_INSTANCE_ID_SHIFT))));

	return (UNIFIED_HANDLE_BIT_DYNAMIC_LIGHT | typeId | (instanceId << UNIFIED_HANDLE_DYNAMIC_LIGHT_INSTANCE_ID_SHIFT));
}

void DynamicLightManager::unifiedHandleToLightIds(UnifiedHandle unifiedHandle, int &typeId, int &instanceId) const
{
	assert(IS_UNIFIED_HANDLE_DYNAMIC_LIGHT(unifiedHandle));

	typeId = ((unifiedHandle & UNIFIED_HANDLE_DYNAMIC_LIGHT_TYPE_ID_MASK) >> UNIFIED_HANDLE_DYNAMIC_LIGHT_TYPE_ID_SHIFT);
	instanceId = ((unifiedHandle & UNIFIED_HANDLE_DYNAMIC_LIGHT_INSTANCE_ID_MASK) >> UNIFIED_HANDLE_DYNAMIC_LIGHT_INSTANCE_ID_SHIFT);

	// note, these are always true regardless of parameters, unless the mask,shift,etc. defines are totally wrong)
	assert(typeId >= 0 && typeId <= UNIFIED_HANDLE_DYNAMIC_LIGHT_TYPE_ID_LAST_VALUE);
	assert(instanceId >= 0 && instanceId <= UNIFIED_HANDLE_DYNAMIC_LIGHT_INSTANCE_ID_LAST_VALUE);
}

bool DynamicLightManager::doesLightExist(UnifiedHandle handle) const
{
	int typeId = 0;
	int instanceId = 0;
	unifiedHandleToLightIds(handle, typeId, instanceId);

	if(typeId < 0 || typeId >= int(data->lights.size()))
		return false;

	LightType &type = data->lights[typeId];
	if(instanceId < 0 || instanceId >= int(type.instances.size()))
		return false;

	return type.instances[instanceId].enabled;
}

} // ui
