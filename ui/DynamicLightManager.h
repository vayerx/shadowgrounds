#ifndef INCLUDED_DYNAMIC_LIGHT_MANAGER_H
#define INCLUDED_DYNAMIC_LIGHT_MANAGER_H

#include <boost/scoped_ptr.hpp>
#include <string>
#include "../game/unified_handle_type.h"

namespace ui {

struct PointLight;
class LightManager;

class DynamicLightManager
{
	struct Data;
	boost::scoped_ptr<Data> data;

public:
	explicit DynamicLightManager(LightManager *lightManager);
	~DynamicLightManager();

	int addLightType(const std::string &name, const PointLight &light);
	int getLightType(const std::string &name);

	int addLightInstance(int lightType, const VC3 &position, float radiusFactor, float brightness);
	void setLightInstancePosition(int typeId, int instanceId, const VC3 &position);
	void setLightInstanceRadiusFactor(int typeId, int instanceId, float radiusFactor);
	void setLightInstanceBrightness(int typeId, int instanceId, float brightness);
	void removeLightInstance(int typeId, int instanceId);

	UnifiedHandle getUnifiedHandle(int typeId, int instanceId) const;
	void unifiedHandleToLightIds(UnifiedHandle unifiedHandle, int &typeId, int &instanceId) const;
	bool doesLightExist(UnifiedHandle handle) const;
};

} //ui

#endif
