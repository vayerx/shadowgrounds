
#include "precompiled.h"

#include "VisualEffectType.h"

#include <string.h>
#include "VisualObject.h"
#include "VisualObjectModel.h"
#include "../system/Logger.h"

namespace ui
{
	VisualEffectType::VisualEffectType()
	{
		name = NULL;
		model = NULL;
		particleEffect = NULL;
		particleEffectHardwareFluid = NULL;
		particleEffectHardwareRigid = NULL;
		decalEffect = NULL;
		effectType = VISUALEFFECT_TYPE_NORMAL;
		follow = VISUALEFFECT_FOLLOW_NONE;
		//visualProto = NULL;
		modelEffects = 0;
		visualProtoModel = NULL;
		lightEffect = NULL;
		pointLightEffect = NULL;
		fadeout = false;
    particleEffectID = -1;
    particleEffectHardwareFluidID = -1;
    particleEffectHardwareRigidID = -1;
    decalEffectID = -1;
    decalAmount = 0;
    decalAmountVariation = 0;
    decalPositionRandom = 0;
    decalAutoRemove = false;
    decalPositioning = DecalPositionCalculator::DECAL_POSITIONING_INVALID;
		cameraShakeAmount = 0;
		attachToSpawnModel = false;
	}


	VisualEffectType::~VisualEffectType()
	{
		// these will delete the arrays...
		setName(NULL);
		setModelFilename(NULL);
		setParticleEffect(NULL);
		setParticleEffectHardwareFluid(NULL);
		setParticleEffectHardwareRigid(NULL);
		setSpotLightEffect(NULL);
		setPointLightEffect(NULL);
		//if (visualProto != NULL)
		//{
		//	delete visualProto;
		//	visualProto = NULL;
		//}
		if (visualProtoModel != NULL)
		{
			delete visualProtoModel;
			visualProtoModel = NULL;
		}
	}


	void VisualEffectType::setName(const char *name)
	{
		if (this->name != NULL)
		{
			delete [] this->name;
			this->name = NULL;
		}
		if (name != NULL)
		{
			this->name = new char[strlen(name) + 1];
			strcpy(this->name, name);
		}
	}


	void VisualEffectType::setModelFilename(const char *model)
	{
		if (this->model != NULL)
		{
			delete [] this->model;
			this->model = NULL;
		}
		if (model != NULL)
		{
			this->model = new char[strlen(model) + 1];
			strcpy(this->model, model);
		}
	}


	void VisualEffectType::setSpotLightEffect(const char *lightEffect)
	{
		if (this->lightEffect != NULL)
		{
			delete [] this->lightEffect;
			this->lightEffect = NULL;
		}
		if (lightEffect != NULL)
		{
			this->lightEffect = new char[strlen(lightEffect) + 1];
			strcpy(this->lightEffect, lightEffect);
		}
	}


	void VisualEffectType::setPointLightEffect(const char *pointLightEffect)
	{
		if (this->pointLightEffect != NULL)
		{
			delete [] this->pointLightEffect;
			this->pointLightEffect = NULL;
		}
		if (pointLightEffect != NULL)
		{
			this->pointLightEffect = new char[strlen(pointLightEffect) + 1];
			strcpy(this->pointLightEffect, pointLightEffect);
		}
	}


	void VisualEffectType::setParticleEffect(const char *particleEffect)
	{
		if (this->particleEffect != NULL)
		{
			delete [] this->particleEffect;
			this->particleEffect = NULL;
		}
		if (particleEffect != NULL)
		{
			this->particleEffect = new char[strlen(particleEffect) + 1];
			strcpy(this->particleEffect, particleEffect);
		}
	}


	void VisualEffectType::setParticleEffectHardwareFluid(const char *particleEffectHardwareFluid)
	{
		if (this->particleEffectHardwareFluid != NULL)
		{
			delete [] this->particleEffectHardwareFluid;
			this->particleEffectHardwareFluid = NULL;
		}
		if (particleEffectHardwareFluid != NULL)
		{
			this->particleEffectHardwareFluid = new char[strlen(particleEffectHardwareFluid) + 1];
			strcpy(this->particleEffectHardwareFluid, particleEffectHardwareFluid);
		}
	}


	void VisualEffectType::setParticleEffectHardwareRigid(const char *particleEffectHardwareRigid)
	{
		if (this->particleEffectHardwareRigid != NULL)
		{
			delete [] this->particleEffectHardwareRigid;
			this->particleEffectHardwareRigid = NULL;
		}
		if (particleEffectHardwareRigid != NULL)
		{
			this->particleEffectHardwareRigid = new char[strlen(particleEffectHardwareRigid) + 1];
			strcpy(this->particleEffectHardwareRigid, particleEffectHardwareRigid);
		}
	}



	void VisualEffectType::setDecalEffect(const char *decalEffect)
	{
		if (this->decalEffect != NULL)
		{
			delete [] this->decalEffect;
			this->decalEffect = NULL;
		}
		if (decalEffect != NULL)
		{
			this->decalEffect = new char[strlen(decalEffect) + 1];
			strcpy(this->decalEffect, decalEffect);
		}
	}


	void VisualEffectType::setModelEffect(int modelEffect)
	{
		modelEffects |= modelEffect;
	}


	void VisualEffectType::setFadeout(bool fadeout)
	{
		this->fadeout = true;
	}


	void VisualEffectType::setType(VISUALEFFECT_TYPE effectType)
	{
		this->effectType = effectType;
	}


	void VisualEffectType::setFollow(VISUALEFFECT_FOLLOW follow)
	{
		this->follow = follow;
	}


	VisualObject *VisualEffectType::getNewVisualObject()
	{
    if (visualProtoModel == NULL)
    {
			Logger::getInstance()->debug("VisualEffectType::getNewVisualObject - Creating a visual proto model.");
			Logger::getInstance()->debug(model);
		  visualProtoModel = new VisualObjectModel(model);
		}
		VisualObject *vo = visualProtoModel->getNewObjectInstance();
		vo->setEffect(modelEffects);
		vo->getStormModel()->CastShadows(false);
		return vo;
	}

	void VisualEffectType::setParticleEffectID(int id) {
		particleEffectID = id;
	}

	int VisualEffectType::getParticleEffectID() {
		return particleEffectID;
	}

	void VisualEffectType::setParticleEffectHardwareFluidID(int id) {
		particleEffectHardwareFluidID = id;
	}

	int VisualEffectType::getParticleEffectHardwareFluidID() {
		return particleEffectHardwareFluidID;
	}

	void VisualEffectType::setParticleEffectHardwareRigidID(int id) {
		particleEffectHardwareRigidID = id;
	}

	int VisualEffectType::getParticleEffectHardwareRigidID() {
		return particleEffectHardwareRigidID;
	}

	int VisualEffectType::getDecalEffectID()
	{
		return decalEffectID;
	}

	void VisualEffectType::setDecalEffectID(int id)
	{
		this->decalEffectID = id;
	}

	int VisualEffectType::getDecalAmount()
	{
		return decalAmount;
	}

	void VisualEffectType::setDecalAmount(int amount)
	{
		this->decalAmount = amount;
	}

	bool VisualEffectType::getDecalAutoRemove()
	{
		return decalAutoRemove;
	}

	void VisualEffectType::setDecalAutoRemove(bool value)
	{
		decalAutoRemove = value;
	}

	DecalPositionCalculator::DECAL_POSITIONING VisualEffectType::getDecalPositioning()
	{
		return this->decalPositioning;
	}

	void VisualEffectType::setDecalPositioning(DecalPositionCalculator::DECAL_POSITIONING positioning)
	{
		this->decalPositioning = positioning;
	}

	int VisualEffectType::getDecalAmountVariation()
	{
		return this->decalAmountVariation;
	}

	void VisualEffectType::setDecalAmountVariation(int amountVariation)
	{
		this->decalAmountVariation = amountVariation;
	}

	int VisualEffectType::getDecalPositionRandom()
	{
		return this->decalPositionRandom;
	}

	void VisualEffectType::setDecalPositionRandom(int positionRandom)
	{
		this->decalPositionRandom = positionRandom;
	}

	int VisualEffectType::getCameraShakeAmount()
	{
		return this->cameraShakeAmount;
	}

	void VisualEffectType::setCameraShakeAmount(int cameraShakeAmount)
	{
		this->cameraShakeAmount = cameraShakeAmount;
	}

}


