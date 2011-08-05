// Copyright 2002-2004 Frozenbyte Ltd.

#include "../Shared/Export_Types.h"
#include "LWExport_Transform.h"
#include "LWExport_Manager.h"

#include <string>

namespace frozenbyte {
namespace exporter {
namespace {

FBMatrix createFBMatrix(LWDVector position, LWDVector angles)
{
	FBMatrix tm;
	FBQuaternion r;

	tm.Set(0, 1);
	tm.Set(1, 0);
	tm.Set(2, 0);
	tm.Set(3, 0);
	
	tm.Set(4, 0);
	tm.Set(5, 1);
	tm.Set(6, 0);
	tm.Set(7, 1);

	tm.Set(8, 0);
	tm.Set(9, 0);
	tm.Set(10, 1);
	tm.Set(11, 0);

	tm.Set(12, 0);
	tm.Set(13, 0);
	tm.Set(14, 0);
	tm.Set(15, 1);

	FBQuaternion x, y, z;
	x.MakeFromAngles(static_cast<double> (angles[1]), 0, 0);
	y.MakeFromAngles(0, static_cast<double> (angles[0]), 0);
	z.MakeFromAngles(0, 0, -static_cast<double> (angles[2]));

	r = z * (x * y);
	
	tm.CreateRotationMatrix(r);

	tm.Set(12, static_cast<double> (position[0]));
	tm.Set(13, static_cast<double> (position[1]));
	tm.Set(14, static_cast<double> (position[2]));

	return tm;
}

} // end of nameless namespace

FBMatrix LWTransforms::GetTransform(LWItemID itemId, float time)
{
	LWItemInfo *itemInfo = Manager::getSingleton()->getItemInfo();
	if(itemInfo == 0)
		return FBMatrix();

	// No longer works in 8.2 -- waah
	FBMatrix transform;
	LWDVector lw_x;
	LWDVector lw_y;
	LWDVector lw_z;
	LWDVector lw_pos;

	itemInfo->param(itemId, LWIP_RIGHT,  time, lw_x);
	itemInfo->param(itemId, LWIP_UP, time, lw_y);
	itemInfo->param(itemId, LWIP_FORWARD, time, lw_z);
	itemInfo->param(itemId, LWIP_W_POSITION, time, lw_pos);

	// Remove scale
	double xScale = sqrt(lw_x[0]*lw_x[0] + lw_x[1]*lw_x[1] + lw_x[2]*lw_x[2]);
	double yScale = sqrt(lw_y[0]*lw_y[0] + lw_y[1]*lw_y[1] + lw_y[2]*lw_y[2]);
	double zScale = sqrt(lw_z[0]*lw_z[0] + lw_z[1]*lw_z[1] + lw_z[2]*lw_z[2]);

	lw_x[0] /= xScale;
	lw_x[1] /= xScale;
	lw_x[2] /= xScale;
	lw_y[0] /= yScale;
	lw_y[1] /= yScale;
	lw_y[2] /= yScale;
	lw_z[0] /= zScale;
	lw_z[1] /= zScale;
	lw_z[2] /= zScale;

	transform.Set(0, lw_x[0]);
	transform.Set(1, lw_x[1]);
	transform.Set(2, lw_x[2]);
	transform.Set(3, 0);

	transform.Set(4, lw_y[0]);
	transform.Set(5, lw_y[1]);
	transform.Set(6, lw_y[2]);
	transform.Set(7, 0);

	transform.Set(8, lw_z[0]);
	transform.Set(9, lw_z[1]);
	transform.Set(10, lw_z[2]);
	transform.Set(11, 0);

	transform.Set(12, lw_pos[0]);
	transform.Set(13, lw_pos[1]);
	transform.Set(14, lw_pos[2]);
	transform.Set(15, 1);
	
	return transform;
	
/*
	LWDVector lw_pos;
	LWDVector lw_rot;
	LWDVector lw_pos_pivot;
	LWDVector lw_rot_pivot;

	FBMatrix result;
	LWItemID currentId = itemId;
	while(currentId != 0)
	{
		itemInfo->param(currentId, LWIP_PIVOT_ROT, 0, lw_rot_pivot);
		itemInfo->param(currentId, LWIP_PIVOT, 0, lw_pos_pivot);
		
		itemInfo->param(currentId, LWIP_ROTATION, time, lw_rot);
		itemInfo->param(currentId, LWIP_POSITION, time, lw_pos);

		FBMatrix tm = createFBMatrix(lw_pos, lw_rot);
		FBMatrix pivot = createFBMatrix(lw_pos_pivot, lw_rot_pivot);
		//result = result * pivot * tm;
		
		result = result * tm * pivot;
		currentId = itemInfo->parent(currentId);
	}

	return result;
*/
}

/*
FBMatrix LWTransforms::GetBoneTransform(LWItemID itemId, float time)
{
	LWItemInfo *itemInfo = Manager::getSingleton()->getItemInfo();
	LWBoneInfo *boneInfo = Manager::getSingleton()->getBoneInfo();

	if((itemInfo == 0) || (boneInfo == 0))
		return FBMatrix();

	LWDVector lw_pos;
	LWDVector lw_rot;

	LWDVector lw_pos_pivot;
	LWDVector lw_rot_pivot;

	FBMatrix result;

	LWItemID currentId = itemId;
	while(currentId != 0)
	{
		itemInfo->param(currentId, LWIP_PIVOT_ROT, 0, lw_rot_pivot);
		itemInfo->param(currentId, LWIP_PIVOT, 0, lw_pos_pivot);
		
		if(itemInfo->type(currentId) == LWI_BONE)
		{
			boneInfo->restParam(currentId, LWIP_ROTATION, lw_rot);
			boneInfo->restParam(currentId, LWIP_POSITION, lw_pos);
		}
		else
		{
			currentId = itemInfo->parent(currentId);
			continue;
		}

		FBMatrix tm = createFBMatrix(lw_pos, lw_rot);
		FBMatrix pivot = createFBMatrix(lw_pos_pivot, lw_rot_pivot);
		result = result * (pivot * tm);

		currentId = itemInfo->parent(currentId);
	}

	return result;
}
*/

FBMatrix LWTransforms::GetBoneTransform(LWItemID itemId, float time)
{
	LWItemInfo *itemInfo = Manager::getSingleton()->getItemInfo();
	if(itemInfo == 0)
		return FBMatrix();

	FBMatrix result = GetTransform(itemId, time);

	LWItemID current = itemId;
	while(current)
	{
		if(LWItemID goalId = itemInfo->goal(current))
		{
			std::string goalName = itemInfo->name(goalId);
			if(goalName.size() > 7)
			{
				for(unsigned int i = 0; i < goalName.size() - 7; ++i)
				{
					std::string subString = goalName.substr(i, 7);
					if(subString == "_match_")
					{
						FBMatrix currentTm = GetTransform(current, time);
						FBMatrix goalTm = GetTransform(goalId, time);
						FBMatrix parentTm;

						FBMatrix rotationTm;
						rotationTm.CreateRotationMatrix(goalTm.GetRotation());
						FBMatrix positionTm; 
						positionTm.CreateTranslationMatrix(currentTm.GetTranslation());

						FBMatrix offsetTm = result * currentTm.GetInverse();
						result = offsetTm * rotationTm * positionTm;
						break;
					}
				}
			}
		}

		current = itemInfo->parent(current);
	}

/*
	FBMatrix result;
	LWItemID current = itemId;
	while(current)
	{
		LWDVector pivot_position = { 0 };
		LWDVector pivot_rotation = { 0 };
		itemInfo->param(current, LWIP_PIVOT, time, pivot_position);
		itemInfo->param(current, LWIP_PIVOT_ROT, time, pivot_rotation);

		LWDVector position = { 0 };
		LWDVector rotation = { 0 };
		itemInfo->param(current, LWIP_POSITION, time, position);
		itemInfo->param(current, LWIP_ROTATION, time, rotation);
		
		FBMatrix pivot = createFBMatrix(pivot_position, pivot_rotation);
		FBMatrix transform = pivot * createFBMatrix(position, rotation);

		result = result * transform;
		current = itemInfo->parent(current);
	}
*/		
	return result;
}


FBMatrix LWTransforms::GetLocalTransform(LWItemID itemId, float time)
{
	LWItemInfo *itemInfo = Manager::getSingleton()->getItemInfo();
	if(itemInfo == 0)
		return FBMatrix();

	LWDVector pivotPosition = { 0 };
	LWDVector pivotRotation = { 0 };
	itemInfo->param(itemId, LWIP_PIVOT, time, pivotPosition);
	itemInfo->param(itemId, LWIP_PIVOT_ROT, time, pivotRotation);

	LWDVector position = { 0 };
	LWDVector rotation = { 0 };
	itemInfo->param(itemId, LWIP_POSITION, time, position);
	itemInfo->param(itemId, LWIP_ROTATION, time, rotation);

	FBMatrix tm = createFBMatrix(position, rotation);
	FBMatrix pivot = createFBMatrix(pivotPosition, pivotRotation);

	return pivot * tm;
}

FBMatrix LWTransforms::GetPivotTransform(LWItemID itemId)
{
	LWItemInfo *itemInfo = Manager::getSingleton()->getItemInfo();
	if(itemInfo == 0)
		return FBMatrix();

	LWDVector position = { 0 };
	LWDVector FBQuaternion = { 0 };
	
	itemInfo->param(itemId, LWIP_PIVOT, 0, position);
	itemInfo->param(itemId, LWIP_PIVOT_ROT, 0, FBQuaternion);

	return createFBMatrix(position, FBQuaternion);
}

FBMatrix LWTransforms::GetBoneRestTransform(LWItemID itemId)
{
	LWItemInfo *itemInfo = Manager::getSingleton()->getItemInfo();
	LWBoneInfo *boneInfo = Manager::getSingleton()->getBoneInfo();

	if((itemInfo == 0) || (boneInfo == 0))
		return FBMatrix();

	LWDVector lw_pos;
	LWDVector lw_rot;

	LWDVector lw_pos_pivot;
	LWDVector lw_rot_pivot;

	FBMatrix result;

	LWItemID currentId = itemId;
	while(currentId != 0)
	{
		itemInfo->param(currentId, LWIP_PIVOT_ROT, 0, lw_rot_pivot);
		itemInfo->param(currentId, LWIP_PIVOT, 0, lw_pos_pivot);
		
		if(itemInfo->type(currentId) == LWI_BONE)
		{
			boneInfo->restParam(currentId, LWIP_ROTATION, lw_rot);
			boneInfo->restParam(currentId, LWIP_POSITION, lw_pos);
		}
		else
		{
			currentId = itemInfo->parent(currentId);
			continue;
		}

		FBMatrix tm = createFBMatrix(lw_pos, lw_rot);
		FBMatrix pivot = createFBMatrix(lw_pos_pivot, lw_rot_pivot);
		result = result * (pivot * tm);

		currentId = itemInfo->parent(currentId);
	}

	return result;
}

double LWTransforms::GetScale(LWItemID itemId, float time)
{
	LWItemInfo *itemInfo = Manager::getSingleton()->getItemInfo();
	if(itemInfo == 0)
		return 1.f;

	LWDVector vector = { 0 };
	itemInfo->param(itemId, LWIP_UP, time, vector);
	
	// Take scale from upvector
	double scale = static_cast<double> (vector[0]*vector[0] + vector[1]*vector[1] + vector[2]*vector[2]);
	scale = static_cast<double> (sqrt(scale));

	return scale;
}

double LWTransforms::GetBoneScale(LWItemID itemId)
{
	LWItemInfo *itemInfo = Manager::getSingleton()->getItemInfo();
	LWBoneInfo *boneInfo = Manager::getSingleton()->getBoneInfo();

	if((itemInfo == 0) || (boneInfo == 0))
		return 1.f;
	
	// Get scale from parent

	LWItemID currentId = itemId;
	while(currentId != 0)
	{
		if(itemInfo->type(currentId) != LWI_BONE)
			return GetScale(currentId, 0.f);

		currentId = itemInfo->parent(currentId);
	}

	return 1.f;
}

} // end of namespace export
} // end of namespace frozenbyte
