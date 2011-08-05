// Copyright 2002-2004 Frozenbyte Ltd.

#ifndef INCLUDED_STORM3D_TERRAIN_MODELS_H
#define INCLUDED_STORM3D_TERRAIN_MODELS_H

#include <boost/scoped_ptr.hpp>
#include <c2_vectors.h>
#include <DatatypeDef.h>

class Storm3D;
class IStorm3D_Model;
class Storm3D_Model;
class Storm3D_Scene;
class Storm3D_Camera;
class Storm3D_Spotlight;
class Storm3D_FakeSpotlight;
struct Storm3D_TerrainModelsData;
struct Storm3D_CollisionInfo;

struct TerrainLight
{
	VC3 position;
	float radius;
	COL color;

	TerrainLight()
	:	radius(0)
	{
	}
};

class Storm3D_TerrainModels
{
	boost::scoped_ptr<Storm3D_TerrainModelsData> data;

public:
	Storm3D_TerrainModels(Storm3D &storm);
	~Storm3D_TerrainModels();

	void addModel(IStorm3D_Model &model);
	void removeModel(IStorm3D_Model &model);
	IStorm3D_Model *popModel();
	void enableCulling(Storm3D_Model &model, bool enable);

	void buildTree(const VC3 &size);
	bool hasTree() const;
	void RayTrace(const VC3 &position, const VC3 &direction, float rayLength, Storm3D_CollisionInfo &info, bool accurate) const;
	void SphereCollision(const VC3 &position, float radius, Storm3D_CollisionInfo &info, bool accurate) const;

	enum MaterialType
	{
		SolidOnly,
		AlphaOnly
	};

	void calculateVisibility(Storm3D_Scene &scene, int timeDelta);
	void renderTextures(MaterialType materialType, Storm3D_Scene &scene);
	void renderLighting(MaterialType materialType, Storm3D_Scene &scene);
	void renderDepth(Storm3D_Scene &scene, Storm3D_Camera &camera, Storm3D_Spotlight &spot, const IStorm3D_Model *skipModel);
	bool renderDepth(Storm3D_Scene &scene, Storm3D_Camera &camera, Storm3D_FakeSpotlight &spot);
	void renderProjection(MaterialType materialType, Storm3D_Scene &scene, Storm3D_Spotlight &spot);
	void renderProjection(Storm3D_Scene &scene, Storm3D_FakeSpotlight &spot);
	void renderGlows(Storm3D_Scene &scene);
	void renderDistortion(Storm3D_Scene &scene);

	enum FillMode
	{
		Solid,
		Wireframe
	};

	void setFillMode(FillMode mode);
	void filterLightmap(bool filter);
	void setCollisionRendering(bool enable);
	void enableAlphaTest(bool enable);
	void renderBoned(bool enable);
	void enableMaterialAmbient(bool enable);
	void forceWhiteBaseTexture(bool force);
	void enableGlow(bool enable);
	void enableDistortion(bool enable);
	void enableReflection(bool enable);
	void renderBackground(Storm3D_Model *model);
	void enableAdditionalAlphaTestPass(bool enable);
	void enableSkyModelGlow(bool enable);

	int addLight(const VC3 &position, float radius, const COL &color);
	void setLightPosition(int index, const VC3 &position);
	void setLightRadius(int index, float radius);
	void setLightColor(int index, const COL &color);
	const TerrainLight &getLight(int index) const;
	void removeLight(int index);
	void clearLights();

	void setTerrainObjectColorFactorBuilding(const COL &factor);
	void setTerrainObjectColorFactorOutside(const COL &factor);
	void setForcedDirectional(bool enabled, const VC3 &direction, const COL &color);
};

#endif

