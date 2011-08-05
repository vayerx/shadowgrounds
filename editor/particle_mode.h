#ifndef INCLUDED_EDITOR_PARTICLE_MODE_H
#define INCLUDED_EDITOR_PARTICLE_MODE_H

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
class IEditorState;
struct Storm;
struct ParticleModeData;

class ParticleMode: public IMode
{
	boost::scoped_ptr<ParticleModeData> data;

public:
	ParticleMode(Gui &gui, Storm &storm);
	~ParticleMode();

	bool handleDialogs(MSG& msg);
	
	void tick();
	void update();
	void reset();

	void export(Exporter &exporter) const;
	filesystem::OutputStream &writeStream(filesystem::OutputStream &stream) const;
	filesystem::InputStream &readStream(filesystem::InputStream &stream);
};

} // end of namespace editor
} // end of namespace frozenbyte

#endif
