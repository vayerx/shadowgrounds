
#include "precompiled.h"

#include "VisualEffect.h"

#include "VisualEffectType.h"
#include "VisualObject.h"
#include "IPointableObject.h"
#include "Muzzleflasher.h"
#include "Ejecter.h"
#include "Spotlight.h"
#include "../game/gamedefs.h"
#include "../system/Logger.h"
#include "../util/DecalManager.h"
#include "../game/unified_handle.h"
#include "../ui/LightManager.h"
#include "../ui/DynamicLightManager.h"

#include <assert.h>

#include <string>
#include <vector>
#include <list>

#include "../editor/parser.h"
#include "../particle_editor2/particleeffect.h"
#include "../particle_editor2/track.h"
#include <IStorm3D_Particle.h>
#include "../particle_editor2/particlesystem.h"
#include "../game/scaledefs.h"

using namespace frozenbyte;
using namespace frozenbyte::particle;

namespace ui
{

	int visual_effect_allocations = 0;

	VisualEffect::VisualEffect(VisualEffectType *effectType,
		IPointableObject *object, IPointableObject *origin,
		const VC3 &position, const VC3 &endPosition, const VC3 &rotation,
		int muzzleFlashBarrelNumber)
	{
		this->deleteFlag = false;
		this->effectType = effectType;
		this->position = VC3(0,0,0);
		this->visualObject = NULL;
		this->follow = NULL;
		this->refCount = 0;
		this->spotlight = NULL;
		this->pointlight = UNIFIED_HANDLE_NONE;
		this->pointlightManager = NULL;
		this->deleteFrameCounter = 0;
		this->advanceDeleteCounter = false;

		init(object, origin, position, endPosition, rotation, muzzleFlashBarrelNumber);

		visual_effect_allocations++;
	}


	VisualEffect::~VisualEffect()
	{
		assert(refCount == 0);

		assert(visual_effect_allocations > 0);
		visual_effect_allocations--;

		// make sure spotlights are deleted
		if (this->spotlight != NULL)
		{
			delete this->spotlight;
			this->spotlight = NULL;
		}

		if (this->pointlight != UNIFIED_HANDLE_NONE)
		{
			if (pointlightManager != NULL)
			{
				int lightTypeId = 0;
				int lightInstanceId = 0;
				this->pointlightManager->unifiedHandleToLightIds(this->pointlight, lightTypeId, lightInstanceId);
				this->pointlightManager->removeLightInstance(lightTypeId, lightInstanceId);
			}
		}

		// special case: muzzleflash
		if (effectType->getType() == VisualEffectType::VISUALEFFECT_TYPE_MUZZLEFLASH)
		{
			if (visualObject != NULL && follow != NULL)
			{
				Muzzleflasher::deleteMuzzleflash(follow);
			}
		}

		if(!muzzleAttachment.empty())
		{
			if(visualObject != NULL && follow != NULL)
			{
				Muzzleflasher::deleteMuzzleflash(follow, muzzleAttachment);
			}
		}

		// special case: eject
		if (effectType->getType() == VisualEffectType::VISUALEFFECT_TYPE_EJECT)
		{
			if (visualObject != NULL && follow != NULL)
			{
				Ejecter::deleteEject(follow);
			}
		}

		if (visualObject != NULL)
		{
			delete visualObject;
		}
	}


	void VisualEffect::init(IPointableObject *object, IPointableObject *origin,
		const VC3 &position, const VC3 &endPosition, const VC3 &rotation,
		int muzzleFlashBarrelNumber)
	{
		// make sure this is called only once.
		assert(visualObject == NULL);

		// set the object we want to follow...
		if (effectType->getFollow() == VisualEffectType::VISUALEFFECT_FOLLOW_NONE)
		{
			follow = NULL;
		} else {
			if (effectType->getFollow() == VisualEffectType::VISUALEFFECT_FOLLOW_OBJECT)
			{
				follow = object;
			} else {
				follow = origin;
			}
		}

		// visual object...
		visualObject = effectType->getNewVisualObject();

		if (effectType->getType() == VisualEffectType::VISUALEFFECT_TYPE_MUZZLEFLASH)
		{
			// special case: muzzleflash
			if (visualObject != NULL && follow != NULL)
			{
				Muzzleflasher::createMuzzleflash(follow, visualObject, muzzleFlashBarrelNumber);
				visualObject->setInScene(false);
			} else {
				//Logger::getInstance()->error("VisualEffect::init - Null visualObject or follow unit for muzzleflash.");
			}

			this->position = position;
			return;
		}

		if (effectType->getType() == VisualEffectType::VISUALEFFECT_TYPE_EJECT)
		{
			this->position = position;
			return;
		}

		// normal and ray visualeffects...

		visualObject->setCollidable(false);
		visualObject->setInScene(true);

		// set position, rotation and scale based on type and the 
		// given parameters...
		if (effectType->getType() == VisualEffectType::VISUALEFFECT_TYPE_NORMAL)
		{
			this->position = position;
		}
		if (effectType->getType() == VisualEffectType::VISUALEFFECT_TYPE_RAY)
		{
			this->position = (position + endPosition) / 2;
			VC3 diffVec = endPosition - position;
			float raylen = diffVec.GetLength();
			VC3 scale(1,1,raylen);
			visualObject->setScale(scale);
		}
		if (effectType->getType() == VisualEffectType::VISUALEFFECT_TYPE_NORMAL)
		{
			this->position = position;
		}

		visualObject->setPosition(this->position);
		visualObject->setRotation(rotation.x, rotation.y, rotation.z);
	
	}

	
	void VisualEffect::moveBetween(const VC3 &position_, const VC3 &endPosition, const VC3 &rotation, float alpha, float scale)
	{
		if (visualObject == NULL)
			return;

		if (effectType->getType() == VisualEffectType::VISUALEFFECT_TYPE_RAY)
		{
			this->position = (position_ + endPosition) / 2;
			VC3 diffVec = endPosition - position_;
			float raylen = diffVec.GetLength();
			VC3 scalevec(1*scale,1*scale,raylen);
			visualObject->setScale(scalevec);
			visualObject->setVisibilityFactor(alpha);
		}
		visualObject->setPosition(this->position);
		visualObject->setRotation(rotation.x, rotation.y, rotation.z);
	}

	void VisualEffect::setObjectVisibility(float alpha)
	{
		if (visualObject == NULL)
			return;

		visualObject->setVisibilityFactor(alpha);
	}


	void VisualEffect::addRemovableDecal(const frozenbyte::DecalIdentifier &id)
	{
		removableDecals.push_back(id);
	}

	void VisualEffect::freeReference()
	{
		assert(refCount > 0);
		refCount--;
		if (refCount == 0)
		{
			delete this;
		}
	}


	void VisualEffect::addReference()
	{
		refCount++;
	}


	void VisualEffect::setDeleteFlag()
	{
		assert(!deleteFlag);
		deleteFlag = true;

		if(particleEffect.get()) {
			particleEffect->kill();
		}
	
	}


	bool VisualEffect::isDeleteFlag()
	{
		return deleteFlag;
	}


	void VisualEffect::setPosition(const VC3 &position)
	{
		this->position = position;
		if(particleEffect.get()) 
		{	
			particleEffect->setPosition(position);
			//particleEffect->setVelocity(position - oldPosition);
		}
		if(visualObject != NULL) 
		{	
			visualObject->setPosition(position);
		}
	}


	void VisualEffect::setRotation(const VC3 &rotation)
	{
		if(particleEffect.get()) 
		{	
			particleEffect->setRotation(rotation);
		}
		if(visualObject != NULL) 
		{	
			visualObject->setRotation(rotation.x, rotation.y, rotation.z);
		}
	}


	/*
	void VisualEffect::setRotation(const QUAT &rotation)
	{
		if(visualObject != NULL) 
		{	
			// TODO: QUAT to axis angles...
			//visualObject->setRotation(rotation.x, rotation.y, rotation.z);
		}
		if(particleEffect.get()) 
		{	
			particleEffect->setRotation(rotation);
		}
	}
	*/


	void VisualEffect::run()
	{
		if (follow != NULL)
		{
			// HACK: particleeffect,etc. don't follow if muzzleflash!
			if (this->effectType->getType() != VisualEffectType::VISUALEFFECT_TYPE_MUZZLEFLASH && muzzleAttachment.empty())
			{
				//Vector oldPosition = position;
				position = follow->getPointerPosition() + follow->getPointerMiddleOffset();
			}
		}
		if(particleEffect) 
			particleEffect->setPosition(position);
		
		if(particleEffect)
			particleEffect->tick();

		if(spotlight)
			spotlight->fadeOut(GAME_TICK_MSEC);
	}


	void VisualEffect::prepareForRender()
	{
		if (visualObject != NULL)
		{
			visualObject->setPosition(position);
			visualObject->prepareForRender();
		}
		if (spotlight != NULL)
		{
			spotlight->prepareForRender();
		}
		if (pointlight != UNIFIED_HANDLE_NONE)
		{
			assert(pointlightManager != NULL);
			if (pointlightManager != NULL)
			{
				int lightTypeId = 0;
				int lightInstanceId = 0;
				this->pointlightManager->unifiedHandleToLightIds(this->pointlight, lightTypeId, lightInstanceId);
				VC3 lightpos = position;
#ifdef PROJECT_CLAW_PROTO
				// HACK: for now, in claw proto, raise the lights always something like 0.5m
				lightpos.y += 0.5f;
#endif
				this->pointlightManager->setLightInstancePosition(lightTypeId, lightInstanceId, lightpos);
			}
		}
	}

	void VisualEffect::setPointlight(UnifiedHandle pointlight, DynamicLightManager *pointlightManager)
	{
		this->pointlight = pointlight;
		this->pointlightManager = pointlightManager;
	}


	void VisualEffect::setParticleEffect(boost::shared_ptr<IParticleEffect> effect,
		const Vector& velocity, const VC3 &rotation) {
		particleEffect = effect;
		if(particleEffect.get()) {	
			particleEffect->setPosition(position);
			particleEffect->setVelocity(velocity);
			particleEffect->setRotation(rotation);
		}
	}

	void VisualEffect::setParticleEffectEmitRate(float rate)
	{
		if(particleEffect)
			particleEffect->setEmitFactor(rate);
	}

	void VisualEffect::setParticleExplosion(const VC3 &pos, bool useExplosion)
	{
		VC3 p = pos;
		p -= position;

		if(particleEffect)
		{
			for(int i = 0; i < particleEffect->getNumSystems(); ++i)
			{
				if(particleEffect->getParticleSystem(i))
					particleEffect->getParticleSystem(i)->setExplosion(p, useExplosion);
			}
		}
	}

	void VisualEffect::setSpotlight(Spotlight *spotlight)
	{
		assert(this->spotlight == NULL);

		this->spotlight = spotlight;
	}

	void VisualEffect::resetSpotlightFadeout(int atTime)
	{
		assert(this->spotlight != NULL);

		if (this->spotlight != NULL)
		{
			this->spotlight->resetFadeOut(atTime);
		}
	}

	void VisualEffect::makeMuzzleAttachment(IPointableObject *unit, const std::string &name, const std::string &helper)
	{
		muzzleAttachment = name;
		follow = unit;

		if (visualObject != NULL && follow != NULL)
		{
			Muzzleflasher::createMuzzleflash(follow, visualObject, name, helper);
			VC3 pos;
			VC3 scale;
			QUAT rot;
			if(Muzzleflasher::getMuzzleflash(follow, visualObject, name, helper, pos, scale, rot))
			{
				this->position = pos;
			}
		}
	}

	void VisualEffect::updateMuzzleAttachment(const std::string &name, const std::string &helper, const VC3 &pos, const VC3 &scale)
	{
		if (effectType->getType() != VisualEffectType::VISUALEFFECT_TYPE_MUZZLEFLASH && muzzleAttachment.empty())
			return;

		if(visualObject != NULL && follow != NULL)
		{
			Muzzleflasher::updateMuzzleflash(follow, visualObject, name, helper, this->position + pos, scale);
		}
	}

	bool VisualEffect::isMuzzleAttachment(IPointableObject *unit, const std::string &name, const std::string &helper)
	{
		if (visualObject == NULL || follow == NULL || muzzleAttachment.empty())
		{
			return false;
		}

		VC3 pos;
		VC3 scale;
		QUAT rot;
		return Muzzleflasher::getMuzzleflash(follow, visualObject, name, helper, pos, scale, rot);
	}
}

