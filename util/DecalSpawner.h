#ifndef INCLUDED_DECALSPAWNER_H
#define INCLUDED_DECALSPAWNER_H

#include <boost/scoped_ptr.hpp>
#include <DatatypeDef.h>

class IStorm3D_Material;

namespace frozenbyte {

struct DecalIdentifier;

class DecalManager;

class DecalSpawner
{
	struct Data;
	boost::scoped_ptr<Data> data;

public:
	DecalSpawner(DecalManager &manager, IStorm3D_Material &material);
	~DecalSpawner();

	enum Type
	{
		Static = 0,
		Dynamic = 1
	};

	enum FadeType
	{
		Blend,
		Scale
	};

	void setType(Type type);
	void setSize(const VC2 &size);
	void setSizeFactor(float factor);
	void setSpawnProperties(int waitTime, int fadeInTime, FadeType type);
	void setAutoFadeOut(int time);
	void setForceSpawn(bool forceSpawn);

	void spawnDecal(DecalIdentifier &identifier, const VC3 &position, const QUAT &rotation, const COL &light, bool inBuilding);
};

} // frozenbyte

#endif
