// Copyright 2002-2004 Frozenbyte Ltd.

#ifndef INCLUDED_EDITOR_STORM_GEOMETRY_H
#define INCLUDED_EDITOR_STORM_GEOMETRY_H

#include <datatypedef.h>

class IStorm3D_Mesh;
class IStorm3D_Model;

namespace frozenbyte {
namespace editor {

struct Storm;

IStorm3D_Mesh *createWireframeObject(Storm &storm, IStorm3D_Model *model, const COL &color, const char *name);
void addBox(IStorm3D_Mesh *mesh, const VC3 &center, float radius);
void addLine(IStorm3D_Mesh *mesh, const VC3 &start, const VC3 &end, float thickness, const VC3 &normal);
void addCone(IStorm3D_Mesh *mesh, const VC3 &origo, float xAngle, float fov, float range, int circleVertices);

VC3 getSize(IStorm3D_Model *model);

} // editor
} // frozenbyte

#endif
