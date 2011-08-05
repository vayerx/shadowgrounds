// Copyright 2002-2004 Frozenbyte Ltd.

#ifndef INCLUDED_EDITOR_EXPORTER_SCRIPTS_H
#define INCLUDED_EDITOR_EXPORTER_SCRIPTS_H

#ifndef INCLUDED_BOOST_SCOPED_PTR_HPP
#define INCLUDED_BOOST_SCOPED_PTR_HPP
#include <boost/scoped_ptr.hpp>
#endif
#ifndef INCLUDED_STRING
#define INCLUDED_STRING
#include <string>
#endif
#ifndef INCLUDED_VECTOR
#define INCLUDED_VECTOR
#include <vector>
#endif
#ifndef INCLUDED_DATATYPEDEF_H
#define INCLUDED_DATATYPEDEF_H
#include <datatypedef.h>
#endif

namespace frozenbyte {
namespace editor {

struct ExportOptions;
struct ExporterScriptsData;
struct UnitProperties;
struct StringProperties;

class ExporterScripts
{
	boost::scoped_ptr<ExporterScriptsData> data;

public:
	ExporterScripts();
	~ExporterScripts();

	enum SubType
	{
		Main = 0,
		Spotted = 1,
		Alerted = 2,
		Hit = 3,
		Hitmiss = 4,
		Noise = 5,
		Execute = 6,
		Special = 7,
		Pointed = 8
	};

	int addScript(const std::string &name);
	void setScript(int id, SubType type, const std::vector<std::string> &script);
	void setInitialization(int id, const std::vector<std::string> &initialization);
	void setNoScript(int id, bool noScript);
	void addPath(int id, const std::string &name, const std::vector<VC2> &points);
	void setProperties(int id, const UnitProperties &properties);
	void setProperties(int id, const StringProperties &properties);

	void save(const ExportOptions &options) const;
};

} // end of namespace editor
} // end of namespace frozenbyte

#endif
