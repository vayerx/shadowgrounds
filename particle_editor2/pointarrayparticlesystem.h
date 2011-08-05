// Copyright 2002-2004 Frozenbyte Ltd.

#ifndef POINT_ARRAY_PARTICLE_SYSTEM_H
#define POINT_ARRAY_PARTICLE_SYSTEM_H

namespace frozenbyte
{
namespace particle
{

class PointArrayParticleSystemEditables : public GenParticleSystemEditables {
public:
	std::string modelFile;
	bool useNormalsAsDirection;
	int firstVertex;
	int lastVertex;
	bool randomizeBetweenVertices;
	bool planePositions;
	Vector scale;
	Vector rotation;
};

class PointArrayParticleSystem : public GenParticleSystem {
	struct PointArray {
		std::vector<Vector> verts;
		std::vector<Vector> normals;
	};
	boost::shared_ptr<PointArrayParticleSystemEditables> m_eds;		
	boost::shared_ptr<PointArray> m_parray;	
	int m_index;
	//MAT rotation;
	PointArrayParticleSystem(); // use createNew();
public:
	static boost::shared_ptr<IParticleSystem> createNew();
	
	boost::shared_ptr<IParticleSystem> clone();

	//void setRotation(const MAT &tm);
	void setParticlePosition(Vector& pos);
	void setParticleVelocity(Vector& vel, const Vector& direction, float speed, const GenParticleSystemEditables& eds);
	void *getId() const;
	static void *getType();

	void init(IStorm3D* s3d, IStorm3D_Scene* scene);
	void prepareForLaunch(IStorm3D* s3d, IStorm3D_Scene* scene);
	void tick(IStorm3D_Scene* scene);
	void render(IStorm3D_Scene* scene);
	void parseFrom(const editor::ParserGroup& pg, const util::SoundMaterialParser &materialParser);

	PointArrayParticleSystemEditables& getEditables();
	const PointArrayParticleSystemEditables& getEditables() const;

	void setCollision(boost::shared_ptr<IParticleCollision> &collision) {}
	void setEmitterRotation(const QUAT &rotation) {}
};


} // particle
} // frozenbyte


#endif
