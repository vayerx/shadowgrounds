// Copyright 2002-2004 Frozenbyte Ltd.

#ifndef PARTICLE_PARAM_UI_H
#define PARTICLE_PARAM_UI_H

namespace frozenbyte
{
namespace particle
{

	enum PARAM_TYPE {
		PARAM_INT,
		PARAM_FLOAT,
		PARAM_BOOL,
		PARAM_VECTOR,
		PARAM_STRING,
		PARAM_FILE,
		PARAM_ANIMATED_FLOAT,
		PARAM_ANIMATED_VECTOR,
		PARAM_SELECTION
	};

	struct ParamDesc 
	{
		ParamDesc(const std::string _name, int _id, PARAM_TYPE _type);
		ParamDesc(const std::string _name, int _id, PARAM_TYPE _type, const std::string &default_);
		ParamDesc(const std::string _name, int _id1, int _id2, int _id3, PARAM_TYPE _type=PARAM_VECTOR);
		ParamDesc(const std::string name_, int id1_, int id2_, const std::string& ext_, const std::string& path_,
			PARAM_TYPE type_ = PARAM_FILE);
		ParamDesc(const std::string name_, int id1_,
			const std::string& selections_, PARAM_TYPE type_ = PARAM_SELECTION);
		
		ParamDesc(const ParamDesc& rhs);
		ParamDesc& operator=(const ParamDesc& rhs);

		PARAM_TYPE type;
		int id1, id2, id3;
		std::string name;
		std::string ext;
		std::string path;
		std::string defaultValue;
		std::vector< std::string > selections;
	};

	struct ParamUIData;
	class ParamUI {
		boost::scoped_ptr<ParamUIData> data;
	public:
		ParamUI(editor::Dialog& parent, int resourceID, editor::ParserGroup& pg, const std::vector<ParamDesc>& pd);
		~ParamUI();
	};


} // particle
} // frozenbyte


#endif