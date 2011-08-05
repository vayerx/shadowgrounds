#ifndef PARSING_H
#define PARSING_H

bool parseIn(ParserGroup& g, const std::string& name, std::string& str);
bool parseIn(ParserGroup& g, const std::string& name, int& value);
bool parseIn(ParserGroup& g, const std::string& name, float& value);
bool parseIn(ParserGroup& g, const std::string& name, Vector& value);

bool parseOut(ParserGroup& g, const std::string& name, const std::string& str);
bool parseOut(ParserGroup& g, const std::string& name, int value);
bool parseOut(ParserGroup& g, const std::string& name, float value);
bool parseOut(ParserGroup& g, const std::string& name, const Vector& value);


#endif
