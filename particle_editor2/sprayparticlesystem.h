// Copyright 2002-2004 Frozenbyte Ltd.

#ifndef SPRAY_PARTICLE_SYSTEM_H
#define SPRAY_PARTICLE_SYSTEM_H

namespace frozenbyte
{
namespace particle
{

class SprayParticleSystemEditables : public GenParticleSystemEditables {
public:
	float spread1;
	float spread2;
};

class SprayParticleSystem : public GenParticleSystem {
	boost::shared_ptr<SprayParticleSystemEditables> m_eds;	

	boost::shared_ptr<IParticleCollision> collision;

	SprayParticleSystem();
public:
	
	static boost::shared_ptr<IParticleSystem> createNew();
	
	boost::shared_ptr<IParticleSystem> clone();

	void setParticlePosition(Vector& pos);
	void setParticleVelocity(Vector& vel, const Vector& dir, float speed, const GenParticleSystemEditables& eds);
	void *getId() const;
	static void *getType();

	void init(IStorm3D* s3d, IStorm3D_Scene* scene);
	void prepareForLaunch(IStorm3D* s3d, IStorm3D_Scene* scene);
	void tick(IStorm3D_Scene* scene);
	void render(IStorm3D_Scene* scene);
	void parseFrom(const editor::ParserGroup& pg, const util::SoundMaterialParser &materialParser);

	SprayParticleSystemEditables& getEditables();
	const SprayParticleSystemEditables& getEditables() const;

	void setCollision(boost::shared_ptr<IParticleCollision> &collision);
	void setEmitterRotation(const QUAT &rotation) {}
};
	
} // particle
} // frozenbyte


#endif
