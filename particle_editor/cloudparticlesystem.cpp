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
#include "cloudparticlesystem.h"


namespace frozenbyte
{
namespace particle
{


static ParamDesc theCloudParticleSystemParamDesc[] = {
	ParamDesc(PB_PCLOUD_RANDOM_DIRECTION, "random_direction", PARAM_INT, false),
	ParamDesc(PB_PCLOUD_DIRECTION, "direction", PARAM_VECTOR, false),
	ParamDesc(PB_PCLOUD_SHAPE, "shape", PARAM_STRING, false),
	ParamDesc(PB_PCLOUD_SPHERE_INNER_RADIUS, "sphere_inner_radius", PARAM_FLOAT, false),
	ParamDesc(PB_PCLOUD_SPHERE_OUTER_RADIUS, "sphere_outer_radius", PARAM_FLOAT, false),
	ParamDesc(PB_PCLOUD_BOX_MIN, "box_min", PARAM_VECTOR, false),
	ParamDesc(PB_PCLOUD_BOX_MAX, "box_max", PARAM_VECTOR, false),
	ParamDesc(PB_PCLOUD_CYLINDER_HEIGHT, "cylinder_height", PARAM_FLOAT, false),
	ParamDesc(PB_PCLOUD_CYLINDER_RADIUS, "cylinder_radius", PARAM_FLOAT, false),
};


class CloudParticleSystem : public GenParticleSystem {

	class Shape {
	public:
		virtual ~Shape() {}
		virtual Shape* create()=0;
		virtual void prepare(ParamBlock* pb)=0;
		virtual void gen(Vector& v)=0;
	};
	class Sphere : public Shape {
	public:
		float innerRadius;
		float outerRadius;
		Shape* create() { return new Sphere(); }
		void prepare(ParamBlock* pb) {
			pb->getValue(PB_PCLOUD_SPHERE_INNER_RADIUS, innerRadius);
			pb->getValue(PB_PCLOUD_SPHERE_OUTER_RADIUS, outerRadius);
		}
		void gen(Vector& v) {
			v.x = (float)(-RAND_MAX) + 2.0f * (float)(rand() % RAND_MAX);
			v.y = (float)(-RAND_MAX) + 2.0f * (float)(rand() % RAND_MAX);
			v.z = (float)(-RAND_MAX) + 2.0f * (float)(rand() % RAND_MAX);
			v.Normalize();
			v *= (innerRadius + outerRadius * (float)(rand() % RAND_MAX) / (float)RAND_MAX);
		}
	};
	class Box : public Shape {
	public:
		Vector min;
		Vector max;
		Shape* create() { return new Box(); }
		void prepare(ParamBlock* pb) {
			pb->getValue(PB_PCLOUD_BOX_MIN, min);
			pb->getValue(PB_PCLOUD_BOX_MAX, max);
		}
		virtual void gen(Vector& v) {
			v.x = min.x + (max.x - min.x) * (float)(rand() % RAND_MAX) / (float)RAND_MAX;
			v.y = min.y + (max.y - min.y) * (float)(rand() % RAND_MAX) / (float)RAND_MAX;
			v.z = min.z + (max.z - min.z) * (float)(rand() % RAND_MAX) / (float)RAND_MAX;
		}
	};
	class Cylinder : public Shape {
	public:
		float height;
		float radius;
		Shape* create() { return new Cylinder(); }
		void prepare(ParamBlock* pb) {
			pb->getValue(PB_PCLOUD_CYLINDER_HEIGHT, height);
			pb->getValue(PB_PCLOUD_CYLINDER_RADIUS, radius);
		}
		void gen(Vector& v) {
			float r;
			do {
				v.x = -radius + 2.0f * radius * (float)(rand() % RAND_MAX) / (float)RAND_MAX;
				v.z = -radius + 2.0f * radius * (float)(rand() % RAND_MAX) / (float)RAND_MAX;
				r = v.x * v.x + v.z + v.z;
			} while(r > radius);
			v.y = -height * 0.5f + height * (float)(rand() % RAND_MAX) / (float)RAND_MAX;  
		}
	};

	static Sphere sphere;
	static Box box;
	static Cylinder cylinder;
	static std::map<std::string, Shape*> m_shapes;
		
	Shape* m_shape;
	int m_randomDirection;
	Vector m_direction;

public:
	
	const char* getClassName() {
		return "cloud";
	}

	const char* getSuperClassName() {
		return "gen_system";
	}

	ParticleSystem* launch() {
		CloudParticleSystem* ps = new CloudParticleSystem;
		baseCopy(ps);
		return ps;
	}
	
	void create() {
		
		GenParticleSystem::create();
		m_pb->addParams(theCloudParticleSystemParamDesc, 9);
		m_pb->setValue(PB_PCLOUD_SHAPE, "sphere");
		m_pb->setValue(PB_PCLOUD_SPHERE_INNER_RADIUS, 0.0f);
		m_pb->setValue(PB_PCLOUD_SPHERE_OUTER_RADIUS, 1.0f);
		m_pb->setValue(PB_PCLOUD_RANDOM_DIRECTION, 1);
		m_pb->setValue(PB_PCLOUD_DIRECTION, Vector(0.0f, 1.0f, 0.0f));
	
	}

	void init(IStorm3D* s3d, IStorm3D_Scene* scene) {

		GenParticleSystem::init(s3d, scene);

		if(m_shapes.empty()) {
			m_shapes["sphere"] = &sphere;
			m_shapes["box"] = &box;
			m_shapes["cylinder"] = &cylinder;
		}
	
	}
		
	void prepareForLaunch(IStorm3D* s3d, IStorm3D_Scene* scene) {

		GenParticleSystem::prepareForLaunch(s3d, scene);

		m_pb->getValue(PB_PCLOUD_RANDOM_DIRECTION, m_randomDirection);
		m_pb->getValue(PB_PCLOUD_DIRECTION, m_direction);
		
		m_shape = NULL;
		std::string shape;
		m_pb->getValue(PB_PCLOUD_SHAPE, shape);
		m_shape = m_shapes[shape]->create();
		m_shape->prepare(m_pb.get());

	}

	void setParticleVelocity(Vector& v, float speed) {
		if(m_randomDirection) {
			float rnd1 = -1.0f + 2.0f * (float)(rand() % RAND_MAX) / (float)RAND_MAX;
			float rnd2 = -1.0f + 2.0f * (float)(rand() % RAND_MAX) / (float)RAND_MAX;
			float rnd3 = -1.0f + 2.0f * (float)(rand() % RAND_MAX) / (float)RAND_MAX;			
			Vector d(rnd1, rnd2, rnd3);
			d.Normalize();
			v = d * speed;
		} else {
			v = m_direction * speed;
		}
	}

	void setParticlePosition(Vector& v) {
		if(m_shape)	
			m_shape->gen(v);
	}


};

CloudParticleSystem::Sphere CloudParticleSystem::sphere;
CloudParticleSystem::Box CloudParticleSystem::box;
CloudParticleSystem::Cylinder CloudParticleSystem::cylinder;
std::map<std::string, CloudParticleSystem::Shape*> CloudParticleSystem::m_shapes;


class CloudParticleSystemClassDesc : public ParticleSystemClassDesc {
public:
	void* create() { return new CloudParticleSystem(); }
	const char* getClassName() { return "cloud"; }
};

static CloudParticleSystemClassDesc theCloudParticleSystemClassDesc;

ParticleSystemClassDesc* getCloudParticleSystemClassDesc() {
	return &theCloudParticleSystemClassDesc;
}

} // frozenbyte
} // particle