#ifndef IPARTICLE_EMITTER_H
#define IPARTICLE_EMITTER_H

class IParticleEmitter {
public:
	virtual ~IParticleEmitter() {}
	virtual IParticleEmitter* clone()=0;
	
	virtual const std::string& getName()=0;

	virtual void setName(const std::string& name)=0;
	
	virtual bool tick(IStorm3D_Scene* scene, const Matrix& tm, const Vector& vel, 
		int timeDif)=0;
	
	virtual void render(IStorm3D_Scene* scene)=0;
};


#endif