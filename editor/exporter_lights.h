// Copyright 2002-2004 Frozenbyte Ltd.

#ifndef INCLUDED_EDITOR_EXPORTER_LIGHTS_H
#define INCLUDED_EDITOR_EXPORTER_LIGHTS_H

#ifndef INCLUDED_BOOST_SCOPED_PTR_HPP
#define INCLUDED_BOOST_SCOPED_PTR_HPP
#include <boost/scoped_ptr.hpp>
#endif
#ifndef INCLUDED_STRING
#define INCLUDED_STRING
#include <string>
#endif
#ifndef INCLUDED_DATATYPEDEF_H
#define INCLUDED_DATATYPEDEF_H
#include <datatypedef.h>
#endif

namespace ui {
	struct SpotProperties;
} // ui

namespace frozenbyte {
namespace editor {

struct ExportOptions;
struct ExporterScriptsData;
struct UnitProperties;

class ExporterLights
{
	struct Data;
	boost::scoped_ptr<Data> data;

public:
	ExporterLights();
	~ExporterLights();

	void addSpot(const VC3 &position, float yAngle, const ui::SpotProperties &properties, bool pointLight);
	void addBuildingLight(const VC3 &position, const COL &color, float range);

	void save(const ExportOptions &options) const;
};

} // end of namespace editor
} // end of namespace frozenbyte

#endif
