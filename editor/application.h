// Copyright 2002-2004 Frozenbyte Ltd.

#ifndef INCLUDED_EDITOR_APPLICATION_H
#define INCLUDED_EDITOR_APPLICATION_H

#ifndef INCLUDED_BOOST_SCOPED_PTR_HPP
#define INCLUDED_BOOST_SCOPED_PTR_HPP
#include <boost/scoped_ptr.hpp>
#endif

namespace frozenbyte {
namespace editor {

struct ApplicationData;

class Application
{
	boost::scoped_ptr<ApplicationData> data;

public:
	Application();
	~Application();

	void run();
};

} // end of namespace editor
} // end of namespace frozenbyte

#endif
