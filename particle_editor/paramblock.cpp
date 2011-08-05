#include <storm3d_ui.h>
#include <boost/shared_ptr.hpp>
#include <boost/scoped_ptr.hpp>
#include <vector>
#include <string>
#include "..\editor\parser.h"
#include "track.h"
#include "paramblock.h"

namespace frozenbyte
{
namespace particle
{
using namespace frozenbyte::editor;

ParamDesc::ParamDesc(int i_, const std::string& name_, int type_, bool animatable_) :
	i(i_), name(name_), type(type_), animatable(animatable_) {
}

void ParamBlock::addParams(ParamDesc* desc, int nParams) {
	int i;
	for(i = 0; i < nParams; i++) {
		m_params.push_back(new Param);
	}
	for(i = 0; i < nParams; i++) {
		Param* param = m_params[desc[i].i];
		param->type = desc[i].type;
		param->name = desc[i].name;
		//param->c = NULL;
		param->animatable = desc[i].animatable;
		switch(desc[i].type) {
		case PARAM_INT:
			{
				m_ints.push_back(int());
				param->offset = m_ints.size()-1;
			} break;
		case PARAM_FLOAT:
			{
				m_floats.push_back(float());
				param->offset = m_floats.size()-1;
				if(param->animatable) {
					boost::shared_ptr<Track> ft(new FloatTrack());
					param->c.swap(ft);
				}			
			} break;
		case PARAM_VECTOR:
			{
				m_vectors.push_back(Vector(0.0f, 0.0f, 0.0f));
				param->offset = m_vectors.size()-1;
				if(param->animatable) { 
					boost::shared_ptr<Track> vt(new VectorTrack());
					param->c.swap(vt);
				}
			} break;		
		case PARAM_STRING:
			{
				m_strings.push_back(std::string(""));
				param->offset = m_strings.size()-1;
			} break;
		default: assert("unkown param type");
		}
	}
}

void ParamBlock::setValue(int i, int value, float t) {
	assert(i >= 0 && i < m_params.size());
	Param& param = *m_params[i];
	if(param.animatable && param.c) {
		param.c->setValue(&value, t);
	} else {
		m_ints[param.offset] = value;
	}
}

void ParamBlock::setValue(int i, float value, float t) {
	assert(i >= 0 && i < m_params.size());
	Param& param = *m_params[i];
	if(param.animatable && param.c) {
		param.c->setValue(&value, t);
	} else {
		m_floats[param.offset] = value;
	}
}

void ParamBlock::setValue(int i, const Vector& value, float t) {
	assert(i >= 0 && i < m_params.size());
	Param& param = *m_params[i];
	if(param.animatable && param.c) {
		param.c->setValue((void*)&value, t);
	} else {
		m_vectors[param.offset] = value;
	}
}

void ParamBlock::setValue(int i, const std::string& str, float t) {
	assert(i >= 0 && i < m_params.size());
	Param& param = *m_params[i];
	if(param.animatable && param.c) {
		param.c->setValue((void*)&str, t);
	} else {
		m_strings[param.offset] = str;
	}
}



//void ParamBlock::setTrack(int i, Track* c) {
//	m_params[i]->c = c;
//}


Track* ParamBlock::getTrack(int i) {
	assert(i >= 0 && i < m_params.size());
	return m_params[i]->c.get();
}

void ParamBlock::getValue(int i, int& value, float t) {
	assert(i >= 0 && i < m_params.size());
	Param& param = *m_params[i];
	if(param.animatable && param.c) {
		param.c->getValue(&value, t);
	} else {
		value = m_ints[param.offset];
	}
}

void ParamBlock::getValue(int i, float& value, float t) {
	assert(i >= 0 && i < m_params.size());
	Param& param = *m_params[i];
	if(param.animatable && param.c) {
		param.c->getValue(&value, t);
	} else {
		value = m_floats[param.offset];
	}
}

void ParamBlock::getValue(int i, Vector& value, float t) {
	assert(i >= 0 && i < m_params.size());
	Param& param = *m_params[i];
	if(param.animatable && param.c) {
		param.c->getValue(&value, t);
	} else {
		value = m_vectors[param.offset];
	}
}

void ParamBlock::getValue(int i, std::string& str, float t) {
	assert(i >= 0 && i < m_params.size());
	Param& param = *m_params[i];
	if(param.animatable && param.c) {
		param.c->getValue(&str, t);
	} else {
		str = m_strings[param.offset];
	}
}

int ParamBlock::getNumParams() {
	return m_params.size();
}

int ParamBlock::getParamType(int i) {
	assert(i >= 0 && i < m_params.size());
	return m_params[i]->type;
}

const std::string& ParamBlock::getParamName(int i) {
	assert(i >= 0 && i < m_params.size());
	return m_params[i]->name;
}


} // particle

} // frozenbyte

