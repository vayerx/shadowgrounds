
#ifndef VISUALEFFECTTYPE_H
#define VISUALEFFECTTYPE_H

#include <stdlib.h>

#include "DecalPositionCalculator.h"

namespace ui
{
	class VisualObject;
	class VisualObjectModel;

	class VisualEffectType
	{
	public:
		enum VISUALEFFECT_TYPE
		{
			VISUALEFFECT_TYPE_NORMAL = 1,
			VISUALEFFECT_TYPE_RAY = 2,
			VISUALEFFECT_TYPE_MUZZLEFLASH = 3,
			VISUALEFFECT_TYPE_EJECT = 4
		};

		enum VISUALEFFECT_FOLLOW
		{
			VISUALEFFECT_FOLLOW_NONE = 1,
			VISUALEFFECT_FOLLOW_OBJECT = 2,
			VISUALEFFECT_FOLLOW_ORIGIN = 3
		};

		VisualEffectType();

		~VisualEffectType();

		inline const char *getName() const
		{
			return name;
		}


		inline const char *getModelFilename() const
		{
			return model;
		}


		inline const char *getSpotLightEffect() const
		{
			return lightEffect;
		}


		inline const char *getPointLightEffect() const
		{
			return pointLightEffect;
		}


		inline const char *getParticleEffect() const
		{
			return particleEffect;
		}

		inline const char *getParticleEffectHardwareFluid() const
		{
			return particleEffectHardwareFluid;
		}

		inline const char *getParticleEffectHardwareRigid() const
		{
			return particleEffectHardwareRigid;
		}


		inline const char *getDecalEffect() const
		{
			return decalEffect;
		}


		inline VISUALEFFECT_FOLLOW getFollow() const
		{
			return follow;
		}


		inline bool hasFadeout() const
		{
			return fadeout;
		}


		inline VISUALEFFECT_TYPE getType() const
		{
			return effectType;
		}


		void setName(const char *name);

		void setModelFilename(const char *model);

		void setSpotLightEffect(const char *lightEffect);
		void setPointLightEffect(const char *pointLightEffect);

		void setModelEffect(int modelEffect);

		void setFadeout(bool fadeout);

		void setParticleEffect(const char *particleEffect);
		void setParticleEffectHardwareFluid(const char *particleEffectHardwareFluid);
		void setParticleEffectHardwareRigid(const char *particleEffectHardwareRigid);

		void setDecalEffect(const char *decalEffect);

		void setType(VISUALEFFECT_TYPE effectType);

		void setFollow(VISUALEFFECT_FOLLOW follow);

		/**
		 * Returns a new visual object created from the visualProtoModel.
		 * Used by VisualEffect to create a suitable visual object.
		 */
		VisualObject *getNewVisualObject();

		int getParticleEffectID();
		void setParticleEffectID(int id);
		int getParticleEffectHardwareFluidID();
		void setParticleEffectHardwareFluidID(int id);
		int getParticleEffectHardwareRigidID();
		void setParticleEffectHardwareRigidID(int id);

		int getDecalEffectID();
		void setDecalEffectID(int id);

		int getDecalAmount();
		void setDecalAmount(int amount);
		int getDecalAmountVariation();
		void setDecalAmountVariation(int amountVariation);
		int getDecalPositionRandom();
		void setDecalPositionRandom(int positionRandom);
		bool getDecalAutoRemove();
		void setDecalAutoRemove(bool value);
		
		bool doesAttachToSpawnModel() { return this->attachToSpawnModel; }
		void setAttachToSpawnModel(bool value) { this->attachToSpawnModel = value; }

		DecalPositionCalculator::DECAL_POSITIONING getDecalPositioning();
		void setDecalPositioning(DecalPositionCalculator::DECAL_POSITIONING positioning);

		int getCameraShakeAmount();
		void setCameraShakeAmount(int cameraShakeAmount);

	private:
		char *name;
		char *model;
		char *particleEffect;
		char *particleEffectHardwareFluid;
		char *particleEffectHardwareRigid;
		char *decalEffect;
		char *lightEffect;
		char *pointLightEffect;
		//VisualObject *visualProto;
		int modelEffects;
		VisualObjectModel *visualProtoModel;
		VISUALEFFECT_TYPE effectType;
		VISUALEFFECT_FOLLOW follow;
		int particleEffectID;		
		int particleEffectHardwareFluidID;		
		int particleEffectHardwareRigidID;		
		bool fadeout;
		int decalEffectID;		
		int decalAmount;
		int decalAmountVariation;
		int decalPositionRandom;
		bool decalAutoRemove;
		DecalPositionCalculator::DECAL_POSITIONING decalPositioning;
		int cameraShakeAmount;
		bool attachToSpawnModel;
	};
}

#endif

