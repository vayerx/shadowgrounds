// Copyright 2002-2004 Frozenbyte Ltd.

#ifndef MODEL_PARTICLE_SYSTEM_H
#define MODEL_PARTICLE_SYSTEM_H

#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>

namespace frozenbyte {
namespace physics {
	//class ConvexMesh;
	//class ConvexActor;
} // physics

namespace particle {

class PhysicsMesh;
class PhysicsActor;

class ModelParticleSystemEditables : public GenParticleSystemEditables {
public:
	std::string modelFile;
	bool useNormalsAsDirection;
	Vector scale;
	Vector rotation;

	std::string particleFile;

	float rotateStart[2];
	float rotateSpeed[2];

	bool collision;
	float slowFactor;
	float slowFactorY;
	float slowFactorXZ;
	bool unitCollision;
	int soundMaterialIndex;

	ModelParticleSystemEditables()
	:	collision(true),
		slowFactor(0.5f),
		slowFactorY(0.5f),
		slowFactorXZ(0.5f),
		unitCollision(true),
		soundMaterialIndex(0)
	{
		rotateStart[0] = rotateStart[1] = 0.f;
		rotateSpeed[0] = rotateSpeed[1] = 0.f;
	}
};

class ModelParticleSystem : public GenParticleSystem {
	struct PointArray {
		std::vector<Vector> verts;
		std::vector<Vector> normals;
	};

	struct Mesh
	{
		VC3 position;
		QUAT rotation;
		IStorm3D_Mesh *mesh;

		Mesh()
		:	mesh(0)
		{
		}
	};

	struct Original
	{
		boost::shared_ptr<IStorm3D_Model> model;
		std::vector<Mesh> meshes;
		std::vector<boost::shared_ptr<PhysicsMesh> > shapes;

		Original()
		{
		}
	};

	boost::shared_ptr<ParticlePhysics> physics;
	boost::shared_ptr<ModelParticleSystemEditables> m_eds;
	boost::shared_ptr<PointArray> m_parray;	

	Original original;

	struct Particle
	{
		IStorm3D_Model_Object *object;
		VC3 position;
		VC3 velocity;
		bool alive;
		float alpha;
		float age;
		float life;
		bool moving;

		VC3 originalPosition;
		QUAT originalRotation;

		// x,y,z -> axis, w angle
		QUAT axis[2];
		float rotateSpeed[2];

		boost::shared_ptr<PhysicsMesh> shape;
		boost::shared_ptr<PhysicsActor> actor;

		Particle()
		:	object(0),
			alive(true),
			alpha(0),
			age(0),
			life(0),
			moving(false)
		{
			rotateSpeed[0] = 0;
			rotateSpeed[1] = 0;
		}
	};

	std::vector<Particle> particles;
	boost::scoped_ptr<IStorm3D_Model> model;

	VC3 position;
	VC3 velocity;
	float time;
	float residue;

	boost::shared_ptr<IParticleCollision> collision;
	MAT emitterTm;

	void apply(Particle &p);
	void spawn();
	void update();
	void getPosition(VC3 &position, int index);
	void getVelocity(VC3 &velocity, const VC3 &dir, float speed, int index);

	ModelParticleSystem(); // use createNew();
public:
	static boost::shared_ptr<IParticleSystem> createNew();
	boost::shared_ptr<IParticleSystem> clone();

	void setParticlePosition(Vector& pos) {}
	void setParticleVelocity(Vector& vel, const Vector& direction, float speed, const GenParticleSystemEditables& eds) {}
	void setLighting(const COL &ambient, const signed short int *lightIndices);
	void setCollision(boost::shared_ptr<IParticleCollision> &collision);
	void setPhysics(boost::shared_ptr<ParticlePhysics> &physics);
	void releasePhysicsResources();
	void setEmitterRotation(const QUAT &rotation);
	void *getId() const;
	static void *getType();

	void init(IStorm3D* s3d, IStorm3D_Scene* scene);
	void prepareForLaunch(IStorm3D* s3d, IStorm3D_Scene* scene);
	void tick(IStorm3D_Scene* scene);
	void render(IStorm3D_Scene* scene);
	void parseFrom(const editor::ParserGroup& pg, const util::SoundMaterialParser &materialParser);

	void setPosition(const Vector &position);
	void setVelocity(const Vector &velocity);

	ModelParticleSystemEditables& getEditables();
	const ModelParticleSystemEditables& getEditables() const;
};

} // particle
} // frozenbyte

#endif
