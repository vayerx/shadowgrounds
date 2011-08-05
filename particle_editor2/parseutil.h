// Copyright 2002-2004 Frozenbyte Ltd.

#ifndef PARTICLE_PARSE_UTIL_H
#define PARTICLE_PARSE_UTIL_H


namespace frozenbyte
{
namespace particle
{

class KeyControl;

Vector convertVectorFromString(const std::string& str);
std::string convertVectorToString(const Vector& v);

void parseFloatKeyControlFrom(const editor::ParserGroup& g, KeyControl* kc);
//void parseFloatKeyControlTo(editor::ParserGroup& g, KeyControl* kc);

void parseVectorKeyControlFrom(const editor::ParserGroup& g, KeyControl* kc);
//void parseVectorKeyControlTo(editor::ParserGroup& g, KeyControl* kc);
/*
void parseParamBlockFrom(const editor::ParserGroup& g, ParamBlock& pb);
//void parseParamBlockTo(editor::ParserGroup& g, ParamBlock& pb);
*/

} // particle
} // frozenbyte


#endif
