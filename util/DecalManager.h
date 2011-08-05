#ifndef INCLUDED_DECALMANAGER_H
#define INCLUDED_DECALMANAGER_H

#include <boost/scoped_ptr.hpp>
#include <DatatypeDef.h>
#include <vector>

class IStorm3D_TerrainDecalSystem;

namespace frozenbyte {

struct DecalIdentifier
{
	int materialId;
	int decalId;
	int id;

	DecalIdentifier()
	:	materialId(0),
		decalId(0),
		id(0)
	{
	}
};

class DecalManager
{
	struct Data;
	boost::scoped_ptr<Data> data;

public:
	DecalManager(IStorm3D_TerrainDecalSystem &system);
	~DecalManager();

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

	void setMaxDecalAmount(int amount);
	void setEraseProperties(int fadeOutTime);
	void eraseDecal(const DecalIdentifier &decal);

	void setSpawnProperties(int waitTime, int fadeInTime, FadeType type);
	void addDecal(DecalIdentifier &id, Type type, int materialId, const VC3 &position, const VC2 &size, const QUAT &rotation, const COL &light, bool inBuilding, int fadeOutWaitTime, bool forceSpawn);	
	void update(int timeDelta);

	IStorm3D_TerrainDecalSystem &getDecalSystem();
};

} // frozenbyte

#endif
