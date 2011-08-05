
#include "precompiled.h"

#include "heightmap_actor.h"
#include "NxPhysics.h"

namespace frozenbyte {
namespace physics {

HeightmapActor::HeightmapActor(NxPhysicsSDK &sdk_, NxScene &scene, const unsigned short *buffer, int samplesX, int samplesY, const VC3 &size)
:	sdk(sdk_),
	heightField(0)
{
	NxHeightFieldDesc heightDesc;
	heightDesc.nbColumns = samplesX;
	heightDesc.nbRows = samplesY;
	heightDesc.verticalExtent = -1000;
	heightDesc.convexEdgeThreshold = 0;
	heightDesc.samples = new NxU32[samplesX * samplesY];
	heightDesc.sampleStride = sizeof(NxU32);

    NxU8 *currentByte = (NxU8 *) heightDesc.samples;
	for(int row = 0; row < samplesY; ++row)
	for(int column = 0; column < samplesX; ++column)
	{            
		NxHeightFieldSample *currentSample = (NxHeightFieldSample *) currentByte;

		//int sample = buffer[row * samplesX + column];
		//int sample = buffer[column * samplesY + row];
		int sample = buffer[(samplesY - row - 1) * samplesX + column];
		//int sample = buffer[column * samplesX + row];
		sample -= 32768;

		currentSample->height = sample;
		currentSample->materialIndex0 = 0;
		currentSample->materialIndex1 = 0;
		currentSample->tessFlag = 0;
		currentByte += heightDesc.sampleStride;
	}

	heightField = sdk.createHeightField(heightDesc);
	delete[] (NxU32 *) heightDesc.samples;

	if(heightField)
	{
		float scaleX = size.x / samplesX;
		float scaleY = 1.f / 65536.f * size.y;
		float scaleZ = size.z / samplesY;

		NxHeightFieldShapeDesc shapeDesc;
		shapeDesc.heightField = heightField;
		shapeDesc.heightScale = scaleY;
		shapeDesc.rowScale = scaleZ;
		shapeDesc.columnScale = scaleX;
		shapeDesc.materialIndexHighBits = 0;
		shapeDesc.holeMaterial = 2;    

		NxVec3 pos;
		pos.x = -size.x * 0.5f;
		pos.y = size.y * 0.5f;
		pos.z = (size.z * 0.5f) - scaleZ;

		NxQuat quat;
		quat.zero();
		quat.fromAngleAxis(90, NxVec3(0, 1, 0));

		NxActorDesc actorDesc;
		actorDesc.shapes.pushBack(&shapeDesc);
		actorDesc.globalPose.t = pos;
		actorDesc.globalPose.M = quat;

		actor = scene.createActor(actorDesc);
	}

	this->scene = &scene;
	init();
}

HeightmapActor::~HeightmapActor()
{
	if(scene && actor)
	{
		scene->releaseActor(*actor);
		actor = 0;
	}

	if(heightField)
		sdk.releaseHeightField(*heightField);
}

bool HeightmapActor::isValid() const
{
	if(heightField && actor)
		return true;

	return false;
}

} // physics
} // frozenbyte
