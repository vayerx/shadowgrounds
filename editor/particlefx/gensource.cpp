#ifndef GENSOURCE_H
#define GENSOURCE_H


// generic particle emitter

class GenParticleSource : public IParticleSource {
	enum SHAPE {
		PSS_SPHERE,
		PSS_BOX
	};
	
	SHAPE shape;
	VectorTrack mDirectionTrack;
	FloatTrack mEmitRateTrack;
	float mMinEmitTime, mMaxEmitTime;
	Vector relativePosition;
	Vector boxMin, boxMax;
	float sphereInner, sphereOuter;
	float velocityFactor;
public:
	VectorTrack& getDirectionTrack();
	FloatTrack& getEmitRateTrack();
	
	virtual void emit(ParticleGroup& group);
};

float GenParticleSource::getEmitTime() {
	
}

float GenParticleSource::getEmitRate(float t) {

}

int GenParticleSource::emit(ParticleGroup& group, float t, const Matrix& tm, const Vector& velocity) {

	Vector pos, vel;
	Vector dir = mDirectionTrack.eval(t);
	
	tm.TransformVector(dir);

	for(int i = 0; i < n; i++) {
		
		if(shape == PSS_SPHERE) {
			pos = boxMin + (boxMax - boxMin) * rnd;
			pos += tm.GetTranslation();
		} else {
			vel = dir;
			vel += velocity * velocityFactory;
		}
		
		group.addParticle(pos, vel);
	}

}


#endif