#ifndef INCLUDED_DECALSYSTEM_H
#define INCLUDED_DECALSYSTEM_H

#include <boost/scoped_ptr.hpp>
#include <DatatypeDef.h>

class IStorm3D;
class IStorm3D_TerrainDecalSystem;

namespace frozenbyte {

struct DecalIdentifier;

class DecalSystem
{
	struct Data;
	boost::scoped_ptr<Data> data;

public:
	DecalSystem(IStorm3D &storm, IStorm3D_TerrainDecalSystem &system);
	~DecalSystem();

	// Returns -1 if no effect with given name found
	int getEffectId(const char *name) const;
	float getMaxSize(int id) const;
	
	void spawnDecal(DecalIdentifier &identifier, int id, const VC3 &position, const QUAT &rotation, const COL &light, bool inBuilding);
	void eraseDecal(const DecalIdentifier &identifier);
	void update(int timeDelta);

	void setMaxDecalAmount(int amount);
	void setEraseProperties(int fadeOutTime);

};

} // frozenbyte

#endif
