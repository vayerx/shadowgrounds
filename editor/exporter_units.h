// Copyright 2002-2004 Frozenbyte Ltd.

#ifndef INCLUDED_EDITOR_EXPORTER_UNITS_H
#define INCLUDED_EDITOR_EXPORTER_UNITS_H

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
struct ExporterUnitsData;
struct UnitProperties;
struct StringProperties;

class ExporterUnits
{
	boost::scoped_ptr<ExporterUnitsData> data;

public:
	ExporterUnits();
	~ExporterUnits();

	void addUnit(const std::vector<std::string> &spawnText, const VC2 &position, float yRotation, float height, const std::string &scriptName, int side, const UnitProperties &properties, const StringProperties &stringProperties);
	void setSpawn(const VC2I &position);
	
	void save(const ExportOptions &options) const;
};

} // end of namespace editor
} // end of namespace frozenbyte

#endif
