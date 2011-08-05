// Copyright 2002-2004 Frozenbyte Ltd.

#ifndef INCLUDED_EDITOR_LIGHT_MODE_H
#define INCLUDED_EDITOR_LIGHT_MODE_H

#include "terrain_lightmap.h"
#include <vector>

#ifndef INCLUDED_EDITOR_IMODE_H
#include "imode.h"
#endif
#ifndef INCLUDED_BOOST_SCOPED_PTR_HPP
#define INCLUDED_BOOST_SCOPED_PTR_HPP
#include <boost/scoped_ptr.hpp>
#endif

namespace frozenbyte {
namespace editor {

class Gui;
struct Storm;
class IEditorState;

class LightMode: public IMode
{
	struct Data;
	boost::scoped_ptr<Data> data;

public:
	LightMode(Gui &gui, Storm &storm, IEditorState &editorState);
	~LightMode();

	void tick();
	void update();
	void reset();

	void hideObjects();
	void showObjects();
	void getLights(std::vector<TerrainLightMap::PointLight> &lights, bool onlyBuilding);

	void nudgeLights(const VC3 &position, const VC3 &direction, float radius);

	void doExport(Exporter &exporter) const;
	filesystem::OutputStream &writeStream(filesystem::OutputStream &stream) const;
	filesystem::InputStream &readStream(filesystem::InputStream &stream);
};

} // end of namespace editor
} // end of namespace frozenbyte

#endif
