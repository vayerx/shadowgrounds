// Copyright 2002-2004 Frozenbyte Ltd.

#ifndef INCLUDED_STORM3D_TERRAIN_UTILS_H
#define INCLUDED_STORM3D_TERRAIN_UTILS_H

#include <boost/shared_ptr.hpp>
#include <string>
#include <vector>
#include <SDL.h>
#include <GL/glew.h>
#include "igios.h"
#include "igios3D.h"

class Storm3D_Texture;
class IStorm3D_Logger;

void setTracing(bool tracing_);
void activeShaderNames();

namespace frozenbyte {
namespace storm {

class VertexShader
{
private:
	GLuint handle;
	std::vector<Element> elements;
	std::string nm;
	void createVertexShader(const std::string &name);

public:
	static void disable();

	VertexShader();
	~VertexShader();

	void createTerrainShader();
	void createNvTerrainShader();
	void createNvLightingShader();

	void createDefaultShader();
	void createLightingShader_0light_noreflection();
	void createLightingShader_0light_localreflection();
	void createLightingShader_0light_reflection();
	void createLightingShader_1light_noreflection();
	void createLightingShader_1light_localreflection();
	void createLightingShader_1light_reflection();
	void createLightingShader_2light_noreflection();
	void createLightingShader_2light_localreflection();
	void createLightingShader_2light_reflection();
	void createLightingShader_3light_noreflection();
	void createLightingShader_3light_localreflection();
	void createLightingShader_3light_reflection();
	void createLightingShader_4light_noreflection();
	void createLightingShader_4light_localreflection();
	void createLightingShader_4light_reflection();
	void createLightingShader_5light_noreflection();
	void createLightingShader_5light_localreflection();
	void createLightingShader_5light_reflection();

	void createSkyboxShader();
	void createDefaultProjectionShaderDirectional();
	void createDefaultProjectionShaderPoint();
	void createDefaultProjectionShaderFlat();
	void createBoneShader();
	void createBasicBoneLightingShader();
	void createBoneLightingShader_0light_noreflection();
	void createBoneLightingShader_0light_reflection();
	void createBoneLightingShader_1light_noreflection();
	void createBoneLightingShader_1light_reflection();
	void createBoneLightingShader_2light_noreflection();
	void createBoneLightingShader_2light_reflection();
	void createBoneLightingShader_3light_noreflection();
	void createBoneLightingShader_3light_reflection();
	void createBoneLightingShader_4light_noreflection();
	void createBoneLightingShader_4light_reflection();
	void createBoneLightingShader_5light_noreflection();
	void createBoneLightingShader_5light_reflection();

	void createBoneProjectionShaderDirectional();
	void createBoneProjectionShaderPoint();
	void createBoneProjectionShaderFlat();

	void createAtiDepthTerrainShader();
	void createNvTerrainShadowShaderDirectional();
	void createNvTerrainShadowShaderPoint();
	void createNvTerrainShadowShaderFlat();

	void createNvConeShader();
	void createConeStencilShader();
	void createDecalShader();
	void createDecalPointShader();
	void createDecalDirShader();
	void createDecalFlatShader();
	void createFakeDepthShader();
	void createFakeShadowShader();
	void createFakeDepthBoneShader();
	void createFakeShadowBoneShader();
	void createFakePlaneShadowShader();
	void createProceduralShader();

	void applyDeclaration() const;
	void apply() const;
};

class PixelShader
{
private:
	GLuint handle;
	std::string nm;
	void createPixelShader(const std::string &name);

public:
	static void disable();

	PixelShader();
	~PixelShader();

	void createTerrainShader();
	void createTerrainLightShader();
	void createGlowShader();
	void createGlowTex8Shader();
	void createGlowPs14Shader();
	void createGlowFinalShader();
	void createLightShader();
	void createNvShadowShader();
	void createNvSmoothShadowShader();
	void createNvNoShadowShader();
	void createNvConeShader_Texture();
	void createNvConeShader_NoTexture();

	void createBasePixelShader();
	void createLightingPixelShader_Lightmap();
	void createLightingPixelShader_Lightmap_Reflection();
	void createLightingPixelShader_Lightmap_LocalReflection();
	void createLightingPixelShader_LightmapNoTexture();
	void createLightingPixelShader_NoLightmap();
	void createLightingPixelShader_NoLightmap_Reflection();
	void createLightingPixelShader_NoLightmap_LocalReflection();
	void createLightingPixelShader_NoLightmapNoTexture();
	void createDepthShader();
	void createShadowPixelShader();
	void createFakeDepthPixelShader();
	void createFakeShadowPixelShader();
	void createDecalPixelShader();
	void createColorEffectPixelShader();
	void createColorEffectOffsetPixelShader();
	void createColorEffectOffsetPixelShader_NoGamma();
	void createProceduralShader();
	void createProceduralOffsetShader();
	void createProceduralOffsetBaseShader();
	void createBlackWhiteShader();
	void createOffsetBlendShader();
	void createSkyboxShader();

	void apply() const;
	bool hasShader() const;
};

class VertexBuffer
{
	GLuint buffer;
	int vertexSize;
	int vertexAmount;
	bool dynamic;

public:
	VertexBuffer();
	~VertexBuffer();

	void release();
	void create(int vertexAmount, int vertexSize, bool dynamic);
	void *lock();
	void *unsafeLock(int offset, int amount);
	void unlock();

	void apply(int stream) const;
	void apply(int stream, int offset) const;
	operator bool() const;
};

class IndexBuffer
{
	GLuint buffer;
	int faceAmount;
	bool dynamic;

	IStorm3D_Logger *logger;

public:
	IndexBuffer();
	~IndexBuffer();

	void setLogger(IStorm3D_Logger *logger);

	void release();
	void create(int faceAmount, bool dynamic);
	unsigned short *lock();
	void unlock();

	void render(int faceAmount, int maxIndex, int vertexOffset = 0, int startIndex = 0) const;
	operator bool() const;
};

boost::shared_ptr<Storm3D_Texture> createSharedTexture(Storm3D_Texture *texture);
void validateDevice(IStorm3D_Logger *logger);

void setInverseCulling(bool enable);
void setCulling(CULLMODE type);

} // storm
} // frozenbyte

#endif
