
#include "precompiled.h"

#ifndef NOMINMAX
#define NOMINMAX
#endif

#include "visualizer.h"
#include "physics_lib.h"
#include <IStorm3D_Scene.h>
#include "NxPhysics.h"


namespace frozenbyte {
namespace physics {
namespace {

	COL getColor(NxU32 color)
	{
		float b = NxF32((color)&0xff)/255.0f;
		float g = NxF32((color>>8)&0xff)/255.0f;
		float r = NxF32((color>>16)&0xff)/255.0f;
		return COL(r, g, b);
	}

	VC3 getPos(NxVec3 vec)
	{
		return VC3(vec.x, vec.y, vec.z);
	}

} // unnamed

void visualize(PhysicsLib &physics, IStorm3D_Scene &scene, float range)
{
	// float rangeSq = range*range;

	NxScene *pscene = physics.getScene();
	if(!pscene)
		return;

	const NxDebugRenderable *debug = pscene->getDebugRenderable();
	if(!debug)
		return;

    // Render points
    {
		NxU32 amount = debug->getNbPoints();
        const NxDebugPoint *buffer = debug->getPoints();
	
		while(amount--)
		{
			COL color = getColor(buffer->color);
			VC3 p1 = getPos(buffer->p);

			scene.AddPoint(p1, color);
			buffer++;
		}
	}
    // Render lines
    {
		NxU32 amount = debug->getNbLines();
        const NxDebugLine *buffer = debug->getLines();

#ifdef LEGACY_FILES
		// nop
#else
		VC3 camPos = scene.GetCamera()->GetPosition();
#endif

		while(amount--)
		{
			COL color= getColor(buffer->color);
			//color.g = 1.f;
			VC3 p1 = getPos(buffer->p0);
			VC3 p2 = getPos(buffer->p1);

#ifdef LEGACY_FILES
			scene.AddLine(p1, p2, color);
#else
//			if ((p1 - camPos).GetSquareLength() < rangeSq
//				&& (p2 - camPos).GetSquareLength() < rangeSq)
//			{
				scene.AddLine(p1, p2, color);
//			}
#endif
			buffer++;
		}
	}
    // Render triangles
    {
		NxU32 amount = debug->getNbTriangles();
        const NxDebugTriangle *buffer = debug->getTriangles();
	
		while(amount--)
		{
			COL color = getColor(buffer->color);
			VC3 p1 = getPos(buffer->p0);
			VC3 p2 = getPos(buffer->p1);
			VC3 p3 = getPos(buffer->p2);

			scene.AddTriangle(p1, p2, p3, color);
			buffer++;
		}
	}
}

} // physics
} // frozenbyte
