// Copyright 2002-2004 Frozenbyte Ltd.

#ifndef PARTICLE_APPLICATION_H
#define PARTICLE_APPLICATION_H

#ifndef INCLUDED_BOOST_SCOPED_PTR_HPP
#define INCLUDED_BOOST_SCOPED_PTR_HPP
#include <boost/scoped_ptr.hpp>
#endif

#include <string>

namespace frozenbyte {
namespace particle {

struct ApplicationData;

class Application
{
	boost::scoped_ptr<ApplicationData> data;

public:
	Application();
	~Application();

	void run(std::string startupFilename);
};

} // end of namespace viewer
} // end of namespace frozenbyte

#endif
