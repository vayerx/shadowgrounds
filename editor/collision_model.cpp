// Copyright 2002-2004 Frozenbyte Ltd.

#pragma warning(disable:4103)
#pragma warning(disable:4786)

#include "collision_model.h"
#include "mapped_object.h"
#include "../util/buildingmap.h"

#include <boost/shared_ptr.hpp>
#include <istorm3d.h>
#include <istorm3d_model.h>
#include <istorm3d_mesh.h>
#include <c2_sphere.h>
#include <c2_ray.h>
#include <c2_collision.h>

namespace frozenbyte {
namespace editor {

struct CollisionModelData
{
	Storm &storm;

	ObjectData data;
	Vector position;
	VC3 rotation;
	COL color;

	boost::shared_ptr<IStorm3D_Model> model;

	CollisionModelData(Storm &storm_)
	:	storm(storm_)
	{
	}

	void addScene()
	{
		storm.scene->AddModel(model.get());
	}

	void removeScene()
	{
		storm.scene->RemoveModel(model.get());
	}

	void createMesh(IStorm3D_Mesh &mesh)
	{
		Vector v[8];
		v[0] = Vector(data.radiusX, 0, data.radiusZ);
		v[1] = Vector(data.radiusX, 0, -data.radiusZ);
		v[2] = Vector(-data.radiusX, 0, data.radiusZ);
		v[3] = Vector(-data.radiusX, 0, -data.radiusZ);

		v[4] = v[0] + Vector(0, data.height, 0);
		v[5] = v[1] + Vector(0, data.height, 0);
		v[6] = v[2] + Vector(0, data.height, 0);
		v[7] = v[3] + Vector(0, data.height, 0);

		mesh.ChangeVertexCount(8);
		mesh.ChangeFaceCount(24);

		Storm3D_Vertex *vertexBuffer = mesh.GetVertexBuffer();
		Storm3D_Face *faceBuffer = mesh.GetFaceBuffer();

		for(int i = 0; i < 8; ++i)
			vertexBuffer[i].position = v[i];

		faceBuffer[0].vertex_index[0] = 0;
		faceBuffer[0].vertex_index[1] = 1;
		faceBuffer[0].vertex_index[2] = 2;
		faceBuffer[1].vertex_index[0] = 1;
		faceBuffer[1].vertex_index[1] = 2;
		faceBuffer[1].vertex_index[2] = 3;
		faceBuffer[2].vertex_index[0] = 4;
		faceBuffer[2].vertex_index[1] = 5;
		faceBuffer[2].vertex_index[2] = 6;
		faceBuffer[3].vertex_index[0] = 5;
		faceBuffer[3].vertex_index[1] = 6;
		faceBuffer[3].vertex_index[2] = 7;
		faceBuffer[4].vertex_index[0] = 0;
		faceBuffer[4].vertex_index[1] = 1;
		faceBuffer[4].vertex_index[2] = 4;
		faceBuffer[5].vertex_index[0] = 4;
		faceBuffer[5].vertex_index[1] = 5;
		faceBuffer[5].vertex_index[2] = 1;
		faceBuffer[6].vertex_index[0] = 2;
		faceBuffer[6].vertex_index[1] = 3;
		faceBuffer[6].vertex_index[2] = 6;			
		faceBuffer[7].vertex_index[0] = 6;
		faceBuffer[7].vertex_index[1] = 7;
		faceBuffer[7].vertex_index[2] = 3;
		faceBuffer[8].vertex_index[0] = 0;
		faceBuffer[8].vertex_index[1] = 2;
		faceBuffer[8].vertex_index[2] = 6;
		faceBuffer[9].vertex_index[0] = 6;
		faceBuffer[9].vertex_index[1] = 4;
		faceBuffer[9].vertex_index[2] = 0;
		faceBuffer[10].vertex_index[0] = 1;
		faceBuffer[10].vertex_index[1] = 3;
		faceBuffer[10].vertex_index[2] = 7;
		faceBuffer[11].vertex_index[0] = 7;
		faceBuffer[11].vertex_index[1] = 5;
		faceBuffer[11].vertex_index[2] = 1;

		faceBuffer[0+12].vertex_index[0] = 0;
		faceBuffer[0+12].vertex_index[1] = 2;
		faceBuffer[0+12].vertex_index[2] = 1;
		faceBuffer[1+12].vertex_index[0] = 1;
		faceBuffer[1+12].vertex_index[1] = 3;
		faceBuffer[1+12].vertex_index[2] = 2;
		faceBuffer[2+12].vertex_index[0] = 4;
		faceBuffer[2+12].vertex_index[1] = 6;
		faceBuffer[2+12].vertex_index[2] = 5;
		faceBuffer[3+12].vertex_index[0] = 5;
		faceBuffer[3+12].vertex_index[1] = 7;
		faceBuffer[3+12].vertex_index[2] = 6;
		faceBuffer[4+12].vertex_index[0] = 0;
		faceBuffer[4+12].vertex_index[1] = 4;
		faceBuffer[4+12].vertex_index[2] = 1;
		faceBuffer[5+12].vertex_index[0] = 4;
		faceBuffer[5+12].vertex_index[1] = 1;
		faceBuffer[5+12].vertex_index[2] = 5;
		faceBuffer[6+12].vertex_index[0] = 2;
		faceBuffer[6+12].vertex_index[1] = 6;
		faceBuffer[6+12].vertex_index[2] = 3;			
		faceBuffer[7+12].vertex_index[0] = 6;
		faceBuffer[7+12].vertex_index[1] = 3;
		faceBuffer[7+12].vertex_index[2] = 7;
		faceBuffer[8+12].vertex_index[0] = 0;
		faceBuffer[8+12].vertex_index[1] = 6;
		faceBuffer[8+12].vertex_index[2] = 2;
		faceBuffer[9+12].vertex_index[0] = 6;
		faceBuffer[9+12].vertex_index[1] = 0;
		faceBuffer[9+12].vertex_index[2] = 4;
		faceBuffer[10+12].vertex_index[0] = 1;
		faceBuffer[10+12].vertex_index[1] = 7;
		faceBuffer[10+12].vertex_index[2] = 3;
		faceBuffer[11+12].vertex_index[0] = 7;
		faceBuffer[11+12].vertex_index[1] = 1;
		faceBuffer[11+12].vertex_index[2] = 5;
	}
};

CollisionModel::CollisionModel(ObjectData &data_, const COL &color, const Vector &position, const VC3 &rotation, Storm &storm)
{
	boost::scoped_ptr<CollisionModelData> tempData(new CollisionModelData(storm));
	tempData->data = data_;
	tempData->position = position;
	tempData->rotation = rotation;
	tempData->color = color;

	data.swap(tempData);
}

CollisionModel::~CollisionModel()
{
	data->removeScene();
}

void CollisionModel::create()
{
	data->model = boost::shared_ptr<IStorm3D_Model> (data->storm.storm->CreateNewModel());
	data->model->CastShadows(false);
	
	IStorm3D_Material *material = data->storm.storm->CreateNewMaterial("ah");
	IStorm3D_Model_Object *object = data->model->Object_New("uh");
	IStorm3D_Mesh *mesh = data->storm.storm->CreateNewMesh();

	material->SetSpecial(true, true);
	material->SetColor(data->color);
	material->SetSelfIllumination(COL(1.f,1.f,1.f));

	mesh->UseMaterial(material);
	data->createMesh(*mesh);
	object->SetMesh(mesh);

	QUAT rotation = getRotation(data->rotation);
	//rotation.MakeFromAngles(0, data->yAngle, 0);

	data->model->SetPosition(data->position);
	data->model->SetRotation(rotation);

	data->addScene();
}

void CollisionModel::clone(CollisionModel &rhs)
{
	data->model = boost::shared_ptr<IStorm3D_Model> (data->storm.storm->CreateNewModel());
	IStorm3D_Model_Object *object = data->model->Object_New("uh");

	IStorm3D_Model_Object *rhsObject = rhs.data->model->SearchObject("uh");
	if(rhsObject)
		object->SetMesh(rhsObject->GetMesh());

	QUAT rotation = getRotation(data->rotation);
	//rotation.MakeFromAngles(0, data->yAngle, 0);

	data->model->SetPosition(data->position);
	data->model->SetRotation(rotation);

	data->model->CastShadows(false);
	data->addScene();
}

void CollisionModel::scale()
{
	VC3 scale;
	scale.x = scale.y = scale.z = 1.01f;
	
	data->model->SetScale(scale);
}

void CollisionModel::hide()
{
	data->removeScene();
}

void CollisionModel::show()
{
	data->addScene();
}

struct CollisionVolumeData
{
	ObjectData &data;
	float boundingRadius;

	CollisionVolumeData(ObjectData &data_)
	:	data(data_), boundingRadius(0)
	{
	}

	bool possibleCollision(const VC3 &objectPosition, CollisionData &collisionData)
	{
		/*
		// This is just BS - doesn't work at all
		VC3 distanceVector = objectPosition - collisionData.rayOrigin;
		float relativeDistance = distanceVector.GetSquareLength();
		if(relativeDistance - boundingRadius*boundingRadius > collisionData.rayLength*collisionData.rayLength)
			return false;

		float projectedDistance = distanceVector.GetDotWith(collisionData.rayDirection);
		if((projectedDistance < 0) && (relativeDistance > boundingRadius*boundingRadius))
			return false;

		float projectionRadius = relativeDistance - projectedDistance*projectedDistance;
		if(projectionRadius > boundingRadius*boundingRadius)
			return false;

		return true;
		*/

		Sphere sphere(objectPosition, boundingRadius);
		Ray ray(collisionData.rayOrigin, collisionData.rayDirection, collisionData.rayLength);
		
		return collision(sphere, ray);
	}

	bool testPlanes(float start, float end, float value1, float value2)
	{
		if((start < value1) && (end > value1))
			return true;
		if((start < value2) && (end > value2))
			return true;
		if((start > value1) && (end < value1))
			return true;
		if((start > value2) && (end < value2))
			return true;

		return false;
	}

	float &get(VC3 &vector, int index)
	{
		if(index == 0)
			return vector.x;
		else if(index == 1)
			return vector.y;
		else
			return vector.z;
	}

	int getInteger(float value)
	{
		union FloatInt
		{
			float f;
			int i;
		};

		FloatInt foo;
		foo.f = value;

		return foo.i;
	}

	bool accurateCollision(const VC3 &objectPosition, CollisionData &collisionData, const VC3 &rayOrigin_, const VC3 &rayDirection_)
	{
		bool inside = true;
		int i = 0;

		VC3 rayOrigin = rayOrigin_;
		VC3 rayDirection = rayDirection_;

		Vector minB = objectPosition - Vector(data.radiusX, 0, data.radiusZ);
		Vector maxB = objectPosition + Vector(data.radiusX, data.height, data.radiusZ);
		Vector maxT(-1, -1, -1);
		Vector collision;

		for(i = 0; i < 3; ++i)
		{
			//if(get(collisionData.rayOrigin, i) < get(minB, i))
			if(get(rayOrigin, i) < get(minB, i))
			{
				get(collision, i) = get(minB, i);
				inside = false;

				//if(getInteger(get(collisionData.rayDirection, i)))
				//	get(maxT, i) = (get(minB,i) - get(collisionData.rayOrigin, i)) / get(collisionData.rayDirection, i);
				if(getInteger(get(rayDirection, i)))
					get(maxT, i) = (get(minB,i) - get(rayOrigin, i)) / get(rayDirection, i);
			}
			//else if(get(collisionData.rayOrigin, i) > get(maxB, i))
			else if(get(rayOrigin, i) > get(maxB, i))
			{
				get(collision, i) = get(maxB, i);
				inside = false;

				//if(getInteger(get(collisionData.rayDirection, i)))
				//	get(maxT, i) = (get(maxB,i) - get(collisionData.rayOrigin, i)) / get(collisionData.rayDirection, i);
				if(getInteger(get(rayDirection, i)))
					get(maxT, i) = (get(maxB,i) - get(rayOrigin, i)) / get(rayDirection, i);
			}
		}

		if(inside)
			return true;

		int whichPlane = 0;
		if(get(maxT, 1) > get(maxT, whichPlane))
			whichPlane = 1;
		if(get(maxT, 2) > get(maxT, whichPlane))
			whichPlane = 2;

		if(getInteger(get(maxT, whichPlane)) & 0x80000000)
			return false;

		for(i = 0; i < 3; ++i)
		{
			if(i == whichPlane)
				continue;

			//get(collision, i) = get(collisionData.rayOrigin, i) + get(maxT, whichPlane) * get(collisionData.rayDirection, i);
			get(collision, i) = get(rayOrigin, i) + get(maxT, whichPlane) * get(rayDirection, i);
			if(get(collision, i) < get(minB, i) || get(collision, i) > get(maxB, i)) 
				return false;
		}

		return true;
	}
};

CollisionVolume::CollisionVolume(ObjectData &data_)
{
	boost::scoped_ptr<CollisionVolumeData> tempData(new CollisionVolumeData(data_));
	tempData->boundingRadius = max(data_.radiusX, data_.radiusZ);
	tempData->boundingRadius *= tempData->boundingRadius;
	tempData->boundingRadius += data_.height*data_.height;
	tempData->boundingRadius = sqrtf(tempData->boundingRadius);

	data.swap(tempData);
}

CollisionVolume::~CollisionVolume()
{
}

void CollisionVolume::initialize(ObjectData &data_)
{
	data->data = data_;

	data->boundingRadius = max(data_.radiusX, data_.radiusZ);
	data->boundingRadius *= data->boundingRadius;
	data->boundingRadius += data_.height*data_.height;
	data->boundingRadius = sqrtf(data->boundingRadius);
}

bool CollisionVolume::testCollision(const VC3 &objectPosition, const VC3 &angles, CollisionData &collisionData, float epsilon)
{
	QUAT rotation = getRotation(angles);
	//rotation.MakeFromAngles(0, -yRotation, 0);
	
	Matrix tm;
	tm.CreateRotationMatrix(rotation);

	// Everything's relative etc
	VC3 rayOrigin = tm.GetTransformedVector(collisionData.rayOrigin - objectPosition) + objectPosition;
	VC3 rayDirection = tm.GetWithoutTranslation().GetTransformedVector(collisionData.rayDirection);

	if(!data->possibleCollision(objectPosition, collisionData))
		return false;
	if(!data->accurateCollision(objectPosition, collisionData, rayOrigin, rayDirection))
		return false;

	float collisionDistance = objectPosition.GetRangeTo(collisionData.rayOrigin);
	VC3 pos = tm.GetInverse().GetTransformedVector(collisionData.collisionPosition - objectPosition) + objectPosition;

	if(collisionData.hasCollision)
	if(collisionDistance > pos.GetRangeTo(collisionData.rayOrigin))
		return false;

	collisionData.rayLength = collisionDistance;
	collisionData.hasCollision = true;
	collisionData.collisionPosition = objectPosition;
	collisionData.objectData = data->data;

	return true;
}

} // end of namespace editor
} // end of namespace frozenbyte
