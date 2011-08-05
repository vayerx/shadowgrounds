// Copyright 2002-2004 Frozenbyte Ltd.

#ifndef INCLUDED_VIEWER_ITEMS_H
#define INCLUDED_VIEWER_ITEMS_H

#ifndef INCLUDED_BOOST_SCOPED_PTR_HPP
#define INCLUDED_BOOST_SCOPED_PTR_HPP
#include <boost/scoped_ptr.hpp>
#endif
#ifndef INCLUDED_BOOST_SHARED_PTR_HPP
#define INCLUDED_BOOST_SHARED_PTR_HPP
#include <boost/shared_ptr.hpp>
#endif
#ifndef INCLUDED_STRING
#define INCLUDED_STRING
#include <string>
#endif

class IStorm3D_Model;

namespace frozenbyte {
namespace editor {
	class Parser;
	struct Storm;
}

namespace viewer {

struct BoneItemsData;

class BoneItems
{
	boost::scoped_ptr<BoneItemsData> data;

public:
	BoneItems();
	~BoneItems();

	void showDialog(IStorm3D_Model *model);
	void applyToModel(boost::shared_ptr<IStorm3D_Model> model, editor::Storm &storm);

	void load(const editor::Parser &parser);
	void save(editor::Parser &parser);
};

} // end of namespace viewer
} // end of namespace frozenbyte

#endif
