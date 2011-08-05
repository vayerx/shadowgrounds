// Copyright 2002-2004 Frozenbyte Ltd.

#ifndef INCLUDED_PROCEDURAL_APPLIER_H
#define INCLUDED_PROCEDURAL_APPLIER_H

class IStorm3D;
class IStorm3D_Terrain;

namespace util {

class ProceduralProperties;
void applyStorm(IStorm3D &storm, const ProceduralProperties &properties);
void applyRenderer(IStorm3D &storm, const ProceduralProperties &properties);

} // util

#endif

