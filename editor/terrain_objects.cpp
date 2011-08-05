// Copyright 2002-2004 Frozenbyte Ltd.

#pragma warning(disable:4103)
#pragma warning(disable:4786)

#include "terrain_objects.h"
#include "terrain_colormap.h"
#include "terrain_lightmap.h"
#include "storm.h"
#include "storm_model_utils.h"
#include "common_dialog.h"
#include "object_settings.h"
#include "exporter.h"
#include "exporter_scene.h"
#include "file_wrapper.h"
#include "exporter_objects.h"
#include "../filesystem/output_stream.h"
#include "../filesystem/input_stream.h"
#include "../filesystem/input_stream_wrapper.h"
#include "../ui/lightmanager.h"
#include "storm_model_utils.h"
#include "ieditor_state.h"
#include "explosion_scripts.h"
#include "editor_object_state.h"
#include "physics_mass.h"
#include "UniqueEditorObjectHandle.h"
#include "UniqueEditorObjectHandleManager.h"

#include <vector>
#include <map>
#include <set>
#include <cassert>
#include <boost/shared_ptr.hpp>
#include <istorm3d.h>
#include <istorm3d_model.h>
#include <istorm3d_mesh.h>
#include <storm3D_obstaclemapdefs.h>
#include <boost/scoped_array.hpp>
#include <boost/lexical_cast.hpp>
#include "collision_model.h"

#include "parser.h"
#include "../filesystem/file_package_manager.h"
#include "../filesystem/input_stream.h"
#include <fstream>

#include <time.h>


using namespace std;

namespace frozenbyte {
namespace editor {

extern std::string mission_id_global;
	
namespace {

	void getStates(IStorm3D_Model &model, EditorObjectState &state)
	{
		boost::scoped_ptr<Iterator<IStorm3D_Model_Object *> > objectIterator(model.ITObject->Begin());
		for(; !objectIterator->IsEnd(); objectIterator->Next())
		{
			IStorm3D_Model_Object *object = objectIterator->GetCurrent();
			if(!object)
				continue;

			std::string name = object->GetName();
			if(name.find("EditorOnly") != name.npos)
			{
				state.setCollision(object);
				object->SetNoCollision(true);
			}
		}
	}

	void setStates(IStorm3D_Model &model, const EditorObjectState &state)
	{
		boost::scoped_ptr<Iterator<IStorm3D_Model_Object *> > objectIterator(model.ITObject->Begin());
		for(; !objectIterator->IsEnd(); objectIterator->Next())
		{
			IStorm3D_Model_Object *object = objectIterator->GetCurrent();
			if(!object)
				continue;

			std::string name = object->GetName();
			if(name.find("EditorOnly") != name.npos)
			{
				object->SetNoCollision(!state.hasCollision(object));
			}
		}
	}

	struct ActiveCollision
	{
		boost::shared_ptr<CollisionModel> collision;
		CollisionData collisionData;
		VC3 position;
		TerrainObject object;

		void set(CollisionData &collisionData_, TerrainObject &terrainObject)
		{
			collisionData = collisionData_;
			object = terrainObject;
		}

		void create(Storm &storm)
		{
			collision = boost::shared_ptr<CollisionModel> (new CollisionModel(collisionData.objectData, COL(1,0,0), collisionData.collisionPosition, object.getRotation(), storm));
			collision->create();
			collision->scale();
		}

		void reset()
		{
			collision.reset();
		}
	};

	struct Object
	{
		VC2 position;
		mutable float terrainHeight;

		VC3 rotation;
		float height;

		COL color;
		COL offset;
		int id;

		float lightMultiplier;
		float pointLightMultiplier;

		UniqueEditorObjectHandle uniqueEditorObjectHandle;

		Object()
		:	terrainHeight(0),
			height(0),
			id(-1),
			lightMultiplier(1.f),
			pointLightMultiplier(1.f)
		{
			//unsigned int internalHandle = time(NULL);
			unsigned int internalHandle = _time32(NULL);

			uniqueEditorObjectHandle = UniqueEditorObjectHandleManager::createNewUniqueHandle(internalHandle);
			uniqueEditorObjectHandle = 0;
		}
	};

	struct TerrainModel
	{
		boost::shared_ptr<IStorm3D_Model> model;
		int terrainId;

		std::vector<Object> objects;
		bool lightmapped;

		TerrainModel()
		:	terrainId(-1)
		{
		}

		explicit TerrainModel(boost::shared_ptr<IStorm3D_Model> m, int terrainId_)
		:	model(m),
			terrainId(terrainId_)
		{
		}
	};

	struct ModelLessSorter: public std::binary_function<bool, std::string, std::string>
	{
		void makeLower(std::string &string) const
		{
			for(unsigned int i = 0; i < string.size(); ++i)
				string[i] = tolower(string[i]);
		}

		bool operator() (const std::string &lhs, const std::string rhs) const
		{
			std::string a = getFileName(lhs);
			std::string b = getFileName(rhs);

			makeLower(a);
			makeLower(b);

			if(a == b && lhs != rhs)
				return lhs < rhs;

			return a < b;
		}
	};
}

struct TerrainObjectsData
{
	Storm &storm;

	typedef std::map<std::string, TerrainModel, ModelLessSorter> ModelContainer;
	ModelContainer models;

	ObjectSettings objectSettings;
	
	typedef std::vector<boost::shared_ptr<CollisionModel> > CollisionList;
	CollisionList collisions;

	ActiveCollision collision;
	std::vector<unsigned short> obstacleMap;

	bool hasTerrainSize;
	VC3 oldTerrainSize;

	IEditorState &state;

	TerrainObjectsData(Storm &storm_, IEditorState &state_)
	:	storm(storm_),
		objectSettings(storm),
		state(state_)
	{
		hasTerrainSize = false;
	}

	float getRadius(const TerrainModel &tm, const Object &object)
	{
		tm.model->SetPosition(getTerrainPosition(object));
		tm.model->SetRotation(getRotation(object.rotation));
		float radius = ui::getRadius(tm.model.get());

		tm.model->SetPosition(VC3());
		tm.model->SetRotation(QUAT());
		return radius;
	}

	void updateLighting(TerrainModel &tm, Object &object, const std::string &fileName_)
	{
		if(!storm.terrain || tm.terrainId == -1 || object.id == -1)
			return;

		const VC2 &position = object.position;
		object.color = state.getColorMap().getColor(position) + state.getLightMap().getColor(position);

		/*
		VC3 lightPos;
		COL lightCol;
		COL ambient;
		float range = 1.f;
		COL tmpCol = object.color + object.offset;
		if(storm.lightManager)
			storm.lightManager->getLighting(tmpCol, getTerrainPosition(object), lightPos, lightCol, range, ambient);

		lightCol *= object.pointLightMultiplier;
		storm.terrain->setInstanceLight(tm.terrainId, object.id, lightPos, lightCol, range, ambient);
		*/

		ui::PointLights lights;
		lights.ambient = object.color + object.offset;
		
		{
			std::string foofoo = fileName_;
			for(unsigned int i = 0; i < foofoo.size(); ++i)
				foofoo[i] = tolower(foofoo[i]);

			if(foofoo.find("blocks") != foofoo.npos)
				lights.ambient = COL();
		}

		
//		tm.model->SetPosition (getTerrainPosition(object));
//		tm.model->SetRotation (getRotation(object.rotation));


		if(storm.lightManager)
			storm.lightManager->getLighting(getTerrainPosition(object), lights, getRadius(tm, object), false, true,
				storm.terrain->getInstanceModel(tm.terrainId, object.id) );

		for(int i = 0; i < LIGHT_MAX_AMOUNT; ++i)
		{
			//lights.lights[i].color *= object.pointLightMultiplier;
			storm.terrain->setInstanceLight(tm.terrainId, object.id, i, lights.lightIndices[i], lights.ambient);
		}
	}

	void updateSun(TerrainModel &tm, Object &object)
	{
		if(!storm.terrain || tm.terrainId == -1 || object.id == -1)
			return;

		VC3 sunDir = state.getSunDirection();
		if(storm.onFloor(object.position))
		{
			object.lightMultiplier = 0.f;
			sunDir = VC3();
		}
		else
			object.lightMultiplier = 1.f;

		storm.terrain->setInstanceSun(tm.terrainId, object.id, sunDir, object.lightMultiplier);
	}

	void updateLightmapping(const std::string &fileName, TerrainModel &tm)
	{
		//if(!storm.terrain)
		//	return;

		tm.lightmapped = true;

		ObjectData &data = objectSettings.getSettings(fileName);
		if(data.fallType > 0)
			tm.lightmapped = false;
		if(!data.explosionObject.empty() && data.explosionObject != "(none)")
			tm.lightmapped = false;
		if(!data.animation.empty() && data.animation != "(empty)")
			tm.lightmapped = false;
		if(data.physicsType > 1)
			tm.lightmapped = false;

		//if(tm.model && tm.model->GetRadius() < 1.f)
		//	tm.lightmapped = false;
	}

	VC3 getTerrainPosition(const Object &object)
	{
		VC3 objectPosition(object.position.x, 0, object.position.y);

		//objectPosition.y = storm.terrain->getHeight(VC2(objectPosition.x, objectPosition.z));
		//object.terrainHeight = storm.getHeight(VC2(objectPosition.x, objectPosition.z));

		//if(fabsf(object.terrainHeight) < 0.01)
			object.terrainHeight = storm.getHeight(VC2(objectPosition.x, objectPosition.z));

		objectPosition.y = object.terrainHeight;
		objectPosition.y += object.height;

		return objectPosition;
	}

	void addTerrain(TerrainModel &tm, Object &object, const ObjectData &objectData, const std::string &fileName)
	{
		if(tm.terrainId == -1)
			tm.terrainId = storm.terrain->addModel(tm.model, tm.model, "", "");

		QUAT rotation = getRotation(object.rotation);
		//rotation.MakeFromAngles(0, object.rotation, 0);

		//bool dynamic = true;
		//if(objectData.fallType == 2)
		//	dynamic = false;

		COL color = object.color + object.offset;
		object.id = storm.terrain->addInstance(tm.terrainId, getTerrainPosition(object), rotation, color);

		updateLighting(tm, object, fileName);
		updateSun(tm, object);

		storm.terrain->setInstanceLightmapped(tm.terrainId, object.id, tm.lightmapped);
	}

	void testCollisions(CollisionData &collisionData, TerrainObject &terrainObject)
	{
		Storm3D_CollisionInfo ci;
		ci.range = 10000000000.f;

		for(ModelContainer::iterator it = models.begin(); it != models.end(); ++it)
		{
			TerrainModel &tm = (*it).second;
			ObjectData &objectData = objectSettings.getSettings((*it).first);

			CollisionVolume collisionVolume(objectData);

			// FIXME: loop all(tm)

			for(unsigned int i = 0; i < tm.objects.size(); ++i)
			{
				Object &object = tm.objects[i];
				VC3 objectPosition = getTerrainPosition(object);

				//if(collisionVolume.testCollision(objectPosition, object.rotation, collisionData, 2.f))
				{
					QUAT r = getRotation(object.rotation);
					//r.MakeFromAngles(0, object.rotation, 0);
					tm.model->SetPosition(objectPosition);
					tm.model->SetRotation(r);
					tm.model->GetMX();

					float range = ci.range;
					tm.model->RayTrace(collisionData.rayOrigin, collisionData.rayDirection, collisionData.rayLength, ci, true);
					if(ci.hit && ci.range < range)
					{
						collisionData.collisionPosition = objectPosition;
						collisionData.objectData = objectData;
						collisionData.hasCollision = true;

						terrainObject.fileName = (*it).first;
						terrainObject.index = i;
						terrainObject.rotation = object.rotation;
						terrainObject.color = object.offset;
						terrainObject.height = object.height;
					}
				}
			}
		}
	}

	void insertTerrainObject(TerrainModel &terrainModel, int index, const ObjectData &objectData, const std::string &fileName)
	{
		addTerrain(terrainModel, terrainModel.objects[index], objectData, fileName);
	}

	void removeTerrainObject(TerrainModel &terrainModel, int index)
	{
		Object &object = terrainModel.objects[index];
		if(object.id >= 0)
			storm.terrain->removeInstance(terrainModel.terrainId, object.id);

		object.id = -1;
	}

	void updateObject(TerrainObject &object, const VC2 &delta)
	{
		ModelContainer::iterator it = models.find(object.fileName);
		if(it == models.end())
			return;

		TerrainModel &terrainModel = (*it).second;
		if(object.index >= int(terrainModel.objects.size()))
			return;

		removeTerrainObject(terrainModel, object.index);
		terrainModel.objects[object.index].position += delta;

		ObjectData &objectData = objectSettings.getSettings(object.fileName);
		insertTerrainObject(terrainModel, object.index, objectData, object.fileName);

		if(object == collision.object)
		{
			collision.collisionData.collisionPosition.x = terrainModel.objects[object.index].position.x;
			collision.collisionData.collisionPosition.z = terrainModel.objects[object.index].position.y;
			collision.create(storm);
		}
	}

	void updateObject(TerrainObject &object, float height)
	{
		ModelContainer::iterator it = models.find(object.fileName);
		if(it == models.end())
			return;

		TerrainModel &terrainModel = (*it).second;
		if(object.index >= int(terrainModel.objects.size()))
			return;

		removeTerrainObject(terrainModel, object.index);
		terrainModel.objects[object.index].height = height;

		ObjectData &objectData = objectSettings.getSettings(object.fileName);
		insertTerrainObject(terrainModel, object.index, objectData, object.fileName);

		if(object == collision.object)
		{
			collision.collisionData.collisionPosition.y = storm.getHeight(terrainModel.objects[object.index].position) + height;
			collision.object.height = height;
			collision.create(storm);

			object.height = height;
		}
	}

	void updateObject(TerrainObject &object, const VC3 &rotation)
	{
		ModelContainer::iterator it = models.find(object.fileName);
		if(it == models.end())
			return;

		TerrainModel &terrainModel = (*it).second;
		if(object.index >= int(terrainModel.objects.size()))
			return;

		removeTerrainObject(terrainModel, object.index);
		//terrainModel.objects[object.index].rotation = storm.unitAligner.getRotation(terrainModel.objects[object.index].rotation, yDelta);
		terrainModel.objects[object.index].rotation = rotation;

		ObjectData &objectData = objectSettings.getSettings(object.fileName);
		insertTerrainObject(terrainModel, object.index, objectData, object.fileName);

		if(object == collision.object)
		{
			collision.object = object;
			collision.create(storm);

			object.rotation = rotation;
		}
	}

	void updateObject(TerrainObject &object, const COL &color)
	{
		ModelContainer::iterator it = models.find(object.fileName);
		if(it == models.end())
			return;

		TerrainModel &terrainModel = (*it).second;
		if(object.index >= int(terrainModel.objects.size()))
			return;

		removeTerrainObject(terrainModel, object.index);
		//terrainModel.objects[object.index].color = color;
		terrainModel.objects[object.index].offset = color;

		ObjectData &objectData = objectSettings.getSettings(object.fileName);
		insertTerrainObject(terrainModel, object.index, objectData, object.fileName);
	}

	void updateObjectLight(TerrainObject &object, float delta)
	{
		ModelContainer::iterator it = models.find(object.fileName);
		if(it == models.end())
			return;

		TerrainModel &terrainModel = (*it).second;
		if(object.index >= int(terrainModel.objects.size()))
			return;

		removeTerrainObject(terrainModel, object.index);

		Object &o = terrainModel.objects[object.index];
		o.pointLightMultiplier += delta;
		if(o.pointLightMultiplier > 1.f)
			o.pointLightMultiplier = 1.f;
		if(o.pointLightMultiplier < 0.f)
			o.pointLightMultiplier = 0.f;

		ObjectData &objectData = objectSettings.getSettings(object.fileName);
		insertTerrainObject(terrainModel, object.index, objectData, object.fileName);
	}

	void removeObject(TerrainObject &object)
	{
		ModelContainer::iterator it = models.find(object.fileName);
		if(it == models.end())
			return;

		TerrainModel &terrainModel = (*it).second;
		if(object.index >= int(terrainModel.objects.size()))
			return;

		removeTerrainObject(terrainModel, object.index);
		terrainModel.objects.erase(terrainModel.objects.begin() + object.index);

		if(object == collision.object)
			collision.reset();
	}

	void removeObjects(const VC3 &position, float radius, const char **filter, int filterAmount, bool invert)
	{
		VC2 position2(position.x, position.z);
		for(ModelContainer::iterator it = models.begin(); it != models.end(); ++it)
		{
			TerrainModel &tm = (*it).second;

			std::vector<Object>::iterator it2 = tm.objects.begin();
			for(; it2 != tm.objects.end(); )
			{
				Object &o = *it2;
				if(o.position.GetRangeTo(position2) < radius
					&& filterImpl(it->first, tm, filter, filterAmount, invert))
				{
					removeTerrainObject(tm, it2 - tm.objects.begin());
					it2 = tm.objects.erase(it2);
				}
				else
					++it2;
			}

			if(tm.objects.empty())
				models.erase(it);
		}

	}

	void nudgeObjects(const VC3 &position, const VC3 &direction, float radius, const char **filter, int filterAmount, bool invert)
	{
		VC2 position2(position.x, position.z);
		for(ModelContainer::iterator it = models.begin(); it != models.end(); ++it)
		{
			TerrainModel &tm = (*it).second;

			std::vector<Object>::iterator it2 = tm.objects.begin();
			for(; it2 != tm.objects.end(); )
			{
				Object &o = *it2;
				if(o.position.GetRangeTo(position2) < radius
					&& filterImpl(it->first, tm, filter, filterAmount, invert))
				{
					int index = it2 - tm.objects.begin();
					Object &object = tm.objects[index];

					if(object.id >= 0)
					{
						VC3 pos = VC3(object.position.x, object.height, object.position.y);
						pos += direction;
						object.position.x = pos.x;
						object.height = pos.y;
						object.position.y = pos.z;
						
						pos.y += storm.terrain->getHeight(VC2(pos.x, pos.z));

						storm.terrain->setInstancePosition(tm.terrainId, object.id, pos);
					}

					it2++;
				}
				else
					++it2;
			}

			if(tm.objects.empty())
				models.erase(it);
		}

	}

	void copyObjects(const VC3 &position, float radius, GroupList::ObjectGroup &group, const char **filter, int filterAmount, bool invert)
	{
		group.original = position;

		VC2 position2(position.x, position.z);
		for(ModelContainer::iterator it = models.begin(); it != models.end(); ++it)
		{
			TerrainModel &tm = it->second;

			std::vector<Object>::iterator it2 = tm.objects.begin();
			for(; it2 != tm.objects.end(); ++it2)
			{
				Object &o = *it2;
				if(o.position.GetRangeTo(position2) < radius)
				{
					if (filterImpl(it->first, tm, filter, filterAmount, invert))
					{
						GroupList::Instance instance;
						instance.model = it->first;
						instance.position.x = o.position.x - position.x;
						instance.position.y = o.height;
						instance.position.z = o.position.y - position.z;
						instance.rotation = o.rotation;
						instance.color = o.offset;

						group.instances.push_back(instance);
					}
				}
			}
		}
	}

	void createObstacles(const TerrainModel &terrainModel, const ObjectData &objectData)
	{
		/*
		if(obstacleMap.empty())
			return;

		int height = int(objectData.height / storm.heightmapSize.y * 0xFFFF / 5);
		height &= OBSTACLE_MAP_MASK_HEIGHT;
	
		int radiusX = int(objectData.radiusX / storm.heightmapSize.x * storm.heightmapResolution.x * 4);
		int radiusZ = int(objectData.radiusZ / storm.heightmapSize.z * storm.heightmapResolution.y * 4);

		for(unsigned int i = 0; i < terrainModel.objects.size(); ++i)
		{
			const VC2 &position = terrainModel.objects[i].position;

			int x = int((position.x + storm.heightmapSize.x / 2) / storm.heightmapSize.x * storm.heightmapResolution.x * 4);
			int y = int((position.y + storm.heightmapSize.z / 2) / storm.heightmapSize.z * storm.heightmapResolution.y * 4);

			for(int j = -radiusZ; j <= radiusZ; ++j)
			for(int k = -radiusX; k <= radiusX; ++k)
				obstacleMap[(y+j) * storm.heightmapResolution.x * 4 + (x+k)] = height;
		}
		*/
	}

	void createObstacles()
	{
		int arraySize = storm.heightmapResolution.x * storm.heightmapResolution.y * 16;
		if(int(obstacleMap.size()) != arraySize)
			obstacleMap.resize(arraySize);

		//boost::scoped_array<unsigned short int> array(new unsigned short int[arraySize]);
		//ZeroMemory(array.get(), arraySize * sizeof(unsigned short int));
		//obstacleMap.swap(array);

		for(ModelContainer::iterator it = models.begin(); it != models.end(); ++it)
		{
			TerrainModel &tm = (*it).second;
			ObjectData &objectData = objectSettings.getSettings((*it).first);

			createObstacles(tm, objectData);
		}
	}

	void scaleObjects()
	{
		if(!hasTerrainSize)
		{
			hasTerrainSize = true;
			oldTerrainSize = storm.heightmapSize;
			return;
		}

		if(fabs(oldTerrainSize.x - storm.heightmapSize.x) < 0.01f)
		if(fabs(oldTerrainSize.z - storm.heightmapSize.z) < 0.01f)
			return;

		VC2 scaleFactor(storm.heightmapSize.x / oldTerrainSize.x, storm.heightmapSize.z / oldTerrainSize.z);
		oldTerrainSize = storm.heightmapSize;

		for(ModelContainer::iterator it = models.begin(); it != models.end(); ++it)
		{
			TerrainModel &tm = (*it).second;

			for(unsigned int i = 0; i < tm.objects.size(); ++i)
				tm.objects[i].position *= scaleFactor;
		}
	}

	void writeStream(filesystem::OutputStream &stream) const
	{
		stream << int(7);
		stream << int(models.size());

		for(ModelContainer::const_iterator it = models.begin(); it != models.end(); ++it)
		{
			stream << (*it).first;
			const TerrainModel &tm = (*it).second;

			stream << int(tm.objects.size());
			for(unsigned int i = 0; i < tm.objects.size(); ++i)
			{
				const Object &object = tm.objects[i];

				stream << object.position.x << object.position.y;
				stream << object.rotation.x << object.rotation.y << object.rotation.z;
				stream << object.color.r << object.color.g << object.color.b;
				stream << object.offset.r << object.offset.g << object.offset.b;
				stream << object.height;
				stream << object.lightMultiplier;
				stream << object.pointLightMultiplier;
				stream << (unsigned int)(object.uniqueEditorObjectHandle & (((UniqueEditorObjectHandle)1<<32)-1));
				stream << (unsigned int)(object.uniqueEditorObjectHandle >> (UniqueEditorObjectHandle)32);
			}
		}
	}

#define HAX_TERRAIN_RENAME

#ifdef HAX_TERRAIN_RENAME
	std::string getTerrainAliasName( std::string filename )
	{
		static bool loaded = false;

		if( loaded == false )
		{
			loadHaxoredFilenames();
			loaded = true;
		}

		if( fileExists("rename_terrainobjects.txt") )
		{
			// std::map< std::string, std::string >::iterator i;
			filename = haxoredFilenames.getValue( filename, filename );
			
		}

		return filename;
	}

	ParserGroup haxoredFilenames;

	void loadHaxoredFilenames()
	{
		filesystem::InputStream stream = filesystem::FilePackageManager::getInstance().getFile( "rename_terrainobjects.txt" );
		// haxoredFilenames.parseGroup( stream );
		stream >> haxoredFilenames;

		std::string outputFileName = "missing_terrainobjects_";
		outputFileName += mission_id_global;
		outputFileName += ".txt";

		//std::fstream files( "missing_terrainobjects.txt", std::ios::out );
		std::fstream files( outputFileName.c_str(), std::ios::out );
		files << "// Missing terrain objects, rename this file as rename_terrainobjects.txt to fix these" << std::endl;
	
	}

#endif

	void readStream(filesystem::InputStream &stream)
	{
		int missingObjectAmount = 0;

		int version = 0;
		stream >> version;

		int size = 0;
		stream >> size;

		int overallCount = 0;
		for(int i = 0; i < size; ++i)
		{
			std::string fileName;
			stream >> fileName;

			// the old terrain object rename screwed up big time if the target filename already existed
			// that meant, that renaming an object to another object that exists would cause either the
			// existing models to be lost or the models to be renamed to be lost.
			// now, attempting to manage such situations appropriately. --jpk
			bool filenameChanged = false;
			bool appendToExistingModel = false;

#ifdef HAX_TERRAIN_RENAME
			std::string originalFilename = fileName;
			fileName = getTerrainAliasName( fileName );
			if (fileName != originalFilename)
			{
				filenameChanged = true;
				ModelContainer::iterator it = models.find(fileName);
				if(it != models.end())
				{
//					appendToExistingModel = true;
				}
			}
#endif

			std::string realFileName = fileName;
			std::string fileModifier = "";
			for (int spos = 0; spos < (int)fileName.length(); spos++)
			{
				if (realFileName[spos] == '@')
				{
					fileModifier = realFileName.substr(spos, realFileName.length() - spos);
					realFileName = realFileName.substr(0, spos);
					break;
				}
			}

#ifdef LEGACY_FILES
			if(!fileExists(realFileName))
			{
				realFileName = FileWrapper::resolveModelName("Data\\Models\\Terrain_objects", realFileName);
				fileName = realFileName + fileModifier;
			}
#else
			if(!fileExists(realFileName))
			{
				realFileName = FileWrapper::resolveModelName("data\\model\\object", realFileName);
				fileName = realFileName + fileModifier;
			}
#endif

			if(!fileExists(realFileName))
			{
				std::string outputFileName = "missing_terrainobjects_";
				outputFileName += mission_id_global;
				outputFileName += ".txt";

				//std::fstream files( "missing_terrainobjects.txt", std::ios::out | std::ios::app );
				std::fstream files( outputFileName.c_str(), std::ios::out | std::ios::app );
#ifdef LEGACY_FILES
				files <<  fileName << " = " << "missing.s3d"  << std::endl;
#else
				files <<  fileName << " = " << "data\\model\\object\\missing.s3d"  << std::endl;
#endif

				missingObjectAmount++;
			}

			int positionCount = 0;
			stream >> positionCount;

			ObjectData &objectDataTmp = objectSettings.getSettings(fileName);
			std::string loadFileName = fileName;
			std::string postfix = objectDataTmp.metaValues["filename_postfix"];
			if(!postfix.empty())
			{
				loadFileName += postfix;
			}

			boost::shared_ptr<IStorm3D_Model> m = createEditorModel(*storm.storm, loadFileName);
			m->FreeMemoryResources();

			int id = -1;
			TerrainModel tmNew(m, id);

			TerrainModel *tm = NULL;
			if (filenameChanged && appendToExistingModel)
			{
				tm = &models[fileName];
			} else {
				tmNew.objects.resize(positionCount);
				tm = &tmNew;
			}

			overallCount += positionCount;

			for(int j = 0; j < positionCount; ++j)
			{
				Object newObject;
				Object *object = &newObject;
				if (filenameChanged && appendToExistingModel)
				{
					// use the new object.
				} else {
					object = &tm->objects[j];
				}

				stream >> object->position.x >> object->position.y;
				if(version < 5)
					stream >> object->rotation.y;
				else
					stream >> object->rotation.x >> object->rotation.y >> object->rotation.z;

				if(version >= 1)
				{
					COL c;
					stream >> c.r >> c.g >> c.b;
					//stream >> object.color.r >> object.color.g >> object.color.b;
				}

				if(version >= 3)
				{
					COL c;
					stream >> c.r >> c.g >> c.b;
					object->offset = c;
				}

				if(version >= 2)
					stream >> object->height;

				if(version >= 4)
					stream >> object->lightMultiplier;
				else
					object->lightMultiplier = 1.f;

				if(version >= 6)
					stream >> object->pointLightMultiplier;
				else
					object->pointLightMultiplier = 1.f;

				if(version >= 7)
				{
					unsigned int lower;
					unsigned int upper;
					stream >> lower;
					stream >> upper;
					object->uniqueEditorObjectHandle = 0;
					object->uniqueEditorObjectHandle |= (UniqueEditorObjectHandle)lower;
					object->uniqueEditorObjectHandle |= ((UniqueEditorObjectHandle)upper << 32);
				} else {
					//unsigned int internalHandle = time(NULL);
					unsigned int internalHandle = _time32(NULL);
					object->uniqueEditorObjectHandle = UniqueEditorObjectHandleManager::createNewUniqueHandle(internalHandle);
				}

				if (filenameChanged && appendToExistingModel)
				{
					tm->objects.push_back(*object);
				} else {
					// do nothing.
				}
			}

			if (filenameChanged && appendToExistingModel)
			{
				// do nothing.
			} else {
				if(positionCount > 0)
					models[fileName] = *tm;
			}
		}

		// HACK: 
		// if no terrain objects were missing, get rid of the useless file... --jpk
		std::string delFileName = "missing_terrainobjects_";
		delFileName += mission_id_global;
		delFileName += ".txt";
		/*
		filesystem::FB_FILE *fp = filesystem::fb_fopen(delFileName.c_str(), "rb");
		if(fp)
		{
			int sizeis = fb_fsize(fp);
			fb_fclose(fp);
			if (sizeis == 88)
			{
				remove(delFileName.c_str());
			}
		}
		*/
		if (missingObjectAmount == 0)
		{
			remove(delFileName.c_str());
		}
		// --- end of hack ---

//std::string msg = std::string("Objects: ") + boost::lexical_cast<std::string> (overallCount);
//MessageBox(0, msg.c_str(), "...", MB_OK);

	}

	void exportModel(ExporterObjects &exporterObjects, const std::string &fileName, const TerrainModel &tm)
	{
		std::string realFileName = fileName;
		for (int i = 0; i < (int)realFileName.length(); i++)
		{
			if (realFileName[i] == '@')
			{
				realFileName = realFileName.substr(0, i);
				break;
			}
		}
		filesystem::FB_FILE *fp = filesystem::fb_fopen(realFileName.c_str(), "rb");
		if(!fp)
			return;
		filesystem::fb_fclose(fp);

		ObjectData &objectData = objectSettings.getSettings(fileName);

		ExporterObjects::CollisionType type = ExporterObjects::CollisionNone;
		if(objectData.type == 1)
			type = ExporterObjects::CollisionCylinder;
		if(objectData.type == 2)
			type = ExporterObjects::CollisionBox;
		if(objectData.type == 3)
			type = ExporterObjects::CollisionMapped;

		ExporterObjects::FallType fallType = ExporterObjects::FallStatic;
		if(objectData.fallType == 1)
			fallType = ExporterObjects::FallTree;
		if(objectData.fallType == 2)
			fallType = ExporterObjects::FallPlant;

//boost::scoped_ptr<IStorm3D_Model> m(storm.storm->CreateNewModel());
//m->LoadS3D(fileName.c_str());

		float radius = max(objectData.radiusX, objectData.radiusZ);
//float radius = m->GetRadius();

		ExplosionScripts explosionScripts;
		explosionScripts.reload();

		string script = explosionScripts.findScript(objectData.explosionScript);
		string projectile = explosionScripts.findProjectile(objectData.explosionProjectile);
		string effect = explosionScripts.findEffect(objectData.explosionEffect);
		vector<string> sounds = explosionScripts.findSounds(objectData.explosionSound);
		string material = explosionScripts.findMaterial(objectData.material);
		const Animation &animation = explosionScripts.findAnimation(objectData.animation);
		VC3 sunDir = state.getSunDirection();

		// ToDo
		VC3 physicsData1;
		VC3 physicsData2;
		if(objectData.physicsType > 1)
		{
			boost::scoped_ptr<IStorm3D_Model> m(storm.storm->CreateNewModel());

			std::string modelFileName = fileName;
			std::string postfix = objectData.metaValues["filename_postfix"];
			if(!postfix.empty())
			{
				modelFileName += postfix;
			}
			m->LoadS3D(modelFileName.c_str());

			m->GetBoundingBox();

			AABB aabb = m->GetBoundingBox();
			VC3 size = aabb.mmax - aabb.mmin;

			if(objectData.physicsType == 2) // box
			{
				physicsData1 = size;
				physicsData1 *= 0.5f;
			}
			else if(objectData.physicsType == 3) // cylinder
			{
#ifdef PROJECT_AOV
				physicsData1.x = size.z;
				physicsData1.y = size.x * 0.5f;
#else
				physicsData1.x = size.y;
				physicsData1.y = size.x * 0.5f;
#endif
			}
			else if(objectData.physicsType == 4) // capsule
			{
				physicsData1.x = size.y - size.x;
				physicsData1.y = size.x * 0.5f;
			}
		}

		PhysicsMass physicsMass;
		float mass = physicsMass.getMass(objectData.physicsWeight);

		int id = exporterObjects.addTerrainObject(fileName, type, fallType, objectData.height, radius, objectData.fireThrough, objectData.explosionObject, script, projectile, effect, sounds, material, objectData.hitpoints, animation, objectData.breakTexture, objectData.physicsType, mass, objectData.physicsMaterial, physicsData1, physicsData2, objectData.durabilityType, objectData.metaValues);
		for(unsigned int i = 0; i < tm.objects.size(); ++i)
		{
			const Object &object = tm.objects[i];
			const VC2 &position = object.position;

			//COL color = state.getColorMap().getColor(position) + state.getLightMap().getColor(position) + object.offset;
			//VC3 lightPos;
			//COL lightCol;
			//COL ambient;
			//float range = 1.f;
			//if(storm.lightManager)
			//	storm.lightManager->getLighting(color, getTerrainPosition(object), lightPos, lightCol, range, ambient);
			//lightCol *= object.pointLightMultiplier;

			ui::PointLights lights;
			lights.ambient = state.getColorMap().getColor(position) + state.getLightMap().getColor(position) + object.offset;
			//if(storm.lightManager)
			//	storm.lightManager->getLighting(getTerrainPosition(object), lights, getRadius(tm, object), false);

			//lights.lights[0].color *= object.pointLightMultiplier;
			//lights.lights[1].color *= object.pointLightMultiplier;

			bool inBuilding = false;
			if(storm.onFloor(object.position))
				inBuilding = true;

			VC3 pos(object.position.x, object.terrainHeight, object.position.y);
			exporterObjects.addObject(id, pos, object.rotation, lights.ambient, object.height, lights.lightIndices, tm.lightmapped, inBuilding, sunDir * object.lightMultiplier, object.lightMultiplier);
		}
	}

	bool TerrainObjectsData::filterImpl(const std::string &fileName, TerrainModel &tm, const char **filters, int filterAmount, bool invert)
	{
		bool filtOk = false;
		if (filters != NULL)
		{
			if (tm.objects.size() > 0)
			{
				for (int i = 0; i < filterAmount; i++)
				{
					assert(filters[i] != NULL);
					if (filters[i][0] == '*')
					{

// oh yeah... ffs...
#define OBJECT_TYPE_NOT_SET -1
#define OBJECT_TYPE_NO_COLLISION 0
#define OBJECT_TYPE_MAPPED_COLLISION 3

#define OBJECT_PHYSICS_TYPE_NO_COLLISION 0
#define OBJECT_PHYSICS_TYPE_STATIC 1

						ObjectSettings &os = this->objectSettings;
						ObjectData &od = os.getSettings(fileName);
						/*
						if (strcmp(&filters[i][1], "INCOMPLETE") == 0)
						{
							if (od.type == OBJECT_TYPE_NOT_SET)
							{
								filtOk = true;
								break;
							}
#ifdef PROJECT_AOV
							if (od.type == OBJECT_TYPE_MAPPED_COLLISION
								&& od.physicsType != OBJECT_PHYSICS_TYPE_STATIC)
							{
								filtOk = true;
								break;
							}
#endif
							if (od.type == OBJECT_TYPE_MAPPED_COLLISION
								&& od.physicsType != OBJECT_PHYSICS_TYPE_STATIC)
							{
								filtOk = true;
								break;
							}
							if (od.material.empty())
							{
								filtOk = true;
								break;
							}
						}
						*/
						if (strcmp(&filters[i][1], "STATIC") == 0)
						{
							if (od.physicsType == OBJECT_PHYSICS_TYPE_STATIC)
							{
								if (!od.material.empty()
									&& od.material != "(empty)")
								{
#ifdef PROJECT_AOV
									if (od.type == OBJECT_TYPE_MAPPED_COLLISION
										|| od.type == OBJECT_TYPE_NO_COLLISION)
									{
										filtOk = true;
										break;
									}
#else
									if (od.type != OBJECT_TYPE_NOT_SET)
									{
										filtOk = true;
										break;
									}
#endif
								}
							}
						}
						else if (strcmp(&filters[i][1], "DYNAMIC") == 0)
						{
							if (od.physicsType != OBJECT_PHYSICS_TYPE_STATIC
								&& od.physicsType != OBJECT_PHYSICS_TYPE_NO_COLLISION)
							{
								if (!od.material.empty()
									&& od.material != "(empty)")
								{
#ifdef PROJECT_AOV
									if (od.type == OBJECT_TYPE_NO_COLLISION)
									{
										filtOk = true;
										break;
									}
#else
									if (od.type != OBJECT_TYPE_NOT_SET)
									{
										filtOk = true;
										break;
									}
#endif
								}
							}
						}
						else if (strcmp(&filters[i][1], "NOCOLLISION") == 0)
						{
							if (od.physicsType == OBJECT_PHYSICS_TYPE_NO_COLLISION)
							{
								if (!od.material.empty()
									&& od.material != "(empty)")
								{
#ifdef PROJECT_AOV
									if (od.type == OBJECT_TYPE_NO_COLLISION)
									{
										filtOk = true;
										break;
									}
#else
									if (od.type != OBJECT_TYPE_NOT_SET)
									{
										filtOk = true;
										break;
									}
#endif
								}
								filtOk = true;
								break;
							}
						} else {
							assert(!"Unsupported terrain object filter string.");
						}
					} else {
						if (strstr(fileName.c_str(), filters[i]) != NULL)
						{
							filtOk = true;
							break;
						}
					}
				}
			}
			if (invert) filtOk = !filtOk;
		} else {
			filtOk = true;
		}

		return filtOk;
	}


};

TerrainObjects::TerrainObjects(Storm &storm, IEditorState &state)
{
	boost::scoped_ptr<TerrainObjectsData> tempData(new TerrainObjectsData(storm, state));
	data.swap(tempData);
}

TerrainObjects::~TerrainObjects()
{
}

void TerrainObjects::clear()
{
	data->models.clear();
	data->objectSettings.saveSettings();
	data->collision.reset();

	data->hasTerrainSize = false;
}

void TerrainObjects::resetTerrain()
{
	data->storm.terrain->removeModels();
	data->collision.reset();

	for(TerrainObjectsData::ModelContainer::iterator it = data->models.begin(); it != data->models.end(); ++it)
	{
		TerrainModel &tm = (*it).second;
		tm.terrainId = -1;

		for(unsigned int i = 0; i < tm.objects.size(); ++i)
			tm.objects[i].terrainHeight = 0;
	}
}

void TerrainObjects::setToTerrain()
{
	data->scaleObjects();
	data->createObstacles();

//	data->storm.terrain->SetObstacleHeightmap(&data->obstacleMap[0]);
//	data->storm.terrain->RecreateCollisionMap();
//	data->storm.terrain->RegenerateTexturing();

	if(!data->storm.terrain)
		return;

	data->storm.terrain->removeInstances();
	updateColors();

	for(TerrainObjectsData::ModelContainer::iterator it = data->models.begin(); it != data->models.end(); ++it)
	{
		TerrainModel &tm = (*it).second;
		if(tm.terrainId == -1)
			tm.terrainId = data->storm.terrain->addModel(tm.model, tm.model, "", "");

		ObjectData &objectData = data->objectSettings.getSettings(it->first);
		for(unsigned int i = 0; i < tm.objects.size(); ++i)
		{
			data->addTerrain(tm, tm.objects[i], objectData, it->first);
		}
	}
}

TerrainObject TerrainObjects::addObject(const std::string &fileName, const VC2 &position, const VC3 &rotation, float height)
{
	TerrainObjectsData::ModelContainer::iterator it = data->models.find(fileName);
	if(it == data->models.end())
	{
		ObjectData &objectDataTmp = data->objectSettings.getSettings(fileName);
		std::string loadFileName = fileName;
		std::string postfix = objectDataTmp.metaValues["filename_postfix"];
		if(!postfix.empty())
		{
			loadFileName += postfix;
		}

		boost::shared_ptr<IStorm3D_Model> m = createEditorModel(*data->storm.storm, loadFileName);
		m->FreeMemoryResources();
		data->models[fileName] = TerrainModel(m, -1);
		//setToTerrain();

		TerrainModel &tm = data->models[fileName];
		data->updateLightmapping(fileName, tm);

		tm.terrainId = data->storm.terrain->addModel(tm.model, tm.model, "", "");
	}

	ObjectData &objectData = data->objectSettings.getSettings(fileName);
	TerrainModel &terrainModel = data->models[fileName];

	Object object;
	object.position = position;
	object.rotation = rotation;
	object.height = height;

	data->addTerrain(terrainModel, object, objectData, fileName);
	terrainModel.objects.push_back(object);

	TerrainObject result;
	result.fileName = fileName;
	result.index = terrainModel.objects.size() - 1;
	result.rotation = rotation;
	result.height = height;

	return result;
}

void TerrainObjects::drawCollision(bool drawState, const std::string &fileName)
{
	if(!drawState)
	{
		data->collisions.clear();
		return;
	}

	if(!data->collisions.empty())
		data->collisions.clear();

	const std::string modelName = fileName;
	TerrainObjectsData::ModelContainer::iterator it = data->models.find(modelName);
	if(it == data->models.end())
		return;

	TerrainModel &tm = (*it).second;
	ObjectData &objectData = data->objectSettings.getSettings(modelName);

	for(unsigned int i = 0; i < tm.objects.size(); ++i)
	{
		Object &object = tm.objects[i];

		VC3 position = data->getTerrainPosition(object);
		boost::shared_ptr<CollisionModel> collision(new CollisionModel(objectData, COL(.5,0,0), position, object.rotation, data->storm));

		if(i == 0)
			collision->create();
		else
			collision->clone(*data->collisions[0]);

		data->collisions.push_back(collision);
	}
}

ObjectSettings &TerrainObjects::getObjectSettings()
{
	return data->objectSettings;
}

TerrainObject TerrainObjects::traceActiveCollision(const VC3 &rayOrigin, const VC3 &rayDirection, float rayLength)
{
	CollisionData collisionData;
	collisionData.rayOrigin = rayOrigin;
	collisionData.rayDirection = rayDirection;
	collisionData.rayLength = rayLength;

	TerrainObject terrainObject;
	data->testCollisions(collisionData, terrainObject);

	if(data->collision.collision)
		data->collision.reset();

	if(!collisionData.hasCollision)
		return TerrainObject();

	data->collision.set(collisionData, terrainObject);
	data->collision.create(data->storm);
	return terrainObject;
}

void TerrainObjects::moveObject(TerrainObject &terrainObject, const VC2 &delta)
{
	data->updateObject(terrainObject, delta);
}

void TerrainObjects::moveObject(TerrainObject &terrainObject, float height)
{
	data->updateObject(terrainObject, height);
}

void TerrainObjects::rotateObject(TerrainObject &terrainObject, const VC3 &rotation)
{
	data->updateObject(terrainObject, rotation);
}

void TerrainObjects::setColor(TerrainObject &terrainObject, int color)
{
	unsigned char r = GetRValue(color);
	unsigned char g = GetGValue(color);
	unsigned char b = GetBValue(color);

	COL result(r / 255.f, g / 255.f, b / 255.f);
	data->updateObject(terrainObject, result);
}

void TerrainObjects::setLightMultiplier(TerrainObject &terrainObject, float delta)
{
	data->updateObjectLight(terrainObject, delta);
}

void TerrainObjects::removeObject(TerrainObject &terrainObject)
{
	data->removeObject(terrainObject);
}

void TerrainObjects::nudgeObjects(const VC3 &position, const VC3 &direction, float radius, const char **filter, int filterAmount, bool invert)
{
	data->nudgeObjects(position, direction, radius, filter, filterAmount, invert);
}

void TerrainObjects::removeObjects(const VC3 &position, float radius, const char **filter, int filterAmount, bool invert)
{
	data->removeObjects(position, radius, filter, filterAmount, invert);
}

void TerrainObjects::copyObjects(const VC3 &position, float radius, GroupList::ObjectGroup &group, const char **filter, int filterAmount, bool invert)
{
	data->copyObjects(position, radius, group, filter, filterAmount, invert);
}

void TerrainObjects::hideObjects()
{
	data->storm.terrain->removeModels();
	
	for(unsigned int i = 0; i < data->collisions.size(); ++i)
		data->collisions[i]->hide();
}

void TerrainObjects::showObjects()
{
	setToTerrain();

	for(unsigned int i = 0; i < data->collisions.size(); ++i)
		data->collisions[i]->show();
}

void TerrainObjects::hideObjectsImpl(const char **filters, int filterAmount, bool invert)
{
	for(TerrainObjectsData::ModelContainer::iterator it = data->models.begin(); it != data->models.end(); ++it)
	{
		TerrainModel &tm = (*it).second;
		int modelId = tm.terrainId;

		bool filtOk = data->filterImpl((*it).first, tm, filters, filterAmount, invert);

		if (filtOk)
		{
			for(unsigned int i = 0; i < tm.objects.size(); ++i)
			{
				Object &o = tm.objects[i];
				data->storm.terrain->setInstanceOccluded(tm.terrainId, o.id, true);
			}
		}
	}
}

void TerrainObjects::showObjectsImpl(const char **filters, int filterAmount, bool invert)
{
	for(TerrainObjectsData::ModelContainer::iterator it = data->models.begin(); it != data->models.end(); ++it)
	{
		TerrainModel &tm = (*it).second;
		int modelId = tm.terrainId;

		bool filtOk = data->filterImpl((*it).first, tm, filters, filterAmount, invert);

		if (filtOk)
		{
			for(unsigned int i = 0; i < tm.objects.size(); ++i)
			{
				Object &o = tm.objects[i];
				data->storm.terrain->setInstanceOccluded(tm.terrainId, o.id, false);
			}
		}
	}
}

void TerrainObjects::updateColors()
{
	for(TerrainObjectsData::ModelContainer::iterator it = data->models.begin(); it != data->models.end(); ++it)
	{
		TerrainModel &tm = (*it).second;
		for(unsigned int i = 0; i < tm.objects.size(); ++i)
		{
			Object &o = tm.objects[i];
			data->updateLighting(tm, o, it->first);
			data->updateSun(tm, o);
		}
	}
}

void TerrainObjects::updateLightmapStates()
{
	for(TerrainObjectsData::ModelContainer::iterator it = data->models.begin(); it != data->models.end(); ++it)
	{
		TerrainModel &tm = it->second;
		data->updateLightmapping(it->first, tm);

		if(data->storm.terrain)
		{
			for(unsigned int i = 0; i < tm.objects.size(); ++i)
			{
				Object &o = tm.objects[i];
				data->storm.terrain->setInstanceLightmapped(tm.terrainId, o.id, tm.lightmapped);
			}
		}
	}
}

void TerrainObjects::updateLighting()
{
	updateColors();
	setToTerrain();
}

void TerrainObjects::getEditorObjectStates(EditorObjectState &states)
{
	for(TerrainObjectsData::ModelContainer::iterator it = data->models.begin(); it != data->models.end(); ++it)
	{
		TerrainModel &tm = it->second;
		if(!tm.model)
			continue;

		getStates(*tm.model, states);
	}

	setToTerrain();
}

void TerrainObjects::setEditorObjectStates(const EditorObjectState &states)
{
	for(TerrainObjectsData::ModelContainer::iterator it = data->models.begin(); it != data->models.end(); ++it)
	{
		TerrainModel &tm = it->second;
		if(!tm.model)
			continue;

		setStates(*tm.model, states);
	}

	setToTerrain();
}

void TerrainObjects::doExport(Exporter &exporter) const
{
	exporter.getScene().setObstaclemap(data->obstacleMap);
	ExporterObjects &exporterObjects = exporter.getObjects();

	std::set<std::string> exportedModels;
	std::set<std::string> models;

	for(TerrainObjectsData::ModelContainer::const_iterator it = data->models.begin(); it != data->models.end(); ++it)
	{
		const TerrainModel &tm = it->second;
		if(!tm.objects.empty())
		{
			exportedModels.insert(it->first);
			data->exportModel(exporterObjects, it->first, it->second);
		}

		ObjectData &metadata = data->objectSettings.getSettings(it->first);
		std::string metaModel = metadata.metaValues["custom_break_object"];
		if(!metaModel.empty())
			models.insert(metaModel);

		/*
		// Ensure export for models which have no instances (explosion objects etc)
		{
			ObjectData &objectData = data->objectSettings.getSettings((*it).first);
			
			const std::string &explosion = objectData.explosionObject;
			if(explosion.empty() || explosion == "(disappear)")
				continue;

			TerrainObjectsData::ModelContainer::const_iterator i = data->models.find(explosion);
			if(i == data->models.end())
			{
				if(models.find(explosion) == models.end())
					models.insert(explosion);
				
				continue;
			}

			const TerrainModel &tm = it->second;
			if(!tm.objects.empty())
				continue;

			if(models.find(explosion) == models.end())
				models.insert(explosion);
		}
		*/

		// We need to loop all childs in hierarchy!

		std::string name = it->first;
		int loopIndex = 0;
		while(!name.empty())
		{
			if(++loopIndex > 100)
				break;

			ObjectData &objectData = data->objectSettings.getSettings(name);
			const std::string &explosion = objectData.explosionObject;
			if(explosion.empty() || explosion == "(disappear)")
				break;

			TerrainObjectsData::ModelContainer::const_iterator i = data->models.find(explosion);
			if(i != data->models.end())
			{
				const TerrainModel &tm = i->second;
				if(!tm.objects.empty())
					break;
			}

			if(models.find(explosion) == models.end())
				models.insert(explosion);
			name = explosion;

			/*
			TerrainObjectsData::ModelContainer::const_iterator i = data->models.find(explosion);
			if(i == data->models.end())
			{
				if(models.find(explosion) == models.end())
					models.insert(explosion);
				
				break;
			}

			const TerrainModel &tm = it->second;
			if(!tm.objects.empty())
				break;
			if(models.find(explosion) == models.end())
				models.insert(explosion);

			name = explosion;
			*/
		}
	}

	for(std::set<std::string>::iterator i = models.begin(); i != models.end(); ++i)
	{
		std::string model = *i;
		if(exportedModels.find(model) == exportedModels.end())
			data->exportModel(exporterObjects, *i, TerrainModel());
	}
}

filesystem::OutputStream &TerrainObjects::writeStream(filesystem::OutputStream &stream) const
{
	data->writeStream(stream);
	return stream;
}

filesystem::InputStream &TerrainObjects::readStream(filesystem::InputStream &stream)
{
	data->readStream(stream);
	updateColors();
	updateLightmapStates();

	return stream;
}

void TerrainObjects::hideHelperObjects()
{
	const char *filters[1] = { "helper\\" };
	hideObjectsImpl(filters, 1, false);
}

void TerrainObjects::showHelperObjects()
{
	const char *filters[1] = { "helper\\" };
	showObjectsImpl(filters, 1, false);
}

void TerrainObjects::hideIncompleteObjects()
{
	//const char *filters[1] = { "*INCOMPLETE" };
	//hideObjectsImpl(filters, 1, false);

	const char *filters[4] = { "helper\\", "*STATIC", "*DYNAMIC", "*NOCOLLISION" };
	hideObjectsImpl(filters, 4, true);
}

void TerrainObjects::showIncompleteObjects()
{
	//const char *filters[1] = { "*INCOMPLETE" };
	//showObjectsImpl(filters, 1, false);

	const char *filters[4] = { "helper\\", "*STATIC", "*DYNAMIC", "*NOCOLLISION" };
	showObjectsImpl(filters, 4, true);
}

void TerrainObjects::hideStaticObjects()
{
	const char *filters[1] = { "*STATIC" };
	hideObjectsImpl(filters, 1, false);
}

void TerrainObjects::showStaticObjects()
{
	const char *filters[1] = { "*STATIC" };
	showObjectsImpl(filters, 1, false);
}

void TerrainObjects::hideDynamicObjects()
{
	const char *filters[1] = { "*DYNAMIC" };
	hideObjectsImpl(filters, 1, false);
}

void TerrainObjects::showDynamicObjects()
{
	const char *filters[1] = { "*DYNAMIC" };
	showObjectsImpl(filters, 1, false);
}

void TerrainObjects::hideNoCollisionObjects()
{
	const char *filters[1] = { "*NOCOLLISION" };
	hideObjectsImpl(filters, 1, false);
}

void TerrainObjects::showNoCollisionObjects()
{
	const char *filters[1] = { "*NOCOLLISION" };
	showObjectsImpl(filters, 1, false);
}

} // end of namespace editor
} // end of namespace frozenbyte
