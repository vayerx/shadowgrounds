// Copyright 2002-2004 Frozenbyte Ltd.

#ifndef INCLUDED_VIEWER_APPLICATION_H
#define INCLUDED_VIEWER_APPLICATION_H

#ifndef INCLUDED_BOOST_SCOPED_PTR_HPP
#define INCLUDED_BOOST_SCOPED_PTR_HPP
#include <boost/scoped_ptr.hpp>
#endif

#include <string>

namespace frozenbyte {
namespace viewer {

struct ApplicationData;

class Application
{
	boost::scoped_ptr<ApplicationData> data;

public:
	Application();
	~Application();

	void run(const std::string &fileName);
};

} // end of namespace viewer
} // end of namespace frozenbyte

#endif
