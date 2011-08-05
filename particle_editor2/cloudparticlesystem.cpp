#include <boost/lexical_cast.hpp>

#include "precompiled.h"

// Copyright 2002-2004 Frozenbyte Ltd.

#ifdef _MSC_VER
#pragma warning( disable : 4800 )
#endif

#include <boost/shared_ptr.hpp>
#include <boost/scoped_ptr.hpp>
//#include "particlerandom.h"
#include <vector>
//#include <string>
#include <map>
#include <list>
//#include <fstream>
#include <Storm3D_UI.h>
#include "../editor/string_conversions.h"
#include "../editor/parser.h"
#include "track.h"
//#include "paramblock.h"
#include "parseutil.h"
#include "particlesystem.h"
#include "particleeffect.h"
#include "cloudparticlesystem.h"


namespace frozenbyte
{
namespace particle
{

namespace {
	int CPSid = 0;
}

using namespace frozenbyte::editor;

class CloudParticleSystemShape {
public:
	virtual ~CloudParticleSystemShape() {}
	virtual void genPos(Vector& v, const CloudParticleSystemEditables& eds)=0;
};

class CloudParticleSystemSphere : public CloudParticleSystemShape {
public:
	
	void genPos(Vector& v, const CloudParticleSystemEditables& eds) {

		const float& innerRadius = eds.sphereInnerRadius;
		const float& outerRadius = eds.sphereOuterRadius;
		
		v.x = (float)(-RAND_MAX) + 2.0f * (float)(rand() % RAND_MAX);
		v.y = (float)(-RAND_MAX) + 2.0f * (float)(rand() % RAND_MAX);
		v.z = (float)(-RAND_MAX) + 2.0f * (float)(rand() % RAND_MAX);
		v.Normalize();
		v *= (innerRadius + outerRadius * (float)(rand() % RAND_MAX) / (float)RAND_MAX);

	}
};
		
class CloudParticleSystemBox  : public CloudParticleSystemShape {
public:
	
	void genPos(Vector& v, const CloudParticleSystemEditables& eds) {
		
		const Vector& min = eds.boxMin;
		const Vector& max = eds.boxMax;
		
		v.x = min.x + (max.x - min.x) * (float)(rand() % RAND_MAX) / (float)RAND_MAX;
		v.y = min.y + (max.y - min.y) * (float)(rand() % RAND_MAX) / (float)RAND_MAX;
		v.z = min.z + (max.z - min.z) * (float)(rand() % RAND_MAX) / (float)RAND_MAX;

	}
};

class CloudParticleSystemCylinder  : public CloudParticleSystemShape  {
public:
	
	void genPos(Vector& v, const CloudParticleSystemEditables& eds) {
		
		const float& radius = eds.cylinderRadius;
		const float& height = eds.cylinderHeight;

		float r;
		do {
			v.x = -radius + 2.0f * radius * (float)(rand() % RAND_MAX) / (float)RAND_MAX;
			v.z = -radius + 2.0f * radius * (float)(rand() % RAND_MAX) / (float)RAND_MAX;
			r = v.x * v.x + v.z + v.z;
		} while(r > radius);
		v.y = -height * 0.5f + height * (float)(rand() % RAND_MAX) / (float)RAND_MAX;  

	}
};


CloudParticleSystem::CloudParticleSystem() {
}

boost::shared_ptr<IParticleSystem> CloudParticleSystem::createNew() {
	CloudParticleSystem* ps = new CloudParticleSystem();
	boost::shared_ptr<IParticleSystem> ptr(ps);
	boost::shared_ptr<CloudParticleSystemEditables> eds(new CloudParticleSystemEditables);
	ps->m_eds.swap(eds);
	return ptr;
}

boost::shared_ptr<IParticleSystem> CloudParticleSystem::clone() {
	CloudParticleSystem* ps = new CloudParticleSystem();
	copyTo(*ps);
	ps->m_eds = m_eds;
	ps->m_shape = m_shape;
	boost::shared_ptr<IParticleSystem> res(ps);
	return res;
}


CloudParticleSystemEditables& CloudParticleSystem::getEditables() {
	return *m_eds;
}
	
const CloudParticleSystemEditables& CloudParticleSystem::getEditables() const {
	return *m_eds;
}


void CloudParticleSystem::init(IStorm3D* s3d, IStorm3D_Scene* scene) {
	GenParticleSystem::defaultInit(s3d, scene, *m_eds);
}

void CloudParticleSystem::prepareForLaunch(IStorm3D* s3d, IStorm3D_Scene* scene) {
	GenParticleSystem::defaultPrepareForLaunch(s3d, scene, *m_eds);
}

void CloudParticleSystem::tick(IStorm3D_Scene* scene) {
	GenParticleSystem::defaultTick(scene, *m_eds);
}

void CloudParticleSystem::render(IStorm3D_Scene* scene) {
	GenParticleSystem::defaultRender(scene, *m_eds);
}


void CloudParticleSystem::parseFrom(const ParserGroup& pg, const util::SoundMaterialParser &materialParser) {
		
	defaultParseFrom(pg, *m_eds);
	m_eds->shape = pg.getValue("shape", "");
	m_eds->randomDirection = static_cast<bool>(convertFromString<int>(pg.getValue("random_direction", ""), 0));
	if(m_eds->shape == "sphere") {		
		boost::shared_ptr<CloudParticleSystemShape> sphere(new CloudParticleSystemSphere);
		m_shape.swap(sphere);
		m_eds->sphereInnerRadius = convertFromString<float>(pg.getValue("sphere_inner_radius", ""), 0);
		m_eds->sphereOuterRadius = convertFromString<float>(pg.getValue("sphere_outer_radius", ""), 0);
	}
	else if(m_eds->shape == "box") {
		boost::shared_ptr<CloudParticleSystemShape> box(new CloudParticleSystemBox);
		m_shape.swap(box);
		m_eds->boxMin = convertVectorFromString(pg.getValue("box_min", "0,0,0"));
		m_eds->boxMax = convertVectorFromString(pg.getValue("box_max", "0,0,0"));
	}
	else if(m_eds->shape == "cylinder") {
		boost::shared_ptr<CloudParticleSystemShape> cyl(new CloudParticleSystemCylinder);
		m_shape.swap(cyl);
		m_eds->cylinderHeight = convertFromString<float>(pg.getValue("cylinder_height", ""), 0);
		m_eds->cylinderRadius = convertFromString<float>(pg.getValue("cylinder_radius", ""), 0);	
	}
	else {
		assert(!"unkown or undefined shape type");
	}
	

}

void CloudParticleSystem::setParticlePosition(Vector& v) {

	if(m_shape.get()!=NULL)	
		m_shape->genPos(v, *m_eds);
	
}

void CloudParticleSystem::setParticleVelocity(Vector& v, const Vector& direction, float speed, const GenParticleSystemEditables& eds) {
	
	if(m_eds->randomDirection) {
		float rnd1 = -1.0f + 2.0f * ((float)rand() / (float)RAND_MAX);
		float rnd2 = -1.0f + 2.0f * ((float)rand() / (float)RAND_MAX);
		float rnd3 = -1.0f + 2.0f * ((float)rand() / (float)RAND_MAX);
		Vector d(rnd1, rnd2, rnd3);
		d.Normalize();
		v = d * speed;
	} else {
		v = direction * speed; // just use plain direction
	}

}

void *CloudParticleSystem::getId() const
{
	return &CPSid;
}

void *CloudParticleSystem::getType()
{
	return &CPSid;
}

} // frozenbyte
} // particle
