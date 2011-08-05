#include <boost/lexical_cast.hpp>
#include <boost/shared_ptr.hpp>
#include <string>
#include <vector>
#include <storm3d_ui.h>
#include "../editor/string_conversions.h"
#include "../editor/parser.h"
#include "track.h"
#include "paramblock.h"
#include "parseutil.h"

namespace frozenbyte
{
namespace particle
{
using namespace frozenbyte::editor;

Vector convertVectorFromString(const std::string& str) {
	std::string x,y,z;
	int i = 0;
	for(int j = 0; j < 3; j++) {	
		for(i; i < str.size(); i++) {
			if(str[i] == ',') {
				i++;
				break;
			}
			if(j == 0)
				x += str[i];
			else if(j == 1)
				y += str[i];
			else
				z += str[i];
		}
	}
	Vector v;
	v.x = convertFromString<float>(x, 0);
	v.y = convertFromString<float>(y, 0);
	v.z = convertFromString<float>(z, 0);
	return v;
}

std::string convertVectorToString(const Vector& v) {
	std::string x = convertToString<float>(v.x);
	std::string y = convertToString<float>(v.y);
	std::string z = convertToString<float>(v.z);
	std::string str = x + "," + y + "," + z;
	return str;
}



void parseFloatKeyControlFrom(const ParserGroup& g, KeyControl* kc) {
	int n = convertFromString<int>(g.getValue("num_keys", "0"), 0);
	kc->setNumKeys(n);
	for(int i = 0; i < n; i++) {
		FloatKey key;
		std::string str = "key" + boost::lexical_cast<std::string>(i);
		key.time = convertFromString<float>(g.getValue((str + ".time"), "0"), 0);
		key.value = convertFromString<float>(g.getValue((str + ".value"), "0"), 0);
		kc->setKey(i, &key);
	}
}

void parseFloatKeyControlTo(ParserGroup& g, KeyControl* kc) {
	g.setValue("num_keys", convertToString<int>(kc->getNumKeys()));
	for(int i = 0; i < kc->getNumKeys(); i++) {
		FloatKey key;
		kc->getKey(i, &key);
		std::string str = "key" + boost::lexical_cast<std::string>(i);
		g.setValue((str + ".time"), convertToString<float>(key.time));
		g.setValue((str + ".value"), convertToString<float>(key.value));		
	}
}

void parseVectorKeyControlFrom(const ParserGroup& g, KeyControl* kc) {
	int n = convertFromString<int>(g.getValue("num_keys", "0"), 0);
	kc->setNumKeys(n);
	for(int i = 0; i < n; i++) {
		VectorKey key;
		std::string str = "key" + boost::lexical_cast<std::string>(i);
		key.time = convertFromString<float>(g.getValue((str + ".time"), "0"), 0);
		key.value = convertVectorFromString(g.getValue((str + ".value"), "0"));
		kc->setKey(i, &key);
	}
}


void parseVectorKeyControlTo(ParserGroup& g, KeyControl* kc) {
	g.setValue("num_keys", convertToString<int>(kc->getNumKeys()));
	for(int i = 0; i < kc->getNumKeys(); i++) {
		VectorKey key;
		kc->getKey(i, &key);
		std::string str = "key" + boost::lexical_cast<std::string>(i);
		g.setValue((str + ".time"), convertToString<float>(key.time));
		g.setValue((str + ".value"), convertVectorToString(key.value));
	}	
}

void parseParamBlockFrom(const editor::ParserGroup& g, ParamBlock& pb) {
	for(int i = 0; i < pb.getNumParams(); i++) {
		std::string paramName = pb.getParamName(i);
		switch(pb.getParamType(i)) {
		case PARAM_INT:
			{
				int def;
				pb.getValue(i, def);
				int val = convertFromString<int>(g.getValue(pb.getParamName(i), ""), def);
				pb.setValue(i, val);
			} break;
		case PARAM_FLOAT:
			{
				if(pb.getTrack(i)) {
					const ParserGroup& kg = g.getSubGroup(pb.getParamName(i)); 
					parseFloatKeyControlFrom(kg, pb.getTrack(i)->getKeyControl());	
				} else {
					float def;
					pb.getValue(i, def);
					float val = convertFromString<float>(g.getValue(pb.getParamName(i), ""), def);
					pb.setValue(i, val);
				}
			} break;
		case PARAM_VECTOR:
			{
				if(pb.getTrack(i)) {
					const ParserGroup& kg = g.getSubGroup(pb.getParamName(i)); 
					parseVectorKeyControlFrom(kg, pb.getTrack(i)->getKeyControl());	
				} else {
					Vector def;
					pb.getValue(i, def);
					Vector val = convertVectorFromString(g.getValue(pb.getParamName(i), "0,0,0"));
					pb.setValue(i, val);
				}
			} break;
		case PARAM_STRING:
			{
				std::string def;
				pb.getValue(i, def);
				std::string val = g.getValue(pb.getParamName(i), def);
				pb.setValue(i, val);
			} break;
		}
	}
}


}

}