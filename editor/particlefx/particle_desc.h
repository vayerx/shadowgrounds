#ifndef PARTICLE_DESC_H
#define PARTICLE_DESC_H


// descibes properties of particles use when emitted and animated

class ParticleDesc {
public:

	class TextureInfo {
		
		friend class ParticleDesc;
		
//		std::string name;
//		std::string path;
	public:
		enum ANIM_TYPE {
			ANIM_LOOP,
			ANIM_LIFE_TIME
		};
		
		TextureInfo();
		
		std::string name;
		std::string path;

		void operator=(const TextureInfo& other);		
	
		void parseIn(ParserGroup& prev);
	
		void parseOut(ParserGroup& prev);

		int nFrames;
		int columns;
		int rows;
		int alphaType;
		int animType;
		float startFrame;
		float fps;
	};

	enum DSTYLE {
		DSTYLE_POINT,
		DSTYLE_QUAD,
		DSTYLE_LINE
	};

	enum CTYPE {
		CTYPE_NONE,
		CTYPE_DIE,
		CTYPE_BOUNCE
	};

	enum DRAG_FUNCTION_TYPE {
		DRAG_NONE,
		DRAG_LINEAR,
		DRAG_QUADRATIC,
		DRAG_CUBIC
	};
	
	TextureInfo texInfo;
	VectorTrack colorTrack;
	FloatTrack alphaTrack;
	FloatTrack sizeTrack;
	float minLife, maxLife;
	float minAngle, maxAngle;
	float minSpin, maxSpin;
	float gravityMultiplier;
	float dragFactor;
	int dragFuncByVelocity, dragFuncBySize;
	int collisionType;
	int drawStyle;
	float bounce;

	ParticleDesc();
	
	const std::string& getName();
	void setName(const std::string& name);

	void loadTexture(IStorm3D* s3d, const std::string& path, const std::string& name);
	
	IStorm3D_Material* getMaterial();

	void setAlphaType(IStorm3D_Material::ATYPE type);

	void parseIn(IStorm3D* s3d, ParserGroup& prev);

	void parseOut(ParserGroup& prev);
protected:

	IStorm3D_Material* mMaterial;
	std::string mName;

};




#endif
