// Copyright 2002-2004 Frozenbyte Ltd.

#ifndef INCLUDED_STORM3D_TERRAIN_UTILS_H
#define INCLUDED_STORM3D_TERRAIN_UTILS_H

#include <boost/shared_ptr.hpp>
#include <string>
#include <d3d9.h>
#include <d3dx9.h>
#include <atlbase.h>
#include <vector>

class Storm3D_Texture;
class IStorm3D_Logger;
struct IDirect3DDevice9;

#ifndef NDEBUG
void activeShaderNames();
void setTracing(bool tracing_);

#else
#define activeShaderNames() ;
#endif

namespace frozenbyte {
namespace storm {

class VertexShader
{
	CComPtr<IDirect3DVertexShader9> handle;
	CComPtr<IDirect3DVertexDeclaration9> declaration;
	
	IDirect3DDevice9 &device;
	std::vector<D3DVERTEXELEMENT9> elements;

	std::string name;
	CComPtr<IDirect3DVertexShader9> createVertexShader(IDirect3DDevice9 &device, const std::string &fileName);

public:
	VertexShader(IDirect3DDevice9 &device);
	~VertexShader();

	void create2DShader2Tex();

	void createTerrainShader();
	void createAtiTerrainShader();
	void createAtiLightingShader();
	void createNvTerrainShader();
	void createNvLightingShader();

	void createDefaultShader();
	//void createLightingShader();
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
	//void createBoneLightingShader();
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

	void createAtiDepthShader();
	void createAtiDepthTerrainShader();
	void createAtiBoneDepthShader();
	void createAtiShadowShaderDirectional();
	void createAtiShadowShaderPoint();
	void createAtiShadowShaderFlat();
	void createAtiBoneShadowShaderDirectional();
	void createAtiBoneShadowShaderPoint();
	void createAtiBoneShadowShaderFlat();
	void createAtiTerrainShadowShaderDirectional();
	void createAtiTerrainShadowShaderPoint();
	void createAtiTerrainShadowShaderFlat();
	void createNvTerrainShadowShaderDirectional();
	void createNvTerrainShadowShaderPoint();
	void createNvTerrainShadowShaderFlat();

	void createAtiConeShader();
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
	CComPtr<IDirect3DPixelShader9> handle;
	IDirect3DDevice9 &device;

	std::vector<D3DVERTEXELEMENT9> elements;

	std::string name;

	CComPtr<IDirect3DPixelShader9> createPixelShader(IDirect3DDevice9 &device, const std::string &fileName);
public:
	PixelShader(IDirect3DDevice9 &device);
	~PixelShader();

	void createTerrainShader();
	void createTerrainLightShader();
	void createGlowShader();
	void createGlowTex8Shader();
	void createGlowPs14Shader();
	void createGlowFinalShader();
	void createLightShader();
	void createAtiLightConeShader_Texture();
	void createAtiLightConeShader_NoTexture();
	void createAtiFloatLightConeShader_Texture();
	void createAtiFloatLightConeShader_NoTexture();
	void createAtiDepthPixelShader();
	void createAtiShadowPixelShader();
	void createAtiShadowSolidPixelShader();
	void createAtiShadowTerrainPixelShader();
	void createAtiNoShadowPixelShader();
	void createAtiNoShadowTerrainPixelShader();
	void createAtiFloatDepthPixelShader();
	void createAtiFloatShadowPixelShader();
	void createAtiFloatShadowTerrainPixelShader();
	void createAtiFloatNoShadowPixelShader();
	void createAtiFloatNoShadowTerrainPixelShader();
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

	void apply() const;
	bool hasShader() const;
};

class VertexBuffer
{
	CComPtr<IDirect3DVertexBuffer9> buffer;
	int vertexSize;
	int vertexAmount;
	bool dynamic;

public:
	VertexBuffer();
	~VertexBuffer();

	void release();
	void create(IDirect3DDevice9 &device, int vertexAmount, int vertexSize, bool dynamic);
	void *lock();
	void *unsafeLock(int offset, int amount);
	void unlock();

	void apply(IDirect3DDevice9 &device, int stream) const;
	operator bool() const;
};

class IndexBuffer
{
	CComPtr<IDirect3DIndexBuffer9> buffer;
	int faceAmount;
	bool dynamic;

	IStorm3D_Logger *logger;

public:
	IndexBuffer();
	~IndexBuffer();

	void setLogger(IStorm3D_Logger *logger);

	void release();
	void create(IDirect3DDevice9 &device, int faceAmount, bool dynamic);
	unsigned short *lock();
	void unlock();

	void render(IDirect3DDevice9 &device, int faceAmount, int maxIndex, int vertexOffset = 0, int startIndex = 0) const;
	operator bool() const;
};

boost::shared_ptr<Storm3D_Texture> createSharedTexture(Storm3D_Texture *texture);
void validateDevice(IDirect3DDevice9 &device, IStorm3D_Logger *logger);

void setCurrentAnisotrophy(int max);
void applyMaxAnisotrophy(IDirect3DDevice9 &device, int stageAmount);
void enableMinMagFiltering(IDirect3DDevice9 &device, int startStage, int endStage, bool enable);
void enableMipFiltering(IDirect3DDevice9 &device, int startStage, int endStage, bool enable);

void setInverseCulling(bool enable);
void setCulling(IDirect3DDevice9 &device, DWORD type);

void dumpD3DXMatrix(const D3DXMATRIX &mat);

} // storm
} // frozenbyte

#endif
