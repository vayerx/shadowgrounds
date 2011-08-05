#include <boost/lexical_cast.hpp>

#include "precompiled.h"

#ifdef _MSC_VER
#pragma warning(disable:4103)
#pragma warning(disable:4786)
#endif

#include "LightManager.h"
#include <istorm3D_terrain_renderer.h>
#include <istorm3d_spotlight.h>
#include <istorm3d_fakespotlight.h>
#include <IStorm3D_Texture.h>
#include <IStorm3D.h>
#include <IStorm3D_Model.h>
#include <c2_frustum.h>
#include <boost/shared_ptr.hpp>
#include <vector>
#include <map>
#include "../util/SelfIlluminationChanger.h"
#include "../util/LightMap.h"
#include "Terrain.h"
#include "../game/unified_handle.h"
#include "../game/SimpleOptions.h"

#include "../game/options/options_graphics.h"
#include "../game/options/options_debug.h"

#include "../storm/include/c2_oobb.h"
#include "../storm/include/c2_collision.h"


#include <string>

using namespace std;
using namespace boost;

//std::vector<VC2> fakeMin;
//std::vector<VC2> fakeSize;


namespace ui {
	static const int LIGHTING_SPOT_AMOUNT = 1;
	static float LIGHTING_SPOT_CULL_RANGE = 30.f * 30.f;
	static float LIGHTING_SPOT_FADEOUT_RANGE = 20.f; // not squared

	static const int LIGHTING_FAKE_SPOT_MAX_AMOUNT = 4;
	int LIGHTING_FAKE_SPOT_AMOUNT = 4;
	static const float LIGHTING_FAKE_SPOT_CULL_RANGE = 50.f * 50.f;

	static const float FAKELIGHT_FACTOR_FADE_RANGE = 5.f;

#ifdef PROJECT_AOV
	const int STATIC_LIGHT_LIMIT = 5;
#elif PROJECT_SURVIVOR
	const int STATIC_LIGHT_LIMIT = 5;
#else
	const int STATIC_LIGHT_LIMIT = 5;
#endif

	float square2dRange(const VC3 &a, const VC3 &b)
	{
		float dx = a.x - b.x;
		float dz = a.z - b.z;

		return dx*dx + dz*dz;
	}

	float square2dRange(const VC2 &point, const VC2 &planeMin, const VC2 &planeMax)
	{
		VC2 planeCenter = planeMin;
		planeCenter += planeMax;
		planeCenter *= .5f;

		return point.GetRangeTo(planeCenter);
	}
/*
	float square2dRange(const VC2 &point, const VC2 &planeMin, const VC2 &planeMax)
	{
		VC2 planeCenter = planeMin;
		planeCenter += planeMax;
		planeCenter *= .5f;
		VC2 delta = point;
		delta -= planeCenter;

		float e0 = planeMax.y - planeMin.y;
		float e1 = planeMax.x - planeMin.x;
		float s0 = delta.y; // dot((0,1), delta)
		float s1 = delta.x; // dot((1,0), delta)
		float result = 0.f;

		float s0pe0 = s0 + e0;
		if(s0pe0 < 0)
			result += s0pe0 * s0pe0;
		else
		{
			float s0me0 = s0 - e0;
			if(s0me0 > 0)
				result += s0me0 * s0me0;
		}

		float s1pe1 = s1 + e1;
		if(s1pe1 < 0)
			result += s1pe1 * s1pe1;
		else
		{
			float s1me1 = s1 - e1;
			if(s1me1 > 0)
				result += s1me1 * s1me1;
		}

		if(point.x > planeMin.x)
		if(point.x < planeMax.x)
		if(point.y > planeMin.y)
		if(point.y < planeMax.y)
		{
			assert(result < 0.0001f);
		}

		return result;
	}
*/

#if 0
	float getFadeFactor(const VC2 &point, const VC2 &planeMin, const VC2 &planeMax, float radius)
	{
		VC2 planeCenter = planeMin;
		planeCenter += planeMax;
		planeCenter *= .5f;

		float result = 1.f;

		/*
		// This really wont work - way too many lights can return 0 for bigger objects
		//  -- just forget [0,1] (?)

		{
			float d = fabsf(point.x - planeMin.x);
			float range = fabsf(planeMin.x - planeCenter.x);
			range += radius;

			float factor = 0;
			if(range > 0.0001f)
				factor = d / range;

			if(factor < result)
				result = factor;
		}
		{
			float d = fabsf(point.y - planeMin.y);
			float range = fabsf(planeMin.y - planeCenter.y);
			range += radius;

			float factor = 0;
			if(range > 0.0001f)
				factor = d / range;

			if(factor < result)
				result = factor;
		}
		{
			float d = fabsf(point.x - planeMax.x);
			float range = fabsf(planeMax.x - planeCenter.x);
			range += radius;

			float factor = 0;
			if(range > 0.0001f)
				factor = d / range;

			if(factor < result)
				result = factor;
		}
		{
			float d = fabsf(point.y - planeMax.y);
			float range = fabsf(planeMax.y - planeCenter.y);
			range += radius;

			float factor = 0;
			if(range > 0.0001f)
				factor = d / range;

			if(factor < result)
				result = factor;
		}
		*/


		{
			float d = planeCenter.x - point.x;
			float range = planeCenter.x - planeMin.x;
			if(d < 0)
				d = range - d;
			
			range += radius;
			float factor = 0;
			if(range > 0.0001f)
				factor = d / range;

			if(factor < result)
				result = factor;
		}
		{
			float d = planeCenter.y - point.y;
			float range = planeCenter.y - planeMin.y;
			if(d < 0)
				d = range - d;
			
			range += radius;
			float factor = 0;
			if(range > 0.0001f)
				factor = d / range;

			if(factor < result)
				result = factor;
		}
		{
			float d = point.x - planeCenter.x;
			float range = planeMax.x - planeCenter.x;
			if(d < 0)
				d = range - d;
			
			range += radius;
			float factor = 0;
			if(range > 0.0001f)
				factor = d / range;

			if(factor < result)
				result = factor;
		}
		{
			float d = point.y - planeCenter.y;
			float range = planeMax.y - planeCenter.y;
			if(d < 0)
				d = range - d;
			
			range += radius;
			float factor = 0;
			if(range > 0.0001f)
				factor = d / range;

			if(factor < result)
				result = factor;
		}

		assert(result >= 0 && result <= 1.f);
		return result;
	}
#endif

	bool intersectX(const VC2 &center, const VC2 &dir, float distance, float height, float &result)
	{
		if(dir.x * distance <= 0)
			return false;
		//if(dir.x <= 0)
		//	return false;

		float factor = distance / dir.x;
		if(fabsf(factor * dir.y) > height)
			return false;

		//result = factor;
		result = fabsf(dir.x / distance);
		return true;
	}

	bool intersectY(const VC2 &center, const VC2 &dir, float distance, float width, float &result)
	{
		if(dir.y * distance <= 0)
			return false;
		//if(dir.y <= 0)
		//	return false;

		float factor = distance / dir.y;
		if(fabsf(factor * dir.x) > width)
			return false;

		//result = factor;
		result = fabsf(dir.y / distance);
		return true;
	}

	float getFadeFactor(const VC2 &point, const VC2 &planeMin, const VC2 &planeMax, float radius)
	{
#if defined(PROJECT_SHADOWGROUNDS) || defined(PROJECT_SURVIVOR) || !defined(PROJECT_CLAW_PROTO)
		VC2 planeCenter = planeMin;
		planeCenter += planeMax;
		planeCenter *= .5f;

		float result = 1.f;
		VC2 dir = point - planeCenter;
		float pointDistance = dir.GetLength();
		dir /= pointDistance;

		float limitDistance = 1.f;
		if(
			intersectX(planeCenter, dir, planeMin.y - planeCenter.y - radius, planeMax.x - planeCenter.x + radius, limitDistance)
				|
			intersectX(planeCenter, dir, planeMax.y - planeCenter.y + radius, planeMax.x - planeCenter.x + radius, limitDistance)
				||
			intersectY(planeCenter, dir, planeMin.x - planeCenter.x - radius, planeMax.y - planeCenter.y + radius, limitDistance)
				||
			intersectY(planeCenter, dir, planeMax.x - planeCenter.x + radius, planeMax.y - planeCenter.y + radius, limitDistance)
		  )
		{
			//return pointDistance / limitDistance;
			result = limitDistance;
		}

		return result;

#else

		VC2 planeCenter = planeMin;
		planeCenter += planeMax;
		planeCenter *= .5f;

		VC2 planeHalfRadius = planeMax - planeCenter;
		planeHalfRadius.x += radius;
		planeHalfRadius.y += radius;

		float factor = 1.f;
		if(point.x > planeCenter.x)
		{
			factor = (point.x - planeCenter.x) / planeHalfRadius.x;
		}
		else
		{
			factor = (planeCenter.x - point.x) / planeHalfRadius.x;
		}

		if(point.y > planeCenter.y)
		{
			float newFactor = (point.y - planeCenter.y) / planeHalfRadius.y;
			if(newFactor < factor)
				factor = newFactor;
		}
		else
		{
			float newFactor = (planeCenter.y - point.y) / planeHalfRadius.y;
			if(newFactor < factor)
				factor = newFactor;
		}

		return factor;
#endif
	}


	template<typename T>
	struct ReleaseDeleter
	{
		void operator () (T *ptr)
		{
			if(ptr)
				ptr->Release();
		}
	};

	struct SpotImp
	{
		VC3 position;
		float yAngle;

		SpotProperties properties;
		int blinkTime;
		int rotateTime;
		int fadeTime;

		float fadeFactor;
		float currentColorMul;
		float colorMul;

		shared_ptr<IStorm3D_Texture> texture;
		shared_ptr<IStorm3D_Texture> coneTexture;
		shared_ptr<IStorm3D_Model> lightModel;

		SpotImp()
		:	yAngle(0),
			blinkTime(0),
			rotateTime(0),
			fadeTime(0),
			fadeFactor(1.f),
			currentColorMul(1.f),
			colorMul(0)
		{
		}

		void animate(int ms)
		{
			blinkTime += ms;
			rotateTime += ms;
			fadeTime += ms;

			if(blinkTime > properties.blinkTime)
				blinkTime -= properties.blinkTime;
			if(rotateTime > properties.rotateTime)
				rotateTime -= properties.rotateTime;
			if(fadeTime > properties.fadeTime)
				fadeTime -= properties.fadeTime;
		}

		VC3 getDirection() const
		{
			QUAT q;
			q.MakeFromAngles(properties.angle, getRotation(), 0);
			MAT tm;
			tm.CreateRotationMatrix(q);
			VC3 direction(0, 0, 1.f);
			tm.RotateVector(direction);

			return direction;
		}

		float getBlinkFactor() const
		{
			if(!properties.blink || !properties.blinkTime)
				return 1.f;

			if(blinkTime < properties.blinkTime)
			{
				if(blinkTime < 1 * properties.blinkTime / 8)
				{
					return 0.0f;
				}
				else if(blinkTime > 7 * properties.blinkTime / 8)
				{
					return 0.0f;
				}
				else 
				{
					if(blinkTime >= 5 * properties.blinkTime / 8 && blinkTime < 6 * properties.blinkTime / 8)
					{
						return 0.0f;
					} 
					else 
					{
						return 1.0f;
					}
				}
			} 

			return 1.0f;
		}

		float getFadeFactor() const
		{
			if(!properties.fade || !properties.fadeTime)
				return 1.f;

			int halfTime = properties.fadeTime / 2;
			float result = 1.f;

			if(fadeTime < halfTime)
				result = 1.f - (float(fadeTime) / halfTime);
			else
				result = float(fadeTime - halfTime) / halfTime;

			result *= 3.14f;
			result = (cosf(result) + 1.f) * .5f;

			result = .25f + 3.f*result/4.f;
			return result;
		}

		float getRotation() const
		{
			if(!properties.rotate || !properties.rotateTime)
				return yAngle;

			float factor = float(rotateTime) / float(properties.rotateTime);
			if(properties.rotateRange > 0.01f)
			{
				if(factor < 0.5f)
					factor = -1.f + (factor * 4.f);
				else
					factor = 1.f - ((factor - 0.5f) * 4.f);

				factor = (factor + 1.f) * .5f;
				factor = cosf(factor * 3.14f);

				factor *= properties.rotateRange * .5f;
			}

			float result = yAngle + (factor * 2 * PI);
			if(result > 2 * PI)
				result -= 2 * PI;
			if(result < 0)
				result += 2 * PI;

			return result;
		}

		float getColorFactor() const
		{
			return getFadeFactor() * getBlinkFactor();
		}
	};


	float fakeRange(const VC3 &point, IStorm3D_Camera *camera, const SpotImp &imp, IStorm3D_Scene *scene = NULL, float colorOverride = -1.0f)
	{

		if( !camera )
			return 1.0f;

		VC2 planeMin = imp.properties.minPlane;
		VC2 planeMax = imp.properties.maxPlane;
		planeMin *= -imp.properties.range;
		planeMax *=  imp.properties.range;
		float planeY = imp.position.y - imp.properties.height;

		planeMin.x += imp.position.x;
		planeMin.y += imp.position.z;
		planeMax.x += imp.position.x;
		planeMax.y += imp.position.z;

		VC3 shadowPlane1[8] =
		{
			VC3( planeMin.x, planeY, planeMin.y),
			VC3( planeMax.x, planeY, planeMin.y),
			VC3( planeMax.x, planeY, planeMax.y),
			VC3( planeMin.x, planeY, planeMax.y)
		};
		VC3 shadowPlane2[8];
		int shadowPlaneVertexAmount = 4;

		VC3 *currentInput = shadowPlane1;
		VC3 *currentOutput = shadowPlane2;

		// Set the camera target a bit more towards the player, and get the frustum.
		PLANE shadowGround;
		shadowGround.MakeFromNormalAndPosition(
			VC3(0, 1, 0),
			VC3( (planeMin.x + planeMax.x)/2.0f, planeY, (planeMin.y + planeMax.y)/2.0f ) );
		VC3 oldTarget = camera->GetTarget();
		VC3 pos = camera->GetPosition();
		VC3 tar = oldTarget;
		pos = shadowGround.GetProjectedVector ( pos ) ;
		tar = shadowGround.GetProjectedVector ( tar ) ;
		VC3 tiltVec = pos - tar;
		tiltVec.Normalize();
		camera->SetTarget ( oldTarget + tiltVec * 2.5f );
		Frustum frustum = camera->getFrustum();
		camera->SetTarget ( oldTarget );

		// Start clipping the plane to the frustum.
		// For each frustum plane:
		for(int j = 1; j < 5; j++)
		{
			PLANE frustumPlane;
			frustumPlane.MakeFromNormalAndPosition( frustum.planeNormals[j], frustum.position);

			int n = 0;

			// Sutherland-Hodgman -algorithm implementation.
			VC3 vS = currentInput[shadowPlaneVertexAmount - 1];
			for(int i = 0; i < shadowPlaneVertexAmount; i++)
			{
				VC3 vP = currentInput[i];
				
				bool insideS = (frustumPlane.GetPointRange ( vS ) > 0);
				bool insideP = (frustumPlane.GetPointRange ( vP ) > 0);

				if(insideP)
				{
					if(!insideS)
					{
						VC3 clipPoint;
#ifndef NDEBUG
						bool clip = 
#endif
							frustumPlane.GetClip( vS, vP, &clipPoint );
						//frustumPlane.clipLine( vS, vP, clipPoint );
//#pragma message("*** this assert seems to be false 'somewhat randomly' while loading mission - uninitialized data or scene changing randomly while loading based on input?. --jpk")
						assert ( clip );
						currentOutput[n++] = clipPoint;
					}
					currentOutput[n++] = vP;					
				}
				else if(insideS)
				{
					VC3 clipPoint;
#ifndef NDEBUG
					bool clip =
#endif
						frustumPlane.GetClip( vP, vS, &clipPoint );
					//frustumPlane.clipLine( vS, vP, clipPoint );
					assert ( clip );
					currentOutput[n++] = clipPoint;				
				}
				vS = vP;
			}

			std::swap( currentInput, currentOutput);
			shadowPlaneVertexAmount = n;
		}
		// currentInput is now the result polygon.

		assert ( shadowPlaneVertexAmount <= 8 ) ; // If this fails, increase maximum vertex amount. Afaik 8 should be sufficient though.

		if(shadowPlaneVertexAmount == 0)
			return 1.0f;

		VC3 * const clippedPolygon = currentInput;
		VC3 projectedPolygon[8];

		// Project polygon points to screen.
		for(int i = 0; i < shadowPlaneVertexAmount; i++)
		{
			float c1 = 0, c2 = 0;
			camera->GetTransformedToScreen(clippedPolygon[i], projectedPolygon[i], c1, c2);
		}



		// Calculate projected triangle areas, sum them together.
		// Since the polygon should be (at least almost always) convex, assume the polygon as a triangle strip.
		float totalArea = 0;
		for(int i = 1; i < shadowPlaneVertexAmount - 1; i++)
		{
			VC3 edge1 = projectedPolygon[i] - projectedPolygon[0];
			VC3 edge2 = projectedPolygon[i + 1] - projectedPolygon[0];
			VC3 crossVec = edge1.GetCrossWith(edge2);
			float triangleArea = crossVec.GetLength() / 2.0f;
			totalArea += triangleArea;
		}

		// Debug rendering, draws clipped shadow planes to screen.
#if !(defined PROJECT_EDITOR) && !defined(PROJECT_VIEWER) && !defined(PROJECT_PARTICLE_EDITOR)
		if(scene && game::SimpleOptions::getBool ( DH_OPT_B_DEBUG_VISUALIZE_POINTLIGHT_SHADOW_AREAS ) )
		{
			float color = totalArea;
			if(color > 1.0f)
				color = 1.0f;
			if(colorOverride != -1.0f)
				color = colorOverride;
			for(int i = 0; i < shadowPlaneVertexAmount; i++)
			{
				VC3 offset(0, 0.1f, 0);
				scene->AddLine( clippedPolygon[i] + offset, clippedPolygon[ (i + 1) % shadowPlaneVertexAmount ] + offset, COL(color, color, color) );
			}
		}
#endif

		float factor = 1.f - totalArea;
		return factor;

	}
/*
	float fakeRange(const VC3 &point, IStorm3D_Camera *camera, const SpotImp &imp)
	{
		VC2 point2(point.x, point.z);
		VC2 planeMin = imp.properties.minPlane;
		VC2 planeMax = imp.properties.maxPlane;
		planeMin *= -imp.properties.range;
		planeMax *=  imp.properties.range;

		planeMin.x += imp.position.x;
		planeMin.y += imp.position.z;
		planeMax.x += imp.position.x;
		planeMax.y += imp.position.z;

		//float range = square2dRange(point2, planeMin, planeMax);
		//range = sqrtf(range);

		if(camera)
		{
			bool visible = false;
			VC2 minValue(1.f, 1.f);
			VC2 maxValue(0.f, 0.f);

			VC3 cameraPos = camera->GetPosition();
			VC3 cameraPosResult;
			float cameraRhw = 0, cameraRealZ = 0;
			camera->GetTransformedToScreen(cameraPos, cameraPosResult, cameraRhw, cameraRealZ);
			bool skipLight = false;

			float minZ =  10000.f;
			for(int i = 0; i < 4; ++i)
			{
				VC3 result;
				float realZ = 0;

				result.x = max(0.f, result.x);
				result.y = max(0.f, result.y);
				result.x = min(1.f, result.x);
				result.y = min(1.f, result.y);

				if(realZ < cameraRealZ)
				{
					result.x = 1.f - result.x;
					result.y = 1.f - result.y;
				}

				minZ = min(minZ, realZ);
				if(realZ < max(cameraRealZ, 1.f))
					skipLight = true;

				if(!visible)
				{
					minValue.x = result.x;
					minValue.y = result.y;
					maxValue.x = result.x;
					maxValue.y = result.y;

					visible = true;
				}
				else
				{
					maxValue.x = max(result.x, maxValue.x);
					maxValue.y = max(result.y, maxValue.y);
					minValue.x = min(result.x, minValue.x);
					minValue.y = min(result.y, minValue.y);
				}
			}

			VC2 delta = maxValue - minValue;

			float crRange = 1.f;
			float cr = max(cameraRealZ, 1.f);
			if(minZ < cr + crRange)
			{
				float factor = (cr - minZ) / crRange;
				delta *= factor;
			}

			float area = delta.x * delta.y;
			float factor = ((1.f - area));

			// Use just planes screen size
			return factor;
		}

		return 1.f;
	}
*/
	typedef vector<SpotImp> SpotList;
	typedef map<int, SpotList> SpotGroups;
	typedef vector<int> IntList;

	typedef vector<Light> LightList;

SpotProperties::SpotProperties()
:	type(Lighting),
	range(0),
	fov(0),
	height(0),
	angle(0),
	strength(.2f),
	color(1.f, 1.f, 1.f),

	group(0),
	priority(1),

	blink(false),
	blinkTime(0),
	rotate(false),
	rotateRange(0),
	rotateTime(0),
	fade(false),
	fadeTime(0),
	cone(0),
	smoothness(5.f),
	shadow(true),

	disableObjectRendering(false),

	sourceHeight(.2f),
	minPlane(0,0),
	maxPlane(1.f, 1.f),

	lightingModelType(Pointlight),
	lightingModelFade(true)
{
}

Light::Light()
:	range(0),
	lightIndex(-1),
	updateRange(0),
	enabled(true),
	dynamic(false)
{
}


	struct LightPoint
	{
		VC3 position;
		COL color;
		float range;

		signed short lightIndex;

		LightPoint()
		:	range(1.f),
			lightIndex(-1)
		{
		}
	};

	typedef vector<LightPoint> PointList;


struct LightManager::Data
{
	IStorm3D &storm; 
	IStorm3D_Scene &scene; 
	IStorm3D_Terrain &terrain;

	SpotList spots;
	SpotGroups spotGroups;
	SpotList fakeSpots;
	SpotGroups fakeSpotGroups;
	IntList activeGroups;

	PointList buildingLights;

	shared_ptr<IStorm3D_Spotlight> lightingSpot[LIGHTING_SPOT_AMOUNT];
	shared_ptr<IStorm3D_FakeSpotlight> fakeSpot[LIGHTING_FAKE_SPOT_MAX_AMOUNT];

	int activeFakes[LIGHTING_FAKE_SPOT_MAX_AMOUNT];
	int shadowLevel;
	int lightLevel;

	LightList lights;
	COL lightColor;

	util::SelfIlluminationChanger selfIlluminationChanger;
	IStorm3D_Camera *camera;
	util::LightMap *lightMap;
	Terrain *uiTerrain;

	int maxLightAmount;

	Data(IStorm3D &storm_, IStorm3D_Scene &scene_, IStorm3D_Terrain &terrain_, util::LightMap *lightMap_, Terrain *uiTerrain_)
	:	storm(storm_),
		scene(scene_),
		terrain(terrain_),
		shadowLevel(100),
		lightLevel(100),
		lightColor(1.f, 1.f, 1.f),
		selfIlluminationChanger(),
		camera(scene.GetCamera()),
		lightMap(lightMap_),
		uiTerrain(uiTerrain_),
		maxLightAmount(LIGHT_MAX_AMOUNT)
	{
		activeGroups.push_back(0);

		for(int i = 0; i < LIGHTING_SPOT_AMOUNT; ++i)
		{
			lightingSpot[i] = terrain.getRenderer().createSpot();
			IStorm3D_Spotlight *spot = lightingSpot[i].get();

			spot->setType(IStorm3D_Spotlight::Point);
			spot->enableFeature(IStorm3D_Spotlight::Fade, true);
			spot->enableFeature(IStorm3D_Spotlight::ScissorRect, false);
			spot->enable(false);
			spot->setColorMultiplier(COL(1.f, 1.f, 1.f));
		}

		for(int i = 0; i < LIGHTING_FAKE_SPOT_MAX_AMOUNT; ++i)
		{
			fakeSpot[i] = terrain.getRenderer().createFakeSpot();
			IStorm3D_FakeSpotlight *spot = fakeSpot[i].get();

			spot->enable(false);
			activeFakes[i] = -1;
		}

#if !(defined PROJECT_EDITOR) && !defined(PROJECT_VIEWER) && !defined(PROJECT_PARTICLE_EDITOR)
		maxLightAmount = game::SimpleOptions::getInt(DH_OPT_I_RENDER_MAX_POINTLIGHTS);

		// make sure we don't crash on incorrect config
		if (maxLightAmount > LIGHT_MAX_AMOUNT)
		{
			maxLightAmount = LIGHT_MAX_AMOUNT;
		};

#endif

	}

	// add or remove a spot from the alien scare lightmap
	void semiDynamicLightModify(const SpotImp &spot, bool add)
	{
		if (this->lightMap == NULL)
		{
			return;
		}

		if (spot.properties.type == SpotProperties::Lighting
			&& spot.properties.angle >= 45.0f * 3.1415926f / 180.0f)
		{
			if (!spot.properties.rotate
				&& !spot.properties.fade)
			{
				VC3 pos = VC3(spot.position.x, 0, spot.position.z);

				// move on ground (direction/ground intersection position)
				VC3 dir = spot.getDirection();
				assert(fabs(dir.y) > 0.01f);
				dir *= (1.0f / -dir.y) * spot.properties.height;
				pos += dir;

				COL col = spot.properties.color;
				float avgc = (col.r + col.g + col.b) / 3.0f;

				int lmapx = this->lightMap->scaledToLightMapX(pos.x);
				int lmapy = this->lightMap->scaledToLightMapY(pos.z);
				float radius = (float)fabs((float)tanf(spot.properties.fov / 2.0f)) * spot.properties.height;
				int lmapradius = this->lightMap->scaledToLightMapRadius(radius);
				if (add)
				{
					this->lightMap->addSemiDynamicLight(lmapx, lmapy, lmapradius, avgc);
				} else {
					this->lightMap->removeSemiDynamicLight(lmapx, lmapy, lmapradius, avgc);
				}
			}
		}
	}		

	void updateLightingSpots(const VC3 &player, int ms)
	{
		// Sorted list for closest lights
		int closest[LIGHTING_SPOT_AMOUNT] = { -1 };
		for(int i = 0; i < LIGHTING_SPOT_AMOUNT; ++i)
			closest[i] = -1;

		for(unsigned int i = 0; i < spots.size(); ++i)
		{
			spots[i].animate(ms);

			float range1 = square2dRange(spots[i].position, player);
			if(range1 > LIGHTING_SPOT_CULL_RANGE )
				continue;

			//if(lightLevel < 75)
			//	continue;
			//if(lightLevel < 100 && spots[i].properties.priority >= 2)
			//	continue;
			if(lightLevel <= 25 && spots[i].properties.priority >= 2)
				continue;

			for(int j = 0; j < LIGHTING_SPOT_AMOUNT; ++j)
			{
				// If no light
				int index = closest[j];
				if(index == -1)
				{
					closest[j] = i;
					break;
				}

				// Closer?
				float range2 = square2dRange(spots[index].position, player);
				if(range1 < range2)
				{
					// Push current lights back
					for(int k = LIGHTING_SPOT_AMOUNT - 1; k >= j; --k)
						closest[k] = closest[k - 1];

					closest[j] = i;
					break;
				}
			}
		}

		for(int i = 0; i < LIGHTING_SPOT_AMOUNT; ++i)
		{
			IStorm3D_Spotlight *spot = lightingSpot[i].get();
			if(!spot)
				continue;

			int index = closest[i];
			if(index == -1)
			{
				spot->enable(false);
				continue;
			}
			else
				spot->enable(true);

			const SpotImp &spotImp = spots[index];
			const SpotProperties &properties = spotImp.properties;
	
			spot->setPosition(spotImp.position);
			spot->setDirection(spotImp.getDirection());
			spot->setFov(properties.fov / PI * 180.f);	
			spot->setRange(properties.range);

			if(spotImp.texture)
				spot->setProjectionTexture(spotImp.texture);
			spot->setConeTexture(spotImp.coneTexture);

			float colorFactor = spotImp.getColorFactor();
			COL color = properties.color;
			color.r *= colorFactor;
			color.g *= colorFactor;
			color.b *= colorFactor;

			if(properties.cone > 0.001f)
			{
				spot->enableFeature(IStorm3D_Spotlight::ConeVisualization, true);
				spot->setConeMultiplier(properties.cone);
			}
			else
				spot->enableFeature(IStorm3D_Spotlight::ConeVisualization, false);

			spot->setSmoothness(properties.smoothness);

			// TEMP HACK!!!
			{
				// TODO: a more efficient implementation please
				//float range = sqrtf(spotImp.position.GetSquareRangeTo(player));
				const VC3 &a = spotImp.position;
				const VC3 &b = player;
				float range = sqrtf(square2dRange(a, b));
				range -= (LIGHTING_SPOT_FADEOUT_RANGE * 0.5f);
				if (range < 0.0f) range = 0.0f;
				range *= 2.0f;
				if (range > LIGHTING_SPOT_FADEOUT_RANGE) range = LIGHTING_SPOT_FADEOUT_RANGE;
				float distanceFactor = (1.0f - (range / LIGHTING_SPOT_FADEOUT_RANGE));
				color *= distanceFactor;
			}

			/*
			if(color.r < 0.01f)
			if(color.g < 0.01f)
			if(color.b < 0.01f)
				spot->enable(false);
			*/

			spot->setColorMultiplier(color);

			//if(shadowLevel > 0 && properties.shadow)
			if(properties.shadow)
				spot->enableFeature(IStorm3D_Spotlight::Shadows, true);
			else
				spot->enableFeature(IStorm3D_Spotlight::Shadows, false);

			if(spotImp.lightModel)
			{
				VC3 pos = spotImp.position;
				QUAT rot = spot->getOrientation();

				spotImp.lightModel->SetPosition(pos);
				spotImp.lightModel->SetRotation(rot);
			}

			spot->setNoShadowModel(spotImp.lightModel.get());

			spot->setType((IStorm3D_Spotlight::Type)properties.lightingModelType);
			spot->enableFeature(IStorm3D_Spotlight::Fade, properties.lightingModelFade);
		}
	}

	void updateFakeSpots(const VC3 &player, int ms)
	{
		for(int i = 0; i < LIGHTING_FAKE_SPOT_MAX_AMOUNT; ++i)
			fakeSpot[i]->enable(false);

		if(LIGHTING_FAKE_SPOT_AMOUNT == 0)
			return;

		// Sorted list for closest lights
		int closestFakeLights[LIGHTING_FAKE_SPOT_MAX_AMOUNT + 1] = { -1 };
		for(int i = 0; i < LIGHTING_FAKE_SPOT_AMOUNT + 1; ++i)
			closestFakeLights[i] = -1;

		// and now a list for their ranges as well to prevent totally excessive calculation. --jpk
		float closestFakeRanges[LIGHTING_FAKE_SPOT_MAX_AMOUNT + 1] = { 0.0f };

		// Find lights
		{
			for(unsigned int i = 0; i < fakeSpots.size(); ++i)
			{
				const SpotImp &imp = fakeSpots[i];
				const SpotProperties &properties = imp.properties;

				if(shadowLevel <= 25)
					continue;
				if(shadowLevel <= 50 && properties.priority >= 2)
					continue;

				float range1 = fakeRange(player, camera, imp, NULL);
				//if(range1 > LIGHTING_FAKE_SPOT_CULL_RANGE)
				//	continue;

				for(int j = 0; j < LIGHTING_FAKE_SPOT_AMOUNT + 1; ++j)
				{
					int index = closestFakeLights[j];
					if(index == -1)
					{
						closestFakeLights[j] = i;
						closestFakeRanges[j] = range1;
						break;
					}

					// Closer?

					// oh no, what is this... making LIGHTING_FAKE_SPOT_AMOUNT+1 calls instead of 1 for each light
					// not efficient at all. optimized to be a bit more sensible... --jpk
					//float range2 = fakeRange(player, camera, fakeSpots[index]);
					float range2 = closestFakeRanges[j];

					if(range1 < range2)
					{
						// Push current lights back
						for(int k = LIGHTING_FAKE_SPOT_AMOUNT; k >= j; --k)
						{
							closestFakeLights[k] = closestFakeLights[k - 1];
							closestFakeRanges[k] = closestFakeRanges[k - 1];
						}

						closestFakeLights[j] = i;
						closestFakeRanges[j] = range1;
						break;
					}
				}

				// Debug rendering.
#if !(defined PROJECT_EDITOR) && !defined(PROJECT_VIEWER) && !defined(PROJECT_PARTICLE_EDITOR)
				if(game::SimpleOptions::getBool ( DH_OPT_B_DEBUG_VISUALIZE_POINTLIGHT_SHADOW_AREAS ) )
				{
					for(int l = 0 ;l < LIGHTING_FAKE_SPOT_AMOUNT + 1; l++)
					{
						if (closestFakeLights[l] >= 0 && closestFakeLights[l] < int(fakeSpots.size()))
							fakeRange( VC3(), camera, fakeSpots[ closestFakeLights[l] ], &scene, 1.0f - (float)l/LIGHTING_FAKE_SPOT_AMOUNT );
					}
				}
#endif

			}
		}

		// Fade lights in or out
		for(int i = 0; i < LIGHTING_FAKE_SPOT_AMOUNT; ++i)
		{
			int current = closestFakeLights[i];
			if(current == -1)
				continue;

			//activeFakes[i] = current;

			SpotImp &imp = fakeSpots[current];
			if(i < LIGHTING_FAKE_SPOT_AMOUNT - 1)
				imp.currentColorMul = imp.colorMul;
			else
			{
				float currentRange = fakeRange(player, camera, imp);
				int a = closestFakeLights[i - 1];
				int b = closestFakeLights[i + 1];
				float rangeA = 0.0f;
				float rangeB = 1.0f;

				if(a >= 0)
				{
					SpotImp &spot = fakeSpots[a];
					rangeA = fakeRange(player, camera, spot);
				}

				if(b >= 0)
				{
					SpotImp &spot = fakeSpots[b];
					rangeB = fakeRange(player, camera, spot);
				}


				// Shouldn't do this linearly. Fade only certain amount of meters or so

				//float factor = (currentRange - rangeA) / (rangeB - rangeA);
				if(rangeB != rangeA)
					imp.fadeFactor = float(currentRange - rangeA) / (rangeB - rangeA);
				else
					imp.fadeFactor = rangeA;

				if(rangeB < 0.00001f)
					imp.fadeFactor = 0.f;

				//imp.fadeFactor = currentRange / rangeB;
				float factor = (imp.fadeFactor * (1.f - imp.colorMul)) + imp.colorMul;
				//float factor = imp.fadeFactor * imp.colorMul;
				//float factor = 1.0f - imp.fadeFactor;

				assert(factor >= imp.colorMul && factor <= 1.f);
				imp.currentColorMul = factor;
			}
		}

		// Apply
		for(int i = 0; i < LIGHTING_FAKE_SPOT_AMOUNT; ++i)
		{
			IStorm3D_FakeSpotlight *spot = fakeSpot[i].get();
			if(!spot)
				continue;

			int index = closestFakeLights[i];
			activeFakes[i] = index;

			if(index == -1)
			{
				spot->enable(false);
				continue;
			}
			else
				spot->enable(true);

			const SpotImp &imp = fakeSpots[index];
			const SpotProperties &properties = imp.properties;

			spot->setPosition(imp.position);
			spot->setDirection(imp.getDirection());
			spot->setFov(properties.fov / PI * 180.f);	
			spot->setRange(properties.range);

			COL color(imp.currentColorMul, imp.currentColorMul, imp.currentColorMul);
			spot->setColor(color, 1.f - imp.colorMul);

			VC2 corner(properties.range, properties.range);
			VC2 minPlane = corner * properties.minPlane;
			VC2 maxPlane = corner * properties.maxPlane;
			spot->setPlane(properties.height, -minPlane, maxPlane);

			spot->renderObjectShadows(!properties.disableObjectRendering);
		}
/*
fakeMin.clear();
fakeSize.clear();
for(i = 0; i < LIGHTING_FAKE_SPOT_AMOUNT; ++i)
{
	int index = activeFakes[i];
	if(index >= 0)
		fakeRange(player, camera, fakeSpots[index]);
}
*/
	}

	bool isActive(int index) const
	{
		IntList::const_iterator it = activeGroups.begin();
		for(; it != activeGroups.end(); ++it)
		{
			if(*it == index)
				return true;
		}

		return false;
	}

	void collectLights()
	{
		spots.clear();
		fakeSpots.clear();

		IntList::iterator it = activeGroups.begin();
		for(; it != activeGroups.end(); ++it)
		{
			SpotList &spotList = spotGroups[*it];
			for(unsigned int i = 0; i < spotList.size(); ++i)
				spots.push_back(spotList[i]);

			SpotList &fakeList = fakeSpotGroups[*it];
			for(unsigned int i = 0; i < fakeList.size(); ++i)
				fakeSpots.push_back(fakeList[i]);
		}
	}

	float getLightIntensity(const Light &l, const VC3 &position, float radius) const
	{
		VC3 pos = position;
		//pos.y = l.position.y;
		if(l.position.y > position.y)
		{
			pos.y = position.y - (radius * 0.5f);
			if(pos.y < l.position.y)
				pos.y = l.position.y;
		}
		else
		{
			pos.y = position.y + (radius * 0.5f);
			if(pos.y > l.position.y)
				pos.y = l.position.y;
		}

		float distance = l.position.GetRangeTo(pos);
		if(distance > l.range)
			return 0.f;

		float lightFactor = 1.f - (distance / l.range);
		COL newColor = l.color * lightFactor;

		float maxColor = newColor.r;
		if(newColor.g > maxColor)
			maxColor = newColor.g;
		if(newColor.b > maxColor)
			maxColor = newColor.b;

		//float intensity = (newColor.r + newColor.g + newColor.b) * 0.33f;
		// Average between max color component and b&w intensity
		//return (intensity + maxColor) * .5f;
	
		return maxColor;
	}

	float getLightFadeFactor(const Light &l, const VC3 &position, float radius) const
	{
		return 1.f - getLightIntensity(l, position, radius);
	}


#define GETLIGHTING_POINT_TEST_OUTSIDE_AREA(a)\
		(	p ## a ## .x < l.minPlane.x	\
		|| p ## a ## .x > l.maxPlane.x	\
		|| p ## a ## .z < l.minPlane.y	\
		|| p ## a ## .z > l.maxPlane.y	)


	void getLighting(const VC3 &position, PointLights &pointLights, bool includeFactor, float radius, bool smoothedTransitions, IStorm3D_Model *model) const
	{
		// on shadowgrounds we want to use value read from lightmap when no lights near
#ifndef PROJECT_SHADOWGROUNDS
		// on survivor we clear it out
		pointLights.ambient = COL();	
#endif
		VC2 player2(position.x, position.z);

		// Do not use OBB -check for models with bones.
		if(model && model->hasBones())
			model = NULL;

		AABB bbox;
		QUAT oldRot;
		VC3 oldPos;
		MAT transf;
		OOBB modelOBB;
		float modelRadius = 0.f;
		if(model)
			modelRadius = model->GetRadius();

		if(model)
		{
			VC3 bboxCenter;

			transf = model->GetMX();
			oldRot = model->GetRotation ();
			oldPos = model->GetPosition ();

			//model->SetRotation ( QUAT (0, 0, 0, 1) );
			//model->SetPosition ( VC3  (0, 0, 0) );
			
			// Get model's bounding box ignoring "EditorOnly" -meshes.
			bbox.mmax = VC3(-100000.0f,-100000.0f,-100000.0f );
			bbox.mmin = VC3( 100000.0f, 100000.0f, 100000.0f );
			boost::scoped_ptr<Iterator<IStorm3D_Model_Object *> > object_iterator(model->ITObject->Begin());
			VC3 v[8];
			for(; !object_iterator->IsEnd(); object_iterator->Next())
			{
				IStorm3D_Model_Object *object = object_iterator->GetCurrent();
				if(!object)
					continue;
				IStorm3D_Mesh *mesh = object->GetMesh();
				if(!mesh)
					continue;
				// const char *foobar = object->GetName();
				if(strstr(object->GetName(), "EditorOnly"))
					continue;

				AABB meshBBox = object->GetBoundingBox () ;
				v[0] = VC3(meshBBox.mmin.x, meshBBox.mmin.y, meshBBox.mmin.z);
				v[1] = VC3(meshBBox.mmax.x, meshBBox.mmin.y, meshBBox.mmin.z);
				v[2] = VC3(meshBBox.mmin.x, meshBBox.mmax.y, meshBBox.mmin.z);
				v[3] = VC3(meshBBox.mmin.x, meshBBox.mmin.y, meshBBox.mmax.z);
				v[4] = VC3(meshBBox.mmax.x, meshBBox.mmax.y, meshBBox.mmin.z);
				v[5] = VC3(meshBBox.mmin.x, meshBBox.mmax.y, meshBBox.mmax.z);
				v[6] = VC3(meshBBox.mmax.x, meshBBox.mmin.y, meshBBox.mmax.z);
				v[7] = VC3(meshBBox.mmax.x, meshBBox.mmax.y, meshBBox.mmax.z);

				for(int i = 0; i < 8; ++i)
				{
					bbox.mmin.x = min(bbox.mmin.x, v[i].x);
					bbox.mmin.y = min(bbox.mmin.y, v[i].y);
					bbox.mmin.z = min(bbox.mmin.z, v[i].z);
					bbox.mmax.x = max(bbox.mmax.x, v[i].x);
					bbox.mmax.y = max(bbox.mmax.y, v[i].y);
					bbox.mmax.z = max(bbox.mmax.z, v[i].z);
				}
			}


			// Construct OBB from mesh AABB + from its orientation.
			bboxCenter = (bbox.mmax + bbox.mmin) / 2.0f;
			//MAT rotMat = transf.GetWithoutTranslation();
			//rotMat.TransformVector ( bboxCenter );
			transf.RotateVector(bboxCenter);
			modelOBB.center  = oldPos + bboxCenter;
			modelOBB.axes[0] = VC3( transf.Get( 0 ), transf.Get( 1 ), transf.Get( 2 ) ) ;
			modelOBB.axes[1] = VC3( transf.Get( 4 ), transf.Get( 5 ), transf.Get( 6 ) ) ;
			modelOBB.axes[2] = VC3( transf.Get( 8 ), transf.Get( 9 ), transf.Get( 10 )) ;
			modelOBB.extents = (bbox.mmax - bbox.mmin) / 2.0f;

		}

		int closest[LIGHT_MAX_AMOUNT + 1] = { 0 };
		for(int i = 0; i <= LIGHT_MAX_AMOUNT; ++i)
			closest[i] = -1;

		for(unsigned int i = 0; i < lights.size(); ++i)
		{
			const Light &l = lights[i];
/*
if(fabsf(l.minPlane.x - l.position.x + l.range) > 0.01f)
	abort();
if(fabsf(l.minPlane.y - l.position.z + l.range) > 0.01f)
	abort();
if(fabsf(l.maxPlane.x - l.position.x - l.range) > 0.01f)
	abort();
if(fabsf(l.maxPlane.y - l.position.z - l.range) > 0.01f)
	abort();

VC2 haxP = l.maxPlane + l.minPlane;
haxP *= 0.5f;
if(fabsf(haxP.x - l.position.x) > 0.1f)
	abort();
if(fabsf(haxP.y - l.position.z) > 0.1f)
	abort();
*/
			if(!l.enabled)
				continue;

			if(!model)
			{
				if(position.x < l.minPlane.x - radius)
					continue;
				if(position.z < l.minPlane.y - radius)
					continue;
				if(position.x > l.maxPlane.x + radius)
					continue;
				if(position.z > l.maxPlane.y + radius)
					continue;
			} 
			else
			{
				// New range check
				float squareRangeToLight = l.position.GetSquareRangeTo(position);
				float squareRange = modelRadius + l.range;
				squareRange *= squareRange;
				if(squareRangeToLight > squareRange)
					continue;

				AABB lightAABB;
				lightAABB.mmax = VC3( l.maxPlane.x, l.position.y + l.range, l.maxPlane.y );
				lightAABB.mmin = VC3( l.minPlane.x, l.position.y - l.range, l.minPlane.y );
				
				if( !collision( lightAABB, modelOBB) )
					continue;
		
			}

			const Light &l2 = lights[i];
			//float f2 = getFadeFactor(player2, l2.minPlane, l2.maxPlane, radius);
			float f2 = getLightFadeFactor(l2, position, radius);

			for(int j = 0; j < maxLightAmount; ++j)
			{
				if(closest[j] == -1)
				{
					closest[j] = i;
					break;
				}

				const Light &l1 = lights[closest[j]];
				//float f1 = getFadeFactor(player2, l1.minPlane, l1.maxPlane, radius);
				float f1 = getLightFadeFactor(l1, position, radius);
				
				bool update = f2 < f1;
				if(l2.dynamic)
				{
					if(j < STATIC_LIGHT_LIMIT)
						update = false;
				}
				if(!l2.dynamic)
				{
					if(j >= STATIC_LIGHT_LIMIT)
						update = false;
				}
				if(l2.dynamic && !l1.dynamic)
				{
					if(j >= STATIC_LIGHT_LIMIT)
						update = true;
				}
				if(!l2.dynamic && l1.dynamic)
				{
					if(j < STATIC_LIGHT_LIMIT)
						update = true;
				}

				/*
				else if(!l2.dynamic && l1.dynamic)
				{
					if(i < STATIC_LIGHT_LIMIT)
						update = false;
				}
				*/

				//if(f2 < f1)
				if(update)
				{
					for(int k = maxLightAmount; k > j; --k)
					{
						int index = closest[k - 1];
						if(index != -1)
						{
							const Light &light = lights[closest[j]];
							if(!light.dynamic && k >= STATIC_LIGHT_LIMIT)
								continue;
						}

						closest[k] = closest[k - 1];
					}

					closest[j] = i;
					break;
				}
			}
		}
		/*
		if(model)
		{
			model->SetRotation(oldRot);
			model->SetPosition(oldPos);
		}
		*/


/*
for(int i = STATIC_LIGHT_LIMIT; i < LIGHT_MAX_AMOUNT; ++i)
{
	int index = closest[i];
	if(index != -1)
	{
//		if(!lights[index].dynamic)
//			closest[i] = -1;
	}
}
*/
		// Set ..
		for(int i = 0; i < LIGHT_MAX_AMOUNT; ++i)
		{
			int index = closest[i];
			pointLights.internalIndices[i] = index;

			if(index != -1)
				index = lights[index].lightIndex;

			pointLights.lightIndices[i] = index;
		}

		if(closest[0] != -1)
		{
			const Light &l0 = lights[closest[0]];
			//float f0 = getFadeFactor(player2, l0.minPlane, l0.maxPlane, radius);
			float f0 = getLightFadeFactor(l0, position, radius);
			if(f0 > 0.8f)
				f0 = 0.8f;

			pointLights.ambient *= f0;
#ifdef PROJECT_SHADOWGROUNDS
		} else {
			pointLights.ambient *= .8;
#endif
		}

		/*
		if(closest[0] == -1)
		{
			for(int i = 0; i < 2; ++i)
			{
				pointLights.lights[i].position = VC3();
				pointLights.lights[i].color = COL();
				pointLights.lights[i].range = 1.f;
			}

			return;
		}

//closest[1] = -1;

		// Light 0
		const Light &l0 = lights[closest[0]];
		float f0 = getFadeFactor(player2, l0.minPlane, l0.maxPlane, radius);
		if(f0 > 0.8f)
			f0 = 0.8f;

		pointLights.lights[0].position = l0.position;
		pointLights.lights[0].range = l0.range;
		if(includeFactor)
			pointLights.lights[0].color = l0.color * lightColor;
		else
			pointLights.lights[0].color = l0.color;

		if(closest[1] == -1)
		{
			pointLights.ambient *= f0;
			return;
		}

		// Light 1
		const Light &l1 = lights[closest[1]];
		float f1 = getFadeFactor(player2, l1.minPlane, l1.maxPlane, radius);
		//if(f1 > 0.8f)
		//	f1 = 0.8f;

		pointLights.lights[1].position = l1.position;
		pointLights.lights[1].range = l1.range;
		if(includeFactor)
			pointLights.lights[1].color = l1.color * lightColor;
		else
			pointLights.lights[1].color = l1.color;

		if(closest[2] == -1)
		{
	//pointLights.lights[1].color *= (1.f - f1);
			pointLights.ambient *= f0;
			return;
		}

		// Light 2 (for fading out light 1)
		const Light &l2 = lights[closest[2]];
		float f2 = getFadeFactor(player2, l2.minPlane, l2.maxPlane, radius);
		//if(f2 > 0.8f)
		//	f2 = 0.8f;
		float fadeFactor = (f2 - f1) / (1.f - f1);
		//float fadeFactor = (f2 - f1) / f1;
		assert(fadeFactor >= 0);

		//pointLights.lights[0].color *= (1.f - (f1 - f0)) * fadeFactor;
		//pointLights.lights[0].color *= (1.f - (f1 - f0)) * fadeFactor;
		//pointLights.lights[0].color *= fadeFactor;
		//pointLights.lights[1].color *= fadeFactor;
		//pointLights.ambient *= (1.f - fadeFactor);
		//pointLights.ambient *= f0;

		if(smoothedTransitions)
			pointLights.lights[1].color *= fadeFactor;

		pointLights.ambient *= f0;
		*/



	}

	/*
	void getLighting(const COL &ambient, const VC3 &playerPosition, VC3 &position, COL &color, float &range, COL &resultAmbient, bool includeFactor) const
	{
		VC2 player2(playerPosition.x, playerPosition.z);

		int closest[2] = { -1, -1 };
		for(unsigned int i = 0; i < lights.size(); ++i)
		{
			const Light &l = lights[i];

			float rangeSquared = playerPosition.GetSquareRangeTo(l.position);
			//if(rangeSquared > l.range * l.range)
			//	continue;
			if(playerPosition.x < l.minPlane.x)
				continue;
			if(playerPosition.x > l.maxPlane.x)
				continue;
			if(playerPosition.z < l.minPlane.y)
				continue;
			if(playerPosition.z > l.maxPlane.y)
				continue;

			if(closest[0] == -1)
			{
				closest[0] = i;
				continue;
			}

			float fadeFactor = getFadeFactor(player2, l.minPlane, l.maxPlane);

			const Light &l1 = lights[closest[0]];
			float f1 = getFadeFactor(player2, l1.minPlane, l1.maxPlane);
			if(fadeFactor > f1)
			{
				closest[1] = closest[0];
				closest[0] = i;

				continue;
			}

			if(closest[1] == -1)
			{
				closest[1] = i;
				continue;
			}

			const Light &l2 = lights[closest[1]];
			float f2 = getFadeFactor(player2, l2.minPlane, l2.maxPlane);
			if(fadeFactor > f2)
			{
				closest[1] = i;
				continue;
			}
		}

		// No lights
		if(closest[0] == -1)
		{
			color = COL();
			resultAmbient = ambient;
			range = 1.f;

			return;
		}

		const Light &l1 = lights[closest[0]];
		float f1 = getFadeFactor(player2, l1.minPlane, l1.maxPlane);
		range = l1.range;

		resultAmbient = ambient;
		position = l1.position;
		if(includeFactor)
			color = l1.color * lightColor;
		else
			color = l1.color;

		// 1 light
		if(closest[1] == -1)
		{
			if(f1 > 0.8f)
				f1 = 0.8f;
			
			resultAmbient *= (1.f - f1);
			return;
		}

		// 2 lights

		const Light &l2 = lights[closest[1]];
		float f2 = getFadeFactor(player2, l2.minPlane, l2.maxPlane);

		assert(f1 >= f2);
		float fadeFactor = f1 - f2;
		resultAmbient *= (1.f - fadeFactor);
		color *= fadeFactor;
	}
	*/

	float getFakelightFactor(const VC3 &position) const
	{
		VC2 position2(position.x, position.z);
		float result = 0.f;

		for(int i = 0; i < LIGHTING_FAKE_SPOT_AMOUNT; ++i)
		{
			int spotIndex = activeFakes[i];
			if(spotIndex < 0 || spotIndex >= (int)fakeSpots.size())
				continue;

			const SpotImp &light = fakeSpots[spotIndex];

			VC2 planeMin = light.properties.minPlane;
			VC2 planeMax = light.properties.maxPlane;
			planeMin *= -light.properties.range;
			planeMax *=  light.properties.range;

			planeMin.x += light.position.x;
			planeMin.y += light.position.z;
			planeMax.x += light.position.x;
			planeMax.y += light.position.z;

			if(position.x < planeMin.x)
				continue;
			if(position.x > planeMax.x)
				continue;
			if(position.z < planeMin.y)
				continue;
			if(position.z > planeMax.y)
				continue;

			result += 1.5f * (getFadeFactor(position2, planeMin, planeMax, 0.f) * (light.fadeFactor));
		}

		if(result > 1.f)
			result = 1.f;

		return result;
	}

	void update(const VC3 &player, const VC3 &center, int ms)
	{
		updateLightingSpots(center, ms);
		
		updateFakeSpots(center, ms);
		//updateFakeSpots(player, ms);
		updateFakeSpots((player + center) * .5f, ms);
	}

	void updateLight(const Light &light)
	{
		VC3 updatePos = light.updatePosition - light.position;
		updatePos *= 0.5f;

		float updateRadius = updatePos.GetLength() + max(light.range, light.updateRange);
		updatePos += light.position;

updateRadius += 2.f;

		// ToDo: compare combined radius to invidual circle areas of
		// each position/radius combo -> current implementation might update
		// whole level if light is warping long distances

#if !(defined PROJECT_EDITOR) && !defined(PROJECT_VIEWER) && !defined(PROJECT_PARTICLE_EDITOR)
		if(uiTerrain)
			uiTerrain->updateLighting(updatePos, updateRadius);
#endif

	}
};

LightManager::LightManager(IStorm3D &storm, IStorm3D_Scene &scene, IStorm3D_Terrain &terrain, util::LightMap *lightMap, Terrain *uiTerrain)
{
	scoped_ptr<Data> tempData(new Data(storm, scene, terrain, lightMap, uiTerrain));
	data.swap(tempData);

	// TEST: ...
	/*
	Light red;
	Light green;
	Light blue;
	red.range = 10000.0f;
	red.color = COL(1,0,0);
	red.minPlane = VC2(-10000.0f, -10000.0f);
	red.maxPlane = VC2(10000.0f, 10000.0f);
	green.range = 10000.0f;
	green.color = COL(0,1,0);
	green.minPlane = VC2(-10000.0f, -10000.0f);
	green.maxPlane = VC2(10000.0f, 10000.0f);
	blue.range = 10000.0f;
	blue.color = COL(0,0,1);
	blue.minPlane = VC2(-10000.0f, -10000.0f);
	blue.maxPlane = VC2(10000.0f, 10000.0f);

	addLight(red);
	addLight(green);
	addLight(blue);
	*/

}

LightManager::~LightManager()
{
}

void LightManager::addBuildingLight(const VC3 &position, const COL &color, float range)
{
	LightPoint p;
	p.position = position;
	p.color = color;
	p.range = range;
	p.lightIndex = data->terrain.addLight(position, range, color);

	data->buildingLights.push_back(p);
}

void LightManager::setBuildingLights(IStorm3D_Model &model)
{
	model.ResetObjectLights();

	boost::scoped_ptr<Iterator<IStorm3D_Model_Object *> > objectIterator(model.ITObject->Begin());
	for(; !objectIterator->IsEnd(); objectIterator->Next())
	{
		IStorm3D_Model_Object *object = objectIterator->GetCurrent();
		if(!object)
			continue;
		std::string oname = object->GetName();
		if(oname.find("OuterWall") == oname.npos && oname.find("BuildingRoof") == oname.npos)
			continue;

		Sphere sphere = object->GetBoundingSphere();
		model.GetMX().TransformVector(sphere.position);

		// Find lights
		int closest[LIGHT_MAX_AMOUNT] = { 0 };
		for(int i = 0; i < LIGHT_MAX_AMOUNT; ++i)
			closest[i] = -1;

		{
			for(unsigned int j = 0; j < data->buildingLights.size(); ++j)
			{
				const LightPoint &light = data->buildingLights[j];

				float dist = sphere.position.GetRangeTo(light.position);
				if(dist - sphere.radius > light.range)
					continue;

				for(int k = 0; k < data->maxLightAmount; ++k)
				{
					if(closest[k] == -1)
					{
						closest[k] = j;
						break;
					}

					const LightPoint &l1 = data->buildingLights[closest[k]];
					float f1 = sphere.position.GetRangeTo(l1.position);
					const LightPoint &l2 = data->buildingLights[j];
					float f2 = sphere.position.GetRangeTo(l2.position);

					if(f2 < f1)
					{
						for(int l = data->maxLightAmount - 1; l > k; --l)
							closest[l] = closest[l - 1];

						closest[k] = j;
						break;
					}
				}

				/*
				if(closest[0] == -1)
				{
					closest[0] = j;
					continue;
				}

				float dist1 = sphere.position.GetRangeTo(data->buildingLights[closest[0]].position);
				if(dist < dist1)
				{
					closest[1] = closest[0];
					closest[0] = j;
					continue;
				}

				if(closest[1] == -1)
				{
					closest[1] = j;
					continue;
				}

				float dist2 = sphere.position.GetRangeTo(data->buildingLights[closest[1]].position);
				if(dist < dist2)
				{
					closest[1] = j;
					continue;			
				}
				*/
			}
		}

		// Apply
		{
			for(int j = 0; j < LIGHT_MAX_AMOUNT; ++j)
			{
				int index = closest[j];
				if(index == -1)
					continue;

				const LightPoint &light = data->buildingLights[index];
				//int id = model.AddObjectLight(light.position, light.color, light.range);

				object->SetLight(j, light.lightIndex);
			}
		}
	}
}

void LightManager::clearBuildingLights()
{
	data->buildingLights.clear();
}

void LightManager::addSpot(const VC3 &position, float yAngle, const SpotProperties &properties)
{
	SpotImp spot;
	spot.position = position;
	spot.position.y += properties.height;
	spot.yAngle = yAngle;
	spot.properties = properties;

	if(properties.type == SpotProperties::Lighting)
	{
		if(!spot.properties.texture.empty())
		{
			IStorm3D_Texture *t = data->storm.CreateNewTexture(spot.properties.texture.c_str());
			if(t)
				spot.texture.reset(t, ReleaseDeleter<IStorm3D_Texture>());
		}

		if(!spot.properties.coneTexture.empty())
		{
			IStorm3D_Texture *t = data->storm.CreateNewTexture(spot.properties.coneTexture.c_str());
			if(t)
				spot.coneTexture.reset(t, ReleaseDeleter<IStorm3D_Texture>());
		}

		if(!spot.properties.lightModel.empty())
		{
			IStorm3D_Model *m = data->storm.CreateNewModel();
			if(m)
			{
				if(m->LoadS3D(spot.properties.lightModel.c_str()))
				{
					spot.lightModel.reset(m);
					data->scene.AddModel(m);
				}
				else
					delete m;
			}
		}

		data->spotGroups[properties.group].push_back(spot);
		if(data->isActive(properties.group))
		{
			data->spots.push_back(spot);
			data->semiDynamicLightModify(spot, true);
		}
	}
	else
	{
		spot.colorMul = properties.strength;

		//spot.position.y += 0.06f;
		//spot.position.y += 0.11f;
		spot.position.y += 0.1f;

		data->fakeSpotGroups[properties.group].push_back(spot);
		if(data->isActive(properties.group))
			data->fakeSpots.push_back(spot);
	}
}

void LightManager::clearSpots()
{
	data->spots.clear();
	data->fakeSpots.clear();
	data->spotGroups.clear();
	data->fakeSpotGroups.clear();
}

void LightManager::enableGroup(int group, bool enable)
{
	if(enable)
	{
		if(data->isActive(group))
		{
			return;
		} else {
			data->activeGroups.push_back(group);

			const SpotList &spots = data->spotGroups[group];
			SpotList::const_iterator i = spots.begin();
			for(; i != spots.end(); ++i)
			{
				const SpotImp &spot = *i;
				data->semiDynamicLightModify(spot, true);
			}
		}
	}
	else
	{
		IntList::iterator it = data->activeGroups.begin();
		for(; it != data->activeGroups.end(); ++it)
		{
			if(*it == group)
			{
				data->activeGroups.erase(it);

				const SpotList &spots = data->spotGroups[group];
				SpotList::const_iterator i = spots.begin();
				for(; i != spots.end(); ++i)
				{
					const SpotImp &spot = *i;
					data->semiDynamicLightModify(spot, false);
				}

				break;
			}
		}
	}

	data->collectLights();
}

void LightManager::setLightingLevel(int level)
{
	data->lightLevel = level;
}

void LightManager::setShadowLevel(int level)
{
	data->shadowLevel = level;

	if(level >= 100)
		LIGHTING_FAKE_SPOT_AMOUNT = LIGHTING_FAKE_SPOT_MAX_AMOUNT;
	else if(level >= 50)
		LIGHTING_FAKE_SPOT_AMOUNT = LIGHTING_FAKE_SPOT_MAX_AMOUNT - 1;
	else
		LIGHTING_FAKE_SPOT_AMOUNT = 0;

}

void LightManager::setMaxLightAmount(int maxLights)
{
	if(maxLights < 1 || maxLights > LIGHT_MAX_AMOUNT)
	{
		assert(!"Invalid parameters");
		return;
	}

	data->maxLightAmount = maxLights;
}

int LightManager::addLight(const Light &light_)
{
	int index = -1;
	for(int i = 0; i < int(data->lights.size()); ++i)
	{
		if(!data->lights[i].enabled)
		{
			index = i;
			break;
		}
	}

	if(index == -1)
	{
		Light light;
		data->lights.push_back(light);

		index = data->lights.size() - 1;
	}

	Light &light = data->lights[index];
	light.position = light_.position;
	light.range = light_.range;
	light.color = light_.color;
	light.minPlane = light_.minPlane;
	light.maxPlane = light_.maxPlane;
	light.dynamic = light_.dynamic;

	light.updatePosition = light.position;
	light.updateRange = light.range;
	light.enabled = true;

	if(light.lightIndex == -1)
	{
		light.lightIndex = data->terrain.addLight(light.position, light.range, light.color);
	}
	else
	{
		data->terrain.setLightPosition(light.lightIndex, light.position);
		data->terrain.setLightColor(light.lightIndex, light.color);
		data->terrain.setLightRadius(light.lightIndex, light.range);
	}

	data->updateLight(light);
	return index;
}

void LightManager::setLightPosition(int lightIndex, const VC3 &position)
{
	if(lightIndex < 0 || lightIndex >= int(data->lights.size()))
	{
		assert(!"Invalid light parameters");
		return;
	}

	Light &light = data->lights[lightIndex];
	assert(light.enabled);
	light.position = position;

	// Hardcoded 10cm update step

	float changeRange = light.updatePosition.GetSquareRangeTo(position);
	if(changeRange > 0.1f * 0.1f)
	{
		/*
		changeRange = sqrtf(changeRange);
		light.minPlane.x -= changeRange;
		light.minPlane.y -= changeRange;
		light.maxPlane.x += changeRange;
		light.maxPlane.y += changeRange;
		*/

		VC3 delta = position - light.updatePosition;
		light.minPlane.x += delta.x;
		light.minPlane.y += delta.z;
		light.maxPlane.x += delta.x;
		light.maxPlane.y += delta.z;
/*
if(light.minPlane.x > light.position.x - light.range + 0.1f)
	abort();
if(light.minPlane.y > light.position.z - light.range + 0.1f)
	abort();
*/
		light.updatePosition = position;
		data->updateLight(light);
	}

	data->terrain.setLightPosition(light.lightIndex, light.position);
}

void LightManager::setLightRadius(int lightIndex, float radius)
{
	if(lightIndex < 0 || lightIndex >= int(data->lights.size()))
	{
		assert(!"Invalid light parameters");
		return;
	}

	Light &light = data->lights[lightIndex];
	assert(light.enabled);
	light.range = radius;

	// Hardcoded 10cm update step (larger)
	// Hardcoded 50cm update step (smaller)

	float rangeDelta = light.updateRange - radius;
	if(rangeDelta > 0.1f || rangeDelta < 0.5f)
	{
		light.minPlane.x -= fabsf(rangeDelta) * ((light.updatePosition.x - light.minPlane.x) / light.range);
		light.minPlane.y -= fabsf(rangeDelta) * ((light.updatePosition.z - light.minPlane.y) / light.range);
		light.maxPlane.x += fabsf(rangeDelta) * ((light.maxPlane.x - light.updatePosition.x) / light.range);
		light.maxPlane.y += fabsf(rangeDelta) * ((light.maxPlane.y - light.updatePosition.z) / light.range);

		light.updateRange = radius;
		data->updateLight(light);
	}

	data->terrain.setLightRadius(light.lightIndex, radius);
}

void LightManager::setLightColor(int lightIndex, const COL &color)
{
	if(lightIndex < 0 || lightIndex >= int(data->lights.size()))
	{
		assert(!"Invalid light parameters");
		return;
	}

	Light &light = data->lights[lightIndex];
	assert(light.enabled);
	light.color = color;

	data->terrain.setLightColor(light.lightIndex, color);
}

void LightManager::removeLight(int lightIndex)
{
	if(lightIndex < 0 || lightIndex >= int(data->lights.size()))
	{
		assert(!"Invalid light parameters");
		return;
	}

	Light &light = data->lights[lightIndex];
	assert(light.enabled);
	if(light.enabled)
		data->updateLight(light);

	light.enabled = false;
	data->terrain.setLightColor(light.lightIndex, COL(0.f, 0.f, 0.f));
}

void LightManager::clearLights()
{
	data->terrain.clearLights();
	data->lights.clear();
}

UnifiedHandle LightManager::getUnifiedHandle(int lightId) const
{
	return UNIFIED_HANDLE_FIRST_LIGHT + lightId;
}

int LightManager::getLightId(UnifiedHandle handle) const
{
	return handle - UNIFIED_HANDLE_FIRST_LIGHT;
}

bool LightManager::doesLightExist(UnifiedHandle handle) const
{
	int lightIndex = getLightId(handle);
	if(lightIndex < 0 || lightIndex >= int(data->lights.size()))
	{
		return false;
	}

	Light &light = data->lights[lightIndex];
	return light.enabled;
}

UnifiedHandle LightManager::getFirstLight() const
{
	UnifiedHandle handle = getUnifiedHandle(0);
	if(doesLightExist(handle))
		return handle;

	return getNextLight(handle);
}

UnifiedHandle LightManager::getNextLight(UnifiedHandle handle) const
{
	int lightIndex = getLightId(handle) + 1;
	int maxLights = data->lights.size();
	if(lightIndex < 0 || lightIndex >= maxLights)
		return UNIFIED_HANDLE_NONE;

	for(int i = lightIndex; i < maxLights; ++i)
	{
		Light &light = data->lights[i];
		if(light.enabled)
			return getUnifiedHandle(i);
	}

	return UNIFIED_HANDLE_NONE;
}

void LightManager::getLighting(const VC3 &position, PointLights &lights, float range, bool smoothedTransitions, bool includeFactor, IStorm3D_Model *override_model) const
{
	data->getLighting(position, lights, includeFactor, range, smoothedTransitions, override_model);
}

float LightManager::getFakelightFactor(const VC3 &point) const
{
	return data->getFakelightFactor(point);
}

COL LightManager::getApproximatedLightingForIndices(const VC3 &position, const PointLights &lights) const
{
	COL result;

	for(int i = 0; i < LIGHT_MAX_AMOUNT; ++i)
	{
		int index = lights.internalIndices[i];
		if(index == -1)
			continue;

		const Light &light = data->lights[index];
		float range = light.range + 1.f;

		VC3 distVec = light.position - position;
		float distance = distVec.GetLength();
		if(distance > range)
			continue;

		float factor = 1.f - (distance / range);
		result += light.color * factor;
	}

	result.Clamp();
/*
	// Hack around bad color/fog combinations
#ifdef PROJECT_SURVIVOR
	float maxLight = max(result.r, result.g);
	maxLight = max(maxLight, result.b);

	result.r = maxLight;
	result.g = maxLight;
	result.b = maxLight;
#endif
*/
	return result;
}

void LightManager::update(const VC3 &player, const VC3 &center, int ms)
{
	data->update(player, center, ms);
}

void LightManager::setLightingSpotCullRange(float cullRange)
{
	// Squared
	LIGHTING_SPOT_CULL_RANGE = cullRange * cullRange;
}

void LightManager::setLightingSpotFadeoutRange(float fadeoutRange)
{
	// NOT squared
	LIGHTING_SPOT_FADEOUT_RANGE = fadeoutRange;
}

void LightManager::setLightColorMultiplier(const COL &color)
{
	data->lightColor = color;
}

util::SelfIlluminationChanger *LightManager::getSelfIlluminationChanger()
{
	return &data->selfIlluminationChanger;
}

float getRadius(IStorm3D_Model *model)
{
	if(!model)
		return 0.f;

	//return model->GetRadius();

	VC3 position = model->GetPosition();
	model->SetPosition(VC3());

	VC3 min;
	VC3 max;
	model->GetVolumeApproximation(min, max);
	model->SetPosition(position);

	min.y = 0;
	max.y = 0;
	return min.GetRangeTo(max) * 0.5f;
}

} // ui
