#ifndef PARTICLE_H
#define PARTICLE_H


class Particle {
public:

	Vector  position;
	Vector	oldPos;
	Vector  velocity;
	Vector	origin;
	float	life;
	float	t;
	float	size;
	Vector	color;
	float	alpha;
	float	angle;
	float	rotSpeed;
	float	frame;
	float	animSpeed;
	float	bounce;
	int		collisionType;
	Particle* next;
	
};




/*
class ParticleSystemClassDesc {
public:
	virtual int getID();
	ParticleSystem* createNew();
};



class ParticleSystem {
protected:
	LinearAnimationTrack animTrack;
	LinearAnimationTrack emitTrack;
	float speed, speedVar;
	float spin, spinVar;
	bool dieAftedEmission;

	struct TextureInfo {
		std::string fileName;
		int nFrames;
		int rows;
		int cols;
		float fps;
		int animType;
		int alphaType;
	};
	TextureInfo& texInfo;
	Particle* particles;
public:

	class ParticleAnimationTrack {
	public:
		struct Key {
			float t;
			float r,g,b,a,s;
		};
	};

	LinearAnimationTrack& getParticleAnimation();
	LinearAnimationTrack& getEmissionTrack();

	void setTextureAnimation(TextureAnimation& anim);

	void setTexture(IStorm3D* s3d, const std::string& fileName) {
		
	}
	
	void tick(IStorm3D_Scene* scene) {
		
	}

	void render(IStorm3D_Scene* scene) {

	}

};

class SuperSpray : public ParticleSystem {
public:
	
	virtual void setParticlePosition();
	virtual void setParticleVelocity();

};

class ParticleCloud : public ParticleSystem {
public:
	
};


class ParticleSystemContainer {
public:
	void addParticleSystem(ParticleSystem* ps);
	void tick();
	void render());
};
*/


#endif
