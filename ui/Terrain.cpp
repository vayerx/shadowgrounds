#include "precompiled.h"

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <deque>
#include <map>
#include <string>
#include <boost/scoped_array.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/lexical_cast.hpp>

#include "Terrain.h"
#include "../game/materials.h"
#include <Storm3D_UI.h>
#include <IStorm3D_Bone.h>
#include <istorm3D_terrain_renderer.h>
#include <Storm3D_ObstacleMapDefs.h>
#include "../util/Parser.h"
#include "../util/AreaMap.h"
#include "../util/FogApplier.h"
#include "../util/fb_assert.h"
#include "../util/ColorMap.h"
#include "../game/gamedefs.h"
#include "../game/GameMap.h"
#include "../system/Logger.h"
#include "../filesystem/input_stream_wrapper.h"
#include "../filesystem/file_package_manager.h"
#include "../filesystem/input_file_stream.h"
#include "../convert/str2int.h"
#include "../game/SimpleOptions.h"
#include "../game/options/options_graphics.h"
#include "../util/Debug_MemoryManager.h"
#include "../game/physics/GamePhysics.h"
#include "../game/physics/BoxPhysicsObject.h"
#include "../game/physics/StaticPhysicsObject.h"
#include "../game/physics/CapsulePhysicsObject.h"
#include "../game/physics/physics_collisiongroups.h"
#include "../physics/physics_lib.h"
#include "../ui/LightManager.h"
#include "../ui/AmbientSoundManager.h"
#ifdef PHYSICS_PHYSX
#include "../game/physics/RackPhysicsObject.h"
#include "../game/physics/TerrainPhysicsObject.h"
#include "../game/physics/ConvexPhysicsObject.h"
#include "../physics/cooker.h"
#include "../game/options/options_physics.h"
#include "../system/FileTimestampChecker.h"
#ifdef PROJECT_SHADOWGROUNDS
#include "../editor/string_conversions.h"
#include "../editor/parser.h"
#endif
#endif
#ifdef PHYSICS_ODE
#include "../game/physics/CylinderPhysicsObject.h"
#endif
#include "../util/SoundMaterialParser.h"
#include "../util/ObjectDurabilityParser.h"
#include "../util/GridOcclusionCuller.h"
#include "../util/StringUtil.h"
#include "../game/physics/PhysicsContactUtils.h"
#include "../game/Unit.h"
#include "../game/tracking/trackable_types.h"
#ifdef PROJECT_CLAW_PROTO
#include "../game/physics/CarPhysicsObject.h"
#endif
#include "../ui/LoadingMessage.h"
#include "../game/GameScene.h"
#include "../game/unified_handle.h"
#include "../game/tracking/SimpleTrackableUnifiedHandleObjectIterator.h"
#include "../editor/UniqueEditorObjectHandle.h"

#include "igios.h"

#include "../survivor/SurvivorConfig.h"

// Sigh
#ifdef PROJECT_CLAW_PROTO
#include "../game/ClawController.h"
extern ::game::ClawController *gamephysics_clawController;
#endif

using namespace frozenbyte;

#define TERRAIN_OBJECT_PHYSICS_TYPE_INVALID -1
#define TERRAIN_OBJECT_PHYSICS_TYPE_NONE 0
#define TERRAIN_OBJECT_PHYSICS_TYPE_STATIC 1
#define TERRAIN_OBJECT_PHYSICS_TYPE_BOX 2
#define TERRAIN_OBJECT_PHYSICS_TYPE_CYLINDER 3
#define TERRAIN_OBJECT_PHYSICS_TYPE_CAPSULE 4
// TODO: ...and what the f*** are the rest of the physics object type numbers? --jpk


#define TERRAIN_OBJECT_BREAK_TEXTURE_TYPE_INVALID -1
#define TERRAIN_OBJECT_BREAK_TEXTURE_TYPE_NONE 0
#define TERRAIN_OBJECT_BREAK_TEXTURE_TYPE_SCALE_HP 1
#define TERRAIN_OBJECT_BREAK_TEXTURE_TYPE_ALWAYS 2
#define TERRAIN_OBJECT_BREAK_TEXTURE_TYPE_SCRIPTABLE 3
// TODO: and apparently there are some different types of break textures??? number 1 and number 2. 
// which mean exactly what?


#define TERRAIN_OBJECT_FALLTYPE_INVALID -1
#define TERRAIN_OBJECT_FALLTYPE_NONE 0
#define TERRAIN_OBJECT_FALLTYPE_SOMEMYSTERIOUSTYPE1 1
// TODO: there was some falltype 2 too or maybe not??? and what does this falltype 1 mean anyway?


#define TERRAIN_DYNAMIC_OBSTACLE_HEIGHT 650


#define TERRAIN_OBJECT_VARIABLES_AMOUNT 8

#define TERRAIN_MAKE_STRING(m) #m
#define TERRAIN_STRINGIFY(m) TERRAIN_MAKE_STRING(m)

extern game::GameScene *gameScene_instance;

// HACK: !!!
extern bool signal_this_terrain_object_break_hack;


bool terrain_object_variables_inited = false;
std::string terrain_object_variable_name[TERRAIN_OBJECT_VARIABLES_AMOUNT] = 
{
	"", "", "", "", "", "", "", "",
};


static void init_terrain_object_variables()
{
	if (!terrain_object_variables_inited)
	{
		terrain_object_variables_inited = true;

		util::SimpleParser sp;
#ifdef LEGACY_FILES
		bool loadOk = sp.loadFile("Data/Misc/terrain_object_variables.txt");
#else
		bool loadOk = sp.loadFile("data/misc/terrain_object_variables.txt");
#endif
		if (loadOk)
		{
			while (sp.next())
			{
				const char *key = sp.getKey();
				if (key != NULL)
				{
					int keyNum = str2int(key);
					if (str2int_errno() == 0)
					{
						// NOTE: 0 excluded!
						if (keyNum > 0 && keyNum < TERRAIN_OBJECT_VARIABLES_AMOUNT)
						{
							const char *value = sp.getValue();
							if (value != NULL
								&& value[0] != '\0')
							{
								terrain_object_variable_name[keyNum] = std::string(value);
							} else {
								sp.error("init_terrain_object_variables - value missing.");
							}
						} else {
							sp.error("init_terrain_object_variables - key value out of range (expected positive integer value below " TERRAIN_STRINGIFY(TERRAIN_OBJECT_VARIABLES_AMOUNT) ").");
						}
					} else {
						sp.error("init_terrain_object_variables - bad key (expected positive integer value).");
					}
				} else {
					sp.error("init_terrain_object_variables - bad line (expected key = value pair).");
				}
			}
		} else {
			sp.error("init_terrain_object_variables - Failed to load terrain object variable names.");
		}
		
		// get rid of empty/unused names. (by naming them _reserved_x)
		for (int i = 0; i < TERRAIN_OBJECT_VARIABLES_AMOUNT; i++)
		{
			if (terrain_object_variable_name[i].empty())
			{
				terrain_object_variable_name[i] = std::string("_reserved_") + int2str(i);
			}
		}
	}
}

static void uninit_terrain_object_variables()
{
	if (terrain_object_variables_inited)
	{
		terrain_object_variables_inited = false;

		// make them all empty.
		for (int i = 0; i < TERRAIN_OBJECT_VARIABLES_AMOUNT; i++)
		{
			terrain_object_variable_name[i].clear();
		}
	}
}

	static const int BLOCK_SIZE = IStorm3D_Terrain::BLOCK_SIZE;
	const float UPDATE_PHYSICS_RANGE = 0.0015f;
	const float UPDATE_LIGHT_RANGE = 0.1f;

	static filesystem::InputStream &operator >> (filesystem::InputStream &stream, VC2I &vector)
	{
		return stream >> vector.x >> vector.y;
	}

	static filesystem::InputStream &operator >> (filesystem::InputStream &stream, VC3 &vector)
	{
		return stream >> vector.x >> vector.y >> vector.z;
	}

	static filesystem::InputStream &operator >> (filesystem::InputStream &stream, TColor<unsigned char> &color)
	{
		return stream >> color.r >> color.g >> color.b;
	}

	static float getRadius2d(const VC3 &a, const VC3 &b)
	{
		float xd = a.x - b.x;
		float yd = a.z - b.z;

		return sqrtf(xd*xd + yd*yd);
	}

	static QUAT getRotation(const VC3 &angles)
	{
		QUAT qx;
		qx.MakeFromAngles(angles.x, 0, 0);
		QUAT qy;
		qy.MakeFromAngles(0, angles.y, 0);
		QUAT qz;
		qz.MakeFromAngles(0, angles.z, 0);

		return qz * qx * qy;
	}

	/** TerrainAnimation **/

	class TerrainAnimation
	{
		struct InstanceData
		{
			IStorm3D_Model *model;

			Vector position;
			Vector velocity;
			float height;

			// Angles
			float heading;
			float pitch;
			float fall_speed;

			int time_elapsed;
		};

		std::deque<InstanceData *> animated_models;
		std::deque<InstanceData *> static_models;

	public:
		TerrainAnimation()
		{
		}

		~TerrainAnimation()
		{
			for(std::deque<InstanceData *>::iterator it = animated_models.begin(); it != animated_models.end(); ++it)
				delete *it;

			for(std::deque<InstanceData *>::iterator it = static_models.begin(); it != static_models.end(); ++it)
				delete *it;	
		}

		// Use these to do something ;-)
		void AddModel(IStorm3D_Model *model, float height, const Vector &blastPosition, float blast_radius, bool reverseDirection)
		{
			InstanceData *data = new InstanceData();
			data->model = model;
			data->time_elapsed = 0;
			data->height = height;

			data->position = model->GetPosition();
			data->position.y += .5f * height;
			data->velocity = (data->position - blastPosition);

			if(data->velocity.y > 1.f)
				data->velocity.y = 1.f;
			if(data->velocity.y < -1.f)
				data->velocity.y = -1.f;

			Vector direction = Vector(data->velocity.x, 0, data->velocity.z).GetNormalized();
			float angle = VC2(direction.x, direction.z).CalculateAngle();
			angle = -(angle + 1.75f); // tweak a bit

			// Randomize
			if (reverseDirection)
				angle += 3.145f;

			data->heading = angle;
			data->pitch = 0.f;

			float distance = data->velocity.GetLength();
			float force = (blast_radius / distance);

			data->fall_speed = 2.f * force;
			data->velocity *= force;
			data->velocity.y *= 1.3f;
			animated_models.push_back(data);
		}

		// Milliseconds
		void Update(int time_elapsed, IStorm3D_Scene *scene, IStorm3D_Terrain *terrain)
		{
			float time_delta = static_cast<float> (time_elapsed) / 1000.f;
			
			Vector model_position;
			Vector top_position;
			Rotation x, y, r;

			for(std::deque<InstanceData *>::iterator it = animated_models.begin(); it != animated_models.end(); )
			{
				// Hack, as always ;-)
				InstanceData *data = *it;
				IStorm3D_Model *model = data->model;
				data->time_elapsed += time_elapsed;

				// Mighty physics

				// position
				data->position += data->velocity * time_delta;
				data->velocity.x *= .98f;
				data->velocity.z *= .98f;
				data->velocity.y -= 5.f * time_delta;

				// Model position
				float min_height = terrain->getHeight(Vector2D(data->position.x, data->position.z));
				model_position = data->position;
				model_position.y -= .5f * data->height;

				if(model_position.y < min_height)
				{
					float delta = min_height - model_position.y;
					data->position.y += delta;
					model_position.y += delta;
				}

				// rotation
				data->fall_speed *= 1.01f;
				//if(data->fall_speed > 2.f)
				//	data->fall_speed = 2.f;
				data->pitch -= data->fall_speed * time_delta;

				bool top_touches_ground = false;

				while(true) // Rotate back as long as top is underground
				{
					r.MakeFromAngles(data->pitch, data->heading, 0);

					top_position = Vector(0, data->height, 0);
					r.RotateVector(top_position);
					top_position += model_position;

					float min_height = terrain->getHeight(Vector2D(top_position.x, top_position.z));
					if(top_position.y < min_height)
					{
						data->pitch += 1.f * time_delta;
						top_touches_ground = true;
					}
					else
						break;
				}

				// Set
				model->SetPosition(model_position);
				model->SetRotation(r);

				// Stop movement?
				if((fabsf(data->velocity.x) + fabsf(data->velocity.z) < 0.01f) && (top_touches_ground == true))
				{
					it = animated_models.erase(it);
				}
				else
					++it;
			}
		}
	};

	struct AnimationDeleter
	{
		void operator() (IStorm3D_BoneAnimation *a)
		{
			if(a)
				a->Release();
		}
	};

	struct ObjectData
	{
		std::string fileName;
		bool fileExists;

		float height;
		int type;
		int fallType;
		int obstacleType;
		int breakTexture;

		std::string explosionObstacle;
		std::string explosionScript;
		std::string explosionProjectile;
		std::string explosionEffect;
		std::string material;
		int hp;

		float radius;
		bool fireThrough;

		std::vector<std::string> explosionSounds;
		mutable int nextSound;

		std::string ambientSound;
		int ambientSoundRange;
		int ambientSoundRollOff;

		std::string bones;
		std::string idleAnimation;
		std::vector<boost::shared_ptr<IStorm3D_BoneAnimation> > explosionAnimations;
		int nextAnimation;

		int physicsType;
		float physicsMass;
		std::string physicsSoundMaterial;
		VC3 physicsData1;
		VC3 physicsData2;
		std::string durabilityType;

		TRACKABLE_TYPEID_DATATYPE trackableType;

		std::map<std::string, std::string> metaValues;

		ObjectData()
		:	fileExists(true),
			height(0),
			type(-1),
			fallType(TERRAIN_OBJECT_FALLTYPE_INVALID),
			obstacleType(-1),
			breakTexture(TERRAIN_OBJECT_BREAK_TEXTURE_TYPE_NONE),
			hp(0),
			radius(0),
			fireThrough(true),
			nextSound(0),
			nextAnimation(0),
			physicsType(TERRAIN_OBJECT_PHYSICS_TYPE_INVALID),
			trackableType(0)
			//originalModel(0),
			//originalInstance(0)
		{
		}

		bool hasExplosion() const
		{
			if(breakTexture != TERRAIN_OBJECT_BREAK_TEXTURE_TYPE_NONE
				&& breakTexture != TERRAIN_OBJECT_BREAK_TEXTURE_TYPE_INVALID)
				return true;

			if(!explosionObstacle.empty() || !explosionScript.empty() || !explosionProjectile.empty() || !explosionEffect.empty() || !explosionSounds.empty())
				return true;

			return false;
		}

		bool hasExplosionOnlyWithBreakTexture() const
		{
			if(breakTexture != TERRAIN_OBJECT_BREAK_TEXTURE_TYPE_NONE
				&& breakTexture != TERRAIN_OBJECT_BREAK_TEXTURE_TYPE_INVALID)
			{
				if(explosionObstacle.empty() && explosionScript.empty() && explosionProjectile.empty() && explosionEffect.empty() && explosionSounds.empty())
					return true;
			}

			return false;
		}

		bool hasPhysics() const
		{
			if(physicsType >= 2)
				return true;

			return false;
			//return hasExplosion();
			//if(!explosionObstacle.empty())
			//	return true;

			//return false;
		}
	};

	struct ObjectInstance
	{
		VC2 position;
		VC3 rotation;
		QUAT setRotation;
		VC3 lightUpdatePosition;

		float height; // this seems to be the actual height, updated by physics
		float heightOffset; // this seems to be the relative height offset to ground (on editor)
		float groundHeight; // the new ground height (on editor) - needed to solve correct height offset based on height

		COL ambient;
		//VC3 lightPos[2];
		//COL lightColor[2];
		//float lightRange[2];
		signed short int lightIndices[LIGHT_MAX_AMOUNT];
		VC3 sunDir;
		float sunStrength;
		bool lightmapped;
		float lightMultiplier;
		int hp;
		std::string originalName;
		bool inBuilding;
		boost::shared_ptr<game::AbstractPhysicsObject> physicsObject;

		int originalModel;
		int originalInstance;
		int latestReplacementModel;
		int latestReplacementInstance;
		bool movedByPhysics;
		int lastEffectTime;
		int *variableValues;

		bool dynamicObstacleExists;
		VC2I dynamicObstaclePosition;
		float dynamicObstacleRotation;

		bool deleted;
		std::string idString;

		int ambientSound;

		UniqueEditorObjectHandle uniqueEditorObjectHandle;

		ObjectInstance()
		:	height(0),
			heightOffset(0),
			groundHeight(0),
			sunStrength(0),
			lightmapped(0),
			lightMultiplier(0),
			hp(0),
			inBuilding(true),
			originalModel(0),
			originalInstance(0),
			latestReplacementModel(0),
			latestReplacementInstance(0),
			movedByPhysics(false),
			lastEffectTime(0),
			variableValues(NULL),
			dynamicObstacleExists(false),
			dynamicObstaclePosition(0,0),
			deleted(false),
			ambientSound(-1),
			uniqueEditorObjectHandle(0)
		{
			//lightRange[0] = lightRange[1] = 5.f;

			for(int i = 0; i < LIGHT_MAX_AMOUNT; ++i)
				lightIndices[i] = -1;
		}

		~ObjectInstance()
		{
			if (variableValues != NULL)
			{
				delete [] variableValues;
				variableValues = NULL;
			}
		}

		ObjectInstance(const ObjectInstance &source)
		{
			*this = source;
		}

		ObjectInstance &operator= (const ObjectInstance &source)
		{
			if (source.variableValues != NULL)
			{
				this->variableValues = new int[TERRAIN_OBJECT_VARIABLES_AMOUNT];
				for (int i = 0; i < TERRAIN_OBJECT_VARIABLES_AMOUNT; i++)
				{
					this->variableValues[i] = source.variableValues[i];
				}
			} else {
				this->variableValues = NULL;
			}

			this->physicsObject = source.physicsObject;

			position = source.position;
			rotation = source.rotation;
			setRotation = source.setRotation;
			lightUpdatePosition = source.lightUpdatePosition;

			height = source.height; // this seems to be the actual height, updated by physics
			heightOffset = source.heightOffset; // this seems to be the relative height offset to ground (on editor)
			groundHeight = source.groundHeight; // the new ground height (on editor) - needed to solve correct height offset based on height

			ambient = source.ambient;
			for (int i = 0; i < LIGHT_MAX_AMOUNT; i++)
			{
				lightIndices[i] = source.lightIndices[i];
			}
			sunDir = source.sunDir;
			sunStrength = source.sunStrength;
			lightmapped = source.lightmapped;
			lightMultiplier = source.lightMultiplier;
			hp = source.hp;
			std::string originalName = source.originalName;
			inBuilding = source.inBuilding;

			originalModel = source.originalModel;
			originalInstance = source.originalInstance;
			latestReplacementModel = source.latestReplacementModel;
			latestReplacementInstance = source.latestReplacementInstance;
			movedByPhysics = source.movedByPhysics;
			lastEffectTime = source.lastEffectTime;

			deleted = source.deleted;
			ambientSound = source.ambientSound;

			dynamicObstacleExists = source.dynamicObstacleExists;
			dynamicObstaclePosition = source.dynamicObstaclePosition;

			return *this;
		}
	};

	typedef std::vector<ObjectInstance> InstanceList;

	struct Object
	{
		boost::shared_ptr<IStorm3D_Model> model;
		boost::shared_ptr<IStorm3D_Model> breakModel;

		ObjectData data;
		InstanceList instances;

		void createBreak(IStorm3D &storm)
		{
			if(!model || data.breakTexture == TERRAIN_OBJECT_BREAK_TEXTURE_TYPE_INVALID
				|| data.breakTexture == TERRAIN_OBJECT_BREAK_TEXTURE_TYPE_NONE)
				return;

			breakModel.reset(storm.CreateNewModel());

			// Slightly unoptimal. Krhmn.

			boost::scoped_ptr<Iterator<IStorm3D_Model_Object *> > object_iterator(model->ITObject->Begin());
			for(; !object_iterator->IsEnd(); object_iterator->Next())
			{
				IStorm3D_Model_Object *object = object_iterator->GetCurrent();
				if(!object)
					continue;
				IStorm3D_Mesh *mesh = object->GetMesh();
				if(!mesh)
					continue;
				IStorm3D_Material *material = mesh->GetMaterial();
				if(!material)
					continue;

				const Storm3D_Face *sourceFaces = mesh->GetFaceBufferReadOnly();
				const Storm3D_Vertex *sourceVertices = mesh->GetVertexBufferReadOnly();
				if(!sourceFaces || !sourceVertices)
					continue;

				int vertexAmount = mesh->GetVertexCount();
				int faceAmount = mesh->GetFaceCount();

				// Create new objects
				IStorm3D_Model_Object *destObject = breakModel->Object_New(object->GetName());
				IStorm3D_Mesh *destMesh = storm.CreateNewMesh();
				IStorm3D_Material *destMaterial = storm.CreateNewMaterial("..");

				// Object properties
				destObject->SetNoCollision(object->GetNoCollision());
				destObject->SetNoRender(object->GetNoRender());
				destObject->SetPosition(object->GetPosition());
				destObject->SetRotation(object->GetRotation());

				// Material properties
				destMesh->UseMaterial(destMaterial);
				//destMaterial->SetBaseTexture(material->GetBaseTexture());
				destMaterial->SetBaseTexture(storm.CreateNewTexture("TerrainObjectDamage_01.dds"));
				destMaterial->SetBaseTexture2(material->GetBaseTexture2());
				destMaterial->SetColor(material->GetColor());
				destMaterial->SetGlow(material->GetGlow());
				destMaterial->SetSelfIllumination(material->GetSelfIllumination());
				destMaterial->SetTransparency(material->GetTransparency());
				//destMaterial->SetAlphaType(IStorm3D_Material::ATYPE_MUL);

				// Mesh properties
				destObject->SetMesh(destMesh);
				destMesh->ChangeFaceCount(faceAmount);
				Storm3D_Face *destFaces = destMesh->GetFaceBuffer();
				for(int i = 0; i < faceAmount; ++i)
					destFaces[i] = sourceFaces[i];
				
				destMesh->ChangeVertexCount(vertexAmount);
				Storm3D_Vertex *destVertices = destMesh->GetVertexBuffer();
				for(int i = 0; i < vertexAmount; ++i)
					destVertices[i] = sourceVertices[i];

			}
		}
	};

	typedef std::vector<Object> ObjectList;
	typedef std::map<std::string, int> ObjectIndices;

	static TerrainObstacle createObstacle(const ObjectData &data, const ObjectInstance &instance)
	{
		TerrainObstacle obstacle;
		if(instance.originalName.empty())
			obstacle.modelFilename = data.fileName;
		else
			obstacle.modelFilename = instance.originalName;

		obstacle.position = instance.position;
		obstacle.rotation = instance.rotation;
		obstacle.terrainObstacleType = data.type;
		//obstacle.heightOffset = instance.heightOffset;
		obstacle.heightOffset = instance.height - instance.groundHeight;
		obstacle.height = data.height;
		obstacle.radius = data.radius;
		obstacle.fireThrough = data.fireThrough;
		obstacle.material = data.material;
		if(data.hasExplosion())
			obstacle.breakable = true;

		return obstacle;
	}

	static ExplosionEvent createEvent(const ObjectData &data, const ObjectInstance &instance, const VC2 &position, const VC3 &velocity, IStorm3D_Terrain *terrain, const VC3 &explosionPosition, bool useExplosion, UnifiedHandle unifiedHandle)
	{
		ExplosionEvent event;
		event.position.x = instance.position.x;
		//event.position.y = terrain->getHeight(instance.position) + instance.heightOffset;
		event.position.y = instance.height;
		event.position.z = instance.position.y;
		//event.velocity.x = event.position.x - position.x;
		//event.velocity.y = 0;
		//event.velocity.z = event.position.z - position.y;
		//event.velocity.Normalize();
		//event.velocity /= GAME_TICKS_PER_SECOND;

		event.explosionPosition = explosionPosition;
		event.useExplosion = useExplosion;

		//event.rotation.x = instance.rotation.x * 180.f / PI;
		//event.rotation.y = instance.rotation.y * 180.f / PI;
		//event.rotation.z = instance.rotation.z * 180.f / PI;
		VC3 angles = instance.setRotation.getEulerAngles();
		event.rotation.x = angles.x * 180.f / PI;
		event.rotation.y = angles.y * 180.f / PI;
		event.rotation.z = angles.z * 180.f / PI;

		event.script = data.explosionScript;
		event.projectile = data.explosionProjectile;
		event.effect = data.explosionEffect;
		event.unifiedHandle = unifiedHandle;
	
		if(!data.explosionSounds.empty())
		{
			event.sound = data.explosionSounds[data.nextSound];

			if(++data.nextSound >= int(data.explosionSounds.size()))
				data.nextSound = 0;
		}

		return event;
	}

	struct BlendPass
	{
		int textureA;
		int textureB;

		// Only 1 used if no legacy texturing
		boost::shared_ptr<IStorm3D_Texture> texture1;
		boost::shared_ptr<IStorm3D_Texture> texture2;

		std::vector<DWORD> weights;

		BlendPass()
		:	textureA(-1),
			textureB(-1)
		{
		}

		BlendPass(int textureA_, int textureB_, const std::vector<DWORD> &weights_)
		:	textureA(textureA_),
			textureB(textureB_),
			weights(weights_)
		{
		}
	};

	struct TerrainLightMap
	{
		std::vector<TColor<unsigned char> > values;
		boost::shared_ptr<IStorm3D_Texture> texture;

		TerrainLightMap()
		{
		}
	};


TerrainObstacle::TerrainObstacle()
:	terrainObstacleType(0),
	height(0),
	heightOffset(0),
	radius(0),
	fireThrough(false),
	breakable(false)
{
}

/** TerrainData **/

#ifdef PHYSICS_PHYSX
// HACK: a really really hacky thing.
static std::vector<IStorm3D_Model *> staticPhysicsTempModels;

void clear_static_physics_temp_models()
{
	for (int i = 0; i < (int)staticPhysicsTempModels.size(); i++)
	{
		delete staticPhysicsTempModels[i];
	}
	staticPhysicsTempModels.clear();
}
#endif


#ifdef PHYSICS_PHYSX
	// HACK: ...
	std::string terrain_cylinderFile;
	IStorm3D_Model *terrain_meshmodel = NULL;
#endif

game::GamePhysics *terrain_gamePhysics = NULL;


struct TerrainData
{
	std::vector<TerrainObstacle> obstacleList;
	TerrainAnimation animation;

	std::vector<boost::shared_ptr<IStorm3D_Texture> > textures;
	std::map<int, std::vector<BlendPass> > blendings;	
	boost::scoped_ptr<IStorm3D_Model> backgroundStormModel;
	std::map<int, TerrainLightMap> lightMaps;
	
	ObjectList objects;
	ObjectIndices objectIndices;

	IStorm3D *storm;
	IStorm3D_Scene *scene;
	IStorm3D_Terrain *terrain;
	game::GameMap *gameMap;
	ui::LightManager *lightManager;
	ui::AmbientSoundManager *ambientSoundManager;

	const util::AreaMap *areaMap;

	boost::scoped_array<WORD> heightMap;
	boost::scoped_array<WORD> obstacleMap;
	boost::scoped_array<WORD> forceMap;
	
	VC2I mapSize;
	VC3 realSize;
	float terrainScale;

	TColor<unsigned char> ambientColor;
	TColor<unsigned char> sunColor;
	TColor<unsigned char> fogColor;
	VC3 sunDirection;

	float cameraRange;
	float fogStart;	
	float fogEnd;

	bool fogEnabled;
	int textureRepeat;

	std::string backgroundModel;
	std::string dirName;

	int blockAmount;
	//COL colorFactor;
	int lightmapSize;
	bool useDynamicObstacles;

	GRIDOCCLUSIONCULLER_DATATYPE occlusionCameraArea;

	util::FogApplier fogApplier;
#ifdef PHYSICS_PHYSX
	boost::scoped_ptr<game::AbstractPhysicsObject> terrainPhysics;
#endif

	TerrainData()
	{
		storm = 0;
		scene = 0;
		terrain = 0;
		terrainScale = 0;
		cameraRange = 0;
		fogStart = 0;
		fogEnd = 0;
		fogEnabled = false;
		textureRepeat = 5;
		blockAmount = 0;
		lightmapSize = 0;
		occlusionCameraArea = GRIDOCCLUSIONCULLER_DEFAULT_AREA_MASK;
#ifdef PROJECT_CLAW_PROTO
		useDynamicObstacles = true;
#else
		useDynamicObstacles = false;
#endif

		//colorFactor = COL(1.f, 1.f, 1.f);
	}

	~TerrainData()
	{
#ifdef PHYSICS_PHYSX  // turol: is this ok?
		clear_static_physics_temp_models();
#endif

		// TODO: should we delete the heightMapData here... 
		// it is a shared buffer (someone else may delete it - gameMap maybe?)

		if(backgroundStormModel)
			scene->RemoveBackGround();
		if(terrain)
			scene->RemoveTerrain(terrain);
		
		delete terrain;
	}

	void unifiedHandleToTerrainIds(int unifiedHandle, int *terrainModelIdOut, int *terrainObstacleIdOut) const
	{
		assert(terrainModelIdOut != NULL);
		assert(terrainObstacleIdOut != NULL);

		assert(IS_UNIFIED_HANDLE_TERRAIN_OBJECT(unifiedHandle));

		*terrainModelIdOut = ((unifiedHandle & UNIFIED_HANDLE_TERRAIN_OBJECT_MODEL_ID_MASK) >> UNIFIED_HANDLE_TERRAIN_OBJECT_MODEL_ID_SHIFT);
		*terrainObstacleIdOut = ((unifiedHandle & UNIFIED_HANDLE_TERRAIN_OBJECT_INSTANCE_ID_MASK) >> UNIFIED_HANDLE_TERRAIN_OBJECT_INSTANCE_ID_SHIFT);

		// note, these are always true regardless of parameters, unless the mask,shift,etc. defines are totally wrong)
		assert(*terrainModelIdOut >= 0 && *terrainModelIdOut <= UNIFIED_HANDLE_TERRAIN_OBJECT_MODEL_ID_LAST_VALUE);
		assert(*terrainObstacleIdOut >= 0 && *terrainObstacleIdOut <= UNIFIED_HANDLE_TERRAIN_OBJECT_INSTANCE_ID_LAST_VALUE);
	}

	UnifiedHandle getUnifiedHandle(int terrainModelId, int terrainObstacleId) const
	{
		assert(terrainModelId >= 0 && terrainModelId <= UNIFIED_HANDLE_TERRAIN_OBJECT_MODEL_ID_LAST_VALUE);
		assert(terrainObstacleId >= 0 && terrainObstacleId <= UNIFIED_HANDLE_TERRAIN_OBJECT_INSTANCE_ID_LAST_VALUE);

		// note, this is always true, assuming the masks, etc. are not totally wrong..
		assert(IS_UNIFIED_HANDLE_TERRAIN_OBJECT(UNIFIED_HANDLE_BIT_TERRAIN_OBJECT | terrainModelId | (terrainObstacleId << UNIFIED_HANDLE_TERRAIN_OBJECT_INSTANCE_ID_SHIFT)));

		return (UNIFIED_HANDLE_BIT_TERRAIN_OBJECT | terrainModelId | (terrainObstacleId << UNIFIED_HANDLE_TERRAIN_OBJECT_INSTANCE_ID_SHIFT));
	}

	void horizline( float fx1, float fx2, float fy, bool add )
	{
		if( fx2 < fx1 ) std::swap( fx1, fx2 );
		
		int x1 = (int)( fx1 + 0.5f );
		int x2 = (int)( fx2 + 0.5f );
		int ty = (int)( fy + 0.5f );

		for( int tx = x1; tx < x2; tx++ )
		{
			if( add )
				gameScene_instance->addDoorObstacle( tx, ty, TERRAIN_DYNAMIC_OBSTACLE_HEIGHT );
			else
				gameScene_instance->removeDoorObstacle( tx, ty, TERRAIN_DYNAMIC_OBSTACLE_HEIGHT );
		}
	}

	void drawTriangle( VC2 A, VC2 B, VC2 C, bool add )
	{
		if( B.y < A.y || C.y < A.y )
		{
			if( B.y < C.y )
				std::swap( A, B );
			else
				std::swap( A, C );
		}

		if( C.y < B.y )
			std::swap( B, C );



		float dx1, dx2, dx3;

		if (B.y-A.y > 0) 
			dx1=(B.x-A.x)/(B.y-A.y); 
		else 
			dx1=B.x - A.x;
		
		if (C.y-A.y > 0) 
			dx2=(C.x-A.x)/(C.y-A.y); 
		else 
			dx2=0;
		
		if (C.y-B.y > 0) 
			dx3=(C.x-B.x)/(C.y-B.y); 
		else 
			dx3=0;

		VC2 S = A;
		VC2 E = A;
		if( dx1 > dx2 ) 
		{
			for( ;S.y<=B.y;S.y++,E.y++,S.x+=dx2,E.x+=dx1 )
				horizline( S.x, E.x, S.y, add );
			E=B;
			for(;S.y<=C.y;S.y++,E.y++,S.x+=dx2,E.x+=dx3)
				horizline( S.x, E.x, S.y, add );
		} else {
			for(;S.y<=B.y;S.y++,E.y++,S.x+=dx1,E.x+=dx2)
				horizline( S.x, E.x, S.y, add );
			S=B;
			for(;S.y<=C.y;S.y++,E.y++,S.x+=dx3,E.x+=dx2)
				horizline( S.x, E.x, S.y, add );
		}
	}

	void paintDynamicObstacle( int modelId, int instanceId, VC2I ox_pos, float angle, bool add )
	{
		if( modelId >= (signed)objects.size() || 
			objects[modelId].model.get() == NULL ||
			objects[modelId].instances[instanceId].deleted )
		{
			Logger::getInstance()->error("Terrain::paintDynamicObstacle() - uknown model" );
			return;
		}

		AABB boundingbox = objects[modelId].model->GetBoundingBox();

		// not so axis aligned box
		if( fabsf( boundingbox.mmax.x - boundingbox.mmin.x ) > 2 ||
			fabsf( boundingbox.mmax.z - boundingbox.mmin.z ) > 2 )
		{
			VC2 corners[4];
			{
				VC2 mini;
				VC2 maxi;
				// AABB temp = objects[modelId].model->GetBoundingBox();
				VC2 temp_min = VC2( boundingbox.mmin.x, boundingbox.mmin.z );
				VC2 temp_max = VC2( boundingbox.mmax.x, boundingbox.mmax.z );

				mini.x = std::min( temp_min.x, temp_max.x );
				mini.y = std::min( temp_min.y, temp_max.y );
				maxi.x = std::max( temp_min.x, temp_max.x );
				maxi.y = std::max( temp_min.y, temp_max.y );

				corners[0] = VC2( mini.x, mini.y );
				corners[1] = VC2( maxi.x, mini.y );
				corners[2] = VC2( maxi.x, maxi.y );
				corners[3] = VC2( mini.x, maxi.y );
			}

			// rotate & convert points
			{
				bool draw = true;

				angle += ( 3.1415962f / 2.0f );
				for( int i = 0; i < 4; i++ )
				{
					// rotate 
					float ca = sin( angle );
					float sa = cos( angle );
		
					float tx = corners[ i ].x * ca - corners[ i ].y * sa;
					float ty = corners[ i ].x * sa + corners[ i ].y * ca;
					corners[ i ].x = tx;
					corners[ i ].y = ty;

					// convert
					corners[ i ].x *= gameMap->getPathfindSizeX() / gameMap->getScaledSizeX();
					corners[ i ].y *= gameMap->getPathfindSizeY() / gameMap->getScaledSizeY();

					corners[ i ] += VC2( (float)ox_pos.x, (float)ox_pos.y );

					if( !( corners[ i ].x > 0 && corners[ i ].x < gameMap->getPathfindSizeX() &&
						corners[ i ].y > 0 && corners[ i ].y < gameMap->getPathfindSizeY() ) )
					{
						draw = false;
						Logger::getInstance()->error( "Terrain::paintDynamicObstacle() - leaking all over the map" ); 
					}
				}

				// draw
				if( draw )
				{
					drawTriangle( corners[ 0 ], corners[ 1 ], corners[ 3 ], add );
					drawTriangle( corners[ 3 ], corners[ 2 ], corners[ 1 ], add );
				}
			}

			if( false )
			{
				VC2 pos2d = objects[modelId].instances[instanceId].position;		
				VC3 rot = objects[modelId].instances[instanceId].setRotation.getEulerAngles();

				int ox = gameMap->scaledToObstacleX(pos2d.x);
				int oy = gameMap->scaledToObstacleY(pos2d.y);

				std::stringstream ss;
				for( int i = 0; i < 4; ++i )
					ss << "corners[" << i << "]: " << corners[ i ].x << ", " << corners[ i ].y << std::endl;

				ss << "ox: " << ox << ", " << oy << std::endl;
				ss << "oxpos" << ox_pos.x << ", " << ox_pos.y << std::endl;
				ss << "rotation" << rot.x << ", " << rot.y << ", " << rot.z << std::endl;
				// ss << "pos2d: " << pos2d.x << ", " << pos2d.y << std::endl;
				Logger::getInstance()->debug( ss.str().c_str() );

			}
		}
		else 		// axis aligned box
		{
			VC2 mini;
			VC2 maxi;

			{
				AABB temp = objects[modelId].model->GetBoundingBox();
				VC2 temp_min = VC2( boundingbox.mmin.x, boundingbox.mmin.z );
				VC2 temp_max = VC2( boundingbox.mmax.x, boundingbox.mmax.z );

				mini.x = std::min( temp_min.x, temp_max.x ) - 0.5f;
				mini.y = std::min( temp_min.y, temp_max.y ) - 0.5f;
				maxi.x = std::max( temp_min.x, temp_max.x ) + 0.5f;
				maxi.y = std::max( temp_min.y, temp_max.y ) + 0.5f;
			}

			float scaledToPathfindX = gameMap->getPathfindSizeX() / gameMap->getScaledSizeX();
			float scaledToPathfindY = gameMap->getPathfindSizeY() / gameMap->getScaledSizeY();

			VC2I ox_min( (int)( scaledToPathfindX * mini.x + 0.5f ) , (int)( scaledToPathfindY * mini.y + 0.5f )  );
			VC2I ox_max( (int)( scaledToPathfindX * maxi.x + 0.5f ) , (int)( scaledToPathfindY * maxi.y + 0.5f )  );
			ox_min += ox_pos;
			ox_max += ox_pos;

			bool draw = true;

			if( !( ox_min.x > 0 && ox_min.x < gameMap->getPathfindSizeX() &&
				ox_min.y > 0 && ox_min.y < gameMap->getPathfindSizeY() ) || 
				!( ox_max.x > 0 && ox_max.x < gameMap->getPathfindSizeX() &&
				ox_max.y > 0 && ox_max.y < gameMap->getPathfindSizeY() ) )
			{
				draw = false;
				Logger::getInstance()->error( "Terrain::paintDynamicObstacle() - leaking all over the map" ); 
			}


			if( draw )
			{
				// FIXME: will crash near the edge of map
				for (int ty = ox_min.y; ty <= ox_max.y; ty++)
				{
					for (int tx = ox_min.x; tx <= ox_max.x; tx++)
					{
						if( add )
							gameScene_instance->addDoorObstacle(tx, ty, TERRAIN_DYNAMIC_OBSTACLE_HEIGHT);
						else 
							gameScene_instance->removeDoorObstacle(tx, ty, TERRAIN_DYNAMIC_OBSTACLE_HEIGHT);
					}
				}
			}
		}
	}

	void removeDynamicObstacle(int modelId, int instanceId)
	{
		if (!useDynamicObstacles) 
			return;

		if (!objects[modelId].instances[instanceId].dynamicObstacleExists)
			return;

		VC2I obstPos = objects[modelId].instances[instanceId].dynamicObstaclePosition;
		float rotation = objects[modelId].instances[instanceId].dynamicObstacleRotation;
		
		paintDynamicObstacle( modelId, instanceId, obstPos, rotation, false );

		objects[modelId].instances[instanceId].dynamicObstacleExists = false;
	}

	void addDynamicObstacle(int modelId, int instanceId)
	{
		if (!useDynamicObstacles) 
			return;

		assert(!objects[modelId].instances[instanceId].dynamicObstacleExists);

		int physType = objects[modelId].data.physicsType;
		if (physType != TERRAIN_OBJECT_PHYSICS_TYPE_NONE
			&& physType != TERRAIN_OBJECT_PHYSICS_TYPE_STATIC)
		{
			VC2 pos2d = objects[modelId].instances[instanceId].position;		

			if (gameMap->isWellInScaledBoundaries(pos2d.x, pos2d.y))
			{
				int ox = gameMap->scaledToObstacleX(pos2d.x);
				int oy = gameMap->scaledToObstacleY(pos2d.y);

				float rotation = objects[modelId].instances[instanceId].setRotation.getEulerAngles().y;

				paintDynamicObstacle( modelId, instanceId, VC2I(ox, oy), rotation, true );

				objects[modelId].instances[instanceId].dynamicObstaclePosition = VC2I(ox, oy);
				objects[modelId].instances[instanceId].dynamicObstacleRotation = rotation;
				objects[modelId].instances[instanceId].dynamicObstacleExists = true;
			}
		}
	}

	void deleteTerrainObject(UnifiedHandle uh)
	{
		int modelId = 0;
		int objectId = 0;
		unifiedHandleToTerrainIds(uh, &modelId, &objectId);

		terrain->removeInstance(modelId, objectId);
		stopAmbientSoundForObject(modelId, objectId);

		objects[modelId].instances[objectId].deleted = true;

		removeDynamicObstacle(modelId, objectId);

		if(objects[modelId].instances[objectId].physicsObject)
			objects[modelId].instances[objectId].physicsObject.reset();

		// FIXME: terrain obstacle does not get properly removed!
		// (if a static or an unmoved dynamic terrain object gets deleted, its obstacle remains!!!)
	}

	void setInstanceDamageTexture(UnifiedHandle uh, float damageTextureFadeFactor)
	{
		assert(VALIDATE_UNIFIED_HANDLE_BITS(uh));
		assert(IS_UNIFIED_HANDLE_TERRAIN_OBJECT(uh));

		assert(damageTextureFadeFactor >= 0.0 && damageTextureFadeFactor <= 1.0f);

		float f = 1.0f - damageTextureFadeFactor;
		int modelId = 0;
		int instanceId = 0;
		this->unifiedHandleToTerrainIds(uh, &modelId, &instanceId);

		terrain->setInstanceFade(modelId, instanceId, f);
	}

	UnifiedHandle createTerrainObject(int newModelId, UnifiedHandle cloneFrom, const VC3 &position, const QUAT &rotation, const VC3 &velocity)
	{
		int newInstanceId = 0;

		// TODO: check that newInstanceId does not overflow properly (max. amount allowed by terrain object unified handles)
		// for now, just this hard coded value check here (which should be correct if defines have not been changed...?)
		if (objects[newModelId].instances.size() >= 8192)
		{
			LOG_ERROR("Terrain::createTerrainObject - Too many instances of given terrain object model.");
			assert(!"Terrain::createTerrainObject - Too many instances of given terrain object model.");
			return UNIFIED_HANDLE_NONE;
		}

		newInstanceId = objects[newModelId].instances.size();
		Object &newObject = objects[newModelId];

		VC3 physVel = VC3(0,0,0);
		VC3 physAngVel = VC3(0,0,0);

		if (cloneFrom != UNIFIED_HANDLE_NONE)
		{
			// COPIED AND MODIFIED FROM breakObjects(...)

			int origModelId = 0;
			int origObjectId = 0;
			unifiedHandleToTerrainIds(cloneFrom, &origModelId, &origObjectId);
			ObjectInstance &originalInstance = objects[origModelId].instances[origObjectId];

			if (originalInstance.physicsObject)
			{
				physVel = originalInstance.physicsObject->getVelocity();
				physAngVel = originalInstance.physicsObject->getAngularVelocity();
			}

			VC3 copyPosition(originalInstance.position.x, 0, originalInstance.position.y);
			copyPosition.y = originalInstance.height;

			COL ambient = originalInstance.ambient;

			terrain->addInstance(newModelId, copyPosition, originalInstance.setRotation, ambient);

			for(int i = 0; i < LIGHT_MAX_AMOUNT; ++i)
				terrain->setInstanceLight(newModelId, newInstanceId, i, originalInstance.lightIndices[i], ambient);

			terrain->setInstanceLightmapped(newModelId, newInstanceId, originalInstance.lightmapped);
			terrain->setInstanceSun(newModelId, newInstanceId, originalInstance.sunDir, originalInstance.sunStrength);

			ObjectInstance copy = originalInstance;
			copy.originalName = objects[origModelId].data.fileName;
			copy.hp = newObject.data.hp;
			copy.originalModel = origModelId;
			copy.originalInstance = origObjectId;

			assert(copy.originalModel != newModelId || copy.originalInstance != newInstanceId);			

			// DON'T COPY THE PHYSICS OBJECT!
			copy.physicsObject.reset();

			// clear the deleted flag!
			copy.deleted = false;

			copy.dynamicObstacleExists = false;
			copy.dynamicObstaclePosition = VC2I(0,0);

			if(newObject.data.breakTexture == TERRAIN_OBJECT_BREAK_TEXTURE_TYPE_SCALE_HP)
			{
				float f = float(copy.hp) / float(newObject.data.hp);
				terrain->setInstanceFade(newModelId, newInstanceId, f);
			}
			else if(newObject.data.breakTexture == TERRAIN_OBJECT_BREAK_TEXTURE_TYPE_ALWAYS)
			{
				terrain->setInstanceFade(newModelId, newInstanceId, 0);
			}

			newObject.instances.push_back(copy);

			updateLatestReplacementInfo(origModelId, origObjectId, newModelId, newInstanceId);

		} else {
			physVel = velocity;

			// NOTE: black (ambient?) color here.
			terrain->addInstance(newModelId, position, rotation, COL(0,0,0));

			// TODO: solve appropriate pointlights and shit.

			ObjectInstance theNewInstance;
			theNewInstance.position = VC2(position.x, position.z);
			theNewInstance.height = position.y;
			theNewInstance.setRotation = rotation;
			theNewInstance.rotation = rotation.getEulerAngles();

			newObject.instances.push_back(theNewInstance);
		}

		ObjectData &data = newObject.data;

		if(data.fileExists)
		{
#ifdef PHYSICS_PHYSX
			createPhysicsMeshForModel(newModelId, terrain_cylinderFile, &terrain_meshmodel);
#endif
			int tmp = 0;
			// TODO: optimize!!!
static util::SoundMaterialParser soundmp;
static util::ObjectDurabilityParser durp;
			createPhysicsForObject(terrain_gamePhysics, newModelId, newInstanceId, false, soundmp, durp, tmp);

			if (newObject.instances[newInstanceId].physicsObject)
			{
				newObject.instances[newInstanceId].physicsObject->setVelocity(physVel);
				newObject.instances[newInstanceId].physicsObject->setAngularVelocity(physAngVel);
			}
			createAmbientSoundForObject(newModelId, newInstanceId);
		}

		return getUnifiedHandle(newModelId, newInstanceId);
	}

	bool hasMetaValue(int terrainModelId, const std::string &metaKey)
	{
		assert(terrainModelId >= 0 && terrainModelId < (int)objects.size());

		std::map<std::string, std::string>::iterator it = objects[terrainModelId].data.metaValues.find(metaKey);
		if (it != objects[terrainModelId].data.metaValues.end())
		{
			// if the value is empty, treat it as if it does not exist...
			if (it->second.empty())
				return false;

			return true;
		} else {
			return false;
		}
	}

	std::string getMetaValue(int terrainModelId, const std::string &metaKey)
	{
		assert(terrainModelId >= 0 && terrainModelId < (int)objects.size());

		std::map<std::string, std::string>::iterator it = objects[terrainModelId].data.metaValues.find(metaKey);
		if (it != objects[terrainModelId].data.metaValues.end())
		{
			return it->second;
		} else {
			LOG_ERROR_W_DEBUG("TerrainData::getMetaValue - Given meta key not found.", metaKey.c_str());
			//assert(!"TerrainData::getMetaValue - Given meta key not found.");
			return "";
		}
	}

	TRACKABLE_TYPEID_DATATYPE parseTrackableTypeFromString(std::string trackableTypeString)
	{
		TRACKABLE_TYPEID_DATATYPE ret = 0;

		if (trackableTypeString == "0")
			return 0;

		std::string value_str = trackableTypeString;
		std::vector<std::string> splitted = util::StringSplit(",", value_str);
		for (int i = 0; i < (int)splitted.size(); i++)
		{
			std::string trimmed = util::StringRemoveWhitespace(splitted[i]);
			TRACKABLE_TYPEID_DATATYPE tid = getTrackableTypeIdForName(trimmed.c_str());
			if (tid != 0)
			{
				ret |= tid;
			} else {
				Logger::getInstance()->error("TerrainData::parseTrackableTypeFromString - trackable type mask name unknown.");
			}
		}

		return ret;
	}
	void loadScene(const char *forceName)
	{
		std::string fileName = dirName + "\\scene.bin";
		//filesystem::InputStream stream = filesystem::createInputFileStream(fileName);
		filesystem::InputStream stream = filesystem::FilePackageManager::getInstance().getFile(fileName);
#ifdef PROJECT_SURVIVOR_DEMO
		unsigned int crc = filesystem::FilePackageManager::getInstance().getCrc(fileName);

		if(crc != 0xF56499B0
			&& crc != 0x7EDF6E64
			&& crc != 0xD550D273
			&& crc != 0x9EC6D639)
			return;
#endif

		if(stream.isEof())
		{
			Logger::getInstance()->error("Terrain::loadScene - Failed to load scene.bin.");
			Logger::getInstance()->debug(dirName.c_str());
			Logger::getInstance()->warning("This is a fatal error, program will terminate now to prevent crashing later on.");
			assert(!"Terrain::loadScene - Failed to load scene.bin.");
			abort();
			return;
		}

		int version = 0;
		stream >> version;
		stream >> mapSize >> realSize;

		if(version <= 2)
			return;

		boost::scoped_array<WORD> newArr (new WORD[mapSize.x * mapSize.y]);
		heightMap.swap(newArr);

		// NOTICE: this obstaclemap thing is totally fucked up!
		// keeping it that way to preserve compatibility with old editor exports.
		// THIS OBSTACLEMAP IS NOT ACTUALLY USED ANYWHERE, TerrainCreator OVERRIDES THE OBSTACLEMAP SET HERE
		#define INCORRECT_TERRAIN_OBST_MAP_AREA_MULT 16
		//#define CORRECT_TERRAIN_OBST_MAP_AREA_MULT (GAMEMAP_HEIGHTMAP_MULTIPLIER*GAMEMAP_HEIGHTMAP_MULTIPLIER * GAMEMAP_PATHFIND_ACCURACY*GAMEMAP_PATHFIND_ACCURACY)

		boost::scoped_array<WORD> newObstacleMap(new WORD[INCORRECT_TERRAIN_OBST_MAP_AREA_MULT * mapSize.x * mapSize.y]);
		obstacleMap.swap(newObstacleMap);

		boost::scoped_array<WORD> newForceMap(new WORD[GAMEMAP_HEIGHTMAP_MULTIPLIER*GAMEMAP_HEIGHTMAP_MULTIPLIER * mapSize.x * mapSize.y]);
		forceMap.swap(newForceMap);

		stream.read(heightMap.get(), mapSize.x * mapSize.y);
		for(int y = 0; y < GAMEMAP_HEIGHTMAP_MULTIPLIER * mapSize.y; ++y)
		for(int x = 0; x < GAMEMAP_HEIGHTMAP_MULTIPLIER * mapSize.x; ++x)
		{
			//stream >> heightMap[y * mapSize.x + x];
			//forceMap[y * COLLISION_HEIGHTMAP_MULT * mapSize.x + x] = heightMap[(y / COLLISION_HEIGHTMAP_MULT) * mapSize.x + (x / COLLISION_HEIGHTMAP_MULT)];
			forceMap[y * GAMEMAP_HEIGHTMAP_MULTIPLIER * mapSize.x + x] = 0;
		}

		if(version <= 7)
		{
			stream.read(obstacleMap.get(), mapSize.x * mapSize.y * INCORRECT_TERRAIN_OBST_MAP_AREA_MULT);
		}

		stream >> textureRepeat;
		stream >> ambientColor >> sunColor >> sunDirection;
		stream >> fogEnabled >> fogColor >> fogStart >> fogEnd;
		if(version < 6)
			fogEnabled = false;

		stream >> backgroundModel;
		stream >> cameraRange;

		if(version >= 7)
		{
			int fogAmount = 0;
			stream >> fogAmount;
			for(int i = 0; i < fogAmount; ++i)
			{
				std::string id;
				util::FogApplier::Fog f;

				stream >> id;
				stream >> f.enabled >> f.cameraCentric;

				TColor<unsigned char> color;
				stream >> color.r >> color.g >> color.b;
				// Warning: color component order below changed from RGB to BGR to fix fog color in DirectX
				f.color = COL(color.b / 255.f, color.g / 255.f, color.r / 255.f);

				stream >> f.start >> f.end;

#ifdef PROJECT_SHADOWGROUNDS
				// FIXME, properly!
				if (f.start < 0.0f)
					f.start = 20.0f;
				if (f.end < 0.0f)
					f.end = 9.0f;
#endif

				fogApplier.setFog(id, f);
			}

			fogApplier.setActiveFog("Default");
		}

		SHOW_LOADING_BAR(30);

		terrainScale = realSize.x / mapSize.x;
		int textureAmount = 0;
		stream >> textureAmount;

		for(int i = 0; i < textureAmount; ++i)
		{
			std::string fileName;
			stream >> fileName;

			IStorm3D_Texture *texture = storm->CreateNewTexture(fileName.c_str());

			if(texture)
			{
				boost::shared_ptr<IStorm3D_Texture> t(texture, std::mem_fun(&IStorm3D_Texture::Release));
				textures.push_back(t);
			}
			else
			{
				// HACK: if texture is missing, try to load dummy texture
				Logger::getInstance()->warning("Terrain::loadScene - Unable to load texture.");
				Logger::getInstance()->debug(fileName.c_str());
#ifdef LEGACY_FILES
				texture = storm->CreateNewTexture("Data/Textures/missing.dds");
#else
				texture = storm->CreateNewTexture("data/texture/missing.dds");
#endif
				boost::shared_ptr<IStorm3D_Texture> t(texture, std::mem_fun(&IStorm3D_Texture::Release));
				textures.push_back(t);
			}
		}

		blockAmount = 0;
		stream >> blockAmount;
	
		for(int i = 0; i < blockAmount; ++i)
		{
			int blockIndex = 0;
			stream >> blockIndex;

			int passes = 0;
			stream >> passes;

			for(int j = 0; j < passes; j += 2)
			{
				int textureA = 0;
				int textureB = 0;
				
				stream >> textureA >> textureB;
				assert(textureA >= 0);

				std::vector<DWORD> weights(BLOCK_SIZE * BLOCK_SIZE);

				int weightSize = BLOCK_SIZE * BLOCK_SIZE;
				if(j + 1 < passes)
					weightSize *= 2;

				std::vector<unsigned char> weightBuffer(weightSize);
				stream.read(&weightBuffer[0], weightSize);

				int weightIndex = 0;
				for(unsigned int k = 0; k < weights.size(); ++k)
				{
					unsigned char a = weightBuffer[weightIndex++];
					unsigned char b = 0;

					if(j + 1 < passes)
						b = weightBuffer[weightIndex++];

					DWORD weight = a | (b << 24);
					weights[k] = weight;
				}

				blendings[blockIndex].push_back(BlendPass(textureA, textureB, weights));
			}
		}

		SHOW_LOADING_BAR(35);

		if(version >= 4)
		{
			int lightmapAmount = 0;
			stream >> lightmapAmount;

			for(int i = 0; i < lightmapAmount; ++i)
			{
				int block = 0;
				stream >> block;

				int values = 0;
				stream >> values;

				TerrainLightMap &map = lightMaps[block];

				map.values.resize(values / 3);
				assert(sizeof(TColor<unsigned char>) == 3);
				stream.read((char *)&map.values[0].r, values);

				/*
				for(int j = 0; j < values / 3; ++j)
				{
					TColor<unsigned char> col;
					stream >> col.r;
					stream >> col.g;
					stream >> col.b;

					map.values.push_back(col);
				}
				*/
			}

			lightmapSize = BLOCK_SIZE * 8;
			if(version >= 5)
				stream >> lightmapSize;
		}
	}

	void loadObjects()
	{
		std::string fileName = dirName + "\\objects.bin";
		filesystem::InputStream stream = filesystem::FilePackageManager::getInstance().getFile(fileName);

		int version = 0;
		float range = 0;
		stream >> version;
		stream >> range >> range >> range;

		int objectAmount = 0;
		stream >> objectAmount;

		objects.resize(objectAmount);

#if defined(PROJECT_SHADOWGROUNDS) && defined(PHYSICS_PHYSX)
		frozenbyte::editor::EditorParser parser(false, false);

		filesystem::InputStream strm = filesystem::FilePackageManager::getInstance().getFile("Data/Misc/objects.fbt");
		strm >> parser;
#endif

		for(int i = 0; i < objectAmount; ++i)
		{
			if (((i * 4) / objectAmount) != (((i-1) * 5) / objectAmount))
			{
				SHOW_LOADING_BAR(40 + ((i * 5) / objectAmount));
			}

			ObjectData data;
			stream >> data.fileName >> data.type >> data.fallType >> data.height >> data.radius >> data.fireThrough;

			if(version >= 3)
			{
				stream >> data.explosionObstacle >> data.explosionEffect;
			}
			if(version >= 5)
			{
				stream >> data.explosionScript >> data.explosionProjectile;
				if(version >= 6)
				{
					if(version == 7 || version == 8)
					{
						std::string sound;
						stream >> sound;
						if(!sound.empty())
							data.explosionSounds.push_back(sound);
					}
					else if(version >= 9)
					{
						int sounds = 0;
						stream >> sounds;

						data.explosionSounds.resize(sounds);
						for(int k = 0; k < sounds; ++k)
							stream >> data.explosionSounds[k];
					}

					stream >> data.material;

					if(version >= 10)
					{
						stream >> data.bones;
						stream >> data.idleAnimation;

						int anims = 0;
						stream >> anims;
					
						if(anims)
						{
							data.explosionAnimations.resize(anims);
							for(int k = 0; k < anims; ++k)
							{
								std::string fname;
								stream >> fname;

								IStorm3D_BoneAnimation *ba = storm->CreateNewBoneAnimation(fname.c_str());
								data.explosionAnimations[k].reset(ba, AnimationDeleter());
							}
						}

						if(!game::SimpleOptions::getBool(DH_OPT_B_ENVIRONMENT_ANIMATIONS) && data.explosionAnimations.empty())
						{
							data.bones.clear();
							data.idleAnimation.clear();
						}
					}
				}

				stream >> data.hp;

				if(version >= 15)
					stream >> data.breakTexture;
				if(version >= 18)
				{
					stream >> data.physicsType;
					stream >> data.physicsMass;
					stream >> data.physicsSoundMaterial;
					stream >> data.physicsData1.x >> data.physicsData1.y >> data.physicsData1.z;
					stream >> data.physicsData2.x >> data.physicsData2.y >> data.physicsData2.z;
					if(version >= 19)
					{
						stream >> data.durabilityType;
					}

					//haxahax -- remove static physics
					//if(data.physicsType == 1)
					//	data.physicsType = 0;
					//haxhax -- static to boxes
					//if(data.physicsType == 1)
					//{
					//	data.physicsType = 2;
					//	data.physicsMass = 1000.f;
					//	data.physicsData1 = VC3(0.5f, 0.5f, 0.5f);
					//}
				}
#if defined(PROJECT_SHADOWGROUNDS) && defined(PHYSICS_PHYSX)
				const frozenbyte::editor::ParserGroup &group = parser.getGlobals().getSubGroup(data.fileName);

				data.physicsType = frozenbyte::editor::convertFromString<int> (group.getValue("physics_type"), 0);
				data.physicsMass = frozenbyte::editor::convertFromString<float> (group.getValue("physics_weight"), 0.0f);
				data.physicsSoundMaterial = group.getValue("physics_material");
				data.physicsData1 = VC3(0.2f, 0.2f, 0.2f);
				data.physicsData2 = VC3(0.0f, 0.0f, 0.0f);
#endif
			}

			if(version >= 21)
			{
				int metaValueAmount = 0;
				stream >> metaValueAmount;

				for(int j = 0; j < metaValueAmount; ++j)
				{
					std::string key;
					std::string value;

					stream >> key;
					stream >> value;

					data.metaValues[key] = value;

					if (key == "filename_prefix")
					{
						data.fileName = value + data.fileName;
						if (!data.explosionObstacle.empty()
							&& data.explosionObstacle != "(disappear)")
							data.explosionObstacle = value + data.explosionObstacle;
					}
					else if (key == "filename_postfix")
					{
						data.fileName += value;
						if (!data.explosionObstacle.empty()
							&& data.explosionObstacle != "(disappear)")
							data.explosionObstacle += value;
					}
				}
			}

			int instanceAmount = 0;
			stream >> instanceAmount;

			boost::shared_ptr<IStorm3D_Model> model(storm->CreateNewModel());

			/*
			static std::vector<std::pair<std::string, IStorm3D_Model *> > modelCache;
			bool foundInCache = false;
			for (int c = 0; c < (int)modelCache.size(); c++)
			{
				if (modelCache[c].first == data.fileName)
				{
					model.reset(modelCache[c].second->GetClone(true, false, false));
					foundInCache = true;
					data.fileExists = true;
					break;
				}
			}
			*/

			//if (!foundInCache)
			{
				data.fileExists = model->LoadS3D(data.fileName.c_str());
				//if (data.fileExists)
				//{
				//	modelCache.push_back(std::pair<std::string, IStorm3D_Model *>(data.fileName, model.get()));
				//}
			}


			boost::scoped_ptr<Iterator<IStorm3D_Model_Object *> > objectIterator(model->ITObject->Begin());
			for(; !objectIterator->IsEnd(); objectIterator->Next())
			{
				IStorm3D_Model_Object *object = objectIterator->GetCurrent();
				if(!object)
					continue;

				const char *objName = object->GetName();
				if(strstr(objName, "FireThrough") != NULL || data.fireThrough)
					object->SetNoCollision(true);
			}

			Object &object = objects[i];
			object.model = model;
			object.data = data; 
			object.instances.resize(instanceAmount);
			objectIndices[data.fileName] = i;

			if(data.physicsType == 0)
				model->SetNoCollision(true);

#if defined(PROJECT_SHADOWGROUNDS) && defined(PHYSICS_PHYSX)
			AABB aabb = model->GetBoundingBox();
			VC3 size = aabb.mmax - aabb.mmin;

			if(data.physicsType == TERRAIN_OBJECT_PHYSICS_TYPE_BOX) // box
			{
				object.data.physicsData1 = size;
				object.data.physicsData1 *= 0.5f;
			}
			else if(data.physicsType == TERRAIN_OBJECT_PHYSICS_TYPE_CYLINDER) // cylinder
			{
				object.data.physicsData1.x = size.y;
				object.data.physicsData1.y = size.x * 0.5f;
			}
			else if(data.physicsType == TERRAIN_OBJECT_PHYSICS_TYPE_CAPSULE) // capsule
			{
				object.data.physicsData1.x = size.y - size.x;
				object.data.physicsData1.y = size.x * 0.5f;
			}
#endif

			object.createBreak(*storm);
			terrain->addModel(model, object.breakModel, data.bones, data.idleAnimation);

			if(data.bones.empty())
			{
/*
				model->FreeMemoryResources();
				if(object.breakModel)
					object.breakModel->FreeMemoryResources();
*/
			}

			// parse trackableType from metadata...
			bool hasTrackableType = hasMetaValue(i, "trackable_type");
			if (hasTrackableType)
			{
				std::string trackable_str = getMetaValue(i, "trackable_type");
				object.data.trackableType = parseTrackableTypeFromString(trackable_str);
			}

			if (hasMetaValue(i, "ambient_sound"))
			{
				object.data.ambientSound = getMetaValue(i, "ambient_sound");
				object.data.ambientSoundRange = str2int(getMetaValue(i, "ambient_sound_range").c_str());
				object.data.ambientSoundRollOff = str2int(getMetaValue(i, "ambient_sound_rolloff").c_str());
			}

			for(int j = 0; j < instanceAmount; ++j)
			{
				ObjectInstance &instance = object.instances[j];
				stream >> instance.position.x >> instance.position.y;

				if(version < 13)
					stream >> instance.rotation.y;
				else
					stream >> instance.rotation.x >> instance.rotation.y >> instance.rotation.z;

				instance.setRotation = getRotation(instance.rotation);
				if(version >= 2)
					stream >> instance.ambient.r >> instance.ambient.g >> instance.ambient.b;

				if(version >= 4)
					stream >> instance.heightOffset;

				if(version >= 8)
					stream >> instance.height;
				else
					instance.height = terrain->getHeight(VC2(instance.position.x, instance.position.y));

				instance.groundHeight = instance.height;
				instance.height += instance.heightOffset;
				VC3 objectPosition(instance.position.x, 0, instance.position.y);
				objectPosition.y = instance.height;

				instance.lightUpdatePosition = objectPosition;

				terrain->addInstance(i, objectPosition, instance.setRotation, instance.ambient);
				createAmbientSoundForObject(i, j);

				if(version >= 4)
				{
					// Read and ignore old stuff
					if(version < 20)
					{
						VC3 oldLightPos[2];
						COL oldLightColor[2];
						float oldLightRange[2];

						stream >> oldLightPos[0].x >> oldLightPos[0].y >> oldLightPos[0].z;
						stream >> oldLightColor[0].r >> oldLightColor[0].g >> oldLightColor[0].b;
						if(version >= 16)
							stream >> oldLightRange[0];

						if(version >= 17)
						{
							stream >> oldLightPos[1].x >> oldLightPos[1].y >> oldLightPos[1].z;
							stream >> oldLightColor[1].r >> oldLightColor[1].g >> oldLightColor[1].b;
							stream >> oldLightRange[1];
						}
					}
					/*
					else
					{
						for(int k = 0; k < LIGHT_MAX_AMOUNT; ++k)
							stream >> instance.lightIndices[k];
					}

					stream >> instance.lightPos[0].x >> instance.lightPos[0].y >> instance.lightPos[0].z;
					stream >> instance.lightColor[0].r >> instance.lightColor[0].g >> instance.lightColor[0].b;
					if(version >= 16)
						stream >> instance.lightRange[0];

					if(version >= 17)
					{
						stream >> instance.lightPos[1].x >> instance.lightPos[1].y >> instance.lightPos[1].z;
						stream >> instance.lightColor[1].r >> instance.lightColor[1].g >> instance.lightColor[1].b;
						stream >> instance.lightRange[1];
					}

					if(game::SimpleOptions::getInt(DH_OPT_I_LIGHTING_LEVEL) < 25)
					{
						VC3 lightPos = instance.lightPos[0];
						lightPos *= 3.f;
						lightPos += instance.lightPos[1];
						lightPos *= 0.25f;

						COL lightCol = instance.lightColor[0];
						lightCol *= 3.f;
						lightCol += instance.lightColor[1];
						lightCol *= 0.25f;

						float lightRange = ((instance.lightRange[0] * 3.f) + instance.lightRange[1]) * 0.25f;

						//terrain->setInstanceLight(i, j, 0, lightPos, lightCol, lightRange, instance.ambient + (instance.lightColor[1]));
						terrain->setInstanceLight(i, j, 0, instance.lightPos[0], instance.lightColor[0], instance.lightRange[0], instance.ambient + (instance.lightColor[1] * 0.1f));

						//for(int k = 0; k < lights; ++k)
						//	terrain->setInstanceLight(i, j, k, instance.lightPos[k], instance.lightColor[k], instance.lightRange[k], instance.ambient);

						//for(int k = 0; k < 1; ++k)
						//	terrain->setInstanceLight(i, j, k, instance.lightPos[k], instance.lightColor[k], instance.lightRange[k], instance.ambient);
					}
					else
					{
						for(int k = 0; k < 2; ++k)
							terrain->setInstanceLight(i, j, k, instance.lightPos[k], instance.lightColor[k], instance.lightRange[k], instance.ambient);
					}

					ui::PointLights lights;
					if(lightManager && gameMap)
					{
						VC3 pos(instance.position.x, instance.height, instance.position.y);

						int ox = gameMap->scaledToObstacleX(pos.x);
						int oy = gameMap->scaledToObstacleY(pos.z);
						if(gameMap->isWellInScaledBoundaries(pos.x, pos.z))
						{
							lights.ambient = gameMap->colorMap->getUnmultipliedColor(pos.x / gameMap->getScaledSizeX() + .5f, pos.z / gameMap->getScaledSizeY() + .5f);

							if(gameMap->getAreaMap()->isAreaAnyValue(ox, oy, AREAMASK_INBUILDING))
								lightManager->getLighting(pos, lights, data.radius, false, false);
							else
								lightManager->getLighting(pos, lights, data.radius, false, false);
						}
					}

					for(int k = 0; k < LIGHT_MAX_AMOUNT; ++k)
					{
						instance.lightIndices[k] = lights.lightIndices[k];
						terrain->setInstanceLight(i, j, k, instance.lightIndices[k], instance.ambient);
					}
					*/
				}

				if(version >= 11)
				{
					stream >> instance.sunDir.x >> instance.sunDir.y >> instance.sunDir.z;
					stream >> instance.sunStrength;
					stream >> instance.lightmapped;

					if(version >= 14)
						stream >> instance.inBuilding;

					terrain->setInstanceLightmapped(i, j, instance.lightmapped);
					terrain->setInstanceSun(i, j, instance.sunDir, instance.sunStrength);
					terrain->setInstanceInBuilding(i, j, instance.inBuilding);
				}

				if(version >= 22)
				{
					unsigned int lower;
					unsigned int upper;
					stream >> lower;
					stream >> upper;
					instance.uniqueEditorObjectHandle = 0;
					instance.uniqueEditorObjectHandle |= (UniqueEditorObjectHandle)lower;
					instance.uniqueEditorObjectHandle |= ((UniqueEditorObjectHandle)upper << 32);
				}

				if(object.data.breakTexture == TERRAIN_OBJECT_BREAK_TEXTURE_TYPE_ALWAYS)
					terrain->setInstanceFade(i, j, 0);

				instance.hp = data.hp;
				instance.originalModel = i;
				instance.originalInstance = j;
			}
		}
	}

	void applyTexturing()
	{
		COL sunFColor(sunColor.r, sunColor.g, sunColor.b);
		sunFColor /= 255.f;
		COL ambientFColor(ambientColor.r, ambientColor.g, ambientColor.b);
		ambientFColor /= 255.f;

		scene->SetAmbientLight(ambientFColor);
		scene->GetCamera()->SetVisibilityRange(cameraRange);

		for(unsigned int i = 0; i < textures.size(); ++i)
		{
			terrain->addTerrainTexture(*textures[i]);
		}

		{
			std::vector<DWORD> weights(BLOCK_SIZE * BLOCK_SIZE);

			std::map<int, std::vector<BlendPass> >::iterator it = blendings.begin();
			for(; it != blendings.end(); ++it)
			{
				int blockIndex = it->first;
				std::vector<BlendPass> &passes = it->second;

				for(unsigned int i = 0; i < passes.size(); ++i)
				{
					BlendPass &pass = passes[i];

						pass.texture1 = boost::shared_ptr<IStorm3D_Texture> (storm->CreateNewTexture(BLOCK_SIZE, BLOCK_SIZE, IStorm3D_Texture::TEXTYPE_BASIC));
						pass.texture1->Copy32BitSysMembufferToTexture(&pass.weights[0]);

						terrain->setBlendMap(blockIndex, *pass.texture1, pass.textureA, pass.textureB);
					}
				}
			}

		{
			// SCALE DOWN IF LOW TEXTURE QUALITY

			int size = lightmapSize;
			int halfSize = size / 2;
			bool scaleDown = false;

			int textureLevel = game::SimpleOptions::getInt(DH_OPT_I_TEXTURE_DETAIL_LEVEL);
			if(textureLevel <= 50)
				scaleDown = true;

			int textureSize = (scaleDown) ? halfSize : size;
			if(textureSize < 1)
				textureSize = 1;

			std::vector<DWORD> buffer(textureSize * textureSize);
			boost::shared_ptr<IStorm3D_Texture> black(storm->CreateNewTexture(1, 1, IStorm3D_Texture::TEXTYPE_BASIC), std::mem_fun(&IStorm3D_Texture::Release));
			black->Copy32BitSysMembufferToTexture(&buffer[0]);

			for(int blockIndex = 0; blockIndex < blockAmount; ++blockIndex)
			{
				std::map<int, TerrainLightMap >::iterator it = lightMaps.find(blockIndex);
				if(it != lightMaps.end() && !it->second.values.empty())
				{
					TerrainLightMap &map = it->second;
					map.texture = boost::shared_ptr<IStorm3D_Texture> (storm->CreateNewTexture(textureSize, textureSize, IStorm3D_Texture::TEXTYPE_BASIC), std::mem_fun(&IStorm3D_Texture::Release));

					if(scaleDown)
					{
						for(int y = 0; y < halfSize; ++y)
						for(int x = 0; x < halfSize; ++x)
						{
							int halfIndex = y * halfSize + x;
							int index = (y * 2 * size) + (x * 2);

							TColor<unsigned char> &col = map.values[index];
							buffer[halfIndex] = (col.r << 16) | (col.g << 8) | (col.b);
						}
					}
					else
					{
						for(int i = 0; i < size * size; ++i)
						{
							TColor<unsigned char> &col = map.values[i];
							buffer[i] = (col.r << 16) | (col.g << 8) | (col.b);
						}
					}

					map.texture->Copy32BitSysMembufferToTexture(&buffer[0]);
					terrain->setLightMap(blockIndex, *map.texture);
				}
				else
					terrain->setLightMap(blockIndex, *black);
			}
		}
	}

	void apply()
	{
		terrain = storm->CreateNewTerrain(16);
		terrain->setHeightMap(heightMap.get(), mapSize, realSize, textureRepeat, forceMap.get(), GAMEMAP_HEIGHTMAP_MULTIPLIER, GAMEMAP_HEIGHTMAP_MULTIPLIER*GAMEMAP_PATHFIND_ACCURACY);
		terrain->setObstacleHeightmap(obstacleMap.get(), areaMap);
		terrain->recreateCollisionMap();
		scene->AddTerrain(terrain);

		applyTexturing();
		loadObjects();

		if(!backgroundModel.empty())
		{
			boost::scoped_ptr<IStorm3D_Model> newModel(storm->CreateNewModel());
			newModel->LoadS3D(backgroundModel.c_str());

			backgroundStormModel.swap(newModel);
			scene->SetBackGround(backgroundStormModel.get());
		}

		COL fogFColor(fogColor.r, fogColor.g, fogColor.b);
		fogFColor /= 255.f;
		scene->SetFogParameters(fogEnabled, fogFColor, float(fogStart), float(fogEnd));
	}

	void load(const char *forceName)
	{
		loadScene(forceName);
		SHOW_LOADING_BAR(40);
		apply();
	}

	bool ValidatePosition(const Vector2D &position, float radius)
	{
		if((position.x - radius < -realSize.x/2) || (position.x + radius > realSize.x/2))
			return false;
		if((position.y - radius < -realSize.z/2) || (position.y + radius > realSize.z/2))
			return false;

		return true;
	}

	bool physicsImpulse(int modelId, int objectId, float factor, const VC3 &velocity_, const VC3 &explosion_position, bool use_explosion)
	{
		Object &originalObject = objects[modelId];
		ObjectInstance &instance = originalObject.instances[objectId];

		if (instance.physicsObject)
		{
			VC3 physvel = velocity_;
			if (use_explosion)
			{
				physvel = VC3(instance.position.x, instance.height + 0.5f, instance.position.y) - explosion_position;
				// NOTE: Some magic numbers/factors here!
				if (physvel.GetSquareLength() < 0.1f*0.1f)
				{
					physvel = VC3(0,1.0f,0);
				}
				float physvellen = physvel.GetLength();
				if (physvellen < 0.5f) physvellen = 0.5f;
				physvel *= (100.0f / physvellen);
			} else {
				physvel *= 5.0f;
			}
			physvel *= factor;
			instance.physicsObject->addImpulse(explosion_position, physvel);
			return true;
		}
		return false;
	}


	void updateLatestReplacementInfo(int modelId, int objectId, int newModelId, int newInstanceId)
	{
		int newIndex = newModelId;

		// solve the very first original model, and set the latestReplacement info...
		int veryFirstModel = modelId;
		int veryFirstInstance = objectId;
		int failureCount = 0;
		while (true)
		{
			assert(veryFirstModel >= 0 && veryFirstModel < (int)objects.size());
			assert(veryFirstInstance >= 0 && veryFirstInstance < (int)objects[veryFirstModel].instances.size());

			objects[veryFirstModel].instances[veryFirstInstance].latestReplacementModel = newIndex;
			objects[veryFirstModel].instances[veryFirstInstance].latestReplacementInstance = newInstanceId;

			if (objects[veryFirstModel].instances[veryFirstInstance].originalModel == veryFirstModel
				&& objects[veryFirstModel].instances[veryFirstInstance].originalInstance == veryFirstInstance)
			{
				// found the very first original model/instance id.
				break;
			}
			int tmp1 = objects[veryFirstModel].instances[veryFirstInstance].originalModel;
			int tmp2 = objects[veryFirstModel].instances[veryFirstInstance].originalInstance;
			veryFirstModel = tmp1;
			veryFirstInstance = tmp2;

			failureCount++;
			if (failureCount > 1000)
			{
				LOG_ERROR("TerrainData::updateLatestReplacementInfo - failure count exceeded while trying to solve the very original model/instance.");
				assert(!"TerrainData::updateLatestReplacementInfo - failure count exceeded while trying to solve the very original model/instance.");
				break;
			}
		}
	}


	bool breakObjects(int modelId, int objectId, int damage, std::vector<TerrainObstacle> &removedObjects, std::vector<ExplosionEvent> &events, const VC2 &position, const VC3 &velocity_, const VC3 &explosion_position, bool use_explosion, bool only_breaktexture)
	{
		Object &originalObject = objects[modelId];
		ObjectInstance &instance = originalObject.instances[objectId];

#ifdef PROJECT_CLAW_PROTO
		bool isClawObject = false;
		if(gamephysics_clawController)
		{
			if(gamephysics_clawController->isClawObject(modelId, objectId))
				isClawObject = true;
		}
#endif

		// This enough to fix continuous sounds?
		if(instance.hp < 0)
			return false;

		if(instance.hp > damage)
		{
			if(originalObject.data.breakTexture == TERRAIN_OBJECT_BREAK_TEXTURE_TYPE_SCALE_HP)
			{
				float f = float(instance.hp) / float(originalObject.data.hp);
				terrain->setInstanceFade(modelId, objectId, f);
			}
			else if(originalObject.data.breakTexture == TERRAIN_OBJECT_BREAK_TEXTURE_TYPE_ALWAYS)
				terrain->setInstanceFade(modelId, objectId, 0);

			instance.hp -= damage;
			return false;
		}

		//if (only_breaktexture)
		// HACK: apply this only_breaktexture only to non-explosive objects... (to get flamer to blow up explosives)
		if (only_breaktexture
			&& originalObject.data.explosionProjectile.empty())
		{
			return false;
		}

		damage -= instance.hp;
		int previousExp = -1;
		int newIndex = modelId;

		VC3 velocity = velocity_;
		if(!use_explosion)
		{
			if(velocity.GetSquareLength() > 0.00001f)
				velocity.Normalize();
		}

		velocity /= GAME_TICKS_PER_SECOND;
		if(originalObject.data.hasExplosion())
		{
			ExplosionEvent event = createEvent(originalObject.data, instance, position, velocity, terrain, explosion_position, use_explosion, this->getUnifiedHandle(modelId, objectId));
			events.push_back(event);
		}

		VC3 physVel = VC3(0,0,0);
		VC3 physAngVel = VC3(0,0,0);
		if (instance.physicsObject)
		{
			physVel = instance.physicsObject->getVelocity();
			physAngVel = instance.physicsObject->getAngularVelocity();
		}

		for(int i = 0; ; ++i)
		{
			Object &previousObject = objects[newIndex];
			previousExp = newIndex;

			/*
			if(previousObject.data.hasExplosion())
			{
				ExplosionEvent event = createEvent(previousObject.data, instance, position, velocity, terrain, explosion_position, use_explosion);
				events.push_back(event);
			}
			*/

			Object &newObject = objects[newIndex];
			if(i > 0)
			{
				damage -= newObject.data.hp;
			}

			if(newObject.data.explosionObstacle.empty())
			{
				break;
			}

			if(damage < 0)
			{
				break;
			}

			if(newObject.data.explosionObstacle == "(disappear)")
			{
				newIndex = -1;
				break;
			}

			if(i > 0 && previousObject.data.hasExplosion())
			{
				ExplosionEvent event = createEvent(previousObject.data, instance, position, velocity, terrain, explosion_position, use_explosion, this->getUnifiedHandle(modelId, objectId));
				events.push_back(event);
			}

// HACK: _loop
if(newObject.data.explosionObstacle.find("_loop") != std::string::npos)
{
	signal_this_terrain_object_break_hack = true;
	return false;
}


			if( i > 100000 )
			{
				Logger::getInstance()->warning("Terrain object trying to break itself - possible endless loop");
				Logger::getInstance()->warning(newObject.data.fileName.c_str() );

				break;
			}

			ObjectIndices::iterator it = objectIndices.find(newObject.data.explosionObstacle);
			if(it == objectIndices.end())
			{
				Logger::getInstance()->warning("Terrain explosion object not properly loaded");
				Logger::getInstance()->warning(newObject.data.explosionObstacle.c_str());
				
				newIndex = -1;
				break;
			}

			newIndex = it->second;
		}

		if(newIndex == modelId)
		{
			return false;
		}

		if(newIndex >= 0)
		{
			Object &newObject = objects[newIndex];

			VC3 position(instance.position.x, 0, instance.position.y);
			//position.y = terrain->getHeight(instance.position) + instance.heightOffset;
			position.y = instance.height;

			//COL lightColor[2] = { instance.lightColor[0], instance.lightColor[1] };
			COL ambient = instance.ambient;
			if(instance.inBuilding)
			{
				//lightColor[0] *= colorFactor;
				//lightColor[1] *= colorFactor;
				//ambient *= colorFactor;
			}

			terrain->addInstance(newIndex, position, instance.setRotation, instance.ambient);
			for(int i = 0; i < LIGHT_MAX_AMOUNT; ++i)
				terrain->setInstanceLight(newIndex, newObject.instances.size(), i, instance.lightIndices[i], ambient);

			terrain->setInstanceLightmapped(newIndex, newObject.instances.size(), instance.lightmapped);
			terrain->setInstanceSun(newIndex, newObject.instances.size(), instance.sunDir, instance.sunStrength);

			ObjectInstance copy = instance;
			copy.originalName = originalObject.data.fileName;
			copy.hp = newObject.data.hp;
			copy.originalModel = modelId;
			copy.originalInstance = objectId;

			assert(copy.originalModel != newIndex || copy.originalInstance != newObject.instances.size());
			

			// DON'T COPY THE PHYSICS OBJECT!
			copy.physicsObject.reset();

			copy.dynamicObstacleExists = false;
			copy.dynamicObstaclePosition = VC2I(0,0);

			if(newObject.data.breakTexture == TERRAIN_OBJECT_BREAK_TEXTURE_TYPE_SCALE_HP)
			{
				float f = float(copy.hp) / float(newObject.data.hp);
				terrain->setInstanceFade(newIndex, newObject.instances.size(), f);
			}
			else if(newObject.data.breakTexture == TERRAIN_OBJECT_BREAK_TEXTURE_TYPE_ALWAYS)
				terrain->setInstanceFade(newIndex, newObject.instances.size(), 0);

			newObject.instances.push_back(copy);
			int newInstanceId = newObject.instances.size() - 1;

			createAmbientSoundForObject(newIndex, newInstanceId);

			updateLatestReplacementInfo(modelId, objectId, newIndex, newInstanceId);

			// NOTE: this is not an error??
			// if _new_ object has no obstacle, remove original obstacle. sounds reasonable in a way - if object just gets battered a bit, don't want to get rid of the obstacle..
			
			if(newObject.data.type == TERRAIN_OBSTACLE_TYPE_NONE)
			{
				if (!instance.movedByPhysics)
				{
					if( instance.originalInstance < (int)originalObject.instances.size() )
					{
						// FIXME: The game crashes here sometimes. Debugger says that originalObject.instances[instance.originalInstance].originalName has bad ptr inside.
						TerrainObstacle obstacle = createObstacle(originalObject.data, originalObject.instances[instance.originalInstance]);
						removedObjects.push_back(obstacle);
					}
				}
			}

			ObjectData &data = newObject.data;

			if(data.fileExists)
			{
#ifdef PHYSICS_PHYSX
				createPhysicsMeshForModel(newIndex, terrain_cylinderFile, &terrain_meshmodel);
#endif
				int tmp = 0;
				// TODO: optimize!!!
static util::SoundMaterialParser soundmp;
static util::ObjectDurabilityParser durp;
				createPhysicsForObject(terrain_gamePhysics, newIndex, newInstanceId, false, soundmp, durp, tmp);

#ifdef PROJECT_CLAW_PROTO
				if(isClawObject && gamephysics_clawController)
				{
					gamephysics_clawController->setNewClawObject(newObject.instances[newInstanceId].physicsObject);
				}
#endif

				if (newObject.instances[newInstanceId].physicsObject)
				{
					newObject.instances[newInstanceId].physicsObject->setVelocity(physVel);
					newObject.instances[newInstanceId].physicsObject->setAngularVelocity(physAngVel);
				}
			}
		}
		else
		{
			Object &o = objects[instance.originalModel];
			if (!instance.movedByPhysics)
			{
				TerrainObstacle obstacle = createObstacle(o.data, o.instances[instance.originalInstance]);
				removedObjects.push_back(obstacle);
			}

			// We need to break objects which are layed on top of broken one
			{
				static const float COLLISION_FACTOR = 0.35f;

				AABB box = originalObject.model->GetBoundingBox();
				float radius = box.mmin.GetRangeTo(box.mmax);
				float radius2 = getRadius2d(box.mmin, box.mmax) * COLLISION_FACTOR;

				VC3 position(instance.position.x, 0, instance.position.y);
				//position.y = terrain->getHeight(instance.position) + instance.heightOffset;
				position.y = instance.height;

				boost::shared_ptr<IStorm3D_TerrainModelIterator> it = terrain->getModelIterator(position, radius + 2.f);
				for(; !it->end(); )
				{
					const VC3 &objectPosition = it->getPosition();
					if(objectPosition.y < position.y + 0.2f)
					{
						it->next();
						continue;
					}

					int modelId = it->getModelId();
					int instanceId = it->getInstanceId();

					Object &collisionObject = objects[modelId];
					AABB collisionBox = collisionObject.model->GetBoundingBox();
					float collisionRadius2 = getRadius2d(collisionBox.mmin, collisionBox.mmax) * COLLISION_FACTOR;

					if(getRadius2d(objectPosition, position) > radius2 + collisionRadius2)
					{
						it->next();
						continue;
					}

					// TEMPHAX -- this seem to be cause infinte recursion which blows up our stack
#ifdef PHYSICS_NONE
					VC2 position2(objectPosition.x, objectPosition.z);
					if(breakObjects(modelId, instanceId, 100000, removedObjects, events,  position2, velocity, explosion_position, use_explosion, only_breaktexture) )
						it->erase();
					else
						it->next();
#else
					VC2 position2(objectPosition.x, objectPosition.z);
					if(!collisionObject.data.hasPhysics() && breakObjects(modelId, instanceId, 100000, removedObjects, events,  position2, velocity, explosion_position, use_explosion, false))
						it->erase();
					else
						it->next();
#endif
				}
			}
		}

		instance.deleted = true;

		removeDynamicObstacle(modelId, objectId);

		stopAmbientSoundForObject(modelId, objectId);

		if(instance.physicsObject)
			instance.physicsObject.reset();

		// duh, the above line already does this...
		//deletePhysicsForObject(modelId, objectId);

		return true;
	}

	void createAmbientSoundForObject(int modelId, int instanceId_)
	{
		Object &object = objects[modelId];
		ObjectData &data = object.data;
		ObjectInstance &instance = object.instances[instanceId_];

		if(data.ambientSound.empty())
			return;
		
		ambientSoundManager->setNextFreeAmbientSound();
		instance.ambientSound = ambientSoundManager->getSelectedAmbientSound();
		ambientSoundManager->setAmbientSoundRange(instance.ambientSound, (float)data.ambientSoundRange);
		ambientSoundManager->setAmbientSoundRollOff(instance.ambientSound, data.ambientSoundRange);
		ambientSoundManager->makeAmbientSoundFromDefString(instance.ambientSound, ("!10000000*" + data.ambientSound + ",*,*").c_str());
		ambientSoundManager->setAmbientSoundPosition(instance.ambientSound, VC3(instance.position.x, instance.height, instance.position.y));
		ambientSoundManager->startAmbientSound(instance.ambientSound);
	}

	void stopAmbientSoundForObject(int modelId, int instanceId)
	{
		if(objects[modelId].instances[instanceId].ambientSound != -1)
		{
			ambientSoundManager->stopAmbientSound(objects[modelId].instances[instanceId].ambientSound, true);
			objects[modelId].instances[instanceId].ambientSound = -1;
		}
	}

	void deletePhysicsForObject(int modelId, int instanceId)
	{
		Object &object = objects[modelId];
		ObjectData &objectData = objects[modelId].data;
		ObjectInstance &instance = object.instances[instanceId];
		if (instance.physicsObject)
		{
			instance.physicsObject.reset();

			if(objectData.physicsType != 1)
			{
				// this flag needs to be set back to false, since terrain physics may be recreated...
				instance.movedByPhysics = false;
				// also update the instance euler angle rotation to match the physics rotation...
				// (this is needed after the stabilizing physics simulation done for the physics cache)
				instance.rotation = instance.setRotation.getEulerAngles();
			}
		}
	}

#ifdef PHYSICS_PHYSX
	void createPhysicsMeshForModel(int modelId, std::string &cylinderFile, IStorm3D_Model **meshmodel)
	{
		Object &object = objects[modelId];
		ObjectData &data = object.data;

		if(data.physicsType == TERRAIN_OBJECT_PHYSICS_TYPE_STATIC)
		{
			// TODO: redo, this is actually some totally shit implementation...

			if ((int)staticPhysicsTempModels.size() > modelId
				&& staticPhysicsTempModels[modelId] != NULL)
			{
				*meshmodel = staticPhysicsTempModels[modelId];
			} else {
				*meshmodel = storm->CreateNewModel();
				(*meshmodel)->LoadS3D(data.fileName.c_str());

				int prevSize = (int)staticPhysicsTempModels.size();
				if (prevSize < modelId + 1)
				{
					// TODO: optimize, could already resize it a bit bigger (or else, we end up resizeing this 
					// each time a new model is loaded..
					staticPhysicsTempModels.resize(modelId + 1);
					// just to be sure, set all of the new pointers to null... (assuming vector does not init them properly)
					for (int i = prevSize; i < modelId + 1; i++)
					{
						staticPhysicsTempModels[i] = NULL;
					}
				}
				staticPhysicsTempModels[modelId] = *meshmodel;
			}
		}

		if(data.physicsType == TERRAIN_OBJECT_PHYSICS_TYPE_CYLINDER)
		{
			float height = data.physicsData1.x;
			float radius = data.physicsData1.y;

#ifdef LEGACY_FILES
			cylinderFile = "Data/Models/cylinder_";
#else
			cylinderFile = "data/model/cylinder_cook/cylinder_";
#endif
			int heightInt = int(height * 100.f);
			cylinderFile += boost::lexical_cast<std::string> (heightInt);
			cylinderFile += "_";
			int radiusInt = int(radius * 100.f);
			cylinderFile += boost::lexical_cast<std::string> (radiusInt);
			cylinderFile += ".cook";

			filesystem::FB_FILE *fp = filesystem::fb_fopen(cylinderFile.c_str(), "rb");
			if(fp)
			{
				filesystem::fb_fclose(fp);
			} else {
				physics::Cooker cooker;
#ifdef GAME_SIDEWAYS
				cooker.cookCylinder(cylinderFile.c_str(), height, radius, -height/2, 2);
#else
				cooker.cookCylinder(cylinderFile.c_str(), height, radius);
#endif
			}
		}
	}
#endif

	void createPhysicsForObject(game::GamePhysics *gamePhysics, int modelId, int instanceId_, bool sleepPhysicsObject, util::SoundMaterialParser &soundmp, util::ObjectDurabilityParser &durp, int &totalCreated)
	{
		assert(terrain_gamePhysics != NULL);

		int i = instanceId_;

#ifdef PHYSICS_PHYSX
		std::string cylinderFile = terrain_cylinderFile;
		IStorm3D_Model *meshmodel = terrain_meshmodel;
#endif

		Object &object = objects[modelId];
		ObjectData &data = object.data;
		ObjectInstance &instance = object.instances[i];

		if(data.hasPhysics())
		{
			VC3 pos = VC3(instance.position.x, instance.height, instance.position.y);
			QUAT rot = instance.setRotation;

			if(data.physicsType == TERRAIN_OBJECT_PHYSICS_TYPE_BOX)
			{
				game::BoxPhysicsObject *bp = new game::BoxPhysicsObject(gamePhysics, data.physicsData1, data.physicsMass, PHYSICS_COLLISIONGROUP_DYNAMIC_TERRAINOBJECTS, pos);
				if (sleepPhysicsObject)
					bp->setToSleep();
				bp->setRotation(rot);
				bp->setSoundMaterial(soundmp.getMaterialIndexByName(data.physicsSoundMaterial));
				bp->setDurabilityType(durp.getDurabilityTypeIndexByName(data.durabilityType));

				int instanceId = i;
				// WARNING: unsafe int to void * cast!
				bp->setCustomData((void *)game::PhysicsContactUtils::calcCustomPhysicsObjectDataForTerrainObject(modelId, instanceId));

				instance.physicsObject.reset(bp);
			}
			else if(data.physicsType == TERRAIN_OBJECT_PHYSICS_TYPE_CYLINDER)
			{
#ifdef PHYSICS_ODE
				float height = data.physicsData1.x;
				float radius = data.physicsData1.y;

				game::CylinderPhysicsObject *cp = new game::CylinderPhysicsObject(gamePhysics, height, radius, data.physicsMass, PHYSICS_COLLISIONGROUP_DYNAMIC_TERRAINOBJECTS, pos);
				if (sleepPhysicsObjects)
					cp->setToSleep();
				cp->setRotation(rot);
				cp->setSoundMaterial(soundmp.getMaterialIndexByName(data.physicsSoundMaterial));
				cp->setDurabilityType(durp.getDurabilityTypeIndexByName(data.durabilityType));
				instance.physicsObject.reset(cp);
#endif
#ifdef PHYSICS_PHYSX
				game::ConvexPhysicsObject *cp = new game::ConvexPhysicsObject(gamePhysics, cylinderFile.c_str(), data.physicsMass, PHYSICS_COLLISIONGROUP_DYNAMIC_TERRAINOBJECTS, pos);
				if (sleepPhysicsObject)
					cp->setToSleep();
				cp->setRotation(rot);
				cp->setSoundMaterial(soundmp.getMaterialIndexByName(data.physicsSoundMaterial));
				cp->setDurabilityType(durp.getDurabilityTypeIndexByName(data.durabilityType));

				int instanceId = i;
				// WARNING: unsafe int to void * cast!
				cp->setCustomData((void *)game::PhysicsContactUtils::calcCustomPhysicsObjectDataForTerrainObject(modelId, instanceId));

				instance.physicsObject.reset(cp);
#endif
			}
			else if(data.physicsType == TERRAIN_OBJECT_PHYSICS_TYPE_CAPSULE)
			{
				float height = data.physicsData1.x;
				float radius = data.physicsData1.y;

				game::CapsulePhysicsObject *cp = new game::CapsulePhysicsObject(gamePhysics, height, radius, data.physicsMass, PHYSICS_COLLISIONGROUP_DYNAMIC_TERRAINOBJECTS, pos);
				if (sleepPhysicsObject)
					cp->setToSleep();
				cp->setRotation(rot);
				cp->setSoundMaterial(soundmp.getMaterialIndexByName(data.physicsSoundMaterial));
				cp->setDurabilityType(durp.getDurabilityTypeIndexByName(data.durabilityType));

				int instanceId = i;
				// WARNING: unsafe int to void * cast!
				cp->setCustomData((void *)game::PhysicsContactUtils::calcCustomPhysicsObjectDataForTerrainObject(modelId, instanceId));

				instance.physicsObject.reset(cp);
			}
#ifdef PHYSICS_PHYSX
			else if(data.physicsType == 5)
			{
				game::RackPhysicsObject *rp = new game::RackPhysicsObject(gamePhysics, data.physicsMass, PHYSICS_COLLISIONGROUP_DYNAMIC_TERRAINOBJECTS, pos);
				if (sleepPhysicsObject)
					rp->setToSleep();
				rp->setRotation(rot);
				rp->setSoundMaterial(soundmp.getMaterialIndexByName(data.physicsSoundMaterial));
				rp->setDurabilityType(durp.getDurabilityTypeIndexByName(data.durabilityType));

				int instanceId = i;
				// WARNING: unsafe int to void * cast!
				rp->setCustomData((void *)game::PhysicsContactUtils::calcCustomPhysicsObjectDataForTerrainObject(modelId, instanceId));

				instance.physicsObject.reset(rp);
			}
#ifdef PROJECT_CLAW_PROTO
			else if(data.physicsType == 6)
			{
				game::CarPhysicsObject *rp = new game::CarPhysicsObject(gamePhysics, data.physicsMass, PHYSICS_COLLISIONGROUP_DYNAMIC_TERRAINOBJECTS, pos);
				if (sleepPhysicsObject)
					rp->setToSleep();
				rp->setRotation(rot);
				rp->setSoundMaterial(soundmp.getMaterialIndexByName(data.physicsSoundMaterial));
				rp->setDurabilityType(durp.getDurabilityTypeIndexByName(data.durabilityType));

				int instanceId = i;
				// WARNING: unsafe int to void * cast!
				rp->setCustomData((void *)game::PhysicsContactUtils::calcCustomPhysicsObjectDataForTerrainObject(modelId, instanceId));

				instance.physicsObject.reset(rp);
			}
#endif
#endif
			totalCreated++;
		} 
		else 
		{
			if(data.physicsType == TERRAIN_OBJECT_PHYSICS_TYPE_STATIC)
			{
				VC3 pos = VC3(instance.position.x, instance.height, instance.position.y);
				QUAT rot = instance.setRotation;
#ifdef PHYSICS_ODE
				game::StaticPhysicsObject *sp = new game::StaticPhysicsObject(gamePhysics, data.fileName.c_str(), NULL, pos, rot);
				instance.physicsObject.reset(sp);
				sp->setSoundMaterial(soundmp.getMaterialIndexByName(data.physicsSoundMaterial));
				sp->setDurabilityType(durp.getDurabilityTypeIndexByName(data.durabilityType));

				int instanceId = i;
				// WARNING: unsafe int to void * cast!
				sp->setCustomData((void *)game::PhysicsContactUtils::calcCustomPhysicsObjectDataForTerrainObject(modelId, instanceId));
#endif
#ifdef PHYSICS_PHYSX
				game::StaticPhysicsObject *sp = new game::StaticPhysicsObject(gamePhysics, data.fileName.c_str(), meshmodel, pos, rot);
				instance.physicsObject.reset(sp);
				sp->setSoundMaterial(soundmp.getMaterialIndexByName(data.physicsSoundMaterial));
				sp->setDurabilityType(durp.getDurabilityTypeIndexByName(data.durabilityType));

				int instanceId = i;
				// WARNING: unsafe int to void * cast!
				sp->setCustomData((void *)game::PhysicsContactUtils::calcCustomPhysicsObjectDataForTerrainObject(modelId, instanceId));
#endif
			}
		}
	}
};

/** Terrain **/

Terrain::Terrain(IStorm3D *storm, IStorm3D_Scene *scene, const char *dirName, const char *forceMapName, const util::AreaMap *areaMap, game::GameMap *gameMap, ui::LightManager *lightManager, ui::AmbientSoundManager *ambientSoundManager)
{
	init_terrain_object_variables();

	boost::scoped_ptr<TerrainData> tempData(new TerrainData());
	tempData->storm = storm;
	tempData->scene = scene;
	tempData->gameMap = gameMap;
	tempData->areaMap = areaMap;
	tempData->lightManager = lightManager;
	tempData->ambientSoundManager = ambientSoundManager;

	if (dirName == NULL)
	{
		fb_assert(!"Terrain - Null dirName parameter given.");
	} else {
		tempData->dirName = dirName;
	}

	tempData->load(forceMapName);
	data.swap(tempData);
}

Terrain::~Terrain()
{
	uninit_terrain_object_variables();
}

IStorm3D_Terrain *Terrain::GetTerrain()
{
	return data->terrain;
}

bool Terrain::ValidatePosition(const Vector2D &position, float radius) const
{
	return data->ValidatePosition(position, radius);
}

void Terrain::getObstacles(std::vector<TerrainObstacle> &obstacles)
{
	int index = 0;

	ObjectList::iterator it = data->objects.begin();
	for(; it != data->objects.end(); ++it)
	{
		Object &object = *it;
		ObjectData &data = object.data;
		obstacles.resize(index + object.instances.size());

		for(unsigned int i = 0; i < object.instances.size(); ++i)
		{
			ObjectInstance &instance = object.instances[i];
			obstacles[index++] = createObstacle(data, instance);
		}
	}
}

void Terrain::setLightManager(ui::LightManager *lightManager)
{
	data->lightManager = lightManager;
}

void Terrain::setAmbientSoundManager(ui::AmbientSoundManager *ambientSoundManager)
{
	data->ambientSoundManager = ambientSoundManager;
}

void Terrain::loadPhysicsCache(game::GamePhysics *gamePhysics, char *mapFilename)
{
	assert(mapFilename != NULL);

	std::string filenamestr = std::string(mapFilename) + std::string("/pcache.bin");
	filesystem::FB_FILE *f = filesystem::fb_fopen(filenamestr.c_str(), "rb");

	int filesize = fb_fsize(f);
	int curpos = 0;

	ObjectList::iterator it = data->objects.begin();
	for(; it != data->objects.end(); ++it)
	{
		Object &object = *it;
#ifndef NDEBUG
		ObjectData &data = object.data;
#endif
		for(unsigned int i = 0; i < object.instances.size(); ++i)
		{
			ObjectInstance &instance = object.instances[i];
			VC3 pos;
			QUAT rotq;
			bool hasphys;

			if (curpos + (int)sizeof(bool)+7*(int)sizeof(float) > filesize)
			{
				assert(!"Terrain::loadPhysicsCache - Cache data amount did not match expected data amount.");
				Logger::getInstance()->error("Terrain::loadPhysicsCache - Cache data amount did not match expected data amount.");
			}
			curpos += sizeof(bool)+7*sizeof(float);

			fb_fread(&hasphys, sizeof(bool), 1, f);
			fb_fread(&pos.x, sizeof(float), 1, f);
			fb_fread(&pos.y, sizeof(float), 1, f);
			fb_fread(&pos.z, sizeof(float), 1, f);
			fb_fread(&rotq.x, sizeof(float), 1, f);
			fb_fread(&rotq.y, sizeof(float), 1, f);
			fb_fread(&rotq.z, sizeof(float), 1, f);
			fb_fread(&rotq.w, sizeof(float), 1, f);

			if (hasphys)
			{
				//assert(data.hasPhysics());
				assert(data.physicsType >= 1);

				VC3 rota = rotq.getEulerAngles();
				instance.position = VC2(pos.x, pos.z);
				instance.height = pos.y;
				instance.setRotation = rotq;
				instance.rotation = rota;
			} else {
				//assert(data.physicsType < 1);
				//assert(!data.hasPhysics());
			}
		}
	}
	
	fb_fclose(f);
}


void Terrain::updateAllPhysicsObjectsLighting()
{
	// update the actual renderable terrain object positions...
	// (SOME "NICE" COPY&PASTE PROGRAMMING HERE, TAKEN FROM UPDATEPHYSICS...)
	for(unsigned int i = 0; i < data->objects.size(); ++i)
	{
		Object &o = data->objects[i];
		if(!o.data.hasPhysics())
			continue;

		for(unsigned int j = 0; j < o.instances.size(); ++j)
		{
			ObjectInstance &instance = o.instances[j];

			VC3 pos = VC3(instance.position.x, instance.height, instance.position.y);
			QUAT rot = instance.setRotation;

			data->terrain->setInstancePosition(i, j, pos);
			data->terrain->setInstanceRotation(i, j, rot);

			ui::PointLights lights;
			//lights.ambient = instance.ambient;

			if(data->lightManager && data->gameMap)
			{
				int ox = data->gameMap->scaledToObstacleX(pos.x);
				int oy = data->gameMap->scaledToObstacleY(pos.z);
				if (data->gameMap->isWellInScaledBoundaries(pos.x, pos.z))
				{
					//lights.ambient = data->gameMap->colorMap->getColor(pos.x / data->gameMap->getScaledSizeX() + .5f, pos.z / data->gameMap->getScaledSizeY() + .5f);
					lights.ambient = data->gameMap->colorMap->getUnmultipliedColor(pos.x / data->gameMap->getScaledSizeX() + .5f, pos.z / data->gameMap->getScaledSizeY() + .5f);

					if(data->gameMap->getAreaMap()->isAreaAnyValue(ox, oy, AREAMASK_INBUILDING))
						data->lightManager->getLighting(pos, lights, o.data.radius, false, false, data->terrain->getInstanceModel(i, j) );
					else
						data->lightManager->getLighting(pos, lights, o.data.radius, false, false, data->terrain->getInstanceModel(i, j) );
				}
			}

			for(int k = 0; k < LIGHT_MAX_AMOUNT; ++k)
			{
				instance.ambient = lights.ambient;
				instance.lightIndices[k] = lights.lightIndices[k];
				//instance.lightPos[k] = lights.lights[k].position;
				//instance.lightRange[k] = lights.lights[k].range;
				//instance.lightColor[k] = lights.lights[k].color;

				data->terrain->setInstanceLight(i, j, k, instance.lightIndices[k], instance.ambient);
			}

			instance.lightUpdatePosition = pos;
		}
	}

}

void Terrain::savePhysicsCache(game::GamePhysics *gamePhysics, char *mapFilename)
{
	assert(mapFilename != NULL);

	std::string filenamestr = std::string(mapFilename) + std::string("/pcache.bin");
	FILE *f = fopen(filenamestr.c_str(), "wb");

	if (f != NULL)
	{
	int savedDataSize = 0;
	int expectedDataSize = 0;

	ObjectList::iterator it = data->objects.begin();
	for(; it != data->objects.end(); ++it)
	{
		Object &object = *it;
		for(unsigned int i = 0; i < object.instances.size(); ++i)
		{
			ObjectInstance &instance = object.instances[i];
			bool hasphys = false;
			if (instance.physicsObject)
			{
				hasphys = true;
			}
			VC3 pos = VC3(instance.position.x, instance.height, instance.position.y);
			QUAT rotq = instance.setRotation;

			savedDataSize += sizeof(bool) * fwrite(&hasphys, sizeof(bool), 1, f);
			savedDataSize += sizeof(float) * fwrite(&pos.x, sizeof(float), 1, f);
			savedDataSize += sizeof(float) * fwrite(&pos.y, sizeof(float), 1, f);
			savedDataSize += sizeof(float) * fwrite(&pos.z, sizeof(float), 1, f);
			savedDataSize += sizeof(float) * fwrite(&rotq.x, sizeof(float), 1, f);
			savedDataSize += sizeof(float) * fwrite(&rotq.y, sizeof(float), 1, f);
			savedDataSize += sizeof(float) * fwrite(&rotq.z, sizeof(float), 1, f);
			savedDataSize += sizeof(float) * fwrite(&rotq.w, sizeof(float), 1, f);

			expectedDataSize += sizeof(bool)+7*sizeof(float);
		}
	}

	if (expectedDataSize != savedDataSize)
	{
		Logger::getInstance()->error("Terrain::savePhysicsCache - Cache data save error.");
		assert(!"Terrain::savePhysicsCache - Cache data save error.");
	}

	fclose(f);
}
}

void Terrain::releasePhysicsResources(void)
{
#ifdef PHYSICS_PHYSX
	data->terrainPhysics.reset();
#endif
}

void Terrain::deletePhysics(game::GamePhysics *gamePhysics)
{
//	ObjectList::iterator it = data->objects.begin();
//	for(; it != data->objects.end(); ++it)
//	{
//		Object &object = *it;
//		ObjectData &data = object.data;
	for(unsigned int modelId = 0; modelId < data->objects.size(); ++modelId)
	{
		Object &object = data->objects[modelId];
		for(unsigned int i = 0; i < object.instances.size(); ++i)
		{
			data->deletePhysicsForObject(modelId, i);
		}
	}

	terrain_gamePhysics = NULL;
}

void Terrain::createPhysics(game::GamePhysics *gamePhysics, unsigned char *clipMap, bool sleepPhysicsObjects, const char *currentMission)
{
	Logger::getInstance()->debug("Terrain::createPhysics - About to create physics actors...");
	util::SoundMaterialParser soundmp;
	util::ObjectDurabilityParser durp;

	// HACK: ...
	terrain_gamePhysics = gamePhysics;

#ifdef PHYSICS_PHYSX
	if (game::SimpleOptions::getBool(DH_OPT_B_PHYSICS_TERRAIN_ENABLED))
	{
#ifdef LEGACY_FILES
		int terrainSoundMatIndex = soundmp.getMaterialIndexByName("Terrain");
#else
		int terrainSoundMatIndex = soundmp.getMaterialIndexByName("terrain");
#endif

#ifdef PHYSICS_PHYSX
		//game::TerrainPhysicsObject *tpo = new game::TerrainPhysicsObject(gamePhysics, data->terrain->getCollisionHeightmap(), data->mapSize.x * GAMEMAP_HEIGHTMAP_MULTIPLIER, data->mapSize.y * GAMEMAP_HEIGHTMAP_MULTIPLIER, data->realSize);
		//tpo->setSoundMaterial(terrainSoundMatIndex);
		//data->terrainPhysics.reset(tpo);

		std::string obstFilename = std::string(currentMission) + std::string("/scene.bin");
		std::string terrainFilename = std::string(currentMission) + std::string("/terrain.cook");
		if(!FileTimestampChecker::isFileUpToDateComparedTo(terrainFilename.c_str(), obstFilename.c_str()))
		{
			physics::Cooker cooker;
			cooker.cookHeightmap(data->heightMap.get(), clipMap, data->mapSize, data->realSize, terrainFilename.c_str());
		}

		//if (game::SimpleOptions::getBool(DH_OPT_B_PHYSICS_USE_HARDWARE))
		{
			boost::shared_ptr<frozenbyte::physics::StaticMesh> mesh = gamePhysics->getPhysicsLib()->createStaticMesh(terrainFilename.c_str());
			
			VC3 physics_mesh_offset;
			if( game::Unit::getVisualizationOffsetInUse() )
				physics_mesh_offset.y = game::Unit::getVisualizationOffset();

			game::StaticPhysicsObject *sp = new game::StaticPhysicsObject(gamePhysics, mesh, physics_mesh_offset, QUAT());
			sp->setSoundMaterial(terrainSoundMatIndex);
			data->terrainPhysics.reset(sp);
		}
		/*
		else
		{
			game::TerrainPhysicsObject *tpo = new game::TerrainPhysicsObject(gamePhysics, data->terrain->getCollisionHeightmap(), data->mapSize.x * GAMEMAP_HEIGHTMAP_MULTIPLIER, data->mapSize.y * GAMEMAP_HEIGHTMAP_MULTIPLIER, data->realSize);
			tpo->setSoundMaterial(terrainSoundMatIndex);
			data->terrainPhysics.reset(tpo);
		}
		*/
#endif

	}
#endif

	int totalCreated = 0;
//	ObjectList::iterator it = data->objects.begin();
//	for(; it != data->objects.end(); ++it)
		//Object &object = *it;
	for (int modelId = 0; modelId < (int)data->objects.size(); modelId++)
	{
		Object &object = data->objects[modelId];
		ObjectData &data = object.data;

		if(!data.fileExists)
			continue;

#ifdef PHYSICS_PHYSX
		this->data->createPhysicsMeshForModel(modelId, terrain_cylinderFile, &terrain_meshmodel);
#endif

		for(unsigned int i = 0; i < object.instances.size(); ++i)
		{
			this->data->createPhysicsForObject(gamePhysics, modelId, i, sleepPhysicsObjects, soundmp, durp, totalCreated);
		}
	}

	Logger::getInstance()->debug("Terrain::createPhysics - Done, total actors created follows:");
	Logger::getInstance()->debug(int2str(totalCreated));
}

void Terrain::updatePhysics(game::GamePhysics *gamePhysics, std::vector<TerrainObstacle> &objectsMovedFirstTime, util::GridOcclusionCuller *culler)
{
	bool useOcclusionCulling = false;
	if (culler != NULL)
		useOcclusionCulling = true;

	for(unsigned int i = 0; i < data->objects.size(); ++i)
	{
		Object &o = data->objects[i];
		if(!o.data.hasPhysics())
			continue;

		for(unsigned int j = 0; j < o.instances.size(); ++j)
		{
			ObjectInstance &instance = o.instances[j];

			if (instance.physicsObject)
			{
				game::AbstractPhysicsObject *bp = instance.physicsObject.get();
				VC3 pos = bp->getPosition();
				QUAT rot = bp->getRotation();

				bool needsUpdate = false;
				if (fabs(instance.position.x - pos.x) > UPDATE_PHYSICS_RANGE || fabs(instance.position.y - pos.z) > UPDATE_PHYSICS_RANGE || fabs(instance.height - pos.y) > UPDATE_PHYSICS_RANGE)
					needsUpdate = true;
				if (fabs(instance.setRotation.x - rot.x) > 0.0015f 
					|| fabs(instance.setRotation.y - rot.y) > 0.0015f
					|| fabs(instance.setRotation.z - rot.z) > 0.0015f
					|| fabs(instance.setRotation.w - rot.w) > 0.0015f)
					needsUpdate = true;

				if(needsUpdate)
				{
					if (!instance.movedByPhysics)
					{
						instance.movedByPhysics = true;

						TerrainObstacle obstacle = createObstacle(o.data, instance);
						objectsMovedFirstTime.push_back(obstacle);
					}

					instance.position = VC2(pos.x, pos.z);
					instance.height = pos.y;
					instance.setRotation = rot;

					data->terrain->setInstancePosition(i, j, pos);
					data->terrain->setInstanceRotation(i, j, rot);

//#ifdef PROJECT_CLAW_PROTO
					if (data->useDynamicObstacles)
					{
						data->removeDynamicObstacle(i, j);
						data->addDynamicObstacle(i, j);
					}
//#endif
					if(instance.ambientSound != -1)
					{
						data->ambientSoundManager->setAmbientSoundPosition(instance.ambientSound, pos);
					}

					// Update lighting less often
					if (fabs(instance.position.x - instance.lightUpdatePosition.x) > UPDATE_LIGHT_RANGE || fabs(instance.position.y - instance.lightUpdatePosition.z) > UPDATE_LIGHT_RANGE || fabs(instance.height - instance.lightUpdatePosition.y) > UPDATE_LIGHT_RANGE)
					{
						ui::PointLights lights;
						//lights.ambient = instance.ambient;

						if(data->lightManager && data->gameMap)
						{
							int ox = data->gameMap->scaledToObstacleX(pos.x);
							int oy = data->gameMap->scaledToObstacleY(pos.z);
							if (data->gameMap->isWellInScaledBoundaries(pos.x, pos.z))
							{
								//lights.ambient = data->gameMap->colorMap->getColor(pos.x / data->gameMap->getScaledSizeX() + .5f, pos.z / data->gameMap->getScaledSizeY() + .5f);
								lights.ambient = data->gameMap->colorMap->getUnmultipliedColor(pos.x / data->gameMap->getScaledSizeX() + .5f, pos.z / data->gameMap->getScaledSizeY() + .5f);

								if(data->gameMap->getAreaMap()->isAreaAnyValue(ox, oy, AREAMASK_INBUILDING))
									data->lightManager->getLighting(pos, lights, o.data.radius, false, false, data->terrain->getInstanceModel(i, j)  );
								else
									data->lightManager->getLighting(pos, lights, o.data.radius, false, false, data->terrain->getInstanceModel(i, j) );

								if (useOcclusionCulling)
								{
									//assert(culler->isWellInScaledBoundaries(pos.x, pos.z));
									// this is an unnecessary check... the gameMap's boundary check few lines above should do the trick.
									//if (culler->isWellInScaledBoundaries(pos.x, pos.z))
									//{
										// TODO: could optimize, an calc these directly from the obstacle ox,oy
										// (but we don't have access the obstacle:occlusion ratio..)
										int occx = culler->scaledToOcclusionX(pos.x);
										int occy = culler->scaledToOcclusionY(pos.z);
										if (culler->isVisibleToArea(occx, occy, data->occlusionCameraArea))
											data->terrain->setInstanceOccluded(i, j, false);
										else
											data->terrain->setInstanceOccluded(i, j, true);
									//}
								}
							} else {
								if (useOcclusionCulling)
								{
									// outside map, everything is visible...
									data->terrain->setInstanceOccluded(i, j, false);
								}
							}
						}

						for(int k = 0; k < LIGHT_MAX_AMOUNT; ++k)
						{
							instance.ambient = lights.ambient;
							instance.lightIndices[k] = lights.lightIndices[k];
							//instance.lightPos[k] = lights.lights[k].position;
							//instance.lightRange[k] = lights.lights[k].range;
							//instance.lightColor[k] = lights.lights[k].color;

							data->terrain->setInstanceLight(i, j, k, instance.lightIndices[k], instance.ambient);
						}

						instance.lightUpdatePosition = pos;
					}
				}
			}
		}
	}

	// FIXME: this is a total hack!
	// should not use some position/radius iterator, instead, see the above commented-out code...
	/*
	VC3 foopos = VC3(0,0,0); 
	float fooradius = 1000.0f;
	boost::shared_ptr<IStorm3D_TerrainModelIterator> it = data->terrain->getModelIterator(foopos, fooradius * data->terrainScale);
	for(; !it->end(); it->next())
	{
		int modelId = it->getModelId();
		int objectId = it->getInstanceId();

		Object &object = data->objects[modelId];
		ObjectInstance &instance = object.instances[objectId];

		if (instance.physicsObject != NULL)
		{
			game::BoxPhysicsObject *bp = (game::BoxPhysicsObject *)instance.physicsObject;
			VC3 pos = bp->getPosition();
			QUAT rot = bp->getRotation();

			if (fabs(instance.position.x - pos.x) > 0.01f 
				|| fabs(instance.position.y - pos.z) > 0.01f
				|| fabs(instance.height - pos.y) > 0.01f)
			{
				instance.position = VC2(pos.x, pos.z);
				instance.height = pos.y;

				data->terrain->setInstancePosition(modelId, objectId, pos);
				data->terrain->setInstanceRotation(modelId, objectId, rot);
			}
		}
	}
	*/
}

void Terrain::clearObstacles()
{
	data->obstacleList.clear();
}

void Terrain::BlastTerrainObjects(const VC3 &position3, float radius, std::vector<TerrainObstacle> &removedObjects, float blastHeight)
{
	VC2 position(position3.x, position3.z);

	boost::shared_ptr<IStorm3D_TerrainModelIterator> it = data->terrain->getModelIterator(position3, radius * data->terrainScale);
	for(; !it->end(); )
	{
		const VC3 &objectPosition = it->getPosition();
		VC2 objectPosition2(objectPosition.x, objectPosition.z);

		{
			int modelId = it->getModelId();
			int objectId = it->getInstanceId();

			Object &object = data->objects[modelId];
			ObjectInstance &instance = object.instances[objectId];

			if(!object.data.explosionAnimations.empty())
			{
				QUAT rot;
				rot.MakeFromAngles(0, instance.rotation.y, 0);

				IStorm3D_Model *foo = object.model->GetClone(true, false, true);
				foo->SetNoCollision(true);
				foo->SetPosition(const_cast<VC3 &> (objectPosition));
				foo->SetRotation(rot);
				foo->SetSelfIllumination(it->getColor());

				for(int i = 0; i < LIGHT_MAX_AMOUNT; ++i)
					foo->SetLighting(i, instance.lightIndices[i]);

				foo->BlendToAnimation(0, object.data.explosionAnimations[object.data.nextAnimation].get(), 100, false);
				if(object.data.nextAnimation >= int(object.data.explosionAnimations.size()))
					object.data.nextAnimation = 0;

				data->scene->AddModel(foo);

				if (!instance.movedByPhysics)
				{
					TerrainObstacle obstacle = createObstacle(object.data, instance);
					removedObjects.push_back(obstacle);
				}
			}
			// Static
			else if(object.data.fallType == TERRAIN_OBJECT_FALLTYPE_NONE)
			{
				it->next();
				continue;
			}
			// Tree
			else if(object.data.fallType == TERRAIN_OBJECT_FALLTYPE_SOMEMYSTERIOUSTYPE1)
			{
				IStorm3D_Model *foo = object.model->GetClone(true,true,true);
				foo->SetNoCollision(false);
				foo->SetPosition(const_cast<VC3 &> (objectPosition));
				foo->SetSelfIllumination(it->getColor());
				for(int i = 0; i < LIGHT_MAX_AMOUNT; ++i)
					foo->SetLighting(i, instance.lightIndices[i]);

				data->scene->AddModel(foo);

				bool reverseDirection = false;

				Vector blastPosition;
				blastPosition.x = position.x;
				blastPosition.z = position.y;
				//blastPosition.y = data->terrain->getHeight(position) + instance.heightOffset;
				blastPosition.y = instance.height;

				data->animation.AddModel(foo, float(object.data.height), blastPosition, radius, reverseDirection);

				if (!instance.movedByPhysics)
				{
					TerrainObstacle obstacle = createObstacle(object.data, instance);
					removedObjects.push_back(obstacle);
				}
			}

			it->erase();
		}
	}
}


UnifiedHandle Terrain::findClosestTerrainObjectOfMaterial(const VC3 &position, const char *material, float maxRadius)
{
	// TODO: ...
	// FIXME: this is a non-working implementation!!!
	// just a quick hack that may work in some specific cases
	assert(!"Terrain::findClosestTerrainObjectOfMaterial - FIXME!");

	std::string material_str = std::string(material);

	int closestModel = -1;
	int closestInstance = -1;
	data->terrain->findObject(position, maxRadius * data->terrainScale, closestModel, closestInstance);
	if(closestModel >= 0 && closestInstance >= 0)
	{
		//ObjectInstance &instance = data->objects[closestModel].instances[closestInstance];

		if (data->objects[closestModel].data.material == material_str)
		{
			return this->getUnifiedHandle(closestModel, closestInstance);
		}
	}

	return UNIFIED_HANDLE_NONE;
}


UnifiedHandle Terrain::findClosestContainer(const VC3 &position, float maxRadius)
{
	/*int closestModel = -1;
	int closestInstance = -1;
	data->terrain->findObject(position, maxRadius * data->terrainScale, closestModel, closestInstance);*/
	boost::shared_ptr<IStorm3D_TerrainModelIterator> it = data->terrain->getModelIterator(position, maxRadius);
	for(; !it->end(); it->next())
	{
		int modelId = it->getModelId();
		int instanceId = it->getInstanceId();

		if (data->objects[modelId].data.explosionScript.substr(0, 5) == "item_")
		{
			return this->getUnifiedHandle(modelId, instanceId);
		}
	}

	return UNIFIED_HANDLE_NONE;
}


const char *Terrain::getTerrainObjectIdString(UnifiedHandle unifiedHandle) const
{
	int terrainModelId = 0;
	int terrainInstanceId = 0;

	unifiedHandleToTerrainIds(unifiedHandle, &terrainModelId, &terrainInstanceId);

#ifdef PROJECT_CLAW_PROTO
// HACK: !!!!
((Terrain *)this)->findTerrainObjectByIdString("1");
((Terrain *)this)->findTerrainObjectByIdString("2");
((Terrain *)this)->findTerrainObjectByIdString("3");
((Terrain *)this)->findTerrainObjectByIdString("4");
#endif


	std::string &str = data->objects[terrainModelId].instances[terrainInstanceId].idString;
	if (str.empty())
		return NULL;
	else
		return str.c_str();
}
#ifdef _MSC_VER
#define strcasecmp(a, b) _stricmp((a), (b))
#endif

UnifiedHandle Terrain::findTerrainObjectByIdString(const char *idString)
{
	std::string idStringStr;
	if (idString != NULL)
	{
		idStringStr = idString;
	} else {
		return UNIFIED_HANDLE_NONE;
	}

	int indexcrap = boost::lexical_cast<int> (idStringStr);

	int index = 0;
	switch(indexcrap)
	{
		case 1:
			index = 3;
			break;
		case 2:
			index = 4;
			break;
		case 3:
			index = 1;
			break;
		case 4:
			index = 2;
			break;
	}
	index -= 1;

	if(index < 0)
		return UNIFIED_HANDLE_NONE;

	for(unsigned int i = 0; i < data->objects.size(); ++i)
	{
		Object &object = data->objects[i];

		igiosWarning("check slashes: \"%s\"\n", object.data.fileName.c_str());
		igios_unimplemented();

		if(strcasecmp(object.data.fileName.c_str(), "data\\models\\terrain_objects\\statue\\obelisk_monument\\obelisk_chainholder.s3d") != 0)
			continue;

		if(index >= int(object.instances.size()))
			return UNIFIED_HANDLE_NONE;

		ObjectInstance &instance = object.instances[index];
		if(instance.deleted)
		{
			if(instance.latestReplacementModel > 0 && instance.latestReplacementInstance >= 0)
			{
				ObjectInstance temp = data->objects[instance.latestReplacementModel].instances[instance.latestReplacementInstance];
				if(!temp.deleted)
				{
					instance.idString = idString;
					data->objects[instance.latestReplacementModel].instances[instance.latestReplacementInstance].idString = idString;

					return getUnifiedHandle(instance.latestReplacementModel, instance.latestReplacementInstance);
				}
			}

			return UNIFIED_HANDLE_NONE;
		}

		return getUnifiedHandle(i, index);
	}

	/*
	for(unsigned int i = 0; i < data->objects.size(); ++i)
	{
		Object &object = data->objects[i];

		for(unsigned int j = 0; j < object.instances.size(); ++j)
		{
			ObjectInstance &instance = object.instances[j];
			if (!instance.idString.empty()
				&& instance.idString == idStringStr)
			{
				return getUnifiedHandle(i, j);
			}
		}
	}
	*/

	return UNIFIED_HANDLE_NONE;
}




void Terrain::physicsImpulse(const VC3 &position, const VC3 &velocity, float radius, float factor, bool closestOnly)
{
	if(!closestOnly)
	{
		boost::shared_ptr<IStorm3D_TerrainModelIterator> it = data->terrain->getModelIterator(position, radius);
		for(; !it->end(); it->next())
		{
			int modelId = it->getModelId();
			int instanceId = it->getInstanceId();
			data->physicsImpulse(modelId, instanceId, factor, velocity, position, !closestOnly);
		}
	} else {
		int closestModel = -1;
		int closestInstance = -1;

		data->terrain->findObject(position, radius * data->terrainScale, closestModel, closestInstance);
		if(closestModel >= 0 && closestInstance >= 0)
		{
			data->physicsImpulse(closestModel, closestInstance, factor, velocity, position, !closestOnly);
		}
	}
}

void Terrain::BreakTerrainObject(UnifiedHandle uh, std::vector<TerrainObstacle> &removedObjects, std::vector<ExplosionEvent> &events, int damage, bool only_breaktexture)
{
	int modelId = 0;
	int objectId = 0;
	data->unifiedHandleToTerrainIds(uh, &modelId, &objectId);

	VC2 position = data->objects[modelId].instances[objectId].position;
	VC3 velocity(0,0,10);
	VC3 position3(position.x, data->objects[modelId].instances[objectId].height, position.y);


	if(data->breakObjects(modelId, objectId, damage, removedObjects, events, position, velocity, position3, false, only_breaktexture))
	{
		data->terrain->removeInstance(modelId, objectId); 
	}
}

void Terrain::BreakTerrainObjects(const VC3 &position3, const VC3 &velocity, float radius, std::vector<TerrainObstacle> &removedObjects, std::vector<ExplosionEvent> &events, int damage, bool closestOnly, bool only_breaktexture)
{
	VC2 position(position3.x, position3.z);
	//radius *= data->terrainScale;

	if(!closestOnly)
	{
		//boost::shared_ptr<IStorm3D_TerrainModelIterator> it = data->terrain->getModelIterator(position3, radius * data->terrainScale);
		boost::shared_ptr<IStorm3D_TerrainModelIterator> it = data->terrain->getModelIterator(position3, radius);
		for(; !it->end(); )
		{
			const VC3 &objectPosition = it->getPosition();
			VC2 objectPosition2(objectPosition.x, objectPosition.z);

			{
				int modelId = it->getModelId();
				int instanceId = it->getInstanceId();

				if(damage <= 0)
					damage = 1;

				if(data->breakObjects(modelId, instanceId, damage, removedObjects, events, position, velocity, position3, !closestOnly, only_breaktexture))
					it->erase();
				else
					it->next();
			}
		}
	}
	else
	{
		int closestModel = -1;
		int closestInstance = -1;

		data->terrain->findObject(position3, radius * data->terrainScale, closestModel, closestInstance);
		if(closestModel >= 0 && closestInstance >= 0)
		{
			if(data->breakObjects(closestModel, closestInstance, damage, removedObjects, events, position, velocity, position3, !closestOnly, only_breaktexture))
				data->terrain->removeInstance(closestModel, closestInstance);
		}
	}
}


UnifiedHandle Terrain::getUnifiedHandle(int terrainModelId, int terrainObstacleId) const
{
	return data->getUnifiedHandle(terrainModelId, terrainObstacleId);
}


void Terrain::unifiedHandleToTerrainIds(int unifiedHandle, int *terrainModelIdOut, int *terrainObstacleIdOut) const
{
	data->unifiedHandleToTerrainIds(unifiedHandle, terrainModelIdOut, terrainObstacleIdOut);
}


bool Terrain::hasObjectTypeMetaValue(UnifiedHandle unifiedHandle, const std::string &metaKey)
{
	int terrainModelId = 0;
	int tmp = 0;

	unifiedHandleToTerrainIds(unifiedHandle, &terrainModelId, &tmp);

	return data->hasMetaValue(terrainModelId, metaKey);
}

std::string Terrain::getObjectTypeMetaValue(UnifiedHandle unifiedHandle, const std::string &metaKey)
{
	int terrainModelId = 0;
	int tmp = 0;

	unifiedHandleToTerrainIds(unifiedHandle, &terrainModelId, &tmp);

	return data->getMetaValue(terrainModelId, metaKey);
}

int Terrain::getObjectVariableNumberByName(const std::string &variableName)
{
	for (int i = 0; i < TERRAIN_OBJECT_VARIABLES_AMOUNT; i++)
	{
		if (terrain_object_variable_name[i] == variableName)
			return i;
	}
	LOG_WARNING_W_DEBUG("Terrain::getObjectVariableNumberByName - no terrain object variable of given name.", variableName.c_str());
  return -1;
}

int Terrain::getObjectVariableValue(UnifiedHandle unifiedHandle, int variableNumber)
{
	assert(variableNumber != -1);
	assert(IS_UNIFIED_HANDLE_TERRAIN_OBJECT(unifiedHandle));
	assert(variableNumber >= 0 && variableNumber < TERRAIN_OBJECT_VARIABLES_AMOUNT);

	int modelId = 0;
	int instanceId = 0;

	unifiedHandleToTerrainIds(unifiedHandle, &modelId, &instanceId);

	assert(doesTerrainObjectExist(modelId, instanceId));

	if (data->objects[modelId].instances[instanceId].variableValues == NULL)
	{
		return 0;
	} else {
		return data->objects[modelId].instances[instanceId].variableValues[variableNumber];
	}
}

void Terrain::setObjectVariableValue(UnifiedHandle unifiedHandle, int variableNumber, int value)
{
	assert(variableNumber != -1);
	assert(IS_UNIFIED_HANDLE_TERRAIN_OBJECT(unifiedHandle));
	assert(variableNumber >= 0 && variableNumber < TERRAIN_OBJECT_VARIABLES_AMOUNT);

	int modelId = 0;
	int instanceId = 0;

	unifiedHandleToTerrainIds(unifiedHandle, &modelId, &instanceId);

	assert(doesTerrainObjectExist(modelId, instanceId));

	// init the array only once necessary - setting some non-default (non-zero) variable value...

	assert(modelId < (int)data->objects.size());
	assert(instanceId < (int)data->objects[modelId].instances.size());

	if (data->objects[modelId].instances[instanceId].variableValues == NULL)
	{
		if (value != 0)
		{
			data->objects[modelId].instances[instanceId].variableValues = new int[TERRAIN_OBJECT_VARIABLES_AMOUNT];
			data->objects[modelId].instances[instanceId].variableValues[variableNumber] = value;
		}
	} else {
		data->objects[modelId].instances[instanceId].variableValues[variableNumber] = value;
	}
}

bool Terrain::doesTerrainObjectExist(int terrainModelId, int terrainObstacleId) const
{
	assert(terrainModelId >= 0 && terrainModelId <= UNIFIED_HANDLE_TERRAIN_OBJECT_MODEL_ID_LAST_VALUE);
	assert(terrainObstacleId >= 0 && terrainObstacleId <= UNIFIED_HANDLE_TERRAIN_OBJECT_INSTANCE_ID_LAST_VALUE);

	if (terrainModelId >= (int)data->objects.size())
	{
		// this may be a sign of some nasty code bug or possibly just a script bug
		assert(!"Terrain::doesTerrainObjectExist - given model id is out of range.");
		return false;
	}
	if (terrainObstacleId >= (int)data->objects[terrainModelId].instances.size())
	{
		// this may be a sign of some nasty code bug or possibly just a script bug
		assert(!"Terrain::doesTerrainObjectExist - given instance id is out of range.");
		return false;
	}

	return !data->objects[terrainModelId].instances[terrainObstacleId].deleted;
}


VC3 Terrain::getTerrainObjectPosition(int terrainModelId, int terrainObstacleId) const
{
	VC2 tmp = data->objects[terrainModelId].instances[terrainObstacleId].position;
	float height = data->objects[terrainModelId].instances[terrainObstacleId].height; 
	return VC3(tmp.x, height, tmp.y);
}


QUAT Terrain::getTerrainObjectRotation(int terrainModelId, int terrainObstacleId) const
{
	// FIXME: this may not work correctly for static non-physics objects?
	// (maybe only physics update sets the "setRotation"?)
	return data->objects[terrainModelId].instances[terrainObstacleId].setRotation;
}


VC3 Terrain::getTerrainObjectVelocity(int terrainModelId, int terrainObstacleId) const
{
	if (data->objects[terrainModelId].instances[terrainObstacleId].physicsObject)
	{
		return data->objects[terrainModelId].instances[terrainObstacleId].physicsObject->getVelocity();
	} else {
		return VC3(0, 0, 0);
	}
}

bool Terrain::doesTrackableUnifiedHandleObjectExist(UnifiedHandle unifiedHandle) const
{
	int modelId = 0;
	int instanceId = 0;
	unifiedHandleToTerrainIds(unifiedHandle, &modelId, &instanceId);
	return doesTerrainObjectExist(modelId, instanceId);
}

VC3 Terrain::getTrackableUnifiedHandlePosition(UnifiedHandle unifiedHandle) const
{
	int modelId = 0;
	int instanceId = 0;
	unifiedHandleToTerrainIds(unifiedHandle, &modelId, &instanceId);
	return getTerrainObjectPosition(modelId, instanceId);
}

QUAT Terrain::getTrackableUnifiedHandleRotation(UnifiedHandle unifiedHandle) const
{
	int modelId = 0;
	int instanceId = 0;
	unifiedHandleToTerrainIds(unifiedHandle, &modelId, &instanceId);
	return getTerrainObjectRotation(modelId, instanceId);
}

VC3 Terrain::getTrackableUnifiedHandleVelocity(UnifiedHandle unifiedHandle) const
{
	int modelId = 0;
	int instanceId = 0;
	unifiedHandleToTerrainIds(unifiedHandle, &modelId, &instanceId);
	return getTerrainObjectVelocity(modelId, instanceId);
}

game::tracking::ITrackableUnifiedHandleObjectIterator *Terrain::getTrackableUnifiedHandleObjectsFromArea(const VC3 &position, float radius, TRACKABLE_TYPEID_DATATYPE typeMask)
{
	game::tracking::SimpleTrackableUnifiedHandleObjectIterator *iter = new game::tracking::SimpleTrackableUnifiedHandleObjectIterator();

	boost::shared_ptr<IStorm3D_TerrainModelIterator> it = data->terrain->getModelIterator(position, radius);
	for(; !it->end(); it->next())
	{
		int modelId = it->getModelId();
		int instanceId = it->getInstanceId();

		// TODO: check for burnable flag instead... 
		//if (strcmp(data->objects[modelId].data.material.c_str(), "wood") == 0)
		if ((data->objects[modelId].data.trackableType & typeMask) != 0)
		{
			iter->addEntry(getUnifiedHandle(modelId, instanceId));
		}
	}

	return iter;
}


UnifiedHandle Terrain::getReplacementForUnifiedHandleObject(UnifiedHandle unifiedHandle) const
{
	int modelId = 0;
	int instanceId = 0;

	unifiedHandleToTerrainIds(unifiedHandle, &modelId, &instanceId);
	
	int origModel = data->objects[modelId].instances[instanceId].originalModel;
	int origInstance = data->objects[modelId].instances[instanceId].originalInstance;

	// not broken?
	if (!data->objects[modelId].instances[instanceId].deleted)
	{
		return UNIFIED_HANDLE_NONE;
	}

	int repModelId = data->objects[origModel].instances[origInstance].latestReplacementModel;
	int repInstanceId = data->objects[origModel].instances[origInstance].latestReplacementInstance;	

	// destroyed for good, no replacement?
	if (repModelId == 0 && repInstanceId == 0)
	{
		return UNIFIED_HANDLE_NONE;
	}

	return getUnifiedHandle(repModelId, repInstanceId);
}


int Terrain::getMaterialForObject(int terrainModelId, int terrainObstacleId)
{
	if (terrainModelId < 0)
	{
		Logger::getInstance()->error("Terrain::getMaterialForObject - Negative model id.");
		assert(!"Terrain::getMaterialForObject - Negative model id.");
		return MATERIAL_CONCRETE;
	}
	if (terrainObstacleId < 0)
	{
		Logger::getInstance()->error("Terrain::getMaterialForObject - Negative obstacle id.");
		assert(!"Terrain::getMaterialForObject - Negative obstacle id.");
		return MATERIAL_CONCRETE;
	}

	// TODO: map to int id...
	//if (data->objects[terrainModelId].data.materialId == -1)
	//{
			for (int i = 0; i < MATERIAL_AMOUNT; i++)
			{
				if (strcmp(data->objects[terrainModelId].data.material.c_str(), game::materialName[i]) == 0)
				{
					//data->objects[terrainModelId].data.materialId = i;
					//break;
					return i;
				}
			}
	//}
	// return data->objects[terrainModelId].data.materialId;
	Logger::getInstance()->warning("Terrain::getMaterialForObject - Unknown material name.");
	Logger::getInstance()->debug(data->objects[terrainModelId].data.material.c_str());
	Logger::getInstance()->debug(data->objects[terrainModelId].data.fileName.c_str());
	return MATERIAL_CONCRETE;
}


int Terrain::getLastObjectEffectTime(int terrainModelId, int terrainObstacleId)
{
	if (terrainModelId < 0)
	{
		Logger::getInstance()->error("Terrain::getLastObjectEffectTime - Negative model id.");
		assert(!"Terrain::getLastObjectEffectTime - Negative model id.");
		return 0;
	}
	if (terrainObstacleId < 0)
	{
		Logger::getInstance()->error("Terrain::getLastObjectEffectTime - Negative obstacle id.");
		assert(!"Terrain::getLastObjectEffectTime - Negative obstacle id.");
		return 0;
	}

	return data->objects[terrainModelId].instances[terrainObstacleId].lastEffectTime;
}


void Terrain::setLastObjectEffectTime(int terrainModelId, int terrainObstacleId, int tick)
{
	if (terrainModelId < 0)
	{
		Logger::getInstance()->error("Terrain::setLastObjectEffectTime - Negative model id.");
		assert(!"Terrain::setLastObjectEffectTime - Negative model id.");
		return;
	}
	if (terrainObstacleId < 0)
	{
		Logger::getInstance()->error("Terrain::setLastObjectEffectTime - Negative obstacle id.");
		assert(!"Terrain::setLastObjectEffectTime - Negative obstacle id.");
		return;
	}

	data->objects[terrainModelId].instances[terrainObstacleId].lastEffectTime = tick;
}


void Terrain::BlastHoleCircle(const VC3 &position, const VC3 &direction, float radius, float depth, std::vector<TerrainObstacle> &removedObjects)
{
	VC3 foo = position;
	foo += direction * (radius/2);

	float terrainHeight = depth * 65535 / data->realSize.y;
	if(fabs(terrainHeight) < .20)
		return;

	BlastTerrainObjects(foo, radius, removedObjects, 0);
}

void Terrain::BlastHoleSphere(const VC3 &position, const VC3 &direction, float radius, float depth, std::vector<TerrainObstacle> &removedObjects)
{
	VC3 foo = position;
	foo += direction * (radius/2);

	float terrainHeight = depth * 65535 / data->realSize.y;
	//if(fabs(terrainHeight < .20f)) ???
	if(fabs(terrainHeight) < .20f)
		return;

	BlastTerrainObjects(foo, radius, removedObjects, 0);
}

void Terrain::ForcemapHeight(const Vector2D &position, float radius, bool above, bool below)
{
	data->terrain->forcemapHeight(position, radius, above, below);
}

WORD *Terrain::GetHeightMap()
{
	return data->heightMap.get();
}

WORD *Terrain::GetDoubleHeightMap()
{
	return data->terrain->getCollisionHeightmap();
}

WORD *Terrain::GetForceMap()
{
	return data->forceMap.get();
}

VC2I Terrain::getHeightMapSize() const
{
	return VC2I(data->mapSize.x, data->mapSize.y);
}

VC2 Terrain::getTerrainSize() const
{
	return VC2(data->realSize.x, data->realSize.z);
}

float Terrain::getTerrainHeight() const
{
	return data->realSize.y;
}

float Terrain::getModelScale() const
{
	return 1.f;
}

float Terrain::getCameraRange() const
{
	return data->cameraRange;
}

const VC3 &Terrain::getSunDirection() const
{
	return data->sunDirection;
}

float Terrain::getSunlightAmount() const
{
	float val = (data->sunColor.r + data->sunColor.g + data->sunColor.b) / 3.0f;
	return val;
}

COL Terrain::getAmbient() const
{
	COL ambientFColor(data->ambientColor.r, data->ambientColor.g, data->ambientColor.b);
	ambientFColor /= 255.f;
	return ambientFColor;
}

void Terrain::setAmbient(COL ambientColor)
{
	COL ambientCharColor = ambientColor * 255.f;	
	data->ambientColor = TColor<unsigned char>((unsigned char)ambientCharColor.r, (unsigned char)ambientCharColor.g, (unsigned char)ambientCharColor.b);
	data->scene->SetAmbientLight(ambientColor);
}

void Terrain::setToColorMultiplier(const COL &colorFactor)
{
	data->terrain->getRenderer().setColorValue(IStorm3D_TerrainRenderer::TerrainObjectInsideFactor, colorFactor);

	/*
	for(unsigned int i = 0; i < data->objects.size(); ++i)
	{
		Object &o = data->objects[i];
		for(unsigned int j = 0; j < o.instances.size(); ++j)
		{
			ObjectInstance &oi = o.instances[j];
			if(!oi.inBuilding)
				continue;

			COL ambient = oi.ambient;
			ambient *= colorFactor;

			for(int k = 0; k < 2; ++k)
			{
				COL lightColor = oi.lightColor[k];
				lightColor *= colorFactor;

				data->terrain->setInstanceLight(i, j, k, oi.lightPos[k], lightColor, oi.lightRange[k], ambient);
			}
		}
	}

	data->colorFactor = colorFactor;
	*/
}

void Terrain::setToOutdoorColorMultiplier(const COL &colorFactor, const VC3 &origo, float radius)
{
	data->terrain->getRenderer().setColorValue(IStorm3D_TerrainRenderer::TerrainObjectOutsideFactor, colorFactor);
	/*
	boost::shared_ptr<IStorm3D_TerrainModelIterator> it = data->terrain->getModelIterator(origo, radius);
	for(; !it->end(); it->next())
	{
		int modelId = it->getModelId();
		int instanceId = it->getInstanceId();

		Object &o = data->objects[modelId];
		ObjectInstance &oi = o.instances[instanceId];
		if(oi.inBuilding)
			continue;

		COL ambient = oi.ambient;
		ambient *= colorFactor;

		for(int k = 0; k < 2; ++k)
		{
			COL lightColor = oi.lightColor[k];
			lightColor *= colorFactor;

			data->terrain->setInstanceLight(modelId, instanceId, k, oi.lightPos[k], lightColor, oi.lightRange[k], ambient);
		}
	}
	*/
}

void Terrain::setFogId(const std::string &id)
{
	data->fogApplier.setActiveFog(id);
}

void Terrain::setFogInterpolate(int type)
{
	data->fogApplier.setInterpolate(type);
}

void Terrain::Animate(int time_elapsed)
{
	data->animation.Update(time_elapsed, data->scene, data->terrain);

	float delta = time_elapsed / 1000.f;
	data->fogApplier.setScene(*data->scene);
	data->fogApplier.update(data->scene->GetCamera()->GetPosition().y, delta);
}

bool Terrain::breakObjects(int modelId, int objectId, int damage, std::vector<TerrainObstacle> &removedObjects, std::vector<ExplosionEvent> &events, const VC2 &position, const VC3 &velocity_, const VC3 &explosion_position, bool use_explosion, bool only_breaktexture)
{
	if (data->breakObjects(modelId, objectId, damage, removedObjects, events, position, velocity_, explosion_position, use_explosion, only_breaktexture))
	{
		data->terrain->removeInstance(modelId, objectId);
		return true;
	} else {
		return false;
	}
}

void Terrain::updateOcclusionForAllObjects(util::GridOcclusionCuller *culler, GRIDOCCLUSIONCULLER_DATATYPE cameraArea)
{	
	data->occlusionCameraArea = cameraArea;

	for (int modelId = 0; modelId < (int)data->objects.size(); modelId++)
	{
		Object &object = data->objects[modelId];

		for(unsigned int i = 0; i < object.instances.size(); ++i)
		{
			//ObjectInstance &instance = object.instances[i];
			this->updateOcclusionForObject(culler, modelId, i, cameraArea);
		}
	}

}

void Terrain::updateOcclusionForObject(util::GridOcclusionCuller *culler, int terrainModelId, int terrainObstacleId, GRIDOCCLUSIONCULLER_DATATYPE cameraArea)
{
	ObjectInstance &instance = data->objects[terrainModelId].instances[terrainObstacleId];
	VC2 pos = instance.position;
	if (culler->isWellInScaledBoundaries(pos.x, pos.y))
	{
		int occx = culler->scaledToOcclusionX(pos.x);
		int occy = culler->scaledToOcclusionY(pos.y);
		if (culler->isVisibleToArea(occx, occy, cameraArea))
			data->terrain->setInstanceOccluded(terrainModelId, terrainObstacleId, false);
		else
			data->terrain->setInstanceOccluded(terrainModelId, terrainObstacleId, true);
	} else {
		data->terrain->setInstanceOccluded(terrainModelId, terrainObstacleId, false);
	}
}

void Terrain::calculateLighting()
{
	//ObjectList::iterator it = data->objects.begin();
	//for(; it != data->objects.end(); ++it)
	for(unsigned int i = 0; i < data->objects.size(); ++i)
	{
		//Object &object = *it;
		Object &object = data->objects[i];
		ObjectData &data = object.data;

		for(unsigned int j = 0; j < object.instances.size(); ++j)
		{
			ObjectInstance &instance = object.instances[j];

			ui::PointLights lights;
			if(this->data->lightManager && this->data->gameMap)
			{
				VC3 pos(instance.position.x, instance.height, instance.position.y);

				int ox = this->data->gameMap->scaledToObstacleX(pos.x);
				int oy = this->data->gameMap->scaledToObstacleY(pos.z);
				if(this->data->gameMap->isWellInScaledBoundaries(pos.x, pos.z))
				{
					lights.ambient = this->data->gameMap->colorMap->getUnmultipliedColor(pos.x / this->data->gameMap->getScaledSizeX() + .5f, pos.z / this->data->gameMap->getScaledSizeY() + .5f);

					if(this->data->gameMap->getAreaMap()->isAreaAnyValue(ox, oy, AREAMASK_INBUILDING))
						this->data->lightManager->getLighting(pos, lights, data.radius, false, false, this->data->terrain->getInstanceModel(i, j) );
					else
						this->data->lightManager->getLighting(pos, lights, data.radius, false, false, this->data->terrain->getInstanceModel(i, j) );
				}
			}

			instance.ambient = lights.ambient;
			for(int k = 0; k < LIGHT_MAX_AMOUNT; ++k)
			{
				instance.lightIndices[k] = lights.lightIndices[k];
				//this->data->terrain->setInstanceLight(it - this->data->objects.begin(), j, k, instance.lightIndices[k], instance.ambient);
				this->data->terrain->setInstanceLight(i, j, k, instance.lightIndices[k], instance.ambient);
			}
		}
	}
}

void Terrain::updateLighting(const VC3 &position, float radius)
{
	boost::shared_ptr<IStorm3D_TerrainModelIterator> it = data->terrain->getModelIterator(position, radius);
	for(; !it->end(); it->next())
	{
		int modelId = it->getModelId();
		int instanceId = it->getInstanceId();

		Object &object = data->objects[modelId];
		ObjectData &data = object.data;
		ObjectInstance &instance = object.instances[instanceId];

		ui::PointLights lights;
		if(this->data->lightManager && this->data->gameMap)
		{
			VC3 pos(instance.position.x, instance.height, instance.position.y);

			int ox = this->data->gameMap->scaledToObstacleX(pos.x);
			int oy = this->data->gameMap->scaledToObstacleY(pos.z);
			if(this->data->gameMap->isWellInScaledBoundaries(pos.x, pos.z))
			{
				lights.ambient = this->data->gameMap->colorMap->getUnmultipliedColor(pos.x / this->data->gameMap->getScaledSizeX() + .5f, pos.z / this->data->gameMap->getScaledSizeY() + .5f);

				if(this->data->gameMap->getAreaMap()->isAreaAnyValue(ox, oy, AREAMASK_INBUILDING))
					this->data->lightManager->getLighting(pos, lights, data.radius, false, false, this->data->terrain->getInstanceModel( modelId, instanceId ) );
				else
					this->data->lightManager->getLighting(pos, lights, data.radius, false, false, this->data->terrain->getInstanceModel( modelId, instanceId ) );
			}
		}

		instance.ambient = lights.ambient;
		for(int k = 0; k < LIGHT_MAX_AMOUNT; ++k)
		{
			instance.lightIndices[k] = lights.lightIndices[k];
			//this->data->terrain->setInstanceLight(it - this->data->objects.begin(), j, k, instance.lightIndices[k], instance.ambient);
			this->data->terrain->setInstanceLight(modelId, instanceId, k, instance.lightIndices[k], instance.ambient);
		}
	}
}

UnifiedHandle Terrain::changeObjectTo(UnifiedHandle uh, int newModelId)
{
	VC3 dummypos = VC3(0,0,0);
	QUAT dummyrot = QUAT();
	VC3 dummyvel = VC3(0,0,0);
	UnifiedHandle ret = createTerrainObject(newModelId, uh, dummypos, dummyrot, dummyvel);

	int modelId = 0;
	int objectId = 0;
	int newModelId2 = 0;
	int newInstanceId = 0;
	this->unifiedHandleToTerrainIds(uh, &modelId, &objectId);
	this->unifiedHandleToTerrainIds(ret, &newModelId2, &newInstanceId);
	assert(newModelId2 == newModelId);
	data->updateLatestReplacementInfo(modelId, objectId, newModelId, newInstanceId);

	deleteTerrainObject(uh);

	return ret;
}

int Terrain::getModelIdForFilename(const char *filename)
{
	std::string filename_str = filename;
	for(unsigned int i = 0; i < data->objects.size(); ++i)
	{
		if (data->objects[i].data.fileName == filename_str)
		{
			return i;
		}
	}
	return -1;
}

void Terrain::deleteTerrainObject(UnifiedHandle uh)
{
	data->deleteTerrainObject(uh);
}

UnifiedHandle Terrain::createTerrainObject(int newModelId, UnifiedHandle cloneFrom, const VC3 &position, const QUAT &rotation, const VC3 &velocity)
{
	return data->createTerrainObject(newModelId, cloneFrom, position, rotation, velocity);
}

void Terrain::setUseDynamicObstacles(bool dynamicObstacles)
{
	data->useDynamicObstacles = dynamicObstacles;
}

bool Terrain::doesUseDynamicObstacles()
{
	return data->useDynamicObstacles;
}

void Terrain::setInstanceDamageTexture(UnifiedHandle uh, float damageTextureFadeFactor)
{
	assert(this->doesTrackableUnifiedHandleObjectExist(uh));

	data->setInstanceDamageTexture(uh, damageTextureFadeFactor);
}



