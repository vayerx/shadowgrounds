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
#include "particlesystem.h"
#include "dragparticleforce.h"

namespace frozenbyte {
namespace particle
{


ParamDesc theDragParticleForceParamDesc[] = {
	ParamDesc(PB_DRAG_FACTOR, "factor", PARAM_FLOAT, false),
};

class DragParticleForce : public GenParticleForce {
	float m_factor;
	int m_function;
public:
	DragParticleForce() {
	}
	void create() {
		GenParticleForce::create();
		m_pb->addParams(theDragParticleForceParamDesc, 1);
	}
	const char* getSuperClassName() {
		return "gen_force";
	}
	const char* getClassName() {
		return "drag";
	}	
	void preCalculate(float t) {
		m_pb->getValue(PB_DRAG_FACTOR, m_factor);
	}
	void calcForce(Vector& force, const Vector& pos, const Vector& vel) {
		float len = vel.GetLength();
		Vector newVel = vel * pow(1.0f - m_factor, len);
		force = newVel - vel;
	}
	void parseFrom(const editor::ParserGroup& pg) {
		GenParticleForce::parseFrom(pg);
	}
	void parseTo(editor::ParserGroup& pg) {
	}
};

class DragParticleForceClassDesc : public ParticleForceClassDesc {
public:
	const char* getClassName() { return "drag"; }
	const char* getSuperClassName() { return "genforce"; }
	void* create() { return new DragParticleForce(); }
};

static DragParticleForceClassDesc theDragParticleForceClassDesc;

ParticleForceClassDesc* getDragParticleForceClassDesc() {
	return &theDragParticleForceClassDesc;
}

} // particle
} // frozenbyte