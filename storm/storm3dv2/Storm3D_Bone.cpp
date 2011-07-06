// Copyright 2002-2004 Frozenbyte Ltd.

#pragma warning(disable:4103)

//------------------------------------------------------------------
// Includes
//------------------------------------------------------------------
#include "Storm3D_Bone.h"
#include "storm3d.h"
#include "VertexFormats.h"
#include <cassert>
#include <stdio.h>

#include "storm3d_model_object.h"
#include "storm3d_helper.h"
#include "storm3d_light.h"
#include "..\..\filesystem\input_stream_wrapper.h"
#include "..\..\util\Debug_MemoryManager.h"

int storm3d_bone_allocs = 0;
using namespace frozenbyte;

//------------------------------------------------------------------
// Storm3D_BoneAnimation
//------------------------------------------------------------------
Storm3D_BoneAnimation::Storm3D_BoneAnimation(const char *filename)
:	reference_count(1), 
	bone_id(0),

	loop_time(0)
{
	successfullyLoaded = false;

	bool combine = false;
	std::string combineWithFilename;
	std::vector<std::string> combineBoneNames;

	char actual_filename[512+1];
	int filename_len = strlen(filename);
	if (filename_len < 512) 
	{
		strcpy(actual_filename, filename);
		for (int i = 0; i < filename_len; i++)
		{
			if (actual_filename[i] == '@')
			{
				int restoreAtPos = -1;
				for (int j = i+1; j < filename_len; j++)
				{
					if (actual_filename[j] == '@')
					{
						actual_filename[j] = '\0';
						restoreAtPos = j;
						break;
					}
				}
				actual_filename[i] = '\0';

				combine = true;

				bool gotFirstOne = false;
				int lastCommaPos = i;
				for (int j = i + 1; j < filename_len + 1; j++)
				{
					if (actual_filename[j] == '+'
						|| actual_filename[j] == '\0')
					{
						// interpret first token as filename, rest as bone names
						if (!gotFirstOne)
						{
							// file name
							assert(j >= (lastCommaPos + 1));
							combineWithFilename = std::string(&actual_filename[lastCommaPos + 1]).substr(0, j - (lastCommaPos + 1));
							if (combineWithFilename.empty())
							{
								assert(!"Storm3D_BoneAnimation - Combine filename empty.");
								/*
								if (Storm3D2->getLogger() != NULL)
								{
									Storm3D2->getLogger()->warning("Storm3D_BoneAnimation - Combine filename empty.");
									Storm3D2->getLogger()->debug(filename);
								}
								*/
							}
							lastCommaPos = j;
							gotFirstOne = true;
						} else {
							// bone name
							assert(j >= (lastCommaPos + 1));
							std::string tmp = std::string(&actual_filename[lastCommaPos + 1]).substr(0, j - (lastCommaPos + 1));
							if (tmp.empty())
							{
								assert(!"Storm3D_BoneAnimation - Combine bone name empty.");
								/*
								if (Storm3D2->getLogger() != NULL)
								{
									Storm3D2->getLogger()->warning("Storm3D_Model::LoadBones - Combine bone name empty.");
									Storm3D2->getLogger()->debug(filename);
								}
								*/
							} else {
								combineBoneNames.push_back(tmp);
							}
							lastCommaPos = j;							
						}
						if (actual_filename[j] == '\0')
							break;
					}
				}
				if (restoreAtPos != -1)
				{
					actual_filename[restoreAtPos] = '@';
				}
			}	
		}
	} else {
		assert(0);
		return;
	}

	std::vector<std::string> boneNamesByNumber;
	if (combine)
	{
		/*
		if (Storm3D2->getLogger() != NULL)
		{
			Storm3D2->getLogger()->debug((std::string("Storm3D_BoneAnimation - Combining with animation: ") + combineWithFilename).c_str());
			for (int i = 0; i < (int)combineBoneNames.size(); i++)
			{
				Storm3D2->getLogger()->debug((std::string("bone: ") + combineBoneNames[i]).c_str());
			}
		}
		*/
		// HACK: since the animation file itself does not contain bone names, i'm reading in 
		// a hacky file which contains the names.
		// TODO: should modify the animation file format and exporter instead (yikes!)
		std::string bonenames = actual_filename;
		for (int i = (int)bonenames.length(); i >= 0; i--)
		{
			if (bonenames[i] == '/'
				|| bonenames[i] == '\\')
			{
				std::string foldername = bonenames.substr(0, i + 1);
				bonenames = foldername + "bone_names";

				if (combineWithFilename.length() >= 5 
					&& combineWithFilename.substr(0, 5) == "data/")
				{
					// absolute path for another animation, so do nothing to it
				} else {
					// relative path
					combineWithFilename = foldername + combineWithFilename;
				}
				break;
			}
		}
		bonenames += ".bnam";

		filesystem::FB_FILE *fp = filesystem::fb_fopen(bonenames.c_str(), "rb");
		if(fp != NULL)
		{
			int filesize = filesystem::fb_fsize(fp);
			char *buf = new char[filesize + 1];
			filesystem::fb_fread(buf, filesize, 1, fp);
			buf[filesize] = '\0';

			int lastPos = 0;
			for (int i = 0; i < filesize; i++)
			{
				if (buf[i] == '\r' || buf[i] == '\n')
				{
					buf[i] = '\0';
					if (i > lastPos)
					{
						std::string tmp = &buf[lastPos];
						boneNamesByNumber.push_back(tmp);
					}
					lastPos = i + 1;
				}
			}

			delete [] buf;
			filesystem::fb_fclose(fp);
		} else {
			assert(!"Storm3D_BoneAnimation - No bone names file found.");
		}
	}
		

	filesystem::FB_FILE *fp = filesystem::fb_fopen(actual_filename, "rb");
	if(fp == 0)
		return;

	char header[5] = { 0 };
	filesystem::fb_fread(header, sizeof(char), 5, fp);
	if((memcmp(header,"ANM11",5) != 0))
	{
		filesystem::fb_fclose(fp);
		return;
	}

	// Bone id
	filesystem::fb_fread(&bone_id, sizeof(int), 1, fp);

	// Loop time in ms
	filesystem::fb_fread(&loop_time, sizeof(int), 1, fp);
	
	int bone_count = 0;
	filesystem::fb_fread(&bone_count, sizeof(int), 1, fp);

	if (combine)
	{
		// TODO: need a logger instead of assert.
		assert(boneNamesByNumber.size() == bone_count);
	}

	// resize containers
	bone_rotations.resize(bone_count);
	bone_positions.resize(bone_count);

	// For every bone
	for(int i = 0; i < bone_count; ++i)
	{
		int key_count = 0;
		filesystem::fb_fread(&key_count, sizeof(int), 1, fp);
		
		int position_key_count = 0;
		filesystem::fb_fread(&position_key_count, sizeof(int), 1, fp);

		for(int j = 0; j < key_count; ++j)
		{
			int time;
			Rotation rotation;
			Vector position;

			filesystem::fb_fread(&time, sizeof(int), 1, fp);
			filesystem::fb_fread(&rotation, sizeof(float), 4, fp);

			bone_rotations[i].push_back(std::pair<int,Rotation>(time,rotation));

			if(position_key_count > 0)
			{
				filesystem::fb_fread(&position, sizeof(float), 3, fp);
				bone_positions[i].push_back(std::pair<int,Vector>(time,position));
			}
		}

		if (combine
			&& boneNamesByNumber.size() == bone_count)
		{
			for (int bn = 0; bn < (int)combineBoneNames.size(); bn++)
			{
				if (boneNamesByNumber[i] == combineBoneNames[bn])
				{
					// replace this bone animation data with another...

					// HACK: copy&paste ahead. should definately refactor this whole thing...
					filesystem::FB_FILE *fp_rep = filesystem::fb_fopen(combineWithFilename.c_str(), "rb");
					if(fp_rep != 0)
					{
						char header_rep[5] = { 0 };
						filesystem::fb_fread(header_rep, sizeof(char), 5, fp_rep);
						if((memcmp(header_rep,"ANM11",5) != 0))
						{
							filesystem::fb_fclose(fp_rep);
							return;
						}

						int dummy1;
						int dummy2;

						// Bone id
						filesystem::fb_fread(&dummy1, sizeof(int), 1, fp_rep);

						// Loop time in ms
						filesystem::fb_fread(&dummy2, sizeof(int), 1, fp_rep);
						
						int bone_count_rep = 0;
						filesystem::fb_fread(&bone_count_rep, sizeof(int), 1, fp_rep);

						assert(bone_count_rep == bone_count);

						// For every bone
						for(int i_rep = 0; i_rep < bone_count_rep; ++i_rep)
						{
							int key_count_rep = 0;
							filesystem::fb_fread(&key_count_rep, sizeof(int), 1, fp_rep);
							
							int position_key_count_rep = 0;
							filesystem::fb_fread(&position_key_count_rep, sizeof(int), 1, fp_rep);

							if (i_rep == i)
							{
								bone_rotations[i].clear();
								bone_positions[i].clear();
							}

							for(int j_rep = 0; j_rep < key_count_rep; ++j_rep)
							{
								int time_rep;
								Rotation rotation_rep;
								Vector position_rep;

								filesystem::fb_fread(&time_rep, sizeof(int), 1, fp_rep);
								filesystem::fb_fread(&rotation_rep, sizeof(float), 4, fp_rep);

								if (i_rep == i)
								{
									bone_rotations[i].push_back(std::pair<int,Rotation>(time_rep,rotation_rep));
								}

								if(position_key_count_rep > 0)
								{
									filesystem::fb_fread(&position_rep, sizeof(float), 3, fp_rep);
									if (i_rep == i)
									{
										bone_positions[i].push_back(std::pair<int,Vector>(time_rep,position_rep));
									}
								}
							}
						}

						filesystem::fb_fclose(fp_rep);
					} else {
						assert(!"Storm3D_BoneAnimation - Failed to open animation file used in combine.");
					}
					// (end of copy&paste)

					break;
				}
			}
		}

	}

	filesystem::fb_fclose(fp);
	successfullyLoaded = true;	
}

Storm3D_BoneAnimation::~Storm3D_BoneAnimation()
{
}


bool Storm3D_BoneAnimation::WasSuccessfullyLoaded() const
{
	return successfullyLoaded;
}

void Storm3D_BoneAnimation::AddReference()
{
	++reference_count;
}

void Storm3D_BoneAnimation::Release()
{
	if(--reference_count == 0)
		delete this;
}

int Storm3D_BoneAnimation::GetLength()
{
	return loop_time;
}

int Storm3D_BoneAnimation::GetId()
{
	return bone_id;
}

// Search for key index
// ToDo: change to binary seach
template<class T>
static int SearchIndexNext(const std::vector<T> &keys, int time)
{
/*
	for(int i = 0; i < keys.size(); ++i)
	{
		if(keys[i].first > time)
			return i;
	}

	return 0;
*/
	// Constant fps 25
	int index =  (time / 40) + 1;
	if(index < int(keys.size()))
		return index;

	return 0;
}

bool Storm3D_BoneAnimation::GetRotation(int bone_index, int time, Rotation *result) const
{
	time %= loop_time;

	// Check
	assert(int(bone_rotations.size()) > bone_index);
	const std::vector<BoneRotationKey> &bone_keys = bone_rotations[bone_index];
	if(bone_keys.size() < 2)
		return false;

	// Search key index
	int next_index = SearchIndexNext(bone_keys, time);
	
	// Properties
	const BoneRotationKey &next_key = bone_keys[next_index];
	BoneRotationKey prev_key;
	float slerp_amount = 0;

	if(next_key.first > time)
	{
		prev_key = bone_keys[next_index - 1];
		slerp_amount = (float) (time - prev_key.first) / (float) (next_key.first - prev_key.first);
	}
	else // between first and last
	{
		prev_key = bone_keys[bone_keys.size() - 1];
		int foo = loop_time - prev_key.first;
		float step = float(foo + next_key.first);
		
		if(time >= prev_key.first)
			slerp_amount = 1 - (loop_time - time) / step;
		else
			slerp_amount = (time + foo) / step;;
	}

	assert(slerp_amount > -0.01f);
	assert(slerp_amount <  1.01f);

	// Slerp (interpolate). Now this is final rotation
	*result = prev_key.second.GetSLInterpolationWith(next_key.second, slerp_amount);
	//*result = prev_key.second;
	return true;
}

bool Storm3D_BoneAnimation::GetPosition(int bone_index, int time, Vector *result) const
{
	time %= loop_time;

	// Check
	assert(int(bone_positions.size()) > bone_index);
	const std::vector<BonePositionKey> &bone_keys = bone_positions[bone_index];
	if(bone_keys.size() < 2)
		return false;

	// Search key index
	int next_index = SearchIndexNext(bone_keys, time);

	// Properties
	const BonePositionKey &next_key = bone_keys[next_index];
	BonePositionKey prev_key;
	float interpolate_amount = 0;

	if(next_key.first > time)
	{
		prev_key = bone_keys[next_index - 1];
		interpolate_amount = (float) (time - prev_key.first) / (float) (next_key.first - prev_key.first);
	}
	else // between first and last
	{
		prev_key = bone_keys[bone_keys.size() - 1];
		int foo = loop_time - prev_key.first;
		float step = float(foo + next_key.first);
		
		if(time >= prev_key.first)
			interpolate_amount = 1 - (loop_time - time) / step;
		else
			interpolate_amount = (time + foo) / step;;
	}
	
	assert(interpolate_amount > -0.01f);
	assert(interpolate_amount <  1.01f);

	*result = prev_key.second * (1.f - interpolate_amount);
	*result += next_key.second * interpolate_amount;
	//*result = prev_key.second;

	return true;
}


//------------------------------------------------------------------
// Storm3D_Bone
//------------------------------------------------------------------
Storm3D_Bone::Storm3D_Bone()
:	name(NULL),
	global_tm_ok(false),
	has_childs(false),
	useForceTransform(false),
	noCollision(false)

{
	storm3d_bone_allocs++;
}

Storm3D_Bone::~Storm3D_Bone()
{
	storm3d_bone_allocs--;
	delete[] name;

	{
		std::set<Storm3D_Model_Object *>::iterator it = objects.begin();
		for(; it != objects.end(); ++it)
		{
			Storm3D_Model_Object *o = *it;
			o->parent_bone = 0;
		}
	}

	{
		std::set<Storm3D_Helper_AInterface *>::iterator it = helpers.begin();
		for(; it != helpers.end(); ++it)
		{
			Storm3D_Helper_AInterface *helper = *it;
			helper->parent_bone = 0;
		}
	}
}

QUAT Storm3D_Bone::GetOriginalGlobalRotation()
{
	return original_inverse_tm.GetRotation().GetInverse();
}

void Storm3D_Bone::SetOriginalProperties(const Vector &position, const Rotation &rotation, const Vector &model_position, const Rotation &model_rotation)
{
	this->position = position;
	this->rotation = rotation;

	// We need original orientations inverse to create vertex tm
	Matrix r;
	r.CreateRotationMatrix(model_rotation);
	Matrix t;
	t.CreateTranslationMatrix(model_position);
	
	original_inverse_tm = r * t;
	original_inverse_tm.Inverse();
}

void Storm3D_Bone::SetSpecialProperties(float length_, float thickness_)
{
	length = length_;
	thickness = thickness_;
}

void Storm3D_Bone::SetParentIndex(int index)
{
	parent_index = index;
}

void Storm3D_Bone::SetNoCollision(bool noCollision)
{
	this->noCollision = noCollision;
}

/*
float Storm3D_Bone::GetLenght()
{
	return length;
}

float Storm3D_Bone::GetThickness()
{
	return thickness;
}

int Storm3D_Bone::GetParentIndex()
{
	return parent_index;
}
*/
void Storm3D_Bone::AnimatePosition(const Vector &animated_position, float interpolate_amount, bool interpolate)
{
	if(interpolate == false)
		current_position = animated_position;
	else
	{
		current_position *= 1.f - interpolate_amount;
		current_position += animated_position * interpolate_amount;
	}
}

void Storm3D_Bone::AnimateRotation(const Rotation &animated_rotation, float interpolate_amount, bool interpolate)
{
	if(interpolate == false)
		current_rotation = animated_rotation;
	else
		current_rotation = current_rotation.GetSLInterpolationWith(animated_rotation, interpolate_amount);
}

void Storm3D_Bone::AnimateRelativeRotation(const Rotation &animated_rotation, float interpolate_amount, bool interpolate)
{
	Rotation relative_rotation = animated_rotation * rotation.GetInverse();
	Rotation final_rotation = relative_rotation * current_rotation;

	if(interpolate == false)
		current_rotation = final_rotation;
	else
		current_rotation = current_rotation.GetSLInterpolationWith(final_rotation, interpolate_amount);
}

void Storm3D_Bone::SetName(const char *name)
{
	delete[] this->name;
	this->name = new char[strlen(name) + 1];
	
	strcpy(this->name, name);
}

const char *Storm3D_Bone::GetName() const
{
	return name;
}

void Storm3D_Bone::ParentMoved(const Matrix &new_parent_tm)
{
	parent_tm = new_parent_tm;
	global_tm_ok = false;
	vertex_tm_ok = false;

	InformChangeToChilds();
}

void Storm3D_Bone::ParentPositionMoved(const VC3 &pos)
{
	// HAX to update bone helper _positions_ instantly
	global_tm_ok = false;
	vertex_tm_ok = false;

	parent_tm.Set(12, pos.x);
	parent_tm.Set(13, pos.y);
	parent_tm.Set(14, pos.z);

	for(std::set<Storm3D_Helper_AInterface *>::iterator ih = helpers.begin(); ih != helpers.end(); ++ih)
		(*ih)->update_globals = true;
}

void Storm3D_Bone::InformChangeToChilds()
{
	// This cyclic, friend trusting systems just sucks
	for(std::set<Storm3D_Model_Object *>::iterator it = objects.begin(); it != objects.end(); ++it)
	{
		(*it)->mxg_update = true;
		(*it)->gpos_update_needed = true;
	}

	// Helpers
	for(std::set<Storm3D_Helper_AInterface *>::iterator ih = helpers.begin(); ih != helpers.end(); ++ih)
		(*ih)->update_globals = true;
}

void Storm3D_Bone::AddChild(IStorm3D_Model_Object *object)
{
	Storm3D_Model_Object *o = static_cast<Storm3D_Model_Object*> (object);

	o->parent = 0;
	o->parent_bone = this;

	objects.insert(o);
	has_childs = true;

	InformChangeToChilds();
}

void Storm3D_Bone::AddChild(IStorm3D_Helper *helper_)
{
/*
	// Reinterpret cast since AInterface should be derived from IHelper
	// This is safe anyway (as far as reinterpret's can)
	Storm3D_Helper_AInterface *helper = reinterpret_cast<Storm3D_Helper_AInterface *> (helper_);
*/
	Storm3D_Helper_AInterface *helper = 0;

	switch(helper_->GetHelperType())
	{
		case IStorm3D_Helper::HTYPE_POINT:
		{
			helper = static_cast<Storm3D_Helper_AInterface *> (static_cast<Storm3D_Helper_Point *> (helper_));
			break;
		}
		case IStorm3D_Helper::HTYPE_VECTOR:
		{
			helper = static_cast<Storm3D_Helper_AInterface *> (static_cast<Storm3D_Helper_Vector *> (helper_));
			break;
		}
		case IStorm3D_Helper::HTYPE_BOX:
		{
			helper = static_cast<Storm3D_Helper_AInterface *> (static_cast<Storm3D_Helper_Box *> (helper_));
			break;
		}
		case IStorm3D_Helper::HTYPE_CAMERA:
		{
			helper = static_cast<Storm3D_Helper_AInterface *> (static_cast<Storm3D_Helper_Camera *> (helper_));
			break;
		}
		case IStorm3D_Helper::HTYPE_SPHERE:
		{
			helper = static_cast<Storm3D_Helper_AInterface *> (static_cast<Storm3D_Helper_Sphere *> (helper_));
			break;
		}

		default:
			assert(!"What? Who added class to hierarchy!");
	}

	helper->parent_object = 0;
	helper->parent_bone = this;

	helpers.insert(helper);
	has_childs = true;

	InformChangeToChilds();
}

void Storm3D_Bone::RemoveChild(IStorm3D_Model_Object *object)
{
	Storm3D_Model_Object *o = static_cast<Storm3D_Model_Object*> (object);
	o->parent_bone = 0;

	objects.erase(o);
}

void Storm3D_Bone::RemoveChild(IStorm3D_Helper *helper_)
{
	Storm3D_Helper_AInterface *helper = 0;

	switch(helper_->GetHelperType())
	{
		case IStorm3D_Helper::HTYPE_POINT:
		{
			helper = static_cast<Storm3D_Helper_AInterface *> (static_cast<Storm3D_Helper_Point *> (helper_));
			break;
		}
		case IStorm3D_Helper::HTYPE_VECTOR:
		{
			helper = static_cast<Storm3D_Helper_AInterface *> (static_cast<Storm3D_Helper_Vector *> (helper_));
			break;
		}
		case IStorm3D_Helper::HTYPE_BOX:
		{
			helper = static_cast<Storm3D_Helper_AInterface *> (static_cast<Storm3D_Helper_Box *> (helper_));
			break;
		}
		case IStorm3D_Helper::HTYPE_CAMERA:
		{
			helper = static_cast<Storm3D_Helper_AInterface *> (static_cast<Storm3D_Helper_Camera *> (helper_));
			break;
		}
		case IStorm3D_Helper::HTYPE_SPHERE:
		{
			helper = static_cast<Storm3D_Helper_AInterface *> (static_cast<Storm3D_Helper_Sphere *> (helper_));
			break;
		}

		default:
			assert(!"What? Who added class to hierarchy!");
	}

	helper->parent_bone = 0;
	helpers.erase(helper);
}

void Storm3D_Bone::SetForceTransformation(bool useForceSettings, const VC3 &position, const QUAT &rotation)
{
	forcePosition = position;
	forceRotation = rotation;
	useForceTransform = useForceSettings;
}

void Storm3D_Bone::TransformBones(std::vector<Storm3D_Bone*> *bones)
{
	Matrix t;

	for(unsigned int index = 0; index < bones->size(); ++index)
	{
		Storm3D_Bone *b = (*bones)[index];

		if(b->useForceTransform)
		{
			b->model_tm.CreateRotationMatrix(b->forceRotation);
			t.CreateTranslationMatrix(b->forcePosition);
			//b->model_tm = t;
			b->model_tm.Multiply(t);
		}
		else
		{
			b->model_tm.CreateRotationMatrix(b->current_rotation);
			t.CreateTranslationMatrix(b->current_position);
			b->model_tm.Multiply(t);
			
			// Apply hierarchy
			if(b->parent_index != -1)
			{
				assert(b->parent_index < int(index));
				b->model_tm.Multiply((*bones)[b->parent_index]->model_tm);
			}
		}
		
		b->global_tm_ok = false;
		b->vertex_tm_ok = false;
		
		b->InformChangeToChilds();
	}

	for(unsigned int index = 0; index < bones->size(); ++index)
	{
		Storm3D_Bone *b = (*bones)[index];

		b->global_tm_ok = false;
		b->vertex_tm_ok = false;
		// Temp
		b->current_rotation = b->rotation;

		b->InformChangeToChilds();
	}

}
