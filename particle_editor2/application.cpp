// Copyright 2002-2004 Frozenbyte Ltd.

#include "application.h"
#include "../editor/string_conversions.h"
#include "../editor/parser.h"
#include "../editor/window.h"
#include "../editor/storm.h"
#include "../editor/dialog.h"
#include "../editor/dialog_utils.h"
#include "../editor/camera.h"
#include "../editor/mouse.h"
#include "../editor/icommand.h"
#include "../editor/command_list.h"
#include "../editor/common_dialog.h"
#include "../editor/color_component.h"
#include "../editor/color_picker.h"
#include "../filesystem/input_file_stream.h"
#include "../filesystem/file_package_manager.h"
#include "../filesystem/standard_package.h"
#include "../util/SoundMaterialParser.h"
#include <istorm3d_mesh.h>
#include <istorm3d.h>
#include <istorm3d_scene.h>
#include <istorm3d_terrain_renderer.h>
#include "resource.h"
#include "track.h"
#include "particlesystem.h"
#include "particleforces.h"
#include <stdio.h>
#include "../viewer/model.h"

#ifdef PHYSICS_PHYSX
  #include "particlephysics.h"
	#include "../physics/physics_lib.h"
	#include "../physics/box_actor.h"
	#include "../physics/convex_actor.h"
	#include "../physics/capsule_actor.h"
	#include "../physics/cooker.h"
	#include "../physics/visualizer.h"
	#include "../physics/spherical_joint.h"
#endif

#pragma comment(lib, "storm3dv2.lib")

#include <fstream>
#include <windowsx.h>
#include "parseutil.h"
#include "paramUI.h"
#include <list>
#include <tchar.h>
#include "particleeffect.h"
#include "orbitcamera.h"
#include "crappyfont.h"

#define PARTICLE_EDITOR_VISUALIZE_RANGE 30

namespace frozenbyte {
namespace particle {
namespace {

using namespace frozenbyte::editor;


	struct GenParticleSystemParamDesc {
		std::vector<ParamDesc> params;
		GenParticleSystemParamDesc() 
		{
			params.push_back(ParamDesc("emit_rate", IDC_EMIT_RATE, PARAM_FLOAT));
			params.push_back(ParamDesc("emit_start", IDC_EMIT_START, PARAM_FLOAT));
			params.push_back(ParamDesc("emit_stop", IDC_EMIT_STOP, PARAM_FLOAT));
			params.push_back(ParamDesc("max_particles", IDC_MAX_PARTICLES, PARAM_INT));
			params.push_back(ParamDesc("die_after_emission", IDC_DIE_AFTER_EMISSION, PARAM_INT));
			params.push_back(ParamDesc("velocity_inheritance_factor", IDC_VELOCITY_INHERITANCE_FACTOR, PARAM_FLOAT));
			params.push_back(ParamDesc("emitter_position", IDC_EMITTER_POSITION_X, IDC_EMITTER_POSITION_Y, 
				IDC_EMITTER_POSITION_Z));
			params.push_back(ParamDesc("emitter_variation", IDC_EMITTER_VARIATION_X, IDC_EMITTER_VARIATION_Y, 
				IDC_EMITTER_VARIATION_Z));
			params.push_back(ParamDesc("default_direction", IDC_DEFAULT_DIRECTION_X, IDC_DEFAULT_DIRECTION_Y, 
				IDC_DEFAULT_DIRECTION_Z));
			params.push_back(ParamDesc("launch_speed", IDC_LAUNCH_SPEED, PARAM_FLOAT));
			params.push_back(ParamDesc("launch_speed_var", IDC_LAUNCH_SPEED_VAR, PARAM_FLOAT));
			params.push_back(ParamDesc("particle_start_angle", IDC_PARTICLE_ANGLE, PARAM_FLOAT));
			params.push_back(ParamDesc("particle_start_angle_var", IDC_PARTICLE_ANGLE_VAR, PARAM_FLOAT));
			params.push_back(ParamDesc("particle_spin", IDC_PARTICLE_SPIN, PARAM_FLOAT));
			params.push_back(ParamDesc("particle_spin_var", IDC_PARTICLE_SPIN_VAR, PARAM_FLOAT));		
			params.push_back(ParamDesc("particle_color", IDC_PARTICLE_COLOR, PARAM_ANIMATED_VECTOR));		
//	params.push_back(ParamDesc("particle_size", IDC_PARTICLE_SIZE, PARAM_ANIMATED_FLOAT));		
	params.push_back(ParamDesc("particle_size", IDC_PARTICLE_SIZE, PARAM_ANIMATED_VECTOR));		
			params.push_back(ParamDesc("particle_alpha", IDC_PARTICLE_ALPHA, PARAM_ANIMATED_FLOAT));		
			params.push_back(ParamDesc("particle_life", IDC_PARTICLE_LIFE, PARAM_FLOAT));
			params.push_back(ParamDesc("particle_life_var", IDC_PARTICLE_LIFE_VAR, PARAM_FLOAT));
#ifdef LEGACY_FILES
			params.push_back(ParamDesc("texture", IDC_PARTICLE_TEXTURE, IDC_PARTICLE_TEXTURE_FILE, "image files\0*.png\0*.jpg\0*.tga\0*.bmp\0*.dds\0\0", "Data\\Textures\\Particles"));
#else
			params.push_back(ParamDesc("texture", IDC_PARTICLE_TEXTURE, IDC_PARTICLE_TEXTURE_FILE, "image files\0*.png\0*.jpg\0*.tga\0*.bmp\0*.dds\0\0", "data\\texture\\particle"));
#endif
			params.push_back(ParamDesc("texture_sub_div_u", IDC_TEXTURE_SUB_DIV_U, PARAM_INT));
			params.push_back(ParamDesc("texture_sub_div_v", IDC_TEXTURE_SUB_DIV_V, PARAM_INT));
			params.push_back(ParamDesc("texture_alpha_type", IDC_TEXTURE_ALPHA_TYPE, PARAM_INT));
			params.push_back(ParamDesc("animation_frame_count", IDC_ANIMATION_FRAME_COUNT, PARAM_INT));
			params.push_back(ParamDesc("animation_start_frame", IDC_ANIMATION_START_FRAME, PARAM_INT));
			params.push_back(ParamDesc("animation_start_frame_var", IDC_ANIMATION_START_FRAME_VAR, PARAM_INT));
			params.push_back(ParamDesc("animation_fps", IDC_ANIMATION_FPS, PARAM_FLOAT));
			params.push_back(ParamDesc("animation_type", IDC_ANIMATION_TYPE, "loop,life", PARAM_SELECTION));
			params.push_back(ParamDesc("element_type", IDC_ELEMENT_TYPE, "point,quad,line_from_origin,line", PARAM_SELECTION));
			params.push_back(ParamDesc("launch_direction", IDC_LAUNCH_DIRECTION, "default,velocity,negative_velocity,explosion,negative_explosion",PARAM_SELECTION));
			params.push_back(ParamDesc("emit_darkening", IDC_EMIT_DARKENING, PARAM_FLOAT));
			params.push_back(ParamDesc("distortion", IDC_DISTORTION, PARAM_BOOL));
			params.push_back(ParamDesc("face_upward", IDC_UPWARD, PARAM_BOOL));
			params.push_back(ParamDesc("outside_only", IDC_OUTSIDE, PARAM_BOOL));
			params.push_back(ParamDesc("obstacle_spawn", IDC_OBSTACLE_HEIGHT, PARAM_BOOL));
			params.push_back(ParamDesc("heightmap_spawn", IDC_HEIGHT_HEIGHT, PARAM_BOOL));
			params.push_back(ParamDesc("outside_fade", IDC_OUTSIDE_FADE, PARAM_BOOL));
		}
	};

	struct SprayParticleSystemParamDesc {
		std::vector<ParamDesc> params;
		SprayParticleSystemParamDesc() 
		{
			GenParticleSystemParamDesc gen;
			params = gen.params;
			params.push_back(ParamDesc("spread1", IDC_SPREAD1, PARAM_FLOAT));
			params.push_back(ParamDesc("spread2", IDC_SPREAD2, PARAM_FLOAT));

			// Fluids
			params.push_back(ParamDesc("physics_type", IDC_PHYSICS_TYPE, "Disabled,Physics,Fluid without interaction,Fluid with interaction", PARAM_SELECTION));
			params.push_back(ParamDesc("fluid_static_restitution", IDC_FLUID_STATIC_RESTITUTION, PARAM_FLOAT, "0.5"));
			params.push_back(ParamDesc("fluid_static_adhesion", IDC_FLUID_STATIC_ADHESION, PARAM_FLOAT, "0.05"));
			params.push_back(ParamDesc("fluid_dynamic_restitution", IDC_FLUID_DYNAMIC_RESTITUTION, PARAM_FLOAT, "0.5"));
			params.push_back(ParamDesc("fluid_dynamic_adhesion", IDC_FLUID_DYNAMIC_ADHESION, PARAM_FLOAT, "0.5"));
			params.push_back(ParamDesc("fluid_damping", IDC_FLUID_DAMPING, PARAM_FLOAT, "0.0"));
			params.push_back(ParamDesc("fluid_motion_limit", IDC_FLUID_MOTION_LIMIT, PARAM_FLOAT, "6.0"));
			params.push_back(ParamDesc("fluid_packet_size_multiplier", IDC_FLUID_PACKET_SIZE_MULTIPLIER, PARAM_INT, "8"));
			params.push_back(ParamDesc("fluid_max_emitter_amount", IDC_FLUID_MAX_EMITTER_AMOUNT, PARAM_INT, "40"));
			params.push_back(ParamDesc("fluid_detail_collision_group", IDC_FLUID_DETAIL_COLLISION_GROUP, PARAM_INT, "0"));

			params.push_back(ParamDesc("fluid_stiffness", IDC_FLUID_STIFFNESS, PARAM_FLOAT, "200.0"));
			params.push_back(ParamDesc("fluid_viscosity", IDC_FLUID_VISCOSITY, PARAM_FLOAT, "22.0"));
			params.push_back(ParamDesc("fluid_kernel_radius_multiplier", IDC_FLUID_KERNEL_RADIUS_MULTIPLIER, PARAM_FLOAT, "2.3"));
			params.push_back(ParamDesc("fluid_rest_particles_per_meter", IDC_FLUID_REST_PARTICLES_PER_METER, PARAM_FLOAT, "10.0"));
			params.push_back(ParamDesc("fluid_rest_density", IDC_FLUID_REST_DENSITY, PARAM_FLOAT, "1000.0"));
		}
	};

	struct PointArrayParticleSystemParamDesc {
		std::vector<ParamDesc> params;
		PointArrayParticleSystemParamDesc() 
		{
			GenParticleSystemParamDesc gen;
			params = gen.params;
#ifdef LEGACY_FILES
			params.push_back(ParamDesc("model", IDC_MODEL, IDC_MODEL_FILE, "s3d files\0*.s3d\0\0", "Data\\Models\\Particles\\Emitter_shapes"));
#else
			params.push_back(ParamDesc("model", IDC_MODEL, IDC_MODEL_FILE, "s3d files\0*.s3d\0\0", "data\\model\\effect\\emitter_shape"));
#endif
			params.push_back(ParamDesc("scale", IDC_SCALE_X, IDC_SCALE_Y, IDC_SCALE_Z));
			params.push_back(ParamDesc("rotation", IDC_ROTATION_X, IDC_ROTATION_Y, IDC_ROTATION_Z));
			params.push_back(ParamDesc("direction_from_normals", IDC_DIRECTION_NORMALS, PARAM_INT));
			params.push_back(ParamDesc("positions_between_vertices", IDC_POSITIONS_BETWEEN_VERTICES, PARAM_INT));
			params.push_back(ParamDesc("plane_positions", IDC_PLANE_POSITIONS, PARAM_INT));
			params.push_back(ParamDesc("first_vertex", IDC_FIRST_VERTEX, PARAM_INT));
			params.push_back(ParamDesc("last_vertex", IDC_LAST_VERTEX, PARAM_INT));

			// Fluids
			params.push_back(ParamDesc("physics_type", IDC_PHYSICS_TYPE, "Disabled,Physics,Fluid without interaction,Fluid with interaction", PARAM_SELECTION));
			params.push_back(ParamDesc("fluid_static_restitution", IDC_FLUID_STATIC_RESTITUTION, PARAM_FLOAT, "0.5"));
			params.push_back(ParamDesc("fluid_static_adhesion", IDC_FLUID_STATIC_ADHESION, PARAM_FLOAT, "0.05"));
			params.push_back(ParamDesc("fluid_dynamic_restitution", IDC_FLUID_DYNAMIC_RESTITUTION, PARAM_FLOAT, "0.5"));
			params.push_back(ParamDesc("fluid_dynamic_adhesion", IDC_FLUID_DYNAMIC_ADHESION, PARAM_FLOAT, "0.5"));
			params.push_back(ParamDesc("fluid_damping", IDC_FLUID_DAMPING, PARAM_FLOAT, "0.0"));
			params.push_back(ParamDesc("fluid_motion_limit", IDC_FLUID_MOTION_LIMIT, PARAM_FLOAT, "6.0"));
			params.push_back(ParamDesc("fluid_packet_size_multiplier", IDC_FLUID_PACKET_SIZE_MULTIPLIER, PARAM_INT, "8"));
			params.push_back(ParamDesc("fluid_max_emitter_amount", IDC_FLUID_MAX_EMITTER_AMOUNT, PARAM_INT, "40"));
			params.push_back(ParamDesc("fluid_detail_collision_group", IDC_FLUID_DETAIL_COLLISION_GROUP, PARAM_INT, "0"));

			params.push_back(ParamDesc("fluid_stiffness", IDC_FLUID_STIFFNESS, PARAM_FLOAT, "200.0"));
			params.push_back(ParamDesc("fluid_viscosity", IDC_FLUID_VISCOSITY, PARAM_FLOAT, "22.0"));
			params.push_back(ParamDesc("fluid_kernel_radius_multiplier", IDC_FLUID_KERNEL_RADIUS_MULTIPLIER, PARAM_FLOAT, "2.3"));
			params.push_back(ParamDesc("fluid_rest_particles_per_meter", IDC_FLUID_REST_PARTICLES_PER_METER, PARAM_FLOAT, "10.0"));
			params.push_back(ParamDesc("fluid_rest_density", IDC_FLUID_REST_DENSITY, PARAM_FLOAT, "1000.0"));
		}
	};

	struct ModelParticleSystemParamDesc {
		std::vector<ParamDesc> params;
		ModelParticleSystemParamDesc() 
		{
			GenParticleSystemParamDesc gen;
			params = gen.params;
#ifdef LEGACY_FILES
			params.push_back(ParamDesc("model", IDC_MODEL, IDC_MODEL_FILE, "s3d files\0*.s3d\0\0", "Data\\Models\\Particles\\Emitter_shapes"));
			params.push_back(ParamDesc("particle_model", IDC_PARTICLE_MODEL, IDC_PARTICLE_MODEL_FILE, "s3d files\0*.s3d\0\0", "Data\\Models"));
#else
			params.push_back(ParamDesc("model", IDC_MODEL, IDC_MODEL_FILE, "s3d files\0*.s3d\0\0", "data\\model\\effect\\emitter_shape"));
			params.push_back(ParamDesc("particle_model", IDC_PARTICLE_MODEL, IDC_PARTICLE_MODEL_FILE, "s3d files\0*.s3d\0\0", "data\\model"));
#endif
			params.push_back(ParamDesc("scale", IDC_SCALE_X, IDC_SCALE_Y, IDC_SCALE_Z));
			params.push_back(ParamDesc("rotation", IDC_ROTATION_X, IDC_ROTATION_Y, IDC_ROTATION_Z));
			params.push_back(ParamDesc("direction_from_normals", IDC_DIRECTION_NORMALS, PARAM_INT));

			params.push_back(ParamDesc("particle1_angle_start", IDC_PARTICLE_ANGLE1_1, PARAM_FLOAT));
			params.push_back(ParamDesc("particle1_angle_speed", IDC_PARTICLE_ANGLE1_2, PARAM_FLOAT));
			params.push_back(ParamDesc("particle2_angle_start", IDC_PARTICLE_ANGLE2_1, PARAM_FLOAT));
			params.push_back(ParamDesc("particle2_angle_speed", IDC_PARTICLE_ANGLE2_2, PARAM_FLOAT));

			params.push_back(ParamDesc("collision", IDC_COLLISION, PARAM_BOOL, "1"));
			params.push_back(ParamDesc("slow_factor", IDC_PARTICLE_SLOW_FACTOR, PARAM_FLOAT, "0.95"));
			params.push_back(ParamDesc("y_slow_factor", IDC_PARTICLE_Y_SLOW_FACTOR, PARAM_FLOAT, "0.30"));
			params.push_back(ParamDesc("xz_slow_factor", IDC_PARTICLE_XZ_SLOW_FACTOR, PARAM_FLOAT, "0.40"));
			params.push_back(ParamDesc("unit_collision", IDC_UNIT_COLLISION, PARAM_BOOL, "1"));

			util::SoundMaterialParser materialParser;
			std::string soundMaterialString;
			for(int i = 0; i < materialParser.getMaterialAmount(); ++i)
			{
				if(i != 0)
					soundMaterialString += ",";

				soundMaterialString += materialParser.getMaterialName(i);
			}

			params.push_back(ParamDesc("sound_material", IDC_SOUND_MATERIAL, soundMaterialString, PARAM_SELECTION));
		}
	};

	struct GravityForceParamDesc {
		std::vector<ParamDesc> params;
		GravityForceParamDesc() {
			params.push_back(ParamDesc("gravity", IDC_GRAVITY, PARAM_FLOAT));
		}
	};

	struct SideGravityForceParamDesc {
		std::vector<ParamDesc> params;
		SideGravityForceParamDesc() {
			params.push_back(ParamDesc("sidegravity", IDC_SIDEGRAVITY, PARAM_FLOAT));
		}
	};

	struct DragForceParamDesc {
		std::vector<ParamDesc> params;
		DragForceParamDesc() {
			params.push_back(ParamDesc("factor", IDC_DRAG, PARAM_FLOAT));
		}
	};

	struct WindForceParamDesc {
		std::vector<ParamDesc> params;
		WindForceParamDesc() {
			params.push_back(ParamDesc("wind_effect_factor", IDC_WIND_EFFECT_FACTOR, PARAM_FLOAT));
			params.push_back(ParamDesc("spiral_amount", IDC_SPIRAL, PARAM_FLOAT));
			params.push_back(ParamDesc("spiral_speed", IDC_SPIRAL_SPEED, PARAM_FLOAT));
		}
	};
	
/*
	struct CloudParticleSystemParamDesc {
		std::vector<ParamDesc> params;
		CloudParticleSystemParamDesc() 
		{
			GenParticleSystemParamDesc gen;
			params = gen.params;
			params.push_back(ParamDesc("shape", IDC_SHAPE, "sphere,box,cylinder", PARAM_SELECTION));
			params.push_back(ParamDesc("sphere_inner_radius", IDC_SPHERE_INNER_RADIUS, PARAM_FLOAT));
			params.push_back(ParamDesc("sphere_outer_radius", IDC_SPHERE_OUTER_RADIUS, PARAM_FLOAT));
			params.push_back(ParamDesc("box_min", IDC_BOX_MIN_X, IDC_BOX_MIN_Y, IDC_BOX_MIN_Z));
			params.push_back(ParamDesc("box_max", IDC_BOX_MAX_X, IDC_BOX_MAX_Y, IDC_BOX_MAX_Z));
			params.push_back(ParamDesc("cylinder_height", IDC_CYLINDER_HEIGHT, PARAM_FLOAT));
			params.push_back(ParamDesc("cylinder_radius", IDC_CYLINDER_RADIUS, PARAM_FLOAT));
		}
	};
*/
	
	static SprayParticleSystemParamDesc theSprayParticleSystemParamDesc;
	static PointArrayParticleSystemParamDesc thePointArrayParticleSystemParamDesc;
	static ModelParticleSystemParamDesc theModelParticleSystemParamDesc;
//	static CouldParticleSystemParamDesc theCloudParticleSystemParamDesc;
	static GravityForceParamDesc theGravityForceParamDesc;
	static SideGravityForceParamDesc theSideGravityForceParamDesc;
	static DragForceParamDesc theDragForceParamDesc;
	static WindForceParamDesc theWindForceParamDesc;

	struct ParticleCollision: public IParticleCollision
	{
		ParticleCollision()
		{
		}

		~ParticleCollision()
		{
		}

		bool spawnPosition(const VC3 &emitter, const VC3 &dir, VC3 &position) const
		{
			return true;
		}

		bool getCollision(const VC3 &oldPos, VC3 &position, VC3 &velocity, float slowFactor, float groundSlowFactor, float wallSlowFactor) const
		{
			bool result = false;
			if(position.y < 0)
			{
				position.y = 0;

				velocity.x *= slowFactor;
				velocity.z *= slowFactor;
				velocity.y *= -groundSlowFactor;

				//if(fabsf(velocity.y) < 0.005f)
				//	velocity.y = 0;

				result = true;
			}

			return result;

			/*
			// hax
			float r = 2.f;
			if(position.x > r)
			{
				position.x = r;

				velocity.x *= -slowFactor;
				velocity.z *= slowFactor;
				velocity.y *= slowFactor;

				if(fabsf(velocity.x) < 0.02f)
					velocity.x = 0;

				result = true;
			}
			if(position.x < -r)
			{
				position.x = -r;

				velocity.x *= -slowFactor;
				velocity.z *= slowFactor;
				velocity.y *= slowFactor;

				if(fabsf(velocity.x) < 0.02f)
					velocity.x = 0;

				result = true;
			}
			if(position.z > r)
			{
				position.z = r;

				velocity.x *= slowFactor;
				velocity.z *= -slowFactor;
				velocity.y *= slowFactor;

				if(fabsf(velocity.z) < 0.02f)
					velocity.z = 0;

				result = true;
			}
			if(position.z < -r)
			{
				position.z = -r;

				velocity.x *= slowFactor;
				velocity.z *= -slowFactor;
				velocity.y *= slowFactor;

				if(fabsf(velocity.z) < 0.02f)
					velocity.z = 0;

				result = true;
			}

			return result;
			*/
		}
	};


	struct ParticleArea: public IParticleArea
	{
		ParticleArea()
		{
		}

		~ParticleArea()
		{
		}

		void biasValues(const VC3 &position, VC3 &velocity) const
		{
		}

		float getObstacleHeight(const VC3 &position) const
		{
			return 0.f;
		}

		float getBaseHeight(const VC3 &position) const
		{
			return 0.f;
		}

		bool isInside(const VC3 &position) const
		{
			return false;

			/*
			float range = 2.f;
			if(position.x > -range && position.x < range)
			if(position.y > -range && position.y < range)
			if(position.z > -range && position.z < range)
				return false;

			return true;
			*/
		}
	};

	struct SharedData 
	{
		ParticleEffectManager& effectManager;

		boost::shared_ptr<physics::PhysicsLib> physics;
		
		boost::shared_ptr<IParticleEffect> effect;
		int effectID;
		Vector effectPosition;
		Vector effectOldPosition;

		editor::Storm &storm;

		editor::Dialog &dialog;
		editor::Dialog &renderDialog;
		editor::Camera &camera;
		editor::ColorComponent &colorComponent;

		boost::shared_ptr<ParamUI> paramUI;
		boost::shared_ptr<ParamUI> forceUI;

		bool systemDisabled[256];		
		int color;

		boost::shared_ptr<Parser> parser;
		bool saved;

		bool skipTimeUpdate;

		std::string currentFilename;
		frozenbyte::viewer::Model model;

		SharedData(editor::Storm &storm_, 
			editor::Dialog &dialog_, editor::Dialog &renderDialog_, 
			editor::Camera &camera_, editor::ColorComponent &colorComponent_,
			ParticleEffectManager& mgr) : storm(storm_),
			dialog(dialog_), renderDialog(renderDialog_), colorComponent(colorComponent_),
			camera(camera_), saved(true), effectManager(mgr),
			skipTimeUpdate(false),
			model(storm_)
		{	
			color = RGB(70, 70, 70);
			colorComponent.setColor(color);

			effectPosition = Vector(0.0f, 0.0f, 0.0f);
			effectOldPosition = Vector(0.0f, 0.0f, 0.0f);

			currentFilename = std::string("");
		}

		~SharedData() 
		{	
		}

		void setBackGround()
		{
			float r = GetRValue(color) / 255.f;
			float g = GetGValue(color) / 255.f;
			float b = GetBValue(color) / 255.f;

			storm.scene->SetBackgroundColor(Color(r, g, b));
		}

		void load(const std::string& defaultFileName) {
/*					
			if(!saved) {
				if(IDYES == MessageBox(0, "Current Effect Not Saved. Save?", "Save?", MB_YESNO)) {
					save();
				}
			}
*/
			for(int i = 0; i < 256; i++)
				systemDisabled[i] = false;

			std::string fileName;
			if(defaultFileName.empty()) {
#ifdef LEGACY_FILES
				fileName = editor::getOpenFileName("txt", "data\\effects\\particles");
#else
				fileName = editor::getOpenFileName("pfx", "data\\effect\\particle");
#endif
			} else {
				fileName = defaultFileName;
			}

			if(fileName.empty())
				return;

			//std::ifstream file(fileName.c_str());
			//assert(file.bad()==false);

			currentFilename = fileName;
			
			boost::shared_ptr<editor::Parser> p(new editor::Parser);
			filesystem::createInputFileStream(fileName) >> *p;
			//file >> *p;

			parser.swap(p);
		}

		/*
		void save() 
		{
			if(parser.get()==NULL)
				return;
			
			std::string fileName = editor::getSaveFileName("txt", "data\\effects\\particles");
			if(fileName.empty())
				return;

			std::ofstream file(fileName.c_str());
			assert(!file.bad());

			file << *parser;

			saved = true;
	
		}
		*/

		bool saveImpl(std::string fileName = "") 
		{
			if(parser.get()==NULL)
				return false;
			
			if(fileName.empty())
				return false;

			currentFilename = fileName;

			std::ofstream file(fileName.c_str());
			assert(!file.bad());

			file << *parser;

			if (file.bad())
			{
				MessageBox(0, "Error saving file.", "Error", MB_OK);
			}

			saved = true;

			return true;	
		}

		void save() 
		{
			if (currentFilename.empty())
			{
				saveAs();
			} else {
				bool success = saveImpl(currentFilename);
				if (!success)
				{
					MessageBox(0, "Effect was not saved.", "Error", MB_OK);
				}
			}
		}

		void saveAs() 
		{
			std::string fileName = currentFilename;
#ifdef LEGACY_FILES
			fileName = editor::getSaveFileName("txt", "data\\effects\\particles");
#else
			fileName = editor::getSaveFileName("pfx", "data\\effect\\particle");
#endif
			bool success = saveImpl(fileName);
			if (success)
			{
				currentFilename = fileName;
			}
		}

		void addForce() {
		
			if(parser.get()==NULL) {
				return;
			}

			int selection = 0;
			if(IDOK == DialogBoxParam(NULL, MAKEINTRESOURCE(IDD_FORCE_SELECT), NULL, forceDlgProc, (LONG)&selection)) 
			{
				std::string fileName;
#ifdef LEGACY_FILES
				if(selection == 0)
					fileName = "Editor/Particles/gravity.txt";
				else if(selection == 1)
					fileName = "Editor/Particles/drag.txt";
				else if(selection == 2)
					fileName = "Editor/Particles/wind.txt";
				else if(selection == 3)
					fileName = "Editor/Particles/sidegravity.txt";
				else
				{
					assert(0);
				}
#else
				if(selection == 0)
					fileName = "editor/particles/gravity.txt";
				else if(selection == 1)
					fileName = "editor/particles/drag.txt";
				else if(selection == 2)
					fileName = "editor/particles/wind.txt";
				else if(selection == 3)
					fileName = "editor/particles/sidegravity.txt";
				else
				{
					assert(0);
				}
#endif
			
				ParserGroup& effect = parser->getGlobals().getSubGroup("effect");

				int selPS = ListBox_GetCurSel(dialog.getItem(IDC_PARTICLE_SYSTEMS));
				if(selPS < 0)
					return;

				std::string name = "system" + convertToString<int>(selPS);
				ParserGroup& pg = effect.getSubGroup(name);

				int numForces = convertFromString<int>(pg.getValue("num_forces", ""), 0);
				std::string forceName = "force" + convertToString<int>(numForces);
				
				ParserGroup& fg = pg.getSubGroup(forceName);
				
				//std::ifstream file(fileName.c_str());
				//file >> fg;
				filesystem::createInputFileStream(fileName) >> fg;

				pg.setValue("num_forces", convertToString<int>(numForces+1));
			
			}
		
		}

		void removeForce() {
			
			if(parser.get()==NULL)
				return;
			
			int selPS = ListBox_GetCurSel(dialog.getItem(IDC_PARTICLE_SYSTEMS));
			if(selPS < 0)
				return;

			int selForce = ListBox_GetCurSel(dialog.getItem(IDC_FORCES));
			if(selForce < 0)
				return;

			ParserGroup& effect = parser->getGlobals().getSubGroup("effect");
			ParserGroup& system = effect.getSubGroup("system" + convertToString<int>(selPS));
			
			int numForces = convertFromString<int>(system.getValue("num_forces", ""), 0);
			
			if(numForces > 0) {

				std::vector<ParserGroup> oldForces;
				oldForces.resize(numForces);
				int i;
				for(i = 0; i < numForces; i++) {
					ParserGroup& force = system.getSubGroup("force" + convertToString<int>(i));
					oldForces[i] = force;
					system.removeSubGroup("force" + convertToString<int>(i));
				}
				int n = 0;
				for(i = 0; i < numForces; i++) {
					if(i == selForce)
						continue;
					ParserGroup& force = system.getSubGroup("force" + convertToString<int>(n));
					force = oldForces[i];
					n++;
				}
				
				system.setValue("num_forces", convertToString<int>(numForces-1));

			}

		}

		void editForce() {
		
			if(parser.get()==NULL)
				return;

			int selPS = ListBox_GetCurSel(dialog.getItem(IDC_PARTICLE_SYSTEMS));
			if(selPS < 0)
				return;

			int selForce = ListBox_GetCurSel(dialog.getItem(IDC_FORCES));
			if(selForce < 0)
				return;

			ParserGroup& effect = parser->getGlobals().getSubGroup("effect");
			ParserGroup& system = effect.getSubGroup("system" + convertToString<int>(selPS));
			
			int numForces = convertFromString<int>(system.getValue("num_forces", ""), 0);

			ParserGroup& force = system.getSubGroup("force" + convertToString<int>(selForce));
			std::string className = force.getValue("class", "");
			if(className == "gravity") {
				boost::shared_ptr<ParamUI> p(new ParamUI(dialog, IDD_GRAVITY,
					force, theGravityForceParamDesc.params));				
				forceUI.swap(p);
			}
			else if(className == "sidegravity") {
				boost::shared_ptr<ParamUI> p(new ParamUI(dialog, IDD_SIDEGRAVITY,
					force, theSideGravityForceParamDesc.params));				
				forceUI.swap(p);
			}
			else if(className == "drag") {
				boost::shared_ptr<ParamUI> p(new ParamUI(dialog, IDD_DRAG,
					force, theDragForceParamDesc.params));				
				forceUI.swap(p);
			}
			else if(className == "wind") {
				boost::shared_ptr<ParamUI> p(new ParamUI(dialog, IDD_WIND,
					force, theWindForceParamDesc.params));				
				forceUI.swap(p);
			}
			else {
				// psd
				//assert(0);
			}
		
		}

		void addParticleSystem() {
		
			if(parser.get()==NULL)
				return;
			
			int selection;
			if(IDOK==DialogBoxParam(NULL, MAKEINTRESOURCE(IDD_PARTICLE_SYSTEM_SELECT),
				NULL, psDlgProc, (LONG)&selection)) {
								
				std::string fileName;
#ifdef LEGACY_FILES
				if(selection == 0) {
					fileName = "Editor/Particles/spray.txt";
				}
				else if(selection == 1) {
					fileName = "Editor/Particles/parray.txt";
				}
				else if(selection == 2) {
					fileName = "Editor/Particles/modelp.txt";
				}
				/*
				else if(selection == 2) {
					fileName = "particle_editor/default_cloud.txt";
				}
				*/
				else {
					assert(0);
				}
#else
				if(selection == 0) {
					fileName = "editor/particles/spray.txt";
				}
				else if(selection == 1) {
					fileName = "editor/particles/parray.txt";
				}
				else if(selection == 2) {
					fileName = "editor/particles/modelp.txt";
				}
				/*
				else if(selection == 2) {
					fileName = "particle_editor/default_cloud.txt";
				}
				*/
				else {
					assert(0);
				}
#endif

				ParserGroup& effect = parser->getGlobals().getSubGroup("effect");
				int numSystems = convertFromString<int>(effect.getValue("num_systems", ""), 0);

				std::string name = "system" + convertToString<int>(numSystems);
				ParserGroup& pg = effect.getSubGroup(name);
			
				//std::ifstream file(fileName.c_str());
				//file >> pg;
				filesystem::createInputFileStream(fileName) >> pg;

				std::string className = pg.getValue("class", "");
				if (className.empty())
				{
					className = pg.getValue("@class", "");
				}

				effect.setValue("num_systems", 
					convertToString<int>(numSystems+1));
			}

		}

		void removeParticleSystem() {
		
			if(parser.get()==NULL)
				return;

			int selection = ListBox_GetCurSel(dialog.getItem(IDC_PARTICLE_SYSTEMS));
			if(selection < 0)
				return;
			
			ParserGroup& effect = parser->getGlobals().getSubGroup("effect");
			int numSystems = convertFromString<int>(effect.getValue("num_systems", ""), 0);
			
			if(numSystems > 0) {
			
				int i;
				std::vector<ParserGroup> oldSystems;
				for(i = 0; i < numSystems; i++) {
					oldSystems.push_back(effect.getSubGroup("system" + convertToString<int>(i)));
					effect.removeSubGroup("system" + convertToString<int>(i));
				}
				int n = 0;
				for(i = 0; i < numSystems; i++) {
					if(i == selection)
						continue;
					ParserGroup& system = effect.getSubGroup("system" + convertToString<int>(n));
					system = oldSystems[i];
					n++;
				}
			
				effect.setValue("num_systems", convertToString<int>(numSystems-1));
			
			}


		}

		void editParticleSystem() {
					
			if(parser.get()==NULL)
				return;

			int selection = ListBox_GetCurSel(dialog.getItem(IDC_PARTICLE_SYSTEMS));
			if(selection < 0)
				return;

			std::string name = "system" + convertToString<int>(selection);
			ParserGroup& effect = parser->getGlobals().getSubGroup("effect");
			ParserGroup& pg = effect.getSubGroup(name);

			std::string className = pg.getValue("class");
			if (className.empty())
			{
				className = pg.getValue("@class");
			}
			if(className == "spray") {
				boost::shared_ptr<ParamUI> p(new ParamUI(dialog, IDD_SPRAY_PARTICLE_SYSTEM, 
					pg, theSprayParticleSystemParamDesc.params));				
				paramUI.swap(p);
			}
			else if(className == "parray") {
				boost::shared_ptr<ParamUI> p(new ParamUI(dialog, IDD_POINT_ARRAY_PARTICLE_SYSTEM,
					pg, thePointArrayParticleSystemParamDesc.params));				
				paramUI.swap(p);
			}
			else if(className == "modelp") {
				boost::shared_ptr<ParamUI> p(new ParamUI(dialog, IDD_MODEL_PARTICLE_SYSTEM,
					pg, theModelParticleSystemParamDesc.params));				
				paramUI.swap(p);
			}
			else {
				assert(!"undefined class");
			}
		

			saved = false;
		}

		void enableParticleSystem() {

			
			int selection = ListBox_GetCurSel(dialog.getItem(IDC_PARTICLE_SYSTEMS));
			if(selection < 0)
				return;

			systemDisabled[selection] = false;
		
			updateDialog();

		}

		void disableParticleSystem() {

		
			int selection = ListBox_GetCurSel(dialog.getItem(IDC_PARTICLE_SYSTEMS));
			if(selection < 0)
				return;

			systemDisabled[selection] = true;

			updateDialog();

		}

		void renameParticleSystem() {
		
			int selection = ListBox_GetCurSel(dialog.getItem(IDC_PARTICLE_SYSTEMS));
			if(selection < 0)
				return;

			ParserGroup& effect = parser->getGlobals().getSubGroup("effect");
			ParserGroup& system = effect.getSubGroup("system" + convertToString<int>(selection));

			std::string name = system.getValue("name", "unnamed" + convertToString<int>(selection));
			if(IDOK==DialogBoxParam(NULL, MAKEINTRESOURCE(IDD_RENAME),
				NULL, nameDlgProc, (LONG)&name)) {
			
				system.setValue("name", name);

				updateDialog();
			
			}
			

		}

		void updateDialog() {

			if(parser.get()==NULL)
				return;

			int selPS = ListBox_GetCurSel(dialog.getItem(IDC_PARTICLE_SYSTEMS));
			
			const ParserGroup& effect = parser->getGlobals().getSubGroup("effect");
			int numSystems = convertFromString<int>(effect.getValue("num_systems", ""), 0);
		
			ListBox_ResetContent(dialog.getItem(IDC_PARTICLE_SYSTEMS));			
			for(int i = 0; i < numSystems; i++) {
				
				const ParserGroup& pg = effect.getSubGroup("system" + convertToString<int>(i));

				std::string name = pg.getValue("name", "unnamed" + convertToString<int>(i));
				std::string className = pg.getValue("class", "");
				if (className.empty())
				{
					className = pg.getValue("@class");
				}
				name += "(";
				name += className;
				name += ")";

				if(systemDisabled[i]) {
					name += " [disabled]";
				}
				
				ListBox_AddString(dialog.getItem(IDC_PARTICLE_SYSTEMS), name.c_str());

				if(i == selPS) {
					ListBox_ResetContent(dialog.getItem(IDC_FORCES));
					int numForces = convertFromString<int>(pg.getValue("num_forces", ""), 0);
					for(int j = 0; j < numForces; j++) {
						std::string forceName = "force" + convertToString<int>(j);
						std::string forceClassName = pg.getSubGroup(forceName).getValue("class", "");
						forceName += "(";
						forceName += forceClassName;
						forceName += ")";
						ListBox_AddString(dialog.getItem(IDC_FORCES), forceName.c_str());
					}
				
				}
			}
		
			ListBox_SetCurSel(dialog.getItem(IDC_PARTICLE_SYSTEMS), selPS);
			if(systemDisabled[selPS]) {
				enableDialogItem(dialog, IDC_ENABLE, true);
				enableDialogItem(dialog, IDC_DISABLE, false);
			} else {
				enableDialogItem(dialog, IDC_ENABLE, false);
				enableDialogItem(dialog, IDC_DISABLE, true);			
			}

		}

	

		void play() 
		{
			stop(true);

			if(parser.get()==NULL)
				return;

			Parser parser2;
			parser2.getGlobals() = parser->getGlobals();
			ParserGroup& effectGroup = parser2.getGlobals().getSubGroup("effect");
			std::vector<ParserGroup> oldSystems;
			int numSystems = convertFromString<int>(effectGroup.getValue("num_systems", ""), 0);
			
			int i;
			for(i = 0; i < numSystems; i++) 
			{
				oldSystems.push_back(effectGroup.getSubGroup("system" + convertToString<int>(i)));		
				effectGroup.removeSubGroup("system" + convertToString<int>(i));
			}

			int n = 0;
			for(i = 0; i < numSystems; i++) 
			{
				if(systemDisabled[i])
					continue;

				ParserGroup& newGroup = effectGroup.getSubGroup("system" + convertToString<int>(n));
				newGroup = oldSystems[i];
				n++;
			}

			effectGroup.setValue("num_systems", convertToString<int>(n));

			if(n > 0) 
			{
				effectManager.reset(false);	
				effectID = effectManager.loadParticleEffect(parser2);
				
				effect = effectManager.addEffectToScene(effectID);
				effect->setPosition(effectPosition);
				effect->setVelocity(effectPosition);
				
				//effect->setLighting(COL(0.33f, 0.33f, 0.33f), VC3(), COL(), 1.f);
				signed short int lightIndices[LIGHT_MAX_AMOUNT] = { 0 };
				for(int i = 1; i < LIGHT_MAX_AMOUNT; ++i)
					lightIndices[i] = -1;

				effect->setLighting(COL(0.33f, 0.33f, 0.33f), lightIndices);

				for(int i = 0; i < effect->getNumSystems(); ++i)
				{
					effect->getParticleSystem(i)->setExplosion(VC3(0.1f,0.1f,0.1f), true);
				}

				boost::shared_ptr<IParticleCollision> col(new ParticleCollision());
				effect->setCollision(col);
				boost::shared_ptr<IParticleArea> area(new ParticleArea());
				effect->setArea(area);
				
				effect->tick();
			}

			skipTimeUpdate = true;
		}

		void tick() {
		
		}

		void render() {
		
		}

		void stop(bool reset = false) 
		{
			if(effect)
			{
				effect->kill();
				effectManager.reset(false);	
			}

			if(reset)
			{
				effectManager.release();
				if(effect)
					while(effect->getNumSystems())
						effect->removeSystem(0);
			}
		}

		static BOOL CALLBACK nameDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
			
			static std::string* name;
			
			switch(msg) {
			case WM_INITDIALOG:
				{
					name = (std::string*)lParam;
					SetDlgItemText(hwnd, IDC_NAME, name->c_str());
				} break;
			case WM_COMMAND:
				{
					if(LOWORD(wParam)==IDOK) {
						char buffer[256];
						GetDlgItemText(hwnd, IDC_NAME, buffer, 256);
						*name = buffer;
						EndDialog(hwnd, IDOK);
						return TRUE;
					}
					if(LOWORD(wParam)==IDCANCEL) {						
						EndDialog(hwnd, IDCANCEL);
						return TRUE;
					}
				} break;
			}
		
			return FALSE;
		}


		
		static BOOL CALLBACK forceDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
			
			static int* selection = 0;
			
			switch(msg) {
			case WM_INITDIALOG:
				{
					selection = (int*)lParam;
					ComboBox_ResetContent(GetDlgItem(hwnd, IDC_FORCE_TYPE));
					ComboBox_AddString(GetDlgItem(hwnd, IDC_FORCE_TYPE), "gravity force");				
					ComboBox_AddString(GetDlgItem(hwnd, IDC_FORCE_TYPE), "drag force");				
					ComboBox_AddString(GetDlgItem(hwnd, IDC_FORCE_TYPE), "wind force");				
					ComboBox_AddString(GetDlgItem(hwnd, IDC_FORCE_TYPE), "sidegravity force");
					ComboBox_SetCurSel(GetDlgItem(hwnd, IDC_FORCE_TYPE), 0);
				} break;
			case WM_COMMAND:
				{
					if(LOWORD(wParam)==IDOK) {
						*selection = ComboBox_GetCurSel(GetDlgItem(hwnd, IDC_FORCE_TYPE));
						EndDialog(hwnd, IDOK);
						return TRUE;
					}
					if(LOWORD(wParam)==IDCANCEL) {						
						EndDialog(hwnd, IDCANCEL);
						return TRUE;
					}
				} break;
			}
		
			return FALSE;
		}

		static BOOL CALLBACK psDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
			
			static int* selection = 0;
			
			switch(msg) {
			case WM_INITDIALOG:
				{
					selection = (int*)lParam;
					ComboBox_ResetContent(GetDlgItem(hwnd, IDC_PARTICLE_SYSTEM_TYPE));
					ComboBox_AddString(GetDlgItem(hwnd, IDC_PARTICLE_SYSTEM_TYPE), "spray particle system");				
					ComboBox_AddString(GetDlgItem(hwnd, IDC_PARTICLE_SYSTEM_TYPE), "point array particle system");				
					ComboBox_AddString(GetDlgItem(hwnd, IDC_PARTICLE_SYSTEM_TYPE), "model particle system");				
					//ComboBox_AddString(GetDlgItem(hwnd, IDC_PARTICLE_SYSTEM_TYPE), "cloud particle system");				
					ComboBox_SetCurSel(GetDlgItem(hwnd, IDC_PARTICLE_SYSTEM_TYPE), 0);
				} break;
			case WM_COMMAND:
				{
					if(LOWORD(wParam)==IDOK) {
						*selection = ComboBox_GetCurSel(GetDlgItem(hwnd, IDC_PARTICLE_SYSTEM_TYPE));
						EndDialog(hwnd, IDOK);
						return TRUE;
					}
					if(LOWORD(wParam)==IDCANCEL) {						
						EndDialog(hwnd, IDCANCEL);
						return TRUE;
					}
				} break;
			}
		
			return FALSE;
		}
	
	
	};

	
	class NewEffectCommand : public editor::ICommand 
	{
		SharedData& m_data;
	public:
		NewEffectCommand(SharedData& data) : m_data(data) {
		}
		void execute(int id) {
			
			
			m_data.load("particle_editor/default_effect.txt");
			m_data.currentFilename = "";

			m_data.updateDialog();
		}
	};

	class LoadEffectCommand : public editor::ICommand 
	{
		SharedData& m_data;
	public:
		LoadEffectCommand(SharedData& data) : m_data(data) {
		}
		void execute(int id) {
			m_data.load("");
			m_data.updateDialog();

		}
	};

	class SaveEffectCommand : public editor::ICommand 
	{
		SharedData& m_data;
	public:
		SaveEffectCommand(SharedData& data) : m_data(data) {
		}
		void execute(int id) {
			m_data.save();
		}	
	};

	class SaveEffectAsCommand : public editor::ICommand 
	{
		SharedData& m_data;
	public:
		SaveEffectAsCommand(SharedData& data) : m_data(data) {
		}
		void execute(int id) {
			m_data.saveAs();
		}	
	};

	class AddParticleSystemCommand : public editor::ICommand 
	{
		SharedData& m_data;
	public:
		
		AddParticleSystemCommand(SharedData& data) : m_data(data) {
			
		}

		void execute(int id) {
			m_data.addParticleSystem();

			m_data.updateDialog();
		}
	};

	class RemoveParticleSystemCommand : public editor::ICommand 
	{
		SharedData& m_data;
	public:

		RemoveParticleSystemCommand(SharedData& data) : m_data(data) {
			
		}

		void execute(int id) {
			m_data.removeParticleSystem();

			m_data.updateDialog();
		}

	};

	class SelectParticleSystemCommand : public ICommand {
		SharedData& m_data;
	public:
		SelectParticleSystemCommand(SharedData& data) : m_data(data) 
		{
		}
		void execute(int id) {
			m_data.updateDialog();
		}
	};

	class InsertForceCommand : public ICommand {
		SharedData& m_data;
	public:
		InsertForceCommand(SharedData& data) : m_data(data) 
		{
		}
		void execute(int id) {
			m_data.addForce();
			m_data.updateDialog();
		}
	};

	class RemoveForceCommand : public ICommand {
		SharedData& m_data;
	public:
		RemoveForceCommand(SharedData& data) : m_data(data)
		{
		}
		void execute(int id) 
		{
			m_data.removeForce();
			m_data.updateDialog();
		}
	};

	class EditForceCommand : public ICommand {
		SharedData& m_data;
	public:
		EditForceCommand(SharedData& data) : m_data(data)
		{
		}
		void execute(int id) 
		{
			m_data.editForce();
			m_data.updateDialog();
		}
	};


	class EditParticleSystemCommand : public ICommand 
	{
		SharedData& m_data;
	public:
		EditParticleSystemCommand(SharedData& data) : m_data(data) {
		}
		void execute(int id) {
			m_data.editParticleSystem();
		}
	};

	class EnableParticleSystemCommand : public editor::ICommand 
	{
		SharedData& m_data;
	public:
		EnableParticleSystemCommand(SharedData& data) : m_data(data) {
		}
		void execute(int id) {
			m_data.enableParticleSystem();
		}	
	};

	class DisableParticleSystemCommand : public editor::ICommand 
	{
		SharedData& m_data;
	public:
		DisableParticleSystemCommand(SharedData& data) : m_data(data) {
		}
		void execute(int id) {
			m_data.disableParticleSystem();
		}	
	};

	class PlayCommand : public editor::ICommand 
	{
		SharedData& m_data;
	public:
		PlayCommand(SharedData& data) : m_data(data) {
		}
		void execute(int id) {
			m_data.play();
		}	
	};

	class StopCommand : public editor::ICommand 
	{
		SharedData& m_data;
	public:
		StopCommand(SharedData& data) : m_data(data) {
		}
		void execute(int id) {
			m_data.stop();
		}	
	};
	
	class ColorCommand: public editor::ICommand
	{
		SharedData &sharedData;

	public:
		ColorCommand(SharedData &sharedData_)
		:	sharedData(sharedData_)
		{
		}

		void execute(int id)
		{
			editor::ColorPicker colorPicker;
			if(colorPicker.run(sharedData.color))
			{
				sharedData.color = colorPicker.getColor();
				sharedData.colorComponent.setColor(sharedData.color);
				sharedData.setBackGround();
			}
		}
	};

	class LoadModelCommand : public editor::ICommand 
	{
		Storm &storm;

		std::vector<std::string> fileNames;
		std::vector<IStorm3D_Model *> models;

	public:
		LoadModelCommand(Storm& storm_)
		: storm(storm_) 
		{
		}
		
		void execute(int id) 
		{
			loadModel();
		}
		
		void loadModel() 
		{
#ifdef LEGACY_FILES
			std::vector<std::string> files = getMultipleOpenFileName("s3d", "Data\\Models");
#else
			std::vector<std::string> files = getMultipleOpenFileName("s3d", "data\\model");
#endif
			for(unsigned int i = 0; i < files.size(); ++i)
			{
				IStorm3D_Model *model = storm.storm->CreateNewModel();
				if(model->LoadS3D(files[i].c_str()))
				{
					storm.scene->AddModel(model);
					model->SetPosition(Vector(0.0f, 0.0f, 0.0f));

					fileNames.push_back(files[i]);
					models.push_back(model);
				}
			}
		}

		void clearAll()
		{
			for(unsigned int i = 0; i < models.size(); ++i)
				delete models[i];

			models.clear();
			fileNames.clear();
		}

		void recreate()
		{
			models.clear();
			IStorm3D_Model *model = storm.storm->CreateNewModel();
			//if(model->LoadS3D("Editor\\Particles\\cube\\cube.s3d"))
			//	storm.scene->AddModel(model);

			for(unsigned int i = 0; i < fileNames.size(); ++i)
			{
				IStorm3D_Model *model = storm.storm->CreateNewModel();
				if(model->LoadS3D(fileNames[i].c_str()))
				{
					storm.scene->AddModel(model);
					model->SetPosition(Vector(0.0f, 0.0f, 0.0f));

					models.push_back(model);
				}
			}
		}
	};

	class ClearModelsCommand: public editor::ICommand
	{
		LoadModelCommand &loadCommand;

	public:
		ClearModelsCommand(LoadModelCommand &loadCommand_)
		:	loadCommand(loadCommand_)
		{
		}

		void execute(int id)
		{
			loadCommand.clearAll();
		}
	};

	class ShowEmittersCommand: public editor::ICommand
	{
		ParticleEffectManager &mgr;
		Storm &storm;
		SharedData &data;

		std::vector<IStorm3D_Model *> models;

		void clearModels()
		{
			for(unsigned int i = 0; i < models.size(); ++i)
				delete models[i];

			models.clear();
		}

		void adjustModel(IStorm3D_Model *model)
		{
			assert(model);

			boost::scoped_ptr<Iterator<IStorm3D_Model_Object *> > objectIterator(model->ITObject->Begin());
			for(; !objectIterator->IsEnd(); objectIterator->Next())
			{
				IStorm3D_Model_Object *object = objectIterator->GetCurrent();
				if(!object)
					continue;

				IStorm3D_Mesh *mesh = object->GetMesh();
				if(!mesh)
					continue;

				IStorm3D_Material *material = mesh->GetMaterial();
				if(!material)
					continue;

				material->SetSpecial(false, true);
			}
		}

		void addEmitter(int index)
		{
			std::string name = "system" + convertToString<int> (index);
			const ParserGroup &effect = data.parser->getGlobals().getSubGroup("effect");
			const ParserGroup &pg = effect.getSubGroup(name);

			std::string fileName = "Editor/Particles/Emitters/default.s3d";

			std::string className = pg.getValue("class");
			if (className.empty())
			{
				className = pg.getValue("@class");
			}
			VC3 position = convertVectorFromString(pg.getValue("emitter_position", "0,0,0"));

			if(className == "parray" || className == "modelp")
				fileName = pg.getValue("model", "");

			IStorm3D_Model *model = storm.storm->CreateNewModel();
			if(model && model->LoadS3D(fileName.c_str()))
			{
				adjustModel(model);
				storm.scene->AddModel(model);

				models.push_back(model);
			}
		}

	public:
		ShowEmittersCommand(ParticleEffectManager &mgr_, Storm &storm_, SharedData &data_)
		:	mgr(mgr_),
			storm(storm_),
			data(data_)
		{
		}

		void execute(int id)
		{
			clearModels();

			if(IsDlgButtonChecked(data.dialog.getWindowHandle(), IDC_SHOW_EMITTERS) != BST_CHECKED)
				return;

			data.play();
			data.stop();
			data.play();

			int selection = ListBox_GetCurSel(data.dialog.getItem(IDC_PARTICLE_SYSTEMS));
			if(selection < 0)
			{
				for(int i = 0; i < mgr.getEffectPrototypeAmount(); ++i)
					addEmitter(i);
			}
			else
			{
				addEmitter(selection);
			}
		}

		void recreate()
		{
			models.clear();
			execute(0);
		}
	};

	class ReloadCommand : public editor::ICommand {
		Storm& storm;
		Dialog& dlg;
		ParticleEffectManager& mgr;
		CrappyFont& font;
		SharedData &data;

		LoadModelCommand &loadCommand;
		ShowEmittersCommand &showCommand;

	public:
		ReloadCommand(Storm& storm_, Dialog& dlg_, ParticleEffectManager& mgr_, CrappyFont& font_, SharedData &data_, LoadModelCommand &loadCommand_, ShowEmittersCommand &showCommand_) 
		:	storm(storm_), 
			dlg(dlg_),
			mgr(mgr_), 
			font(font_),
			data(data_),
			loadCommand(loadCommand_),
			showCommand(showCommand_)
		{
		}

		void execute(int id) 
		{
			bool success = data.saveImpl("particle.tmp");
			//if (!success)
			//{
			//	MessageBox(0, "Error saving temporary particle effect file.", "Error", MB_OK);
			//}
			data.stop(true);

			data.model.freeResources();
			storm.recreate(dlg.getWindowHandle());
			mgr.recreate(storm.storm, storm.scene);
			font.recreate();

			loadCommand.recreate();
			showCommand.recreate();

			unsigned short buffer[32*32] = { 0 };
			storm.terrain = storm.storm->CreateNewTerrain(32);
			storm.terrain->setHeightMap(buffer, VC2I(32,32), VC3(500,.01f,500), 4, 0, 1, 1);
			//storm.terrain->getRenderer().setFloatValue(IStorm3D_TerrainRenderer::ForceAmbient, 0.5f);
			storm.terrain->getRenderer().enableFeature(IStorm3D_TerrainRenderer::Glow, false);
			//storm.terrain->getRenderer().enableFeature(IStorm3D_TerrainRenderer::Glow, true);
			//storm.terrain->getRenderer().setRenderMode(IStorm3D_TerrainRenderer::TexturesOnly);
			//storm.terrain->getRenderer().enableFeature(IStorm3D_TerrainRenderer::Distortion, true);
			storm.scene->AddTerrain(storm.terrain);

			data.load("particle.tmp");
			remove("particle.tmp");

			storm.terrain->addLight(VC3(0, 0, 0), 10.f, COL(0.33f, 0.33f, 0.33f));
			//effect->setLighting(COL(0.33f, 0.33f, 0.33f), VC3(), COL(), 1.f);
			data.model.loadResources();
			data.model.playAnimation(0, 0, true);
		}
	};

	class RenameParticleSystemCommand : public editor::ICommand {
		SharedData& m_data;
	public:
		RenameParticleSystemCommand(SharedData& data) : m_data(data) {
		}
		void execute(int id) {
			m_data.renameParticleSystem();
		}
	};

	struct SlowMotionCommand: public editor::ICommand
	{
		HWND hwnd;
		bool &slowMotion;

		SlowMotionCommand(bool &slowMotion_, HWND hwnd_)
		:	slowMotion(slowMotion_),
			hwnd(hwnd_)
		{
		}

		void execute(int id)
		{
			if(IsDlgButtonChecked(hwnd, IDC_SLOW_MOTION) == BST_CHECKED)
				slowMotion = true;
			else
				slowMotion = false;
		}
	};

	struct EffectOrderCommand: public editor::ICommand
	{
		SharedData &data;
		bool up;

		EffectOrderCommand(SharedData &data_, bool upDirection)
		:	data(data_),
			up(upDirection)
		{
		}

		void execute(int id)
		{
			ParserGroup &effect = data.parser->getGlobals().getSubGroup("effect");
			int numSystems = convertFromString<int>(effect.getValue("num_systems", ""), 0);
			int selection = ListBox_GetCurSel(data.dialog.getItem(IDC_PARTICLE_SYSTEMS));

			if(up)
			{
				if(selection <= 0)
					return;

				std::string s1 = "system" + convertToString<int>(selection);
				ParserGroup e1 = effect.getSubGroup(s1);
				std::string s2 = "system" + convertToString<int>(selection - 1);
				ParserGroup e2 = effect.getSubGroup(s2);

				effect.getSubGroup(s1) = e2;
				effect.getSubGroup(s2) = e1;
			}
			else
			{
				if(selection >= numSystems - 1)
					return;

				std::string s1 = "system" + convertToString<int>(selection);
				ParserGroup e1 = effect.getSubGroup(s1);
				std::string s2 = "system" + convertToString<int>(selection + 1);
				ParserGroup e2 = effect.getSubGroup(s2);

				effect.getSubGroup(s1) = e2;
				effect.getSubGroup(s2) = e1;
			}

			data.updateDialog();
		}
	};

	class LoadCommand : public editor::ICommand
	{
		SharedData& m_data;
	public:
		LoadCommand(SharedData& data)
		:	m_data(data)
		{
		}

		void execute(int id)
		{
			std::string fileName = editor::getOpenFileName("fbv", "Editor\\Viewer");
			m_data.model.load(fileName);
			m_data.model.playAnimation(0, 0, true);

			setDialogItemText(m_data.dialog, IDC_VIEWER_CONFIG, fileName);
		}
	};


	class PhysicsCommand : public editor::ICommand
	{
		SharedData &data;
	public:
		PhysicsCommand(SharedData &data_) : data(data_)
		{
		}

		void execute(int id)
		{
			if(IsDlgButtonChecked(data.dialog.getWindowHandle(), IDC_PHYSICS))
			{
				data.effectManager.enablePhysics(true);
				data.effectManager.enableParticlePhysics(true);
				data.effectManager.setPhysics(data.physics);
			}
			else
			{
				data.effectManager.enablePhysics(false);
				data.effectManager.enableParticlePhysics(false);
			}
		}
	};


	bool hideCursor = false;

} // end of unnamed namespace

struct ApplicationData
{
	editor::Window window;
	editor::Dialog mainDialog;
	editor::Dialog renderDialog;

	editor::Storm storm;
	bool mustQuit;

	editor::Camera camera;
	editor::Mouse mouse;

	editor::ColorComponent colorComponent;
	SharedData sharedData;

	OrbitCamera orbitCamera;
	CrappyFont font;
	
	NewEffectCommand newEffectCommand;
	LoadEffectCommand loadEffectCommand;
	SaveEffectCommand saveEffectCommand;
	SaveEffectAsCommand saveEffectAsCommand;
	AddParticleSystemCommand addParticleSystemCommand;
	RemoveParticleSystemCommand removeParticleSystemCommand;
	EnableParticleSystemCommand enableParticleSystemCommand;
	DisableParticleSystemCommand disableParticleSystemCommand;
	EditParticleSystemCommand editParticleSystemCommand;
	PlayCommand playCommand;
	StopCommand stopCommand;
	ColorCommand colorCommand;
	LoadModelCommand loadModelCommand;
	ClearModelsCommand clearModelsCommand;
	ShowEmittersCommand showEmittersCommand;
	ReloadCommand reloadCommand;
	InsertForceCommand insertForceCommand;
	RemoveForceCommand removeForceCommand;
	EditForceCommand editForceCommand;
	SelectParticleSystemCommand selectParticleSystemCommand;
	RenameParticleSystemCommand renameParticleSystemCommand;
	PhysicsCommand physicsCommand;

	ParticleEffectManager effectManager;
	bool slowMotion;

	SlowMotionCommand slowMotionCommand;
	EffectOrderCommand effectUpCommand;
	EffectOrderCommand effectDownCommand;
	LoadCommand loadCommand;

	ApplicationData()
	:	window("Frozenbyte Particle Editor", IDI_ICON1, true, false),
		mainDialog(IDD_MAIN, window, editor::Dialog::ATTACH_BOTTOM),
		renderDialog(IDD_RENDER, window, editor::Dialog::ATTACH_ALL),
		storm(renderDialog.getWindowHandle()),
		effectManager(storm.storm, storm.scene),
		mustQuit(false),

		camera(storm),

		colorComponent(mainDialog.getWindowHandle(), 120, 602, 90, 22),
		sharedData(storm, mainDialog, renderDialog, camera, colorComponent, effectManager),
		
		newEffectCommand(sharedData),
		loadEffectCommand(sharedData),
		saveEffectCommand(sharedData),
		saveEffectAsCommand(sharedData),
		addParticleSystemCommand(sharedData),
		removeParticleSystemCommand(sharedData),
		editParticleSystemCommand(sharedData),
		enableParticleSystemCommand(sharedData),
		disableParticleSystemCommand(sharedData),
		colorCommand(sharedData),
		playCommand(sharedData),
		stopCommand(sharedData), 
		loadModelCommand(storm),
		clearModelsCommand(loadModelCommand),
		showEmittersCommand(effectManager, storm, sharedData),
		reloadCommand(storm, renderDialog, effectManager, font, sharedData, loadModelCommand, showEmittersCommand),
		insertForceCommand(sharedData),
		removeForceCommand(sharedData),
		editForceCommand(sharedData),
		selectParticleSystemCommand(sharedData),
		renameParticleSystemCommand(sharedData),
		font(storm),
		slowMotion(false),
		slowMotionCommand(slowMotion, sharedData.dialog.getWindowHandle()),
		effectUpCommand(sharedData, true),
		effectDownCommand(sharedData, false),
		loadCommand(sharedData),
		physicsCommand(sharedData)
	{
		mainDialog.setPosition(0, 0);
		renderDialog.setPosition(230, 0);

		window.setMouse(mouse);
		renderDialog.setMouse(mouse);
		mouse.setTrackWindow(renderDialog.getWindowHandle());

		mainDialog.getCommandList().addCommand(IDC_NEW_EFFECT, &newEffectCommand);
		mainDialog.getCommandList().addCommand(IDC_LOAD_EFFECT, &loadEffectCommand);
		mainDialog.getCommandList().addCommand(IDC_SAVE_EFFECT, &saveEffectCommand);
		mainDialog.getCommandList().addCommand(IDC_SAVE_EFFECT_AS, &saveEffectAsCommand);
		mainDialog.getCommandList().addCommand(IDC_INSERT_PARTICLE_SYSTEM, &addParticleSystemCommand);
		mainDialog.getCommandList().addCommand(IDC_REMOVE_PARTICLE_SYSTEM, &removeParticleSystemCommand);
		mainDialog.getCommandList().addCommand(IDC_EDIT_PARTICLE_SYSTEM, &editParticleSystemCommand);
		mainDialog.getCommandList().addCommand(IDC_COLOR, &colorCommand);
		mainDialog.getCommandList().addCommand(IDC_PLAY, &playCommand);
		mainDialog.getCommandList().addCommand(IDC_STOP, &stopCommand);
		mainDialog.getCommandList().addCommand(IDC_RELOAD, &reloadCommand);
		mainDialog.getCommandList().addCommand(IDC_LOAD_MODEL, &loadModelCommand);
		mainDialog.getCommandList().addCommand(IDC_CLEAR_MODELS, &clearModelsCommand);
		mainDialog.getCommandList().addCommand(IDC_INSERT_FORCE, &insertForceCommand);
		mainDialog.getCommandList().addCommand(IDC_REMOVE_FORCE, &removeForceCommand);
		mainDialog.getCommandList().addCommand(IDC_EDIT_FORCE, &editForceCommand);
		mainDialog.getCommandList().addCommand(IDC_PARTICLE_SYSTEMS, &selectParticleSystemCommand);
		mainDialog.getCommandList().addCommand(IDC_ENABLE, &enableParticleSystemCommand);
		mainDialog.getCommandList().addCommand(IDC_DISABLE, &disableParticleSystemCommand);
		mainDialog.getCommandList().addCommand(IDC_RENAME, &renameParticleSystemCommand);

		mainDialog.getCommandList().addCommand(IDC_SLOW_MOTION, &slowMotionCommand);
		mainDialog.getCommandList().addCommand(IDC_SHOW_EMITTERS, &showEmittersCommand);
		mainDialog.getCommandList().addCommand(IDC_EFFECT_UP, &effectUpCommand);
		mainDialog.getCommandList().addCommand(IDC_EFFECT_DOWN, &effectDownCommand);
		mainDialog.getCommandList().addCommand(IDC_VIEWER_CONFIG, &loadCommand);

		enableCheck(sharedData.dialog, IDC_PHYSICS, true);
		mainDialog.getCommandList().addCommand(IDC_PHYSICS, &physicsCommand);

		camera.setToSky();

		orbitCamera.setPosition(Vector(0.0f, 20.0f, -20.0f));
		orbitCamera.setTarget(Vector(0.0f, 0.0f, 0.0f));
		orbitCamera.setFov(120.0f);

		enableCheck(sharedData.dialog, IDC_DISTORTION, true);
		enableCheck(sharedData.dialog, IDC_GLOW, false);
		enableCheck(sharedData.dialog, IDC_FORCE_AMBIENT, true);

		setSliderRange(sharedData.dialog, IDC_TIME_FACTOR, 0, 15);
		setSliderValue(sharedData.dialog, IDC_TIME_FACTOR, 6);
		enableDialogItem(sharedData.dialog, IDC_TIME_FACTOR, false);
	}

	bool ignoreMessage(MSG message)
	{

		// Ignore keys that are used as controls
		if((message.message == WM_KEYDOWN) || (message.message == WM_KEYUP))
		{
			switch(message.wParam)
			{
				case 'W':
				case 'A':
				case 'S':
				case 'D':
				case 'Q':
				case 'E':
				case VK_ADD:
				case VK_SUBTRACT:
//				case VK_LEFT:
//				case VK_RIGHT:
//				case VK_UP:
//				case VK_DOWN:
					return true;
			}
		}

		return false;
	}

	void handleMessage()
	{
		MSG windowsMessage = { 0 };
		
		if(GetMessage(&windowsMessage, 0, 0, 0) <= 0)
		{
			mustQuit = true;
			return;
		}

		if(windowsMessage.message == WM_MOUSELAST+1)
			windowsMessage.hwnd = window.getWindowHandle();

		if(ignoreMessage(windowsMessage))
			return;

if(hideCursor)
	SetCursor(0);

		TranslateMessage(&windowsMessage);
		DispatchMessage(&windowsMessage);
	}

	void windowsMessages()
	{
		if(!window.isActive())
			handleMessage();

		MSG windowsMessage = { 0 };
		while(PeekMessage(&windowsMessage, 0, 0, 0, PM_NOREMOVE)) 
			handleMessage();

		Sleep(0);
	}

	bool isKeyDown(int key) {

		if(GetKeyState(key) & 0x80)
			return true;
	
		return false;
	}

	void tick(float t)
	{
		mouse.update();
		camera.update(mouse, true);
		/*
		float rotateFactor = 0.001f;
		float moveFactor = 0.01f;
		if(isKeyDown(VK_SHIFT))
			moveFactor = 0.1f;

		float dy = 0.0f;
		float dx = 0.0f;
		float zoom = 0.0f;
		if(isKeyDown('A')) {
			dy += rotateFactor * t;
		}
		if(isKeyDown('D')) {
			dy -= rotateFactor * t;
		}		
		if(isKeyDown('W')) {
			dx += rotateFactor * t;	
		}
		if(isKeyDown('S')) {
			dx -= rotateFactor * t;	
		}
		if(isKeyDown('Z') || isKeyDown(VK_ADD)) {
			zoom += moveFactor * t;	
		}
		if(isKeyDown('X') || isKeyDown(VK_SUBTRACT)) {
			zoom -= moveFactor * t;
		}

		orbitCamera.orbit(dy, dx);
		orbitCamera.truck(zoom);
		orbitCamera.apply(storm.scene->GetCamera());
		*/
	}

	void tickEffects(float factor) 
	{
		sharedData.tick();
    
		// 10 msec advance in wind too (jpk)
		frozenbyte::particle::WindParticleForce::advanceWind(factor * (float)10 / 1000.0f);
	}
};

Application::Application()
{
	filesystem::FilePackageManager &manager = filesystem::FilePackageManager::getInstance();
	boost::shared_ptr<filesystem::IFilePackage> standardPackage(new filesystem::StandardPackage());
	manager.addPackage(standardPackage, 0);

	boost::scoped_ptr<ApplicationData> tempData(new ApplicationData());
	data.swap(tempData);
}

Application::~Application()
{
}

#ifdef PHYSICS_PHYSX
	struct Hack
	{
		IStorm3D_Model *m;
		boost::shared_ptr<physics::BoxActor> actor;
	};
	struct Hack2
	{
		IStorm3D_Model *m;
		boost::shared_ptr<physics::ConvexActor> actor;
	};

	struct JointThing
	{
		std::vector<boost::shared_ptr<physics::ActorBase> > actors;
		std::vector<boost::shared_ptr<physics::SphericalJoint> > joints;

		//static const int BOX_AMOUNT = 16;
		static const int BOX_AMOUNT = 20;
		//static const int BOX_AMOUNT = 24;
		//static const int BOX_AMOUNT = 32;

		void createStuff(boost::shared_ptr<physics::PhysicsLib> &physics)
		{
			if(!physics)
				return;

			const float HEIGHT = 1.f;
			const bool USE_BOX = TRUE;

			for(int i = 0; i < BOX_AMOUNT; ++i)
			{
				VC3 size(0.5f, 0.5f, 1.f);
				VC3 pos(0, HEIGHT, i * 2.f);

				if(i == 0)
				{
					size = VC3(0.1f, 0.1f, 0.1f);
					pos = VC3(0, HEIGHT, 0.9f);
				}

				boost::shared_ptr<physics::ActorBase> actor;

				if(USE_BOX)
					actor = physics->createBoxActor(size, pos);
				else
				{
					actor = physics->createCapsuleActor(1.5f, 0.25f, pos);
					QUAT rot;
					rot.MakeFromAngles(PI/2.f, 0, 0);
					actor->setRotation(rot);
				}

				if(actor)
				{
					if(i == 0)
						actor->enableFeature(physics::ActorBase::KINEMATIC_MODE, true);

					actor->enableFeature(physics::ActorBase::DISABLE_GRAVITY, true);
					actor->setMass(30.f);
					actor->setDamping(3.f, 0.f);

					if(i == BOX_AMOUNT - 1)
						actor->setMass(70.f);
					else
						actor->setMass(30.f);

					//actor->setCollisionGroup(4);
					actors.push_back(actor);
				}

				if(i != 0)
				{
					VC3 anchor;
					if(USE_BOX)
						anchor = VC3(0, HEIGHT + 0.5f, (i * 2.f) - 1.f);
					else
						anchor= VC3(0, HEIGHT, i * 2.f);

					boost::shared_ptr<physics::SphericalJoint> joint = physics->createSphericalJoint(actors[i - 1], actors[i], anchor);
					joints.push_back(joint);
				}
			}
		}

		void moveHead(const VC3 &force)
		{
			if(!actors.empty())
				actors[actors.size() - 1]->addImpulse(force);
		}

		void update()
		{
			static float angle = 0.f;
			angle += 0.02f;
			QUAT rot;
			rot.MakeFromAngles(0, angle, 0);
			//actors[0]->setRotation(rot);
		}

		void moveTowards(const VC3 &pos)
		{
			if(!actors.empty())
			{
				boost::shared_ptr<physics::ActorBase> actor = actors[actors.size() - 1];
				VC3 actorPos;
				actor->getMassCenterPosition(actorPos);

				VC3 force = pos - actorPos;
				
				/*
				// HAXHAX
				{
					VC3 vel;
					vel.y = 0.f;
					actor->getVelocity(vel);

					VC3 temp = pos + (vel / 60.f);
					if(temp.GetSquareLength() > pos.GetSquareLength())
						actor->setVelocity(VC3());
				}
				*/

				force *= 89.f;
//force *= 0.1f;

				actor->addImpulse(force);

force *= 0.05f;
//force *= 0.1f;
//force *= 0.075f;

				for(unsigned int i = 1; i < actors.size() - 1; ++i)
					actors[i]->addImpulse(force);
			}
		}

		void moveWithDelta(const VC2I delta)
		{
			if(!actors.empty())
			{
				boost::shared_ptr<physics::ActorBase> actor = actors[actors.size() - 1];

				VC3 actorPos;
				actor->getMassCenterPosition(actorPos);
				actorPos.y = 1.f;
				actorPos.x += float(delta.x) * 0.4f;
				actorPos.z -= float(delta.y) * 0.4f;

				moveTowards(actorPos);
			}
		}

		void clear()
		{
			actors.clear();
			joints.clear();
		}
	};
#endif

void Application::run(std::string startupFilename)
{
	data->storm.scene->DrawBones(true);
	data->camera.setToOrigo();
	data->reloadCommand.execute(0);
	data->sharedData.setBackGround();

	if (!startupFilename.empty())
	{
		data->sharedData.load(startupFilename);
		data->sharedData.updateDialog();
	}

	int fps = 0;
	int fpsTimer = 0;
	int fpsFrames = 0;

#ifdef PHYSICS_PHYSX
	boost::shared_ptr<physics::PhysicsLib> physics(new physics::PhysicsLib(false, false, true));
	data->sharedData.physics = physics;
	//boost::shared_ptr<physics::PhysicsLib> physics(new physics::PhysicsLib(true, false, true));
	data->sharedData.effectManager.enablePhysics(true);
	data->sharedData.effectManager.enableParticlePhysics(true);
	data->sharedData.effectManager.setPhysics(data->sharedData.physics);

	std::vector<Hack> boxes;
	std::vector<Hack2> convexes;

	physics->setTimeStep(1/67.f);
	//physics->addGroundPlane(-200.f);
	physics->addGroundPlane(0.f);
	//physics->enableCollision(2, 2, false);
	//physics->enableCollision(1, 2, false);

	boost::shared_ptr<physics::StaticMeshActor> meshActor;
	boost::shared_ptr<physics::StaticMesh> staticMesh;
	boost::shared_ptr<physics::ConvexMesh> cylinderMesh;

	//std::vector<unsigned short> fooBuffer(4);
	//boost::shared_ptr<physics::HeightmapActor> heightmap = physics->createHeightmapActor(&fooBuffer[0], 2, 2, VC3(10.f, 1.f, 10.f));
	//boost::shared_ptr<physics::CapsuleActor> capsule = physics->createCapsuleActor(1.3f, 0.3f, VC3());

	data->sharedData.effectManager.setModelParticleParameters(500, 50);
	//physics->enableFeature(physics::PhysicsLib::VISUALIZE_FLUIDS, true);
	//physics->enableFeature(physics::PhysicsLib::VISUALIZE_DYNAMIC, true);
	//physics->enableFeature(physics::PhysicsLib::VISUALIZE_COLLISION_SHAPES, true);
	//physics->enableFeature(physics::PhysicsLib::VISUALIZE_JOINTS, true);
	//physics->enableFeature(physics::PhysicsLib::VISUALIZE_STATIC, true);
	//physics->enableFeature(physics::PhysicsLib::VISUALIZE_COLLISION_CONTACTS, true);

	//JointThing jointThing;
	//jointThing.createStuff(physics);
#endif

/*
IStorm3D_Model *model = data->storm.storm->CreateNewModel();
IStorm3D_BoneAnimation *anim = data->storm.storm->CreateNewBoneAnimation("Data/Animations/Humans/die_1.anm");
//IStorm3D_BoneAnimation *anim = data->storm.storm->CreateNewBoneAnimation("Data/Animations/Humans/walk.anm");
model->LoadS3D("Data/Models/Humans/Wesley_tyler/Wesley_tyler.s3d");
model->LoadBones("Data/Models/Humans/human_bones.b3d");
model->SetAnimation(0, anim, true);
data->storm.scene->AddModel(model);
*/

	int lastTime = timeGetTime();
	int timeCount = 0;
	while(!data->mustQuit)
	{

if(data->sharedData.effect)
{
	for(int i = 0; i < data->sharedData.effect->getNumSystems(); ++i)
	{
		IParticleSystem *system = data->sharedData.effect->getParticleSystem(i);
		system->setSpawnModel(data->sharedData.model.getModel());
	}
}

if(data->storm.scene)
{
	boost::scoped_ptr<Iterator<IStorm3D_Model *> > it(data->storm.scene->ITModel->Begin());
	for(; !it->IsEnd(); it->Next())
	{
		IStorm3D_Model *m = it->GetCurrent();
		if(!m)
			continue;

		m->SetSelfIllumination(COL(0.33f, 0.33f, 0.33f));
		m->SetLighting(0, 0);
		for(int i = 1; i < LIGHT_MAX_AMOUNT; ++i)
		{
//			m->SetLighting(i, -1);
		}
	}
}


#ifdef PHYSICS_PHYSX
		for(unsigned int i = 0; i < boxes.size(); ++i)
		{
			Hack &h = boxes[i];
			
			VC3 pos;
			h.actor->getPosition(pos);
			QUAT rot;
			h.actor->getRotation(rot);

			h.m->SetPosition(pos);
			h.m->SetRotation(rot);
		}
		for(unsigned int i = 0; i < convexes.size(); ++i)
		{
			Hack2 &h = convexes[i];
			
			VC3 pos;
			h.actor->getPosition(pos);
			QUAT rot;
			h.actor->getRotation(rot);

			h.m->SetPosition(pos);
			h.m->SetRotation(rot);
		}

		{
			/*
			for(int i = 1; i < jointThing.actors.size() - 2; ++i)
			{
				VC3 vel;
				jointThing.actors[i]->getVelocity(vel);
				vel *= 0.5f;
				jointThing.actors[i]->setVelocity(vel);
			}
			*/

			/*
			float forceFactor = 50.f;
			if(GetKeyState(VK_LEFT) & 0x80)
				jointThing.moveHead(VC3(-forceFactor, 0, 0));
			if(GetKeyState(VK_RIGHT) & 0x80)
				jointThing.moveHead(VC3(forceFactor, 0, 0));
			if(GetKeyState(VK_UP) & 0x80)
				jointThing.moveHead(VC3(0, 0, forceFactor));
			if(GetKeyState(VK_DOWN) & 0x80)
				jointThing.moveHead(VC3(0, 0, -forceFactor));

			jointThing.update();

			//if(GetKeyState(VK_SPACE) & 0x80)
			if(data->mouse.isLeftButtonDown() && !(GetKeyState(VK_RETURN) & 0x80))
			{
				VC2I foofoo(data->mouse.getX(), data->mouse.getY());
				VC3 cursorPos;
				VC3 cursorDir;
				float wantedHeight = 1.f;

				data->storm.scene->GetEyeVectors(foofoo, cursorPos, cursorDir);
				float distance = (wantedHeight - cursorPos.y) / cursorDir.y;
				VC3 pos = cursorPos + (cursorDir * distance);
				jointThing.moveTowards(pos);
			}

			{
				static bool dragState = false;
				bool Retpressed = false;
				if(GetKeyState(VK_RETURN) & 0x80)
				{
					if(!dragState)
						Retpressed = true;
					dragState = true;
				}
				else
					dragState = false;

				static int mouseX = data->mouse.getX();
				static int mouseY = data->mouse.getY();

				int MOUSE_X_CENTER = 300;
				int MOUSE_Y_CENTER = 300;

				if(Retpressed)
					SetCursorPos(MOUSE_X_CENTER, MOUSE_Y_CENTER);

				if(!Retpressed && GetKeyState(VK_RETURN) & 0x80)
				{
					hideCursor = true;
					//VC2I delta(data->mouse.getX() - mouseX, data->mouse.getY() - mouseY);

					int x = 0;
					int y = 0;

					POINT p = { 0 };
					GetCursorPos(&p);

					VC2I delta(p.x - MOUSE_X_CENTER, p.y - MOUSE_Y_CENTER);

					if(data->mouse.isLeftButtonDown())
						delta *= 2;
					jointThing.moveWithDelta(delta);

					SetCursorPos(MOUSE_X_CENTER,MOUSE_Y_CENTER);
				}
				else
					hideCursor = false;

				mouseX = data->mouse.getX();
				mouseY = data->mouse.getY();
			}
			*/
		}

		static bool Idown = false;
		bool Ipressed = false;
		if(GetKeyState('I') & 0x80)
		{
			if(!Idown)
				Ipressed = true;
			Idown = true;
		}
		else
			Idown = false;

		if(Ipressed)
		{
			Hack h;
			VC3 start(0, 10.f, 0);

			h.m = data->storm.storm->CreateNewModel();
			h.m->LoadS3D("Data/Models/Terrain_Objects/Containers/Crates/Crate1.s3d");
			data->storm.scene->AddModel(h.m);

			AABB aabb = h.m->GetBoundingBox();
			VC3 size = aabb.mmax - aabb.mmin;
			size *= 0.5f;

			h.m->SetPosition(start);

			h.actor = physics->createBoxActor(size, start);
			if(h.actor)
			{
				h.actor->setMass(50.f);
				h.actor->setCollisionGroup(7);
				boxes.push_back(h);
			}
		}

		static bool Odown = false;
		bool Opressed = false;
		if(GetKeyState('O') & 0x80)
		{
			if(!Odown)
				Opressed = true;
			Odown = true;
		}
		else
			Odown = false;

		if(Opressed)
		{
			Hack2 h;
			VC3 start(0, 10.f, 0);

			h.m = data->storm.storm->CreateNewModel();
			h.m->LoadS3D("Data/Models/Terrain_Objects/Containers/Barrels/Barrel1.s3d");
			data->storm.scene->AddModel(h.m);

			static bool cooked = false;
			if(!cooked)
			{
				AABB aabb = h.m->GetBoundingBox();
				VC3 size = aabb.mmax - aabb.mmin;

				float height = size.y;
				float radius = size.x * 0.5f;

				physics::Cooker cooker;
				cooker.cookCylinder("cylinder.dat", height, radius);
				cooked = true;

				cylinderMesh = physics->createConvexMesh("cylinder.dat");
				assert(cylinderMesh->isValidForHardware());
			}

			h.m->SetPosition(start);
			h.actor = physics->createConvexActor(cylinderMesh, start);
			if(h.actor)
			{
				h.actor->setMass(25.f);
				h.actor->setCollisionGroup(7);
				convexes.push_back(h);
			}
		}

		static bool Mdown = false;
		if(!Mdown && GetKeyState('M') & 0x80)
		{
			//VC3 pos(7.0f, -1.0f, 0.0f);
			VC3 pos(0.0f, 0.0f, 0.0f);

			Mdown = true;
			IStorm3D_Model *m = data->storm.storm->CreateNewModel();
			m->LoadS3D("Data/Models/Buildings/Powerplant/powerplant.s3d");
			//m->LoadS3D("Data/Models/Buildings/CommunicationCenter/communicationcenter.s3d");
			//m->LoadS3D("Data/Models/Buildings/WTF/WTF_undergroundlevel.s3d@180");
			data->storm.scene->AddModel(m);

			physics::Cooker cooker;
			cooker.cookMesh("building.dat@180", m);

			QUAT q;
			//q.MakeFromAngles(0.4f, 0.2f, 0);
			m->SetRotation(q);
			m->SetPosition(pos);

			staticMesh = physics->createStaticMesh("building.dat@180");
			meshActor = physics->createStaticMeshActor(staticMesh, pos, q);

			boost::scoped_ptr<Iterator<IStorm3D_Model_Object *> > objectIterator(m->ITObject->Begin());
			for(; !objectIterator->IsEnd(); objectIterator->Next())
			{
				IStorm3D_Model_Object *object = objectIterator->GetCurrent();
				if(!object)
					continue;

				if(strstr(object->GetName(), "BuildingRoof"))
					object->SetNoRender(true);
			}
		}

		static bool Spdown = false;
		bool Sppressed = false;
		if(GetKeyState('U') & 0x80)
		{
			if(!Spdown)
				Sppressed = true;
			Spdown = true;
		}
		else
			Spdown = false;

		if(Sppressed)
		{
			IStorm3D_Camera *c = data->storm.scene->GetCamera();
			VC3 p = c->GetPosition();
			VC3 d = (c->GetTarget() - p).GetNormalized();

			Hack h;

			h.m = data->storm.storm->CreateNewModel();
			h.m->LoadS3D("Data/Models/Terrain_Objects/Containers/Crates/AlienCrate_1.s3d");
			data->storm.scene->AddModel(h.m);

			AABB aabb = h.m->GetBoundingBox();
			VC3 size = aabb.mmax - aabb.mmin;
			size *= 0.5f;

			h.m->SetPosition(p);
			h.actor = physics->createBoxActor(size, p);
			if(h.actor)
			{
				h.actor->setMass(30.f);
				h.actor->setVelocity(d * 50.f);

				boxes.push_back(h);
			}
		}

		static bool Ydown = false;
		bool Ypressed = false;
		if(GetKeyState('Y') & 0x80)
		{
			if(!Ydown)
				Ypressed = true;
			Ydown = true;
		}
		else
			Ydown = false;

		if(Ypressed)
		{
			//VC3 impulse(0, 1000.f, 0);
			float strength = 400.f;
			VC3 pos(0, 0, 0);

			for(unsigned int i = 0; i < boxes.size(); ++i)
			{
				Hack &h = boxes[i];

				VC3 impulse;
				h.actor->getMassCenterPosition(impulse);
				impulse -= pos;
				float distance = impulse.GetLength();
				if(distance > 0.001f)
				{
					impulse /= distance;
					if(distance < 2)
						distance = 2;

					impulse *= strength;
					impulse /= distance;
				}
				else
					impulse = VC3(0, strength, 0);

				//impulse.y *= 0.75f;
				h.actor->addImpulse(pos, impulse);
			}

			for(unsigned int i = 0; i < convexes.size(); ++i)
			{
				Hack2 &h = convexes[i];

				VC3 impulse;
				h.actor->getMassCenterPosition(impulse);
				impulse -= pos;
				float distance = impulse.GetLength();
				if(distance > 0.001f)
				{
					impulse /= distance;
					if(distance < 2)
						distance = 2;

					impulse *= strength;
					impulse /= distance;
				}
				else
					impulse = VC3(0, strength, 0);

				//impulse.y *= 0.75f;
				h.actor->addImpulse(pos, impulse);
			}

			data->sharedData.effectManager.addPhysicsExplosion(VC3(), 1.f);
		}

		static bool Tdown = false;
		if(!Tdown && GetKeyState('T') & 0x80)
		{
			Tdown = true;

			Hack2 h;
			VC3 start(0, 5.f, 0);

			h.m = data->storm.storm->CreateNewModel();
			h.m->LoadS3D("Data/Models/Terrain_Objects/Containers/Crates/Crate1.s3d");
			data->storm.scene->AddModel(h.m);


			boost::shared_ptr<physics::ConvexMesh> mesh;
			{
				physics::Cooker cooker;

				boost::scoped_ptr<Iterator<IStorm3D_Model_Object *> > objectIterator(h.m->ITObject->Begin());
				IStorm3D_Model_Object *o = 0;
				for(; !objectIterator->IsEnd(); objectIterator->Next())
				{
					if(!o)
						o = objectIterator->GetCurrent();
				}

				/*
				AABB aabb = h.m->GetBoundingBox();
				VC3 size = aabb.mmax - aabb.mmin;

				float height = size.y;
				float radius = size.x * 0.5f;

				physics::Cooker cooker(*physics);
				cooker.cookCylinder("cylinder.dat", height, radius);
				cooked = true;
				*/

				cooker.cookApproxConvex("testconvex.dat", o);
				mesh = physics->createConvexMesh("testconvex.dat");
			}

			h.m->SetPosition(start);
			h.actor = physics->createConvexActor(mesh, start);
			if(h.actor)
			{
				h.actor->setMass(25.f);
				h.actor->setCollisionGroup(7);
				convexes.push_back(h);
			}
		}
#endif

		//int time = timeGetTime();
		//int deltaTime = time - lastTime;

		if(data->storm.terrain)
		{
#ifdef PHYSICS_PHYSX
			data->storm.terrain->getRenderer().enableFeature(IStorm3D_TerrainRenderer::Collision, isCheckEnabled(data->sharedData.dialog, IDC_GLOW));
			data->storm.terrain->getRenderer().enableFeature(IStorm3D_TerrainRenderer::Wireframe, isCheckEnabled(data->sharedData.dialog, IDC_GLOW));
			physics->enableFeature(frozenbyte::physics::PhysicsLib::VISUALIZE_FLUIDS, isCheckEnabled(data->sharedData.dialog, IDC_FLUID_STUFF));
#endif

			data->storm.terrain->getRenderer().enableFeature(IStorm3D_TerrainRenderer::Glow, isCheckEnabled(data->sharedData.dialog, IDC_GLOW));
			data->storm.terrain->getRenderer().enableFeature(IStorm3D_TerrainRenderer::Distortion, isCheckEnabled(data->sharedData.dialog, IDC_DISTORTION));

			if(isCheckEnabled(data->sharedData.dialog, IDC_FORCE_AMBIENT))
				data->storm.terrain->getRenderer().setFloatValue(IStorm3D_TerrainRenderer::ForceAmbient, 0.5f);
			else
				data->storm.terrain->getRenderer().setFloatValue(IStorm3D_TerrainRenderer::ForceAmbient, 0.0f);
		}

#ifdef PHYSICS_PHYSX
	data->sharedData.effectManager.enableParticlePhysics(isCheckEnabled(data->sharedData.dialog, IDC_DISTORTION));
#endif

		int time = 0;
		int deltaTime = 0;
		while(deltaTime < 20)
		{
			time = timeGetTime();
			deltaTime = time - lastTime;
		}

		float timeFactor = 1.f;
		if(isCheckEnabled(data->sharedData.dialog, IDC_SLOW_MOTION))
		{
			enableDialogItem(data->sharedData.dialog, IDC_TIME_FACTOR, true);
			timeFactor = getSliderValue(data->sharedData.dialog, IDC_TIME_FACTOR) / 6.f;
		}
		else
		{
			enableDialogItem(data->sharedData.dialog, IDC_TIME_FACTOR, false);
		}

		int realDeltaTime = deltaTime;
		deltaTime = int((float(deltaTime) * timeFactor));
		if(data->sharedData.skipTimeUpdate)
		{
			deltaTime = 0;
			data->sharedData.skipTimeUpdate = false;
		}

		timeCount += deltaTime;
		lastTime = time;
		while(timeCount > 20) 
		{
			timeCount -= 20;
			data->tickEffects(timeFactor);
			data->effectManager.tick();
		}

		data->tick((float)realDeltaTime); // move
		data->windowsMessages();

		if(!data->window.isActive())
			continue;

		float yAngleDelta = data->mouse.getWheelDelta() / 2000.f;
		yAngleDelta += data->camera.getHorizontal();
		float xAngleDelta = data->camera.getVertical();

		data->effectManager.render();

		fpsTimer += realDeltaTime;
		++fpsFrames;
		if(fpsTimer > 500)
		{
			fps = fpsFrames * 2;
			fpsTimer = 0;
			fpsFrames = 0;
		}

		const ParticleEffectManager::Stats& stats = data->effectManager.getStats();
		std::string maxParticles = "max particles = " + convertToString<int>(stats.maxParticles);
		std::string numParticles = "num particles = " + convertToString<int>(stats.numParticles);
		std::string maxParticlesPerSystem = "max particles per system = " + convertToString<int>(stats.maxParticlesPerSystem);
		std::string fpsString = "fps = " + convertToString<int>(fps);

#ifdef PHYSICS_PHYSX
//HAXHAX
fpsString += "  -- actor create count " + convertToString<int> (physics::getActorBaseCreateCount());
#endif

		data->font.renderText(32,16,numParticles);
		data->font.renderText(32,32,maxParticles);
		data->font.renderText(32,48,maxParticlesPerSystem);
		data->font.renderText(32,66,fpsString);

#ifdef PHYSICS_PHYSX
		std::string physicsStats = physics->getStatistics();
		data->font.renderText(32,100,physicsStats);
#endif

//physics->enableFeature(physics::PhysicsLib::VISUALIZE_COLLISION_SHAPES, true);
//physics->enableFeature(physics::PhysicsLib::VISUALIZE_STATIC, true);
//data->storm.terrain->getRenderer().enableFeature(IStorm3D_TerrainRenderer::Collision, true);

		//physics->enableFeature(physics::PhysicsLib::VISUALIZE_COLLISION_SHAPES, true);
		//data->storm.terrain->getRenderer().enableFeature(IStorm3D_TerrainRenderer::Collision, true);

#ifdef PHYSICS_PHYSX
		physics::visualize(*physics, *data->storm.scene, PARTICLE_EDITOR_VISUALIZE_RANGE);
		physics->startSimulation(deltaTime / 1000.f);
#endif
		data->storm.scene->RenderScene();
#ifdef PHYSICS_PHYSX
		physics->finishSimulation();
		data->sharedData.effectManager.updatePhysics();
#endif

		/*		
		physics->finishSimulation();
		physics::visualize(*physics, *data->storm.scene, PARTICLE_EDITOR_VISUALIZE_RANGE);
		data->sharedData.effectManager.updatePhysics();
		physics->startSimulation(deltaTime / 1000.f);
		data->storm.scene->RenderScene();
		*/
	}

#ifdef PHYSICS_PHYSX
	//jointThing.clear();	
#endif

	data->sharedData.stop();
	if(data->sharedData.effect)
		data->sharedData.effect.reset();
#ifdef PHYSICS_PHYSX
	data->sharedData.effectManager.releasePhysicsResources();
#endif
	data->sharedData.effectManager.reset();
	data->sharedData.effect.reset();
}

} // end of namespace viewer
} // end of namespace frozenbyte
