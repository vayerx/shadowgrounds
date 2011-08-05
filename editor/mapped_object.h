// Copyright 2002-2004 Frozenbyte Ltd.

#ifndef INCLUDED_EDITOR_MAPPED_OBJECT_H
#define INCLUDED_EDITOR_MAPPED_OBJECT_H

#ifndef INCLUDED_BOOST_SCOPED_PTR_HPP
#define INCLUDED_BOOST_SCOPED_PTR_HPP
#include <boost/scoped_ptr.hpp>
#endif
#ifndef INCLUDED_STRING
#define INCLUDED_STRING
#include <string>
#endif

class IStorm3D_Model;

namespace frozenbyte {
namespace editor {

struct MappedObjectData;

class MappedObject
{
	boost::scoped_ptr<MappedObjectData> data;

public:
	MappedObject(const std::string &fileName, IStorm3D_Model &model);
	~MappedObject();
};

} // end of namespace editor
} // end of namespace frozenbyte

#endif
