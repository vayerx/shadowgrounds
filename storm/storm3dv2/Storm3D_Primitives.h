/*

  Storm3D v2.0 T&L Graphics Engine
  (C) Sebastian Aaltonen 2000

  Basic Primitives
	
*/


#pragma once


//------------------------------------------------------------------
// Includes (etc)
//------------------------------------------------------------------
#include "Storm3DV2.h"



//------------------------------------------------------------------
// Storm3D_Model_Object_Sphere
//------------------------------------------------------------------
class Storm3D_Model_Object_Sphere : public Storm3D_Model_Object
{
	// Parameters
	float radius;
	float texrep_ring,texrep_height;

	// Virtual functions
	void ReCreate();	

	// Creation/delete (only models use these)
	Storm3D_Model_Object_Sphere(Storm3D_Model *_parent_model,
		float _radius,float _texrep_ring,float _texrep_height);
	~Storm3D_Model_Object_Sphere();

public:

	// Parameter change
	ChangeSize(float _radius);
	ChangeTextureRepeat(float _texrep_ring=2.0f,float _texrep_height=1.0f);

	friend Storm3D_Model;
};



//------------------------------------------------------------------
// Storm3D_Model_Object_Plane
//------------------------------------------------------------------
class Storm3D_Model_Object_Plane : public Storm3D_Model_Object
{
	// Parameters
	float height,width;
	float texrep_height,texrep_width;

	// Virtual functions
	void ReCreate();	

	// Creation/delete (only models use these)
	Storm3D_Model_Object_Plane(Storm3D_Model *_parent_model,
		float _height,float _width,float _texrep_height,float _texrep_width);
	~Storm3D_Model_Object_Plane();

public:

	// Parameter change
	ChangeSize(float _height,float _width);
	ChangeTextureRepeat(float _texrep_height=1.0f,float _texrep_width=1.0f);

	friend Storm3D_Model;
};



//------------------------------------------------------------------
// Storm3D_Model_Object_Box
//------------------------------------------------------------------
class Storm3D_Model_Object_Box : public Storm3D_Model_Object
{
	// Parameters
	float width,height,length;
	float texrep_width,texrep_height,texrep_length;

	// Virtual functions
	void ReCreate();	

	// Creation/delete (only models use these)
	Storm3D_Model_Object_Box(Storm3D_Model *_parent_model,
		float _width,float _height,float _length,
		float _texrep_width,float _texrep_height,float _texrep_length);
	~Storm3D_Model_Object_Box();

public:

	// Parameter change
	ChangeSize(float _width,float _height,float _length);
	ChangeTextureRepeat(float _texrep_width=1.0f,float _texrep_height=1.0f,float _texrep_length=1.0f);

	friend Storm3D_Model;
};


