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
#include "particlesystemmanager.h"
#include "particleeffect.h"
#include "sprayparticlesystem.h"

namespace frozenbyte
{
namespace particle
{

static ParamDesc sprayParticleSystemParamDesc[] = {
	ParamDesc(PB_SPREAD1, "spread1", PARAM_FLOAT, false),
	ParamDesc(PB_SPREAD2, "spread2", PARAM_FLOAT, false)
};

class SprayParticleSystem : public GenParticleSystem {
public:
	
	SprayParticleSystem();
	
	const char* getClassName();
	const char* getSuperClassName();

	ParticleSystem* launch();
	
	void create();
	void init(IStorm3D* s3d, IStorm3D_Scene* scene);
	void setParticlePosition(Vector& v);
	void setParticleVelocity(Vector& v, float speed);

};

SprayParticleSystem::SprayParticleSystem() {
	
}

void SprayParticleSystem::create() {

	GenParticleSystem::create();

	m_pb->addParams(sprayParticleSystemParamDesc, 2);
	m_pb->setValue(PB_SPREAD1, 0.0f);
	m_pb->setValue(PB_SPREAD2, 0.0f);

}


void SprayParticleSystem::init(IStorm3D* s3d, IStorm3D_Scene* scene) {
	
	GenParticleSystem::init(s3d, scene);


}

ParticleSystem* SprayParticleSystem::launch() {
	
	SprayParticleSystem* ps = new SprayParticleSystem();
	baseCopy(ps);

	return ps;

}

const char* SprayParticleSystem::getClassName() {
	return "spray";
}

const char* SprayParticleSystem::getSuperClassName() {
	return "gen_system";
}

void SprayParticleSystem::setParticlePosition(Vector& v) {
	v = Vector(0.0f, 0.0f, 0.0f);
}

void SprayParticleSystem::setParticleVelocity(Vector& v, float speed) {
	
	float spread1;
	float spread2;

	m_pb->getValue(PB_SPREAD1, spread1);
	m_pb->getValue(PB_SPREAD2, spread2);
	
	Vector dir(0.0f, 1.0f, 0.0f);
	dir.Normalize();
	
	Vector up(0.0f, 0.0f, 1.0f);
	Vector u = dir.GetCrossWith(up);
	if(u.GetDotWith(u)<0.0001f) {
		up = Vector(0.0f, 1.0f, 0.0f);	
	}
	
	Vector left;
	left = dir.GetCrossWith(up);
	left.Normalize();
	up = left.GetCrossWith(dir);
	up.Normalize();
	
	int seed1 = rand() % RAND_MAX;
	int seed2 = rand() % RAND_MAX;
	
	float rnd1 = -1.0f + (float)seed1 / (float)RAND_MAX * 2.0f;
	float rnd2 = -1.0f + (float)seed2 / (float)RAND_MAX * 2.0f;
	
	float a1 = rnd1 * spread1;
	float a2 = rnd2 * spread2;
	
	QUAT q1;
	QUAT q2;
	
	q1.MakeFromAxisRotation(left, a1);
	q2.MakeFromAxisRotation(up, a2);
	
	q1.RotateVector(dir);
	q2.RotateVector(dir);
	
	v = dir * speed;
}

class SprayParticleSystemClassDesc : public ParticleSystemClassDesc {
public:
	void* create() { return new SprayParticleSystem(); }
	const char* getClassName() { return "spray"; }
};
 
static SprayParticleSystemClassDesc theSprayParticleSystemClassDesc;
						 
ParticleSystemClassDesc* getSprayParticleSystemClassDesc() {
	return &theSprayParticleSystemClassDesc;
}

} // particle

} // frozenbyte