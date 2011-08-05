#include <boost/shared_ptr.hpp>
#include <boost/scoped_ptr.hpp>
#include <vector>
#include <string>
#include <map>
#include <list>
#include <fstream>
#include <storm3d_ui.h>
#include "..\editor\string_conversions.h"
#include "..\editor\parser.h"
#include "track.h"
#include "paramblock.h"
#include "parseutil.h"
#include "particletiming.h"
#include "particlesystem.h"
#include "particlegravity.h"


namespace frozenbyte
{
namespace particle
{

class ParticleGravity;
class ParticleGravityClassDesc;

static ParamDesc theParticleGravityParamDesc[] = {
	ParamDesc(PB_GRAVITY, "gravity", PARAM_FLOAT, false)
};


class ParticleGravity : public GenParticleForce {
	float m_gravity;
public:

	ParticleGravity() {
	}

	void create() {
		GenParticleForce::create();
		m_pb->addParams(theParticleGravityParamDesc, 1);
	}
	
	const char* getClassName() {
		return "gravity";
	}

	const char* getSuperClassName() {
		return "gen_force";
	}
				
	void parseFrom(const editor::ParserGroup& pg) {
		GenParticleForce::parseFrom(pg);
	}
	
	void parseTo(editor::ParserGroup& pg) {
	
	}
	
	void preCalculate(float t) {
		m_pb->getValue(PB_GRAVITY, m_gravity);
		m_gravity *= PARTICLE_TIME_SCALE;
	}
		
	void calcForce(Vector& force, const Vector& pos, const Vector& vel) {
		force.x = 0;
		force.y = 0;
		force.y = -m_gravity;
	}
	
};

class ParticleGravityClassDesc : public ParticleForceClassDesc {
public:
	void* create() { return new ParticleGravity(); }
	const char* getClassName() { return "gravity"; }
};

static ParticleGravityClassDesc theParticleGravityClassDesc;

ParticleForceClassDesc* getParticleGravityClassDesc() {
	return &theParticleGravityClassDesc;
}


} // particle
} // frozenbyte