
#ifndef SPOTLIGHT_H
#define SPOTLIGHT_H

#include <boost/scoped_ptr.hpp>
#include <DatatypeDef.h>
#include <string>

class IStorm3D;
class IStorm3D_Terrain;
class IStorm3D_Scene;
class IStorm3D_Model;

namespace ui
{
	struct SpotlightData;

	class SpotTypeProperties
	{
	public:
		enum FLASH_TYPE
		{
			FLASH_TYPE_NONE,
			FLASH_TYPE_EXPLOSION,
			FLASH_TYPE_ELECTRIC_FLOW,
			FLASH_TYPE_FLAMETHROWER,
			FLASH_TYPE_MUZZLE,
			FLASH_TYPE_FLAMER_MUZZLE,
			FLASH_TYPE_SCRIPT
		};

		enum SPOT_LIGHT_MODEL_TYPE
		{
			SPOT_LIGHT_MODEL_TYPE_NONE,  // (fake only)
			SPOT_LIGHT_MODEL_TYPE_FLAT,
			SPOT_LIGHT_MODEL_TYPE_POINT,
			SPOT_LIGHT_MODEL_TYPE_DIRECTIONAL
		};

		std::string name;
		std::string textureFilename;
		std::string coneTextureFilename;
		float fov;
		float coneFov;
		float range;
		SPOT_LIGHT_MODEL_TYPE type;
		float coneAlphaMultiplier;
		std::string modelFilename;
		bool shadows;
		bool fade;
		bool noShadowFromOrigin;
		bool fakelight;
		float fakeFadeRange;
		bool scissor;
		VC3 direction;
		VC3 positionOffset;
		COL color;
		COL fakelightColor;
		FLASH_TYPE flashType;
		std::string flashTypeScript;
		std::string lowDetailVersion;
		
		SpotTypeProperties()
			: fov(50.0f),
			coneFov(40.0f),
			range(20.0f),
			type(SPOT_LIGHT_MODEL_TYPE_DIRECTIONAL),
			coneAlphaMultiplier(0.3f),
			shadows(true),
			fade(true),
			noShadowFromOrigin(true),
			fakelight(false),
			fakeFadeRange(10.0f),
			scissor(true),
			direction(0,-1,0.001f),
			positionOffset(0,0,0),
			color(1,1,1),
			fakelightColor(1,1,1),
			flashType(FLASH_TYPE_NONE)
		{ 
			// (nothing else.)
		}
	};


	class Spotlight
	{
		boost::scoped_ptr<SpotlightData> data;

		public:
			Spotlight(IStorm3D &storm, IStorm3D_Terrain &terrain,
				IStorm3D_Scene &scene, IStorm3D_Model *originModel,
				const std::string &spottype);
			~Spotlight();

			void setPosition(const VC3 &position);
			void setFakePosition(const VC3 &position);
			void setDirection(const VC3 &direction);
			void setDirectionToward(const VC3 &direction, int timeElapsed);
			void setEnableClip( bool enable );

			void fadeOut(int timeElapsed);
			void resetFadeOut(int atTime = 0);

			void setOriginModel(IStorm3D_Model *originModel);

			void setFakelightParams(float fadeRange, COL color);
			void setFakelightBrightness(float brightness);

			void setRange( float range );

			void setSpotlightParams(COL color);

			void prepareForRender();

			// new method for adding new spot types from scripts instead of hard coded ones.
			// notice, this is not a well multithreadable system.
			static void addNewType(const char *spotTypeName);
			static SpotTypeProperties *getAddingSpotTypeProperties();
			static void addNewTypeDone();

	};
}

#endif



