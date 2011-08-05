// Copyright 2002-2004 Frozenbyte Ltd.

#ifdef _MSC_VER
#pragma warning(disable:4103)
#endif

//------------------------------------------------------------------
// Includes
//------------------------------------------------------------------
#include "storm3d_model_object.h"
#include "storm3d_mesh.h"
#include "storm3d_model.h"
#include "storm3d_light.h"
#include "storm3d_helper.h"
#include "VertexFormats.h"
#include <boost/lexical_cast.hpp>

#include "Storm3D_Bone.h"
#include "../../util/Debug_MemoryManager.h"


#ifdef PROJECT_AOV
#define DEFAULT_ALPHA_TEST_VALUE 0x70
#else
#define DEFAULT_ALPHA_TEST_VALUE 0x50
#endif

//------------------------------------------------------------------
// Storm3D_Model_Object::Storm3D_Model_Object
//------------------------------------------------------------------
Storm3D_Model_Object::Storm3D_Model_Object(Storm3D *s2,const char *name,Storm3D_Model *_parent_model, Storm3D_ResourceManager &resourceManager_) :
	Storm3D2(s2),
	resourceManager(resourceManager_),
	name(0),
	mesh(NULL),
	gpos_update_needed(true),
	scale(1,1,1),
	mx_update(true),
	mxg_update(true),
	parent_model(_parent_model),
	parent(NULL),
	parent_bone(0),
	no_collision(false),
	no_render(false),
	light_object(false),
	sphere_collision_only(false),
	sphere_ok(false),
	box_ok(false),
	object_box_ok(false),
	visibilityFlag(0),
	force_alpha(0),
	force_lighting_alpha_enable(false),
	force_lighting_alpha(0),
	spot_transparency_factor(1.f),
	distortion_only(false),
	renderPassMask(RENDER_PASS_MASK_VALUE_NONE),
	alphaTestPassConditional(false),
	alphaTestValue(DEFAULT_ALPHA_TEST_VALUE),
	sort_data(0.f)
{
	visibility_id = 0;
	real_visibility_id = 0;

	for(int i = 0; i < LIGHT_MAX_AMOUNT; ++i)
		light_index[i] = -1;

	for (int i = 0; i < RENDER_PASS_BITS_AMOUNT; i++)
	{
		renderPassOffset[i] = VC3(0,0,0);
		renderPassScale[i] = VC3(0,0,0);
	}

	/*
	// Create name
	name=new char[strlen(_name)+1];
	strcpy(name,_name);
	*/
	SetName(name);
}



//------------------------------------------------------------------
// Storm3D_Model_Object::~Storm3D_Model_Object
//------------------------------------------------------------------
Storm3D_Model_Object::~Storm3D_Model_Object()
{
	if(parent)
		parent->RemoveChild(this);
	if(parent_bone)
		parent_bone->RemoveChild(this);
	if(mesh)
		resourceManager.removeUser(mesh, this);

	{
		set<IStorm3D_Model_Object *>::iterator it = child_objects.begin();
		for(; it != child_objects.end(); ++it)
		{
			Storm3D_Model_Object *o = static_cast<Storm3D_Model_Object *> (*it);
			if(!o)
				continue;

			o->parent = 0;
		}
	}

	{
		set<IStorm3D_Helper *>::iterator it = child_helpers.begin();
		for(; it != child_helpers.end(); ++it)
		{
			IStorm3D_Helper *ihelper = *it;
			if(!ihelper)
				continue;

			switch (ihelper->GetHelperType())
			{
				case IStorm3D_Helper::HTYPE_POINT:
					((Storm3D_Helper_Point*)ihelper)->parent_object=0;
					break;

				case IStorm3D_Helper::HTYPE_VECTOR:
					((Storm3D_Helper_Vector*)ihelper)->parent_object=0;
					break;
					
				case IStorm3D_Helper::HTYPE_CAMERA:
					((Storm3D_Helper_Camera*)ihelper)->parent_object=0;
					break;
					
				case IStorm3D_Helper::HTYPE_BOX:
					((Storm3D_Helper_Box*)ihelper)->parent_object=0;
					break;

				case IStorm3D_Helper::HTYPE_SPHERE:
					((Storm3D_Helper_Sphere*)ihelper)->parent_object=0;
					break;
			}

		}
	}


	// Delete name
	delete[] name;
}

const Sphere &Storm3D_Model_Object::GetBoundingSphere()
{
	if(!parent_model->bones.empty() || !mesh)
	{
		// TODO: should also be scaled ?
		bounding_sphere.position = position;
		bounding_sphere.radius = mesh->GetRadius();

		sphere_ok = false;
		return bounding_sphere;
	}

	if(!sphere_ok)
	{
		if(mx_update)
		{
			mx_update=false;
			MAT ms,mr,mp;
			ms.CreateScaleMatrix(scale);
			mr.CreateRotationMatrix(rotation);
			mp.CreateTranslationMatrix(position);
			mx=ms*mr*mp;
		}

		const Sphere &sphere = mesh->getBoundingSphere();
		bounding_sphere.position = mx.GetTransformedVector(sphere.position);

		/*
		VC3 modelScale = this->parent_model->GetScale();
		float maxScale = modelScale.x;
		if (modelScale.y > maxScale) maxScale = modelScale.y;
		if (modelScale.z > maxScale) maxScale = modelScale.z;
		bounding_sphere.radius = sphere.radius * maxScale;
		*/
		bounding_sphere.radius = sphere.radius;
		sphere_ok = true;
	}

	const Sphere &sphere = mesh->getBoundingSphere();
	bounding_sphere.radius = sphere.radius * parent_model->max_scale;

	return bounding_sphere;
}

const OOBB &Storm3D_Model_Object::GetObjectBoundingBox()
{
	if(!mesh || !parent_model->bones.empty())
	{
		object_box_ok = false;
		return object_bounding_box;
	}

	if(!object_box_ok)
	{
		/*
		if(mx_update)
		{
			mx_update=false;
			MAT ms,mr,mp;
			ms.CreateScaleMatrix(scale);
			mr.CreateRotationMatrix(rotation);
			mp.CreateTranslationMatrix(position);
			mx=ms*mr*mp;
		}
		*/

		const AABB &aabb = mesh->getBoundingBox();
		object_bounding_box.center = aabb.mmax + aabb.mmin;
		object_bounding_box.center *= .5f;

		/*
		mx.TransformVector(object_bounding_box.center);

		object_bounding_box.axes[0].x = mx.Get(0);
		object_bounding_box.axes[0].y = mx.Get(1);
		object_bounding_box.axes[0].z = mx.Get(2);
		object_bounding_box.axes[1].x = mx.Get(4);
		object_bounding_box.axes[1].y = mx.Get(5);
		object_bounding_box.axes[1].z = mx.Get(6);
		object_bounding_box.axes[2].x = mx.Get(8);
		object_bounding_box.axes[2].y = mx.Get(9);
		object_bounding_box.axes[2].z = mx.Get(10);
		*/

		object_bounding_box.axes[0].x = 1.f;
		object_bounding_box.axes[0].y = 0.f;
		object_bounding_box.axes[0].z = 0.f;
		object_bounding_box.axes[1].x = 0.f;
		object_bounding_box.axes[1].y = 1.f;
		object_bounding_box.axes[1].z = 0.f;
		object_bounding_box.axes[2].x = 0.f;
		object_bounding_box.axes[2].y = 0.f;
		object_bounding_box.axes[2].z = 1.f;

		object_bounding_box.extents.x = (aabb.mmax.x - aabb.mmin.x) * .5f;
		object_bounding_box.extents.y = (aabb.mmax.y - aabb.mmin.y) * .5f;
		object_bounding_box.extents.z = (aabb.mmax.z - aabb.mmin.z) * .5f;

		object_box_ok = true;
	}

	return object_bounding_box;
}

const AABB &Storm3D_Model_Object::GetBoundingBox()
{
	if(!box_ok && mesh)
	{
		if(mx_update)
		{
			mx_update=false;
			MAT ms,mr,mp;
			ms.CreateScaleMatrix(scale);
			mr.CreateRotationMatrix(rotation);
			mp.CreateTranslationMatrix(position);
			mx=ms*mr*mp;
		}

		/*
		bounding_box.mmin = VC3(100000.f, 100000.f, 100000.f);
		bounding_box.mmax = VC3(-100000.f, -100000.f, -100000.f);
		const Storm3D_Vertex *vbuffer = mesh->GetVertexBufferReadOnly();
		int vertex_amount = mesh->GetVertexCount();

		VC3 tmp_pos;
		for(int i = 0; i < vertex_amount; ++i)
		{
			tmp_pos = vbuffer[i].position;
			mx.TransformVector(tmp_pos);

			bounding_box.mmin.x = min(bounding_box.mmin.x, tmp_pos.x);
			bounding_box.mmin.y = min(bounding_box.mmin.y, tmp_pos.y);
			bounding_box.mmin.z = min(bounding_box.mmin.z, tmp_pos.z);
			bounding_box.mmax.x = max(bounding_box.mmax.x, tmp_pos.x);
			bounding_box.mmax.y = max(bounding_box.mmax.y, tmp_pos.y);
			bounding_box.mmax.z = max(bounding_box.mmax.z, tmp_pos.z);
		}
*/

		AABB bbox( VC3( 10000.0f, 10000.0f, 10000.0f), VC3(-10000.0f,-10000.0f,-10000.0f) );

		const AABB &meshAABB = mesh->getBoundingBox ();
		VC3 v[8] = {
			VC3( meshAABB.mmax.x, meshAABB.mmax.y,  meshAABB.mmax.z),
			VC3( meshAABB.mmax.x, meshAABB.mmax.y,  meshAABB.mmin.z),
			VC3( meshAABB.mmax.x, meshAABB.mmin.y,  meshAABB.mmax.z),
			VC3( meshAABB.mmax.x, meshAABB.mmin.y,  meshAABB.mmin.z),
			VC3( meshAABB.mmin.x, meshAABB.mmax.y,  meshAABB.mmax.z),
			VC3( meshAABB.mmin.x, meshAABB.mmax.y,  meshAABB.mmin.z),
			VC3( meshAABB.mmin.x, meshAABB.mmin.y,  meshAABB.mmax.z),
			VC3( meshAABB.mmin.x, meshAABB.mmin.y,  meshAABB.mmin.z)
		};

		for(int i = 0; i < 8; i++)
		{
			mx.TransformVector( v[i] );
			bbox.mmin.x = min(bbox.mmin.x, v[i].x);
			bbox.mmin.y = min(bbox.mmin.y, v[i].y);
			bbox.mmin.z = min(bbox.mmin.z, v[i].z);
			bbox.mmax.x = max(bbox.mmax.x, v[i].x);
			bbox.mmax.y = max(bbox.mmax.y, v[i].y);
			bbox.mmax.z = max(bbox.mmax.z, v[i].z);
		}

		bounding_box = bbox;
		box_ok = true;
	}

	return bounding_box;
}

//------------------------------------------------------------------
// Storm3D_Model_Object::GetMesh
//------------------------------------------------------------------
IStorm3D_Mesh *Storm3D_Model_Object::GetMesh()
{
	return mesh;
}



//------------------------------------------------------------------
// Storm3D_Model_Object::SetMesh
//------------------------------------------------------------------
void Storm3D_Model_Object::SetMesh(IStorm3D_Mesh *_mesh)
{
	if(mesh)
		resourceManager.removeUser(mesh, this);

	mesh=(Storm3D_Mesh*)_mesh;
	if (mesh != NULL)
	{
		sphere_ok = false;
		box_ok = false;
		object_box_ok = false;
		const Sphere sphere = GetBoundingSphere();

		if(sphere_ok)
			parent_model->updateRadiusToContain(sphere.position, sphere.radius);
		else
			parent_model->updateRadiusToContain(position, mesh->GetRadius());
	}

	parent_model->updateEffectTexture();

	if(mesh)
		resourceManager.addUser(mesh, this);
}



//------------------------------------------------------------------
// Storm3D_Model_Object::GetMXG
//------------------------------------------------------------------
MAT &Storm3D_Model_Object::GetMXG()
{
	// Rebuild MX if needed
	if (mx_update)
	{
		mx_update=false;
		mxg_update=false;
		MAT ms,mr,mp;
		ms.CreateScaleMatrix(scale);
		mr.CreateRotationMatrix(rotation);
		mp.CreateTranslationMatrix(position);
		mx=ms*mr*mp;

		if (parent) 
			mxg=mx*parent->GetMXG();
		else if (parent_bone)
			mxg = mx*parent_bone->GetTM();
		else 
			mxg=mx*parent_model->GetMX();	// model is parent for top level
	}
	else
	// Rebuild MXG if needed
	if (mxg_update)
	{
		mxg_update=false;
		
		if (parent) 
			mxg=mx*parent->GetMXG();
		else if(parent_bone)
			mxg = mx*parent_bone->GetTM();
		else 
			mxg=mx*parent_model->GetMX();	// model is parent for top level
	}

	// Return MXG
	return mxg;
}



//------------------------------------------------------------------
// Storm3D_Model_Object::AddChild
//------------------------------------------------------------------
void Storm3D_Model_Object::AddChild(IStorm3D_Model_Object *iobject)
{
	Storm3D_Model_Object *object=(Storm3D_Model_Object*)iobject;

	// Break old link (if exists)
	if (object->parent) object->parent->RemoveChild(object);

	// Add child
	child_objects.insert(object);
	object->parent=this;
}



//------------------------------------------------------------------
// Storm3D_Model_Object::RemoveChild
//------------------------------------------------------------------
void Storm3D_Model_Object::RemoveChild(IStorm3D_Model_Object *iobject)
{
	Storm3D_Model_Object *object=(Storm3D_Model_Object*)iobject;

	child_objects.erase(object);
	object->parent=NULL;
}


//------------------------------------------------------------------
// Storm3D_Model_Object::AddChild
//------------------------------------------------------------------
void Storm3D_Model_Object::AddChild(IStorm3D_Helper *ihelper)
{
	switch (ihelper->GetHelperType())
	{
		case IStorm3D_Helper::HTYPE_POINT:
			if (((Storm3D_Helper_Point*)ihelper)->parent_object) ((Storm3D_Helper_Point*)ihelper)->parent_object->RemoveChild(ihelper);
			((Storm3D_Helper_Point*)ihelper)->parent_object=this;
			break;

		case IStorm3D_Helper::HTYPE_VECTOR:
			if (((Storm3D_Helper_Point*)ihelper)->parent_object) ((Storm3D_Helper_Vector*)ihelper)->parent_object->RemoveChild(ihelper);
			((Storm3D_Helper_Vector*)ihelper)->parent_object=this;
			break;
			
		case IStorm3D_Helper::HTYPE_CAMERA:
			if (((Storm3D_Helper_Camera*)ihelper)->parent_object) ((Storm3D_Helper_Camera*)ihelper)->parent_object->RemoveChild(ihelper);
			((Storm3D_Helper_Camera*)ihelper)->parent_object=this;
			break;
			
		case IStorm3D_Helper::HTYPE_BOX:
			if (((Storm3D_Helper_Point*)ihelper)->parent_object) ((Storm3D_Helper_Box*)ihelper)->parent_object->RemoveChild(ihelper);
			((Storm3D_Helper_Box*)ihelper)->parent_object=this;
			break;

		case IStorm3D_Helper::HTYPE_SPHERE:
			if (((Storm3D_Helper_Sphere*)ihelper)->parent_object) ((Storm3D_Helper_Sphere*)ihelper)->parent_object->RemoveChild(ihelper);
			((Storm3D_Helper_Sphere*)ihelper)->parent_object=this;
			break;
	}

	child_helpers.insert(ihelper);
}



//------------------------------------------------------------------
// Storm3D_Model_Object::RemoveChild
//------------------------------------------------------------------
void Storm3D_Model_Object::RemoveChild(IStorm3D_Helper *ihelper)
{
	child_helpers.erase(ihelper);

	switch (ihelper->GetHelperType())
	{
		case IStorm3D_Helper::HTYPE_POINT:
			((Storm3D_Helper_Point*)ihelper)->parent_object=NULL;
			break;

		case IStorm3D_Helper::HTYPE_VECTOR:
			((Storm3D_Helper_Vector*)ihelper)->parent_object=NULL;
			break;
			
		case IStorm3D_Helper::HTYPE_CAMERA:
			((Storm3D_Helper_Camera*)ihelper)->parent_object=NULL;
			break;
			
		case IStorm3D_Helper::HTYPE_BOX:
			((Storm3D_Helper_Box*)ihelper)->parent_object=NULL;
			break;

		case IStorm3D_Helper::HTYPE_SPHERE:
			((Storm3D_Helper_Sphere*)ihelper)->parent_object=NULL;
			break;
	}
}



//------------------------------------------------------------------
// Storm3D_Model_Object::InformChangeToChilds
// Called when object is rotated/scaled/moved
//------------------------------------------------------------------
void Storm3D_Model_Object::InformChangeToChilds()
{
	// Objects
	for(set<IStorm3D_Model_Object*>::iterator io=child_objects.begin();io!=child_objects.end();++io)
	{
		// Typecast to simplify code
		Storm3D_Model_Object *md=(Storm3D_Model_Object*)*io;

		// Set global update flags
		md->mxg_update=true;
		md->gpos_update_needed=true; //v3

		// (Recursively) inform child's childs etc... (if they havent been informed)
		if (!md->mx_update) 
			md->InformChangeToChilds();
	}

	// Helpers
	for(set<IStorm3D_Helper*>::iterator ih=child_helpers.begin();ih!=child_helpers.end();++ih)
	{
		switch ((*ih)->GetHelperType())
		{
			case IStorm3D_Helper::HTYPE_POINT:
				((Storm3D_Helper_Point*)(*ih))->update_globals=true;
				break;

			case IStorm3D_Helper::HTYPE_VECTOR:
				((Storm3D_Helper_Vector*)(*ih))->update_globals=true;
				break;
			
			case IStorm3D_Helper::HTYPE_CAMERA:
				((Storm3D_Helper_Camera*)(*ih))->update_globals=true;
				break;
			
			case IStorm3D_Helper::HTYPE_BOX:
				((Storm3D_Helper_Box*)(*ih))->update_globals=true;
				break;

			case IStorm3D_Helper::HTYPE_SPHERE:
				((Storm3D_Helper_Sphere*)(*ih))->update_globals=true;
				break;
		}
	}
}



//------------------------------------------------------------------
// Storm3D_Model_Object::SetPosition
//------------------------------------------------------------------
void Storm3D_Model_Object::SetPosition(const VC3 &_position)
{
	position=_position;

	// Inform child objects and lights of change
	// Only if they haven't been already informed (optimization)
	if (!mx_update) InformChangeToChilds();

	// Set update flags
	mx_update=true;
	mxg_update=true;
	gpos_update_needed=true; //v3
	sphere_ok = false;
	box_ok = false;

	if (mesh != NULL)
	{
		const Sphere &sphere = GetBoundingSphere();
		if(sphere_ok)
			parent_model->updateRadiusToContain(sphere.position, sphere.radius);
		else
			parent_model->updateRadiusToContain(position, mesh->GetRadius());
	}
}



//------------------------------------------------------------------
// Storm3D_Model_Object::SetRotation
//------------------------------------------------------------------
void Storm3D_Model_Object::SetRotation(const QUAT &_rotation)
{
	rotation=_rotation;

	// Inform child objects and lights of change
	// Only if they haven't been already informed (optimization)
	if (!mx_update) InformChangeToChilds();

	// Set update flags
	mx_update=true;
	mxg_update=true;
	gpos_update_needed=true; //v3
	sphere_ok = false;
	box_ok = false;
}



//------------------------------------------------------------------
// Storm3D_Model_Object::SetScale
//------------------------------------------------------------------
void Storm3D_Model_Object::SetScale(const VC3 &_scale)
{
	scale=_scale;

	// Inform child objects and lights of change
	// Only if they haven't been already informed (optimization)
	if (!mx_update) InformChangeToChilds();

	// Set update flag
	mx_update=true;
	mxg_update=true;
	gpos_update_needed=true; //v3
}



//------------------------------------------------------------------
// Storm3D_Model_Object::GetPosition
//------------------------------------------------------------------
VC3 &Storm3D_Model_Object::GetPosition()
{
	return position;
}



//------------------------------------------------------------------
// Storm3D_Model_Object::GetRotation
//------------------------------------------------------------------
QUAT &Storm3D_Model_Object::GetRotation()
{
	return rotation;
}



//------------------------------------------------------------------
// Storm3D_Model_Object::GetScale
//------------------------------------------------------------------
VC3 &Storm3D_Model_Object::GetScale()
{
	return scale;
}



//------------------------------------------------------------------
// Storm3D_Model_Object::GetGlobalPosition
//------------------------------------------------------------------
VC3 Storm3D_Model_Object::GetGlobalPosition()
{
	// v3 optimization
	if (gpos_update_needed)
	{
		if (parent) 
			gpos=parent->GetMXG().GetTransformedVector(position);
		else if(parent_bone)
			gpos=parent_bone->GetTM().GetTransformedVector(position);
		else 
			gpos=parent_model->GetMX().GetTransformedVector(position);
		gpos_update_needed=false;
	}
	return gpos;
}

IStorm3D_Bone *Storm3D_Model_Object::GetParentBone()
{
	return parent_bone;
}


//------------------------------------------------------------------
// Storm3D_Model_Object::CopyFrom
//------------------------------------------------------------------
void Storm3D_Model_Object::CopyFrom(IStorm3D_Model_Object *iother, bool only_render_settings)
{
	// Test
	if (iother==NULL) return;

	Storm3D_Model_Object *other=(Storm3D_Model_Object*)iother;

	if(!only_render_settings)
	{
		mesh=other->mesh;

		position=other->position;
		rotation=other->rotation;
		scale=other->scale;

		no_collision=other->no_collision;
		no_render=other->no_render;
		light_object = other->light_object;

		if (other->parent) other->parent->AddChild(this);

		gpos_update_needed=true;
		mx_update=true;
		mxg_update=true;
		sphere_ok = false;
		object_box_ok = false;
		box_ok = false;
		InformChangeToChilds();
	}

  renderPassMask = other->renderPassMask;
	for(int i = 0; i < RENDER_PASS_BITS_AMOUNT; i++)
	{
		renderPassOffset[i] = other->renderPassOffset[i];
		renderPassScale[i] = other->renderPassOffset[i];
	}

	alphaTestPassConditional = other->alphaTestPassConditional;
	alphaTestValue = other->alphaTestValue;
}



//------------------------------------------------------------------
// Storm3D_Model_Object::RayTrace
//------------------------------------------------------------------
void Storm3D_Model_Object::RayTrace(const VC3 &position,const VC3 &direction_normalized,float ray_length,Storm3D_CollisionInfo &rti, bool accurate)
{
	if(!mesh) 
		return;
	if(parent_model->no_collision)
		return;
	if(rti.onlySolidSurfaces && mesh && mesh->GetMaterial() && mesh->GetMaterial()->GetAlphaType() != IStorm3D_Material::ATYPE_NONE)
		return;

	// Test if the ray intersects object's bounding sphere
	// v3: 7 multiplies (old: 8 mul + 1 sqrt)
	VC3 vectoray(position - GetGlobalPosition());
	float vectoraylen2=vectoray.GetSquareLength();							// 3 mul
	float kantalen=direction_normalized.GetDotWith(vectoray);			// 3 mul
	float dist2=vectoraylen2-kantalen*kantalen;								// 1 mul

	if (dist2> mesh->GetSquareRadius()) return;

	// psd: 
	// Added these two checks. insane to start doing per-polygon before these

	// Sphere behind ray?
	if((kantalen > 0) && (vectoraylen2 > mesh->GetSquareRadius()))
		return;

	// Out of range
	// fixed: was ray_length, not ray_length squared as it should be.
	// fixed: was ray^2 > len^2+radius^2, should be (ray+radius)^2 < len^2.
	// -- jpk
	float meshRadiusSq = mesh->GetSquareRadius();
	if((ray_length + meshRadiusSq) * (ray_length + meshRadiusSq) < vectoraylen2)
		return;


	Sphere sphere = GetBoundingSphere();
	//GetMXG().TransformVector(sphere.position);
	parent_model->mx.TransformVector(sphere.position);
	if(!collision(sphere, Ray(position, direction_normalized, ray_length)))
		return;

	if (sphere_collision_only && !accurate)
	{
		VC3 pos = GetGlobalPosition() - position;
		// notice: this is not the actual hit range, but the object's range
		// but that's close enough for us.
		float sphereRange = pos.GetLength();
		if ((sphereRange < rti.range) && (sphereRange < ray_length) && (sphereRange > 0))
		{      
			rti.hit = true;
			rti.range = sphereRange;
			rti.position = position + direction_normalized * sphereRange;
			rti.object = this;
			rti.model = parent_model;

			// well, really don't know what to put here, just stuff 
			// something there, we are not interested about it anyway... ;)
			rti.plane_normal=VC3();
		}

		return;
	}

	// Transform ray to object's coordinates
	Matrix inverse = GetMXG().GetInverse();
	VC3 tr_pos=inverse.GetTransformedVector(position);
	VC3 tr_dir=inverse.GetWithoutTranslation().GetTransformedVector(direction_normalized);

	// First check AABB
	{
		Ray ray(tr_pos, tr_dir, ray_length);
		const AABB &aabb = mesh->getBoundingBox();
		
		if(!collision(aabb, ray))
			return;
	}

	// Raytrace
	if (mesh->RayTrace(tr_pos,tr_dir,ray_length,rti, accurate))
	{
		rti.object=this;
		rti.model=parent_model;
		rti.terrainInstanceId = parent_model->terrainInstanceId;
		rti.terrainModelId = parent_model->terrainModelId;
		GetMXG().TransformVector(rti.position);
		//GetMXG().GetWithoutTranslation().TransformVector(rti.plane_normal);
		GetMXG().RotateVector(rti.plane_normal);
	}
}



//------------------------------------------------------------------
// Storm3D_Model_Object::SphereCollision
//------------------------------------------------------------------
void Storm3D_Model_Object::SphereCollision(const VC3 &position,float radius,Storm3D_CollisionInfo &cinf, bool accurate)
{
	// Needs a mesh to collide!!
	if(!mesh) 
		return;

	// If object has no_collision skip it
	if (no_collision) return;

	// If object is too far away do not test it
	float r=(mesh->GetRadius()+radius);
	if (position.GetSquareRangeTo(GetGlobalPosition())>r*r) return;

	Sphere sphere = GetBoundingSphere();
	parent_model->mx.TransformVector(sphere.position);
	if(!collision(sphere, Sphere(position, radius)))
		return;

	// Transform sphere to object's coordinates
	VC3 tr_pos=GetMXG().GetInverse().GetTransformedVector(position);

	// First check AABB
	{
		Sphere sphere(tr_pos, radius);
		const AABB &aabb = mesh->getBoundingBox();

		if(!collision(aabb, sphere))
			return;
	}

	// Raytrace
	if (mesh->SphereCollision(tr_pos,radius,cinf, accurate))
	{
		cinf.object=this;
		cinf.model=parent_model;
		GetMXG().TransformVector(cinf.position);
		GetMXG().GetWithoutTranslation().TransformVector(cinf.plane_normal);
	}
}



//------------------------------------------------------------------
// Storm3D_Model_Object::SetName
//------------------------------------------------------------------
void Storm3D_Model_Object::SetName(const char *_name)
{
	distortion_only = false;
	if(!_name)
		return;

	delete[] name;
	name=new char[strlen(_name)+1];
	strcpy(name,_name);

	if(strstr(name, "DistortionOnly"))
		distortion_only = true;
}



//------------------------------------------------------------------
// Storm3D_Model_Object::GetName
//------------------------------------------------------------------
const char *Storm3D_Model_Object::GetName()
{
	return name;
}


//------------------------------------------------------------------
// Storm3D_Model_Object - Change special properties
//------------------------------------------------------------------
void Storm3D_Model_Object::SetNoCollision(bool no_collision_)
{
	if(no_collision != no_collision_)
	{
		if(no_collision_)
			parent_model->RemoveCollision(this);
		else
		{
			parent_model->SetCollision(this);
			mesh->UpdateCollisionTable();
		}
	}

	no_collision = no_collision_;
}


void Storm3D_Model_Object::SetNoRender(bool _no_render)
{
	no_render=_no_render;
}


void Storm3D_Model_Object::SetSphereCollisionOnly(bool sphere_collision_only)
{
	this->sphere_collision_only = sphere_collision_only;
}

void Storm3D_Model_Object::UpdateVisibility()
{
	if(parent_model && parent_model->observer)
		parent_model->observer->updateVisibility(this);
}

// Get special properties
bool Storm3D_Model_Object::GetNoCollision() const
{
	return no_collision;
}

bool Storm3D_Model_Object::GetNoRender() const
{
	return no_render;
}

bool Storm3D_Model_Object::IsLightObject() const
{
	return light_object;
}

void Storm3D_Model_Object::SetAsLightObject()
{
	light_object = true;

	std::set<IStorm3D_Model_Object *>::iterator it = parent_model->objects.begin();
	for(; it != parent_model->objects.end(); ++it)
	{
		Storm3D_Model_Object *o = static_cast<Storm3D_Model_Object *> (*it);

		if(!o->light_object)
			parent_model->light_objects.erase(o);
	}
}


void Storm3D_Model_Object::EnableRenderPass(int renderPassBit)
{
	this->renderPassMask |= (1 << renderPassBit);
}

void Storm3D_Model_Object::SetRenderPassParams(int renderPassBit, const VC3 &offset, const VC3 &scale)
{
	assert(renderPassBit >= 0 && renderPassBit < RENDER_PASS_BITS_AMOUNT);
	this->renderPassOffset[renderPassBit] = offset;
	this->renderPassScale[renderPassBit] = scale;	
}

void Storm3D_Model_Object::SetAlphaTestPassParams(bool conditional, int alphaValue)
{
	this->alphaTestPassConditional = conditional;
	this->alphaTestValue = alphaValue;	
}


void Storm3D_Model_Object::DisableRenderPass(int renderPassBit)
{
	this->renderPassMask &= ~(1 << renderPassBit);
}

int Storm3D_Model_Object::GetRenderPassMask()
{
	return this->renderPassMask;
}

void Storm3D_Model_Object::SetRenderPassMask(int renderPassMask)
{
	this->renderPassMask = renderPassMask;
}

