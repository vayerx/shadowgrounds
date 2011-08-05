// Copyright 2002-2004 Frozenbyte Ltd.

#ifdef _MSC_VER
#pragma warning(disable:4103)
#endif

//------------------------------------------------------------------
// Includes
//------------------------------------------------------------------
#include "storm3d_helper.h"
#include "storm3d_model_object.h"
#include "storm3d_model.h"
#include "Storm3D_Bone.h"
#include "../../util/Debug_MemoryManager.h"


Storm3D_Helper_Camera::Storm3D_Helper_Camera(const char *_name,Storm3D_Model *_parent_model,VC3 _position,VC3 _direction,VC3 _up) :
	Storm3D_Helper_AInterface(_name,_parent_model,_position, this),
	animation(this),direction(_direction),upvec(_up)
{
}

IStorm3D_Helper::HTYPE Storm3D_Helper_Camera::GetHelperType()
{
	return IStorm3D_Helper::HTYPE_CAMERA;
}

void Storm3D_Helper_Camera::SetName(const char *_name)
{
	delete[] name;
	name=new char[strlen(_name)+1];
	strcpy(name,_name);
}

const char *Storm3D_Helper_Camera::GetName()
{
	return name;
}

IStorm3D_Bone *Storm3D_Helper_Camera::GetParentBone()
{
	return parent_bone;
}

void Storm3D_Helper_Camera::SetPosition(const VC3 &_position)
{
	position=_position;
	update_globals=true;
}

void Storm3D_Helper_Camera::SetDirection(const VC3 &_direction)
{
	direction=_direction;
	update_globals=true;
}

void Storm3D_Helper_Camera::SetUpVector(const VC3 &_up)
{
	upvec=_up;
	update_globals=true;
}

VC3 &Storm3D_Helper_Camera::GetPosition()
{
	return position;
}

VC3 &Storm3D_Helper_Camera::GetGlobalPosition()
{
	// Update global values if needed
	UpdateGlobals();
	return position_global;
}

VC3 &Storm3D_Helper_Camera::GetDirection()
{
	return direction;
}

VC3 &Storm3D_Helper_Camera::GetGlobalDirection()
{
	// Update global values if needed
	UpdateGlobals();
	return direction_global;
}

VC3 &Storm3D_Helper_Camera::GetUpVector()
{
	return upvec;
}

VC3 &Storm3D_Helper_Camera::GetGlobalUpVector()
{
	// Update global values if needed
	UpdateGlobals();
	return upvec_global;
}

Matrix Storm3D_Helper_Camera::GetTM()
{
	Matrix result;

	// x
	Vector x = upvec.GetCrossWith(direction);
	result.Set(0, x.x);
	result.Set(1, x.y);
	result.Set(2, x.z);
	result.Set(3, 0);

	// y
	result.Set(4, upvec.x);
	result.Set(5, upvec.y);
	result.Set(6, upvec.z);
	result.Set(7, 0);

	// z
	result.Set(8, direction.x);
	result.Set(9, direction.y);
	result.Set(10, direction.z);
	result.Set(11, 0);

	// pos
	result.Set(12, position.x);
	result.Set(13, position.y);
	result.Set(14, position.z);
	result.Set(15, 1);

	return result;
}

void Storm3D_Helper_Camera::Animation_Clear()
{
	animation.Clear();
}

void Storm3D_Helper_Camera::Animation_AddNewPositionKeyFrame(int time,const VC3 &position)
{
	animation.AddNewPositionKeyFrame(time,position);
}

void Storm3D_Helper_Camera::Animation_AddNewDirectionKeyFrame(int time,const VC3 &position)
{
	animation.AddNewDirectionKeyFrame(time,position);
}

void Storm3D_Helper_Camera::Animation_AddNewUpVectorKeyFrame(int time,const VC3 &position)
{
	animation.AddNewUpVectorKeyFrame(time,position);
}

void Storm3D_Helper_Camera::Animation_SetLoop(int end_time)
{
	animation.SetLoop(end_time);
}

void Storm3D_Helper_Camera::Animation_SetCurrentTime(int time)
{
	animation.SetCurrentTime(time);
}

void Storm3D_Helper_Camera::UpdateGlobals()
{
	if (update_globals) 
	{
		update_globals=false;
		position_global=position;
		direction_global=direction;
		upvec_global=upvec;
		
		if(parent_object)
		{
			parent_object->GetMXG().TransformVector(position_global);
			parent_object->GetMXG().RotateVector(direction_global);
			parent_object->GetMXG().RotateVector(upvec_global);
		}
		else if(parent_bone)
		{
			parent_bone->GetTM().TransformVector(position_global);
			parent_bone->GetTM().RotateVector(direction_global);
			parent_bone->GetTM().RotateVector(upvec_global);
		}
		else
		{
			parent_model->GetMX().TransformVector(position_global);
			parent_model->GetMX().RotateVector(direction_global);
			parent_model->GetMX().RotateVector(upvec_global);
		}
	}
}


Storm3D_Helper_Vector::Storm3D_Helper_Vector(const char *_name,Storm3D_Model *_parent_model,VC3 _position,VC3 _direction) :
	Storm3D_Helper_AInterface(_name,_parent_model,_position, this),
	animation(this), direction(_direction)
{
}

IStorm3D_Helper::HTYPE Storm3D_Helper_Vector::GetHelperType()
{
	return IStorm3D_Helper::HTYPE_VECTOR;
}

void Storm3D_Helper_Vector::SetName(const char *_name)
{
	delete[] name;
	name=new char[strlen(_name)+1];
	strcpy(name,_name);
}

const char *Storm3D_Helper_Vector::GetName()
{
	return name;
}

IStorm3D_Bone *Storm3D_Helper_Vector::GetParentBone()
{
	return parent_bone;
}

void Storm3D_Helper_Vector::SetPosition(const VC3 &_position)
{
	position=_position;
	update_globals=true;
}

void Storm3D_Helper_Vector::SetDirection(const VC3 &_direction)
{
	direction=_direction;
	update_globals=true;
}

VC3 &Storm3D_Helper_Vector::GetPosition()
{
	return position;
}

VC3 &Storm3D_Helper_Vector::GetGlobalPosition()
{
	// Update global values if needed
	UpdateGlobals();
	return position_global;
}

VC3 &Storm3D_Helper_Vector::GetDirection()
{
	return direction;
}

VC3 &Storm3D_Helper_Vector::GetGlobalDirection()
{
	// Update global values if needed
	UpdateGlobals();
	return direction_global;
}

Matrix Storm3D_Helper_Vector::GetTM()
{
	Matrix result;

	// z
	result.Set(8, direction.x);
	result.Set(9, direction.y);
	result.Set(10, direction.z);
	result.Set(11, 0);

	// pos
	result.Set(12, position.x);
	result.Set(13, position.y);
	result.Set(14, position.z);
	result.Set(15, 1);

	return result;
}

void Storm3D_Helper_Vector::Animation_Clear()
{
	animation.Clear();
}

void Storm3D_Helper_Vector::Animation_AddNewPositionKeyFrame(int time,const VC3 &position)
{
	animation.AddNewPositionKeyFrame(time,position);
}

void Storm3D_Helper_Vector::Animation_AddNewDirectionKeyFrame(int time,const VC3 &position)
{
	animation.AddNewDirectionKeyFrame(time,position);
}

void Storm3D_Helper_Vector::Animation_SetLoop(int end_time)
{
	animation.SetLoop(end_time);
}

void Storm3D_Helper_Vector::Animation_SetCurrentTime(int time)
{
	animation.SetCurrentTime(time);
}

void Storm3D_Helper_Vector::UpdateGlobals()
{
	if (update_globals) 
	{
		update_globals=false;
		position_global=position;
		direction_global=direction;
		
		if (parent_object)
		{
			parent_object->GetMXG().TransformVector(position_global);
			parent_object->GetMXG().RotateVector(direction_global);
		}
		else if(parent_bone)
		{
			parent_bone->GetTM().TransformVector(position_global);
			parent_bone->GetTM().RotateVector(direction_global);
		}
		else
		{
			parent_model->GetMX().TransformVector(position_global);
			parent_model->GetMX().RotateVector(direction_global);
		}
	}
}


Storm3D_Helper_Point::Storm3D_Helper_Point(const char *_name,Storm3D_Model *_parent_model,VC3 _position) :
	Storm3D_Helper_AInterface(_name,_parent_model,_position, helper),animation(this)
{
}

IStorm3D_Helper::HTYPE Storm3D_Helper_Point::GetHelperType()
{
	return IStorm3D_Helper::HTYPE_POINT;
}

void Storm3D_Helper_Point::SetName(const char *_name)
{
	delete[] name;
	name=new char[strlen(_name)+1];
	strcpy(name,_name);
}

const char *Storm3D_Helper_Point::GetName()
{
	return name;
}

IStorm3D_Bone *Storm3D_Helper_Point::GetParentBone()
{
	return parent_bone;
}

void Storm3D_Helper_Point::SetPosition(const VC3 &_position)
{
	position=_position;
	update_globals=true;
}


VC3 &Storm3D_Helper_Point::GetPosition()
{
	return position;
}


VC3 &Storm3D_Helper_Point::GetGlobalPosition()
{
	// Update global values if needed
	UpdateGlobals();
	return position_global;
}

Matrix Storm3D_Helper_Point::GetTM()
{
	Matrix result;
	result.CreateTranslationMatrix(position);

	return result;
}

void Storm3D_Helper_Point::Animation_Clear()
{
	animation.Clear();
}


void Storm3D_Helper_Point::Animation_AddNewPositionKeyFrame(int time,const VC3 &position)
{
	animation.AddNewPositionKeyFrame(time,position);
}


void Storm3D_Helper_Point::Animation_SetLoop(int end_time)
{
	animation.SetLoop(end_time);
}


void Storm3D_Helper_Point::Animation_SetCurrentTime(int time)
{
	animation.SetCurrentTime(time);
}


void Storm3D_Helper_Point::UpdateGlobals()
{
	if(update_globals)
	{
		update_globals=false;
		position_global=position;
		
		if(parent_object)
		{
			parent_object->GetMXG().TransformVector(position_global);
		}
		else if(parent_bone)
		{
			parent_bone->GetTM().TransformVector(position_global);
		}
		else
		{
			parent_model->GetMX().TransformVector(position_global);
		}
	}
}


Storm3D_Helper_Box::Storm3D_Helper_Box(const char *_name,Storm3D_Model *_parent_model,VC3 _position,VC3 _size) :
	Storm3D_Helper_AInterface(_name,_parent_model,_position, helper),
	animation(this),size(_size)
{
}

IStorm3D_Helper::HTYPE Storm3D_Helper_Box::GetHelperType()
{
	return IStorm3D_Helper::HTYPE_BOX;
}

void Storm3D_Helper_Box::SetName(const char *_name)
{
	delete[] name;
	name=new char[strlen(_name)+1];
	strcpy(name,_name);
}

const char *Storm3D_Helper_Box::GetName()
{
	return name;
}

IStorm3D_Bone *Storm3D_Helper_Box::GetParentBone()
{
	return parent_bone;
}

void Storm3D_Helper_Box::SetPosition(const VC3 &_position)
{
	position=_position;
	update_globals=true;
}

void Storm3D_Helper_Box::SetSize(VC3 &_size)
{
	size=_size;
}

VC3 &Storm3D_Helper_Box::GetPosition()
{
	return position;
}

VC3 &Storm3D_Helper_Box::GetGlobalPosition()
{
	// Update global values if needed
	UpdateGlobals();
	return position_global;
}

VC3 &Storm3D_Helper_Box::GetSize()
{
	return size;
}

Matrix Storm3D_Helper_Box::GetTM()
{
	return Matrix();
}

void Storm3D_Helper_Box::Animation_Clear()
{
	animation.Clear();
}

void Storm3D_Helper_Box::Animation_AddNewPositionKeyFrame(int time,const VC3 &position)
{
	animation.AddNewPositionKeyFrame(time,position);
}

void Storm3D_Helper_Box::Animation_AddNewSizeKeyFrame(int time,const VC3 &position)
{
	animation.AddNewDirectionKeyFrame(time,position);
}

void Storm3D_Helper_Box::Animation_SetLoop(int end_time)
{
	animation.SetLoop(end_time);
}

void Storm3D_Helper_Box::Animation_SetCurrentTime(int time)
{
	animation.SetCurrentTime(time);
}

void Storm3D_Helper_Box::UpdateGlobals()
{
	if (update_globals) 
	{
		update_globals=false;
		position_global=position;
		if (parent_object)
		{
			parent_object->GetMXG().TransformVector(position_global);
		}
		else
		{
			parent_model->GetMX().TransformVector(position_global);
		}
	}
}


Storm3D_Helper_Sphere::Storm3D_Helper_Sphere(const char *_name,Storm3D_Model *_parent_model,VC3 _position,float _radius) :
	Storm3D_Helper_AInterface(_name,_parent_model,_position, helper),
	animation(this),radius(_radius)
{
}

IStorm3D_Helper::HTYPE Storm3D_Helper_Sphere::GetHelperType()
{
	return IStorm3D_Helper::HTYPE_SPHERE;
}

void Storm3D_Helper_Sphere::SetName(const char *_name)
{
	delete[] name;
	name=new char[strlen(_name)+1];
	strcpy(name,_name);
}

const char *Storm3D_Helper_Sphere::GetName()
{
	return name;
}

IStorm3D_Bone *Storm3D_Helper_Sphere::GetParentBone()
{
	return parent_bone;
}

void Storm3D_Helper_Sphere::SetPosition(const VC3 &_position)
{
	position=_position;
	update_globals=true;
}

void Storm3D_Helper_Sphere::SetRadius(float _radius)
{
	radius=_radius;
}

VC3 &Storm3D_Helper_Sphere::GetPosition()
{
	return position;
}

VC3 &Storm3D_Helper_Sphere::GetGlobalPosition()
{
	// Update global values if needed
	UpdateGlobals();
	return position_global;
}

float Storm3D_Helper_Sphere::GetRadius()
{
	return radius;
}

Matrix Storm3D_Helper_Sphere::GetTM()
{
	return Matrix();
}

void Storm3D_Helper_Sphere::Animation_Clear()
{
	animation.Clear();
}

void Storm3D_Helper_Sphere::Animation_AddNewPositionKeyFrame(int time,const VC3 &position)
{
	animation.AddNewPositionKeyFrame(time,position);
}

void Storm3D_Helper_Sphere::Animation_AddNewRadiusKeyFrame(int time,float radius)
{
	animation.AddNewDirectionKeyFrame(time,VC3(radius,0,0));
}

void Storm3D_Helper_Sphere::Animation_SetLoop(int end_time)
{
	animation.SetLoop(end_time);
}

void Storm3D_Helper_Sphere::Animation_SetCurrentTime(int time)
{
	animation.SetCurrentTime(time);
}

void Storm3D_Helper_Sphere::UpdateGlobals()
{
	if (update_globals) 
	{
		update_globals=false;
		position_global=position;
		if (parent_object)
		{
			parent_object->GetMXG().TransformVector(position_global);
		}
		else
		{
			parent_model->GetMX().TransformVector(position_global);
		}
	}
}
