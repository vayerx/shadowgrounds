// Copyright 2002-2004 Frozenbyte Ltd.

#ifndef INCLUDED_EDITOR_EXPORTER_H
#define INCLUDED_EDITOR_EXPORTER_H

#ifndef INCLUDED_BOOST_SCOPED_PTR_HPP
#define INCLUDED_BOOST_SCOPED_PTR_HPP
#include <boost/scoped_ptr.hpp>
#endif
#ifndef INCLUDED_STRING
#define INCLUDED_STRING
#include <string>
#endif

namespace frozenbyte {
namespace editor {

class ExporterScene;
class ExporterObjects;
class ExporterUnits;
class ExporterScripts;
class ExporterLights;
struct ExportOptions;
struct ExporterData;

class Exporter
{
	boost::scoped_ptr<ExporterData> data;

public:
	Exporter();
	~Exporter();

	ExporterScene &getScene();
	ExporterObjects &getObjects();
	ExporterUnits &getUnits();
	ExporterScripts &getScripts();
	ExporterLights &getLights();

	void save(const ExportOptions &options) const;
};

} // end of namespace editor
} // end of namespace frozenbyte

#endif
