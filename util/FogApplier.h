#ifndef INCLUDED_FOG_APPLIER_H
#define INCLUDED_FOG_APPLIER_H

#include <string>
#include <boost/scoped_ptr.hpp>
#include <DatatypeDef.h>

class IStorm3D_Scene;

namespace util {

class FogApplier
{
	struct Data;
	boost::scoped_ptr<Data> data;

public:
	FogApplier();
	~FogApplier();

	struct Fog
	{
		float start;
		float end;
		COL color;
		bool enabled;
		bool cameraCentric;

		Fog()
		:	start(1),
			end(0),
			enabled(false),
			cameraCentric(false)
		{
		}
	};

	void setScene(IStorm3D_Scene &scene);
	void setInterpolate(int type);
	void setFog(const std::string &id, const Fog &fog);
	void setActiveFog(const std::string &id);
	void update(float cameraHeight, float timeDelta);
};

} // util

#endif
