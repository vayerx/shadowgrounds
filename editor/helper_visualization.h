// Copyright 2002-2004 Frozenbyte Ltd.

#ifndef INCLUDED_EDITOR_HELPER_VISUALIZATION_H
#define INCLUDED_EDITOR_HELPER_VISUALIZATION_H

#ifndef INCLUDED_BOOST_SCOPED_PTR_HPP
#define INCLUDED_BOOST_SCOPED_PTR_HPP
#include <boost/scoped_ptr.hpp>
#endif
#ifndef INCLUDED_DATATYPEDEF_H
#define INCLUDED_DATATYPEDEF_H
#include <datatypedef.h>
#endif

namespace frozenbyte {
namespace editor {

class UnitHelpers;
struct Storm;
struct HelperVisualizationData;

class HelperVisualization
{
	boost::scoped_ptr<HelperVisualizationData> data;

public:
	HelperVisualization(Storm &storm);
	~HelperVisualization();

	void clear();
	void visualize(const UnitHelpers &helpers, int activeIndex, const VC2 &unitPosition);
};

} // end of namespace editor
} // end of namespace frozenbyte

#endif
