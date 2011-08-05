
#include "precompiled.h"

#include "../editor/parser.h"

#include <ostream>
#include <istream>
#include <map>
#include <vector>
#include <cassert>
#include <boost/shared_ptr.hpp>

namespace frozenbyte {
namespace editor {
namespace {
	std::string tabString = "   ";

	// These to utils tjsp
	bool isWhiteSpace(char character)
	{
		if(character == ' ')
			return true;
		if(character == '\t')
			return true;
		if(character == '\r')
			return true;
		if(character == '\n')
			return true;

		return false;
	}

	void removeLeadingWhiteSpaces(std::string &string, int &textStart, int &textEnd)
	{
		for(int i = 0; i < (int)string.size(); ++i)
		{
			if(textStart == -1)
			if(!isWhiteSpace(string[i]))
				textStart = i;
			
			// Remove comment
			if(i < (int)string.size() - 2)
			if(string[i] == '/')
			if(string[i+1] == '/')
			{
				textEnd = i;
				break;
			}
		}
	}

	void removeTrailingWhiteSpaces(std::string &string, int &textStart, int &textEnd)
	{
		if(textEnd - 1 >= 0)
		for(int i = textEnd - 1; i >= 0; --i)
		{
			if(!isWhiteSpace(string[i]))
			{
				textEnd = i + 1;
				break;
			}
		}
	}

	void removeWhiteSpaces(std::string &string)
	{
		if(string.empty())
			return;

		int textStart = -1;
		int textEnd = string.size();

		removeLeadingWhiteSpaces(string, textStart, textEnd);
		removeTrailingWhiteSpaces(string, textStart, textEnd);

		if(textStart == -1)
			string = "";
		else
		{
			int stringStart = (textStart >= 0) ? textStart : 0;
			int stringEnd = textEnd - stringStart;

			string = string.substr(stringStart, stringEnd);
		}
	}

	bool readLine(std::istream &stream, std::string &result)
	{
		if(stream.eof() || stream.fail())
			return false;

		std::getline(stream, result);
		removeWhiteSpaces(result);

		if(result.empty())
			return readLine(stream, result);

		return true;
	}

	void splitString(const std::string &string, std::string &first, std::string &second, char splitter)
	{
		int position = string.find_first_of(splitter);
		if(position == std::string::npos)
			return;

		for(int i = position - 1; i > 0; --i)
		{
			if(isWhiteSpace(string[i]))
				continue;

			first = string.substr(0, i+1);
			break;
		}

		for(unsigned int j = position + 1; j < string.size(); ++j)
		{
			if(isWhiteSpace(string[j]))
				continue;

			second = string.substr(j, string.size());
			break;
		}

		return;
	}

	ParserGroup *parserGroupFactory();
}

struct ParserGroupData
{
	typedef std::map<std::string, std::string> ValueMap;
	ValueMap values;

	typedef std::map<std::string, boost::shared_ptr<ParserGroup> > GroupMap;
	GroupMap groups;
	
	typedef std::vector<std::string> LineList;
	LineList lines;

	std::string emptyString;

	const std::string &getValue(const std::string &name, const std::string &defaultValue) const
	{
		ValueMap::const_iterator it = values.find(name);
		if(it == values.end())
			return defaultValue;

		return (*it).second;
	}

	void parseSubGroup(std::istream &stream, const std::string &string)
	{
		std::string groupName = string;
		std::string superName;

		splitString(string, groupName, superName, ':');
		boost::shared_ptr<ParserGroup> group(parserGroupFactory());

		if(!superName.empty())
		{
			GroupMap::iterator it = groups.find(superName);
			if(it != groups.end())
				*group.get() = *(*it).second;
		}

		stream >> *group;
		groups[groupName] = group;
	}

	void parseValue(const std::string string)
	{
		std::string name;
		std::string value;

		if(string == "{")
			return;
		if(string == "}")
			return;

		splitString(string, name, value, '=');
		if(name.empty())
		{
			if(!string.empty())
				lines.push_back(string);

			return;
		}

		values[name] = value;
	}

	void parseGroup(std::istream &stream)
	{
		std::string currentLine;
		std::string nextLine;

		readLine(stream, currentLine);
		while(readLine(stream, nextLine))
		{
			if(nextLine[0] == '{')
			{
				parseSubGroup(stream, currentLine);
				//readLine(stream, currentLine);
				currentLine = "";
				continue;
			}

			parseValue(currentLine);
			if(nextLine[0] == '}')
				return;

			currentLine.swap(nextLine);		
		}
	}

	void copy(ParserGroupData &rhs)
	{
		values = rhs.values;
		lines = rhs.lines;
		groups.clear();

		for(GroupMap::iterator it = rhs.groups.begin(); it != rhs.groups.end(); ++it)
		{
			boost::shared_ptr<ParserGroup> g(parserGroupFactory());
			*g.get() = *(*it).second.get();

			groups[(*it).first] = g;
		}
	}

	void writeTabs(std::ostream &stream, int tabCount)
	{
		for(int i = 0; i < tabCount; ++i)
			stream << tabString;
	}

	void writeStream(std::ostream &stream, int tabCount)
	{
		for(ValueMap::iterator vi = values.begin(); vi != values.end(); ++vi)
		{
			writeTabs(stream, tabCount);
			stream << (*vi).first << " = " << (*vi).second << std::endl;
		}

		for(GroupMap::iterator gi = groups.begin(); gi != groups.end(); ++gi)
		{
			if((gi != groups.begin()) || (!values.empty()))
				stream << std::endl;

			writeTabs(stream, tabCount);
			stream << (*gi).first << std::endl;

			writeTabs(stream, tabCount);
			stream << "{" << std::endl;

			(*gi).second->writeStream(stream, tabCount + 1),
			
			writeTabs(stream, tabCount);
			stream << "}" << std::endl;			
		}

		if(!lines.empty() && ((!groups.empty() || !values.empty())))
			stream << std::endl;

		for(LineList::iterator li = lines.begin(); li != lines.end(); ++li)
		{
			writeTabs(stream, tabCount);

			std::string &f = (*li);
			stream << (*li) << std::endl;
		}
	}
};

ParserGroup::ParserGroup()
{
	boost::scoped_ptr<ParserGroupData> tempData(new ParserGroupData());
	data.swap(tempData);
}

ParserGroup::ParserGroup(const ParserGroup &rhs)
{
	boost::scoped_ptr<ParserGroupData> tempData(new ParserGroupData());
	tempData->copy(*rhs.data.get());

	data.swap(tempData);
}

ParserGroup::~ParserGroup()
{
}

ParserGroup &ParserGroup::operator = (const ParserGroup &rhs)
{
	data->copy(*rhs.data.get());
	return *this;
}

const std::string &ParserGroup::getValue(const std::string &name) const
{
	return getValue(name, data->emptyString);
}

const std::string &ParserGroup::getValue(const std::string &name, const std::string &defaultValue) const
{
	return data->getValue(name, defaultValue);
}

const ParserGroup &ParserGroup::getSubGroup(const std::string &name) const
{
	ParserGroupData::GroupMap::const_iterator it = data->groups.find(name);
	if(it != data->groups.end())
		return *(*it).second.get();

	static ParserGroup emptyGroup;
	return emptyGroup;
}

ParserGroup &ParserGroup::getSubGroup(const std::string &name)
{
	ParserGroupData::GroupMap::iterator it = data->groups.find(name);
	if(it == data->groups.end())
	{
		boost::shared_ptr<ParserGroup> g(new ParserGroup());
		data->groups[name] = g;
	}

	ParserGroup &p = *data->groups[name];
	return *data->groups[name];
}

int ParserGroup::getLineCount() const
{
	return data->lines.size();
}

const std::string &ParserGroup::getLine(int index) const
{
	assert((index >= 0) && (index < getLineCount()));
	return data->lines[index];
}

void ParserGroup::setValue(const std::string &name, const std::string &value)
{
	data->values[name] = value;
}

void ParserGroup::addLine(const std::string &value)
{
	data->lines.push_back(value);
}

void ParserGroup::addSubGroup(const std::string &name, ParserGroup &group)
{
	boost::shared_ptr<ParserGroup> g(new ParserGroup());
	g->data->copy(*group.data.get());

	data->groups[name] = g;
}

std::ostream &ParserGroup::writeStream(std::ostream &stream, int tabCount) const
{
	data->writeStream(stream, tabCount);
	return stream;
}

std::istream &ParserGroup::readStream(std::istream &stream)
{
	data->parseGroup(stream);
	return stream;
}

namespace {
	ParserGroup *parserGroupFactory()
	{
		return new ParserGroup();
	}
}

/******/

struct ParserData
{
	ParserGroup globals;
};

Parser::Parser()
{
	boost::scoped_ptr<ParserData> tempData(new ParserData());
	data.swap(tempData);
}

Parser::~Parser()
{
}

const ParserGroup &Parser::getGlobals() const
{
	return data->globals;
}

ParserGroup &Parser::getGlobals()
{
	return data->globals;
}

std::ostream &Parser::writeStream(std::ostream &stream) const
{
	data->globals.writeStream(stream, 0);
	return stream << std::endl;
}

std::istream &Parser::readStream(std::istream &stream)
{
	return stream >> data->globals;
}

} // end of namespace editor
} // end of namespace frozenbyte
