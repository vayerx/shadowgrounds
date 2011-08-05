// Copyright 2002-2004 Frozenbyte Ltd.

#ifndef CLOUD_PARTICLE_SYSTEM_H
#define CLOUD_PARTICLE_SYSTEM_H

namespace frozenbyte
{
namespace particle
{

struct CloudParticleSystemEditables : public GenParticleSystemEditables 
{
	bool randomDirection;
	std::string shape;
	float sphereInnerRadius;
	float sphereOuterRadius;
	Vector boxMin;
	Vector boxMax;
	float cylinderHeight;
	float cylinderRadius;

	CloudParticleSystemEditables()
	:	randomDirection(false),
		sphereInnerRadius(0),
		sphereOuterRadius(0),
		cylinderHeight(0),
		cylinderRadius(0)
	{
	}
};

class CloudParticleSystemShape;

class CloudParticleSystem : public GenParticleSystem 
{
	boost::shared_ptr<CloudParticleSystemShape> m_shape;		
	boost::shared_ptr<CloudParticleSystemEditables> m_eds;	

	CloudParticleSystem();
public:
	
	static boost::shared_ptr<IParticleSystem> createNew();
	boost::shared_ptr<IParticleSystem> clone();
	
	void setLighting(const COL &ambient, const VC3 lightPos, const COL &lightCol, float lightRange) {}
	void setCollision(boost::shared_ptr<IParticleCollision> &collision) {}
	void setEmitterRotation(const QUAT &rotation) {}
	void setParticlePosition(Vector& pos);
	void setParticleVelocity(Vector& vel, const Vector& dir, float speed, const GenParticleSystemEditables& eds);
	void *getId() const;
	static void *getType();

	void init(IStorm3D* s3d, IStorm3D_Scene* scene);
	void prepareForLaunch(IStorm3D* s3d, IStorm3D_Scene* scene);
	void tick(IStorm3D_Scene* scene);
	void render(IStorm3D_Scene* scene);
	void parseFrom(const editor::ParserGroup& pg, const util::SoundMaterialParser &materialParser);
	
	CloudParticleSystemEditables& getEditables();
	const CloudParticleSystemEditables& getEditables() const;

};

} // particle
} // frozenbyte

#endif
