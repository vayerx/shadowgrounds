// Copyright 2002-2004 Frozenbyte Ltd.

#ifndef INCLUDED_EDITOR_STORM_MODEL_UTILS_H
#define INCLUDED_EDITOR_STORM_MODEL_UTILS_H

#ifndef INCLUDED_BOOST_SHARED_PTR_HPP
#define INCLUDED_BOOST_SHARED_PTR_HPP
#include <boost/shared_ptr.hpp>
#endif
#ifndef INCLUDED_STRING
#define INCLUDED_STRING
#include <string>
#endif

class IStorm3D;
class IStorm3D_Model;
class IStorm3D_Helper_Camera;

namespace frozenbyte {
namespace editor {

IStorm3D_Helper_Camera *getBoneHelper(const boost::shared_ptr<IStorm3D_Model> &model, const std::string &name);
void addCloneModel(const boost::shared_ptr<IStorm3D_Model> &original, boost::shared_ptr<IStorm3D_Model> &clone, const std::string &helperName);
void addCloneModel(const boost::shared_ptr<IStorm3D_Model> &original, boost::shared_ptr<IStorm3D_Model> &clone);

boost::shared_ptr<IStorm3D_Model> createEditorModel(IStorm3D &storm, const std::string &fileName);

} // editor
} // frozenbyte

#endif
