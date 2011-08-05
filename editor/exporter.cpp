// Copyright 2002-2004 Frozenbyte Ltd.

#pragma warning(disable:4103)
#pragma warning(disable:4786)

#include "exporter.h"
#include "exporter_scene.h"
#include "exporter_objects.h"
#include "exporter_units.h"
#include "exporter_scripts.h"
#include "exporter_lights.h"
#include "export_options.h"

#include <stdio.h>

namespace frozenbyte {
namespace editor {

struct ExporterData
{
	ExporterScene scene;
	ExporterObjects objects;
	ExporterUnits units;
	ExporterScripts scripts;
	ExporterLights lights;
};

Exporter::Exporter()
{
	boost::scoped_ptr<ExporterData> tempData(new ExporterData());
	data.swap(tempData);
}

Exporter::~Exporter()
{
}

ExporterScene &Exporter::getScene()
{
	return data->scene;
}

ExporterObjects &Exporter::getObjects()
{
	return data->objects;
}

ExporterUnits &Exporter::getUnits()
{
	return data->units;
}

ExporterScripts &Exporter::getScripts()
{
	return data->scripts;
}

ExporterLights &Exporter::getLights()
{
	return data->lights;
}

void Exporter::save(const ExportOptions &options) const
{
	data->scene.save(options);
	data->objects.save(options);
	data->scripts.save(options);
	data->units.save(options);
	data->lights.save(options);

	// run scriptdev to check the exported scripts for errors...
  const char *missionname = options.fileName.c_str();
  const char *missionid = options.id.c_str();
	char missionfile[256];
	assert(strlen(missionname) < 200);
	assert(strlen(missionid) < 50);
	strcpy(missionfile, missionname);
	strcat(missionfile, "\\");
	strcat(missionfile, missionid);
	strcat(missionfile, ".dhm");

	char scriptdevCmd[512];
	strcpy(scriptdevCmd, "scriptdev.exe -stdout ");
	strcat(scriptdevCmd, missionfile);
#ifdef LEGACY_FILES
	strcat(scriptdevCmd, " > export_log.txt");
#else
	strcat(scriptdevCmd, " > logs\\export_log.txt");
#endif

	system(scriptdevCmd);

	// view export log if it contains something...
#ifdef LEGACY_FILES
	FILE *f = fopen("export_log.txt", "rb");
#else
	FILE *f = fopen("logs/export_log.txt", "rb");
#endif
	if (f != NULL)
	{
		fseek(f, 0, SEEK_END);
		int flen = ftell(f);
		if (flen > 0)
		{
#ifdef LEGACY_FILES
			system("exportlogview.bat export_log.txt");
#else
			system("tools\\exportlogview.bat logs\\export_log.txt");
#endif
		}
		fclose(f);
	}
}

} // end of namespace editor
} // end of namespace frozenbyte
