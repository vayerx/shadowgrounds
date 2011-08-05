
#ifndef VISUALEFFECT_H
#define VISUALEFFECT_H

#include <DatatypeDef.h>

#ifndef INCLUDED_BOOST_SHARED_PTR_HPP
#define INCLUDED_BOOST_SHARED_PTR_HPP
#include <boost/shared_ptr.hpp>
#endif

#include "../game/unified_handle_type.h"

#include "../util/DecalManager.h"
#include <vector>

namespace game
{
	class Unit;
}

//namespace frozenbyte;

namespace frozenbyte {
	// this no longer works for std::vector since STLport 5.1.0-RC3...
	// (don't really understand how the hell it worked in the first place... ;) --jpk
	//struct DecalIdentifier;

	namespace particle {
		class IParticleEffect;
	}
}

namespace ui
{
	class IPointableObject;
	class VisualEffectType;
	class VisualObject;
	class Spotlight;
	class VisualEffectManager;
	class DynamicLightManager;
//	class frozenbyte::particle::IParticleEffect;

	class VisualEffect
	{
	public:
		VisualEffect(VisualEffectType *effectType,
			IPointableObject *object, IPointableObject *origin,
			const VC3 &position, const VC3 &endPosition, const VC3 &rotation,
			int muzzleFlashBarrelNumber = 1);

		~VisualEffect();

		void freeReference();
		void addReference();

		void setDeleteFlag();
		bool isDeleteFlag();

		void run();

		void prepareForRender();
		void setPosition(const VC3 &position);
		void setRotation(const VC3 &rotation);
		//void setRotation(const QUAT &rotation);
		void setParticleEffect(boost::shared_ptr<frozenbyte::particle::IParticleEffect> effect, 
			const VC3& velocity, const VC3 &rotation);
		void setParticleEffectEmitRate(float rate);
		void setParticleExplosion(const VC3 &position, bool useExplosion);

		void setSpotlight(Spotlight *spotlight);
		void resetSpotlightFadeout(int atTime = 0);

		void setPointlight(UnifiedHandle pointlight, DynamicLightManager *pointlightManager);

		void moveBetween(const VC3 &position, const VC3 &endPosition, const VC3 &rotation, float alpha, float scale);
		void addRemovableDecal(const frozenbyte::DecalIdentifier &id);

		void setObjectVisibility(float alpha);

		void makeMuzzleAttachment(IPointableObject *unit, const std::string &name, const std::string &helper);
		bool isMuzzleAttachment(IPointableObject *unit, const std::string &name, const std::string &helper);
		void updateMuzzleAttachment(const std::string &name, const std::string &helper, const VC3 &pos, const VC3 &scale);

	private:
		void init(IPointableObject *object, IPointableObject *origin,
			const VC3 &position, const VC3 &endPosition, const VC3 &rotation,
			int muzzleFlashBarrelNumber = 1);

		bool deleteFlag;
		VC3 position;
		VisualObject *visualObject;
		VisualEffectType *effectType;
		IPointableObject *follow;
		Spotlight *spotlight;
		UnifiedHandle pointlight;
		DynamicLightManager *pointlightManager;
		int refCount;
		int deleteFrameCounter;
		bool advanceDeleteCounter;

		friend class VisualEffectManager;
		boost::shared_ptr<frozenbyte::particle::IParticleEffect> particleEffect;
		std::vector<frozenbyte::DecalIdentifier> removableDecals;

		std::string muzzleAttachment;
	};

}


#endif


