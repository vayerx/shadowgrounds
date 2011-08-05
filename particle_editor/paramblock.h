#ifndef PARTICLE_PARAM_BLOCK_H
#define PARTICLE_PARAM_BLOCK_H

namespace frozenbyte
{
namespace particle
{


struct ParamDesc {
	ParamDesc(int i_, const std::string& name_, int type_, bool animatable);
	int i;
	std::string name;
	int type;
	bool bounds;
	bool animatable;
};

enum PARAM_TYPE
{
	PARAM_INT,
		PARAM_FLOAT,
		PARAM_VECTOR,
		PARAM_STRING
};

class ParamBlock {
	struct Param {
		std::string name;
		int type;
		int offset;
		bool animatable;
		boost::shared_ptr<Track> c;
	};
	std::vector<int> m_ints;
	std::vector<float> m_floats;
	std::vector<Vector> m_vectors;
	std::vector<std::string> m_strings;
	std::vector<Param*> m_params;
public:
		
	void addParams(ParamDesc* desc, int nParams);
	
	void setValue(int i, const std::string& str, float t=0);
	void setValue(int i, int value, float t=0);
	void setValue(int i, float value, float t=0);
	void setValue(int i, const Vector& value, float t=0);
//	void setTrack(int i, Track* c);
	
	void getValue(int i, std::string& str, float t=0);
	void getValue(int i, int& value, float t=0);
	void getValue(int i, float& value, float t=0);
	void getValue(int i, Vector& value, float t=0);
	Track* getTrack(int i);

	int getNumParams();
	int getParamType(int i);
	const std::string& getParamName(int i);

};


} // particle

} // frozenbyte

#endif