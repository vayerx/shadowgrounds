
#include "precompiled.h"

#include "FogApplier.h"
#include <IStorm3D_Scene.h>
#include <map>

using namespace std;
using namespace boost;

namespace util {

typedef map<string, FogApplier::Fog> FogMap;
static const float CAMERA_FUDGE = 40.f;

struct FogApplier::Data
{
	FogMap fogMap;
	string active;

	IStorm3D_Scene *scene;

	float currentStart;
	float currentEnd;
	COL currentColor;
	float cameraHeight;

	int interpolate;

	Data()
	:	scene(0),
		currentStart(0),
		currentEnd(0),
		cameraHeight(0),
		interpolate(1)
	{
	}

	void setActive(const string &id)
	{
		const Fog &fog = fogMap[id];
		if(active.empty())
		{
			currentStart = fog.start;
			currentEnd = fog.end;
			if(fog.cameraCentric)
			{
				currentStart += cameraHeight - CAMERA_FUDGE;
				currentEnd += cameraHeight - CAMERA_FUDGE;
			}

			currentColor = fog.color;
			active = id;
		}
		else
		{
			active = id;
		}
	}

	void update(float cameraHeight_, float timeDelta)
	{
		cameraHeight = cameraHeight_;
		if(!scene)
			return;

		if(active.empty())
			scene->SetFogParameters(false, COL(), 0.f, 1.f);
		else
		{
			const Fog &fog = fogMap[active];
			if(!fog.enabled)
				scene->SetFogParameters(false, COL(), 0.f, 1.f);
			else
			{
				if(interpolate)
				{
					float startTarget = fog.start;
					float endTarget = fog.end;
					if(fog.cameraCentric)
					{
						startTarget += cameraHeight - CAMERA_FUDGE;
						endTarget += cameraHeight - CAMERA_FUDGE;
					}

					float startDif = startTarget - currentStart;
					float endDif = endTarget - currentEnd;
					COL colDif = fog.color - currentColor;

					float newStartDif = startDif * 1.f * timeDelta;
					float newEndDif = endDif * 1.f * timeDelta;
					COL newColDif = colDif * 1.f * timeDelta;
					if(fabsf(newStartDif) > fabsf(startDif))
						newStartDif = startDif;
					if(fabsf(newEndDif) > fabsf(endDif))
						newEndDif = endDif;
					if(fabsf(newColDif.r) > fabsf(colDif.r))
						newColDif.r = colDif.r;
					if(fabsf(newColDif.g) > fabsf(colDif.g))
						newColDif.g = colDif.g;
					if(fabsf(newColDif.b) > fabsf(colDif.b))
						newColDif.b = colDif.b;

					currentStart += newStartDif;
					currentEnd += newEndDif;
					currentColor += newColDif;
				}
				else
				{
					currentStart = fog.start;
					currentEnd = fog.end;
					if(fog.cameraCentric)
					{
						currentStart += cameraHeight - CAMERA_FUDGE;
						currentEnd += cameraHeight - CAMERA_FUDGE;
					}

					currentColor = fog.color;
				}

				//scene->SetBackgroundColor(currentColor);
				scene->SetFogParameters(true, currentColor, currentStart, currentEnd);
			}
		}
	}
};

FogApplier::FogApplier()
:	data(new Data())
{
}

FogApplier::~FogApplier()
{
}

void FogApplier::setScene(IStorm3D_Scene &scene)
{
	data->scene = &scene;
}

void FogApplier::setInterpolate(int type)
{
	data->interpolate = type;
}

void FogApplier::setFog(const std::string &id, const Fog &fog)
{
	data->fogMap[id] = fog;
}

void FogApplier::setActiveFog(const std::string &id)
{
	data->setActive(id);
}

void FogApplier::update(float cameraHeight, float timeDelta)
{
	data->update(cameraHeight, timeDelta);
}

} // util
