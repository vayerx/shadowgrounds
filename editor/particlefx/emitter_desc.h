#ifndef EMITTER_DESC_H
#define EMITTER_DESC_H


enum EMITTER_DESC_TYPE {
	ED_SPRAY = 0,
	ED_POINT_ARRAY = 1,
	ED_CLOUD = 2
};
/*

class EmitterClassDesc {
public:
	virtual EmitterDesc* createNew()=0;
	virtual int getTypeID();
};
*/
// describes emitter for particlesystem

class EmitterDesc {
protected:

	std::string mName;

	float fRand(float min, float max);

public:
	FloatTrack emitRateTrack;
	
	float minEmitTime;
	float maxEmitTime;

	bool dieAfterEmission;

	float velocityFactor;

	Vector position;

	EmitterDesc(std::string name="");
	virtual ~EmitterDesc() {}

//	virtual void copy(EmitterDesc* other)=0;
	
	const std::string& getName();
	void setName(const std::string& name);
	
	virtual int getType()=0;
	
	virtual void parseIn(ParserGroup& prev);
	virtual void parseOut(ParserGroup& prev);
	
	virtual void genVelocity(Vector& vel)=0;
	virtual void genPosition(Vector& pos)=0;

};


class SprayEmitterDesc : public EmitterDesc {

	Vector mWorldPosition;

public:

	float spread1;
	float spread2;

	float minSpeed;
	float maxSpeed;

	Vector position;

	SprayEmitterDesc();

	int getType();
	
	void parseIn(ParserGroup& prev);
	void parseOut(ParserGroup& prev);

	void genVelocity(Vector& vel);
	void genPosition(Vector& pos);

};

class PointArrayEmitterDesc : public EmitterDesc {
	
	std::vector<Vector> mVerts;
	std::vector<Vector> mNormals;

	int mIndex;

public:
	
	float minSpeed;
	float maxSpeed;

	int rangeStart;
	int rangeEnd;

	void setModel(IStorm3D_Mesh* obj);

	void loadModel(IStorm3D* s3d, const std::string& name);
	
	PointArrayEmitterDesc();
	
	int getType();

	void parseIn(ParserGroup& prev);
	void parseOut(ParserGroup& prev);

	void genVelocity(Vector& vel);
	void genPosition(Vector& pos);

};

class CloudEmitterDesc : public EmitterDesc {
public:

	enum SHAPE_TYPE {
		SHAPE_BOX,
		SHAPE_SPHERE,
		SHAPE_CYLINDER
	};

	int shapeType;
	bool randomDirection;
	Vector direction;
	float minSpeed;
	float maxSpeed;
	
	// box
	Vector bmin;
	Vector bmax;
	// sphere
	float sInnerRadius;
	float sOuterRadius;
	// cylinder
	float cRadius;
	float cHeight;


	CloudEmitterDesc();

	int getType();

	void parseIn(ParserGroup& prev);
	void parseOut(ParserGroup& prev);

	void genVelocity(Vector& vel);
	void genPosition(Vector& pos);

};



#endif
