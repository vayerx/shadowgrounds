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
#include "pointarrayparticlesystem.h"


namespace frozenbyte
{
namespace particle
{


static ParamDesc thePointArrayParticleSystemParamDesc[] = {
	ParamDesc(PB_POINT_ARRAY_MODEL_FILE, "model", PARAM_STRING, false),
	ParamDesc(PB_POINT_ARRAY_FIRST_VERTEX, "first_vertex", PARAM_INT, false),
	ParamDesc(PB_POINT_ARRAY_LAST_VERTEX, "last_vertex", PARAM_INT, false)
};

struct ParticleMesh {
	std::vector<Vector> verts;
	std::vector<Vector> normals;
};

class PointArrayParticleSystem : public GenParticleSystem {
	boost::shared_ptr<ParticleMesh> m_mesh;
	int m_index;
	int m_firstVertex;
	int m_lastVertex;
public:
	
	PointArrayParticleSystem() {
	}
	
	const char* getClassName() {
		return "point_array";
	}
	
	const char* getSuperClassName() {
		return "gen_system";
	}
	
	ParticleSystem* launch() {
		PointArrayParticleSystem* ps = new PointArrayParticleSystem();
		baseCopy(ps);
		ps->m_mesh = m_mesh;
		return ps;
	}
	
	void create() {

		GenParticleSystem::create();

		m_pb->addParams(thePointArrayParticleSystemParamDesc, 3);
		m_pb->setValue(PB_POINT_ARRAY_FIRST_VERTEX, 0);
		m_pb->setValue(PB_POINT_ARRAY_LAST_VERTEX, 0);

	}
	
	void init(IStorm3D* s3d, IStorm3D_Scene* scene) {
				
		GenParticleSystem::init(s3d, scene);
		
		std::string fileName;
		m_pb->getValue(PB_POINT_ARRAY_MODEL_FILE, fileName);
		if(fileName.empty())
			return;
		
		IStorm3D_Model* model = s3d->CreateNewModel();
		if(model->LoadS3D(fileName.c_str())) 
		{
			Iterator<IStorm3D_Model_Object*>* obj = model->ITObject->Begin();
			IStorm3D_Mesh* mesh = obj->GetCurrent()->GetMesh();
			if(mesh) {
				boost::shared_ptr<ParticleMesh> pm(new ParticleMesh());
				pm->verts.resize(mesh->GetVertexCount());
				pm->normals.resize(mesh->GetVertexCount());
				Storm3D_Vertex* v = mesh->GetVertexBuffer();
				for(int i = 0; i < mesh->GetVertexCount(); i++) {
					pm->verts[i] = v[i].position;
					pm->normals[i] = v[i].normal;
				}
				m_mesh.swap(pm);
			}
			delete obj;
		}
		delete model;

	}

	void prepareForLaunch(IStorm3D* s3d, IStorm3D_Scene* scene) {
		
		GenParticleSystem::prepareForLaunch(s3d, scene);

		m_pb->getValue(PB_POINT_ARRAY_FIRST_VERTEX, m_firstVertex);
		m_pb->getValue(PB_POINT_ARRAY_LAST_VERTEX, m_lastVertex);

	}
	
	void setParticlePosition(Vector& v) {
		if(m_mesh.get()==NULL)
			return;
		
		m_index = m_firstVertex + rand() % (m_lastVertex - m_firstVertex);
		if(m_index >= m_mesh->verts.size())
			m_index = 0;
		
		v = m_mesh->verts[m_index];
	}
		
	void setParticleVelocity(Vector& v, float speed) {
		if(m_mesh.get()==NULL)
			return;
		
		v = m_mesh->normals[m_index] * speed;	
	}

};

class PointArrayParticleSystemClassDesc : public ParticleSystemClassDesc {
public:
	void* create() { return new PointArrayParticleSystem(); }
	const char* getClassName() { return "point_array"; }
};
 
static PointArrayParticleSystemClassDesc thePointArrayParticleSystemClassDesc;

ParticleSystemClassDesc* getPointArrayParticleSystemClassDesc() {
	return &thePointArrayParticleSystemClassDesc;
}

} // particle

} // frozenbyte