// Copyright 2002-2004 Frozenbyte Ltd.

#ifdef _MSC_VER
#pragma warning(disable:4103)
#endif

#include <fstream>
#include <vector>
#include <stdio.h>

#include "storm3d_terrain_utils.h"
#include "storm3d_texture.h"
#include "IStorm3D_Logger.h"

extern int storm3d_dip_calls;

#include "../../filesystem/input_stream.h"
#include "../../filesystem/file_package_manager.h"
#include "../../util/Debug_MemoryManager.h"

static std::string activePixelShader;
static std::string activeVertexShader;
static bool tracing = false;

void setTracing(bool tracing_) {
	if (tracing != tracing_) {
		tracing = tracing_;
		igiosWarning("Tracing %s\n", tracing_ ? "enabled" : "disabled");
		if (tracing_) {
			igiosWarning("Current shaders:\n");
			activeShaderNames();
		}
	}
}

void activeShaderNames() {
	igiosWarning("vertex: %s\npixel: %s\n", activeVertexShader.c_str(), activePixelShader.c_str());
}

namespace frozenbyte {
namespace storm {
namespace {

	void readFile(std::string &result, const std::string &fileName)
	{
		filesystem::InputStream stream = filesystem::FilePackageManager::getInstance().getFile(fileName);
		while(!stream.isEof())
		{
			std::string line;
			while(!stream.isEof())
			{
				signed char c;
				stream >> c;

				if(c == '\r')
					continue;
				else if(c == '\n')
					break;
				
				line += c;
			}

			result += line;
			result += "\r\n";
		}
	}


} // unnamed

void PixelShader::disable() {
	glDisable(GL_FRAGMENT_PROGRAM_ARB);
	activePixelShader = "<none>";
	if (tracing) {
		igiosWarning("pixel shader disabled\n");
	}
}

void VertexShader::disable() {
	glDisable(GL_VERTEX_PROGRAM_ARB);
	activeVertexShader = "<none>";
	if (tracing) {
		igiosWarning("vertex shader disabled\n");
	}
}

void PixelShader::createPixelShader(const std::string &name) {
	nm = name;
	std::string fileName = "data/glshaders/" + name + ".txt";  // FIXME:change path
	std::string shaderString;
	readFile(shaderString, fileName);

	glEnable(GL_FRAGMENT_PROGRAM_ARB);
	glGenProgramsARB(1, &handle);
	glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, handle);
	// WHAT THE FUCK is wrong with shaderString.length() ?
	glProgramStringARB(GL_FRAGMENT_PROGRAM_ARB, GL_PROGRAM_FORMAT_ASCII_ARB, strlen(shaderString.c_str()), shaderString.c_str());

	GLint errorPos = -1;
	glGetIntegerv(GL_PROGRAM_ERROR_POSITION_ARB, &errorPos);
	if (errorPos != -1) {
		igiosWarning("error creating shader %s: %s\n", name.c_str(), glGetString(GL_PROGRAM_ERROR_STRING_ARB));
	}
	frozenbyte::storm::PixelShader::disable();

	return;
}

void VertexShader::createVertexShader(const std::string &name) {
	nm = name;
	std::string fileName = "data/glshaders/" + name + ".txt";  // FIXME:change path
	std::string shaderString;
	readFile(shaderString, fileName);

	glEnable(GL_VERTEX_PROGRAM_ARB);
	glGenProgramsARB(1, &handle);
	glBindProgramARB(GL_VERTEX_PROGRAM_ARB, handle);
	// WHAT THE FUCK is wrong with shaderString.length() ?
	glProgramStringARB(GL_VERTEX_PROGRAM_ARB, GL_PROGRAM_FORMAT_ASCII_ARB, strlen(shaderString.c_str()), shaderString.c_str());

	GLint errorPos = -1;
	glGetIntegerv(GL_PROGRAM_ERROR_POSITION_ARB, &errorPos);
	if (errorPos != -1) {
		igiosWarning("error creating shader %s: %s\n", name.c_str(), glGetString(GL_PROGRAM_ERROR_STRING_ARB));
	}

	frozenbyte::storm::PixelShader::disable();
	return;
}

VertexShader::VertexShader()
: handle(0)
{
}

VertexShader::~VertexShader()
{
	if (handle != 0) {
		glDeleteProgramsARB(1, &handle);
		handle = 0;
	}
}

void VertexShader::createTerrainShader()
{
	elements.clear();
	elements.push_back(Element(0, 0, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_POSITION));
	elements.push_back(Element(0, 3*4, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_NORMAL));
	elements.push_back(Element(1, 0, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 0));
	elements.push_back(Element(1, 2*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 1));
	elements.push_back(Element(1, 4*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 2));
	handle = 0; // FIXME: unused
	igios_unimplemented();
}


void VertexShader::createNvTerrainShader()
{
	elements.clear();
	elements.push_back(Element(0, 0, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_POSITION));
	elements.push_back(Element(0, 3*4, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_NORMAL));
	elements.push_back(Element(1, 0, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 0));
	elements.push_back(Element(1, 2*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 1));
	elements.push_back(Element(1, 4*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 2));
	createVertexShader("nv_terrain_vertex_shader");
}

void VertexShader::createNvLightingShader()
{
	elements.clear();
	elements.push_back(Element(0, 0, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_POSITION));
	elements.push_back(Element(0, 3*4, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_NORMAL));
	elements.push_back(Element(1, 0, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 0));
	elements.push_back(Element(1, 2*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 1));
	elements.push_back(Element(1, 4*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 2));
	createVertexShader("nv_terrain_lighting_vertex_shader");
}

void VertexShader::createDefaultShader()
{
	elements.clear();
	elements.push_back(Element(0, 0, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_POSITION));
	elements.push_back(Element(0, 3*4, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_NORMAL));
	elements.push_back(Element(0, 6*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 0));
	elements.push_back(Element(0, 8*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 1));
	createVertexShader("default_shader");
}

void VertexShader::createLightingShader_0light_noreflection()
{
	elements.clear();
	elements.push_back(Element(0, 0, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_POSITION));
	elements.push_back(Element(0, 3*4, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_NORMAL));
	elements.push_back(Element(0, 6*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 0));
	elements.push_back(Element(0, 8*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 1));
	createVertexShader("lighting_shader_0light_noreflection");
}

void VertexShader::createLightingShader_0light_localreflection()
{
	elements.clear();
	elements.push_back(Element(0, 0, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_POSITION));
	elements.push_back(Element(0, 3*4, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_NORMAL));
	elements.push_back(Element(0, 6*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 0));
	elements.push_back(Element(0, 8*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 1));
	createVertexShader("lighting_shader_0light_localreflection");
}

void VertexShader::createLightingShader_0light_reflection()
{
	elements.clear();
	elements.push_back(Element(0, 0, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_POSITION));
	elements.push_back(Element(0, 3*4, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_NORMAL));
	elements.push_back(Element(0, 6*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 0));
	elements.push_back(Element(0, 8*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 1));
	createVertexShader("lighting_shader_0light_reflection");
}

void VertexShader::createLightingShader_1light_noreflection()
{
	elements.clear();
	elements.push_back(Element(0, 0, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_POSITION));
	elements.push_back(Element(0, 3*4, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_NORMAL));
	elements.push_back(Element(0, 6*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 0));
	elements.push_back(Element(0, 8*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 1));
	createVertexShader("lighting_shader_1light_noreflection");
}

void VertexShader::createLightingShader_1light_localreflection()
{
	elements.clear();
	elements.push_back(Element(0, 0, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_POSITION));
	elements.push_back(Element(0, 3*4, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_NORMAL));
	elements.push_back(Element(0, 6*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 0));
	elements.push_back(Element(0, 8*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 1));
	createVertexShader("lighting_shader_1light_localreflection");
}

void VertexShader::createLightingShader_1light_reflection()
{
	elements.clear();
	elements.push_back(Element(0, 0, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_POSITION));
	elements.push_back(Element(0, 3*4, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_NORMAL));
	elements.push_back(Element(0, 6*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 0));
	elements.push_back(Element(0, 8*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 1));
	createVertexShader("lighting_shader_1light_reflection");
}

void VertexShader::createLightingShader_2light_noreflection()
{
	elements.clear();
	elements.push_back(Element(0, 0, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_POSITION));
	elements.push_back(Element(0, 3*4, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_NORMAL));
	elements.push_back(Element(0, 6*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 0));
	elements.push_back(Element(0, 8*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 1));
	createVertexShader("lighting_shader_2light_noreflection");
}

void VertexShader::createLightingShader_2light_localreflection()
{
	elements.clear();
	elements.push_back(Element(0, 0, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_POSITION));
	elements.push_back(Element(0, 3*4, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_NORMAL));
	elements.push_back(Element(0, 6*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 0));
	elements.push_back(Element(0, 8*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 1));
	createVertexShader("lighting_shader_2light_localreflection");
}

void VertexShader::createLightingShader_2light_reflection()
{
	elements.clear();
	elements.push_back(Element(0, 0, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_POSITION));
	elements.push_back(Element(0, 3*4, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_NORMAL));
	elements.push_back(Element(0, 6*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 0));
	elements.push_back(Element(0, 8*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 1));
	createVertexShader("lighting_shader_2light_reflection");
}

void VertexShader::createLightingShader_3light_noreflection()
{
	elements.clear();
	elements.push_back(Element(0, 0, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_POSITION));
	elements.push_back(Element(0, 3*4, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_NORMAL));
	elements.push_back(Element(0, 6*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 0));
	elements.push_back(Element(0, 8*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 1));
	createVertexShader("lighting_shader_3light_noreflection");
}

void VertexShader::createLightingShader_3light_localreflection()
{
	elements.clear();
	elements.push_back(Element(0, 0, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_POSITION));
	elements.push_back(Element(0, 3*4, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_NORMAL));
	elements.push_back(Element(0, 6*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 0));
	elements.push_back(Element(0, 8*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 1));
	createVertexShader("lighting_shader_3light_localreflection");
}

void VertexShader::createLightingShader_3light_reflection()
{
	elements.clear();
	elements.push_back(Element(0, 0, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_POSITION));
	elements.push_back(Element(0, 3*4, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_NORMAL));
	elements.push_back(Element(0, 6*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 0));
	elements.push_back(Element(0, 8*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 1));
	createVertexShader("lighting_shader_3light_reflection");
}

void VertexShader::createLightingShader_4light_noreflection()
{
	elements.clear();
	elements.push_back(Element(0, 0, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_POSITION));
	elements.push_back(Element(0, 3*4, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_NORMAL));
	elements.push_back(Element(0, 6*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 0));
	elements.push_back(Element(0, 8*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 1));
	createVertexShader("lighting_shader_4light_noreflection");
}

void VertexShader::createLightingShader_4light_localreflection()
{
	elements.clear();
	elements.push_back(Element(0, 0, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_POSITION));
	elements.push_back(Element(0, 3*4, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_NORMAL));
	elements.push_back(Element(0, 6*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 0));
	elements.push_back(Element(0, 8*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 1));
	createVertexShader("lighting_shader_4light_localreflection");
}

void VertexShader::createLightingShader_4light_reflection()
{
	elements.clear();
	elements.push_back(Element(0, 0, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_POSITION));
	elements.push_back(Element(0, 3*4, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_NORMAL));
	elements.push_back(Element(0, 6*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 0));
	elements.push_back(Element(0, 8*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 1));
	createVertexShader("lighting_shader_4light_reflection");
}

void VertexShader::createLightingShader_5light_noreflection()
{
	elements.clear();
	elements.push_back(Element(0, 0, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_POSITION));
	elements.push_back(Element(0, 3*4, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_NORMAL));
	elements.push_back(Element(0, 6*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 0));
	elements.push_back(Element(0, 8*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 1));
	createVertexShader("lighting_shader_5light_noreflection");
}

void VertexShader::createLightingShader_5light_localreflection()
{
	elements.clear();
	elements.push_back(Element(0, 0, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_POSITION));
	elements.push_back(Element(0, 3*4, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_NORMAL));
	elements.push_back(Element(0, 6*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 0));
	elements.push_back(Element(0, 8*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 1));
	createVertexShader("lighting_shader_5light_localreflection");
}

void VertexShader::createLightingShader_5light_reflection()
{
	elements.clear();
	elements.push_back(Element(0, 0, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_POSITION));
	elements.push_back(Element(0, 3*4, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_NORMAL));
	elements.push_back(Element(0, 6*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 0));
	elements.push_back(Element(0, 8*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 1));
	createVertexShader("lighting_shader_5light_reflection");
}

void VertexShader::createSkyboxShader()
{
	elements.clear();
	elements.push_back(Element(0, 0, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_POSITION));
	elements.push_back(Element(0, 3*4, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_NORMAL));
	elements.push_back(Element(0, 6*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 0));
	elements.push_back(Element(0, 8*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 1));
	createVertexShader("skybox_shader");
}

void VertexShader::createDefaultProjectionShaderDirectional()
{
	elements.clear();
	elements.push_back(Element(0, 0, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_POSITION));
	elements.push_back(Element(0, 3*4, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_NORMAL));
	elements.push_back(Element(0, 6*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 0));
	elements.push_back(Element(0, 8*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 1));
	createVertexShader("default_projection_shader_dir");
}

void VertexShader::createDefaultProjectionShaderPoint()
{
	elements.clear();
	elements.push_back(Element(0, 0, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_POSITION));
	elements.push_back(Element(0, 3*4, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_NORMAL));
	elements.push_back(Element(0, 6*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 0));
	elements.push_back(Element(0, 8*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 1));
	createVertexShader("default_projection_shader_point");
}

void VertexShader::createDefaultProjectionShaderFlat()
{
	elements.clear();
	elements.push_back(Element(0, 0, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_POSITION));
	elements.push_back(Element(0, 3*4, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_NORMAL));
	elements.push_back(Element(0, 6*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 0));
	elements.push_back(Element(0, 8*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 1));
	createVertexShader("default_projection_shader_flat");
}

void VertexShader::createBoneShader()
{
	elements.push_back(Element(0, 0, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_POSITION));
	elements.push_back(Element(0, 3*4, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_NORMAL));
	elements.push_back(Element(0, 6*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 0));
	elements.push_back(Element(0, 8*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 1));
	elements.push_back(Element(0, 10*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 2));
	elements.push_back(Element(0, 12*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 3));
	createVertexShader("bone_shader_1");
}


void VertexShader::createBasicBoneLightingShader()
{
	elements.push_back(Element(0, 0, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_POSITION));
	elements.push_back(Element(0, 3*4, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_NORMAL));
	elements.push_back(Element(0, 6*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 0));
	elements.push_back(Element(0, 8*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 1));
	elements.push_back(Element(0, 10*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 2));
	elements.push_back(Element(0, 12*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 3));
	createVertexShader("basic_bone_lighting_shader_1");
}

void VertexShader::createBoneLightingShader_0light_noreflection()
{
	elements.push_back(Element(0, 0, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_POSITION));
	elements.push_back(Element(0, 3*4, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_NORMAL));
	elements.push_back(Element(0, 6*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 0));
	elements.push_back(Element(0, 8*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 1));
	elements.push_back(Element(0, 10*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 2));
	elements.push_back(Element(0, 12*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 3));
	createVertexShader("bone_lighting_shader_0light_noreflection");
}

void VertexShader::createBoneLightingShader_0light_reflection()
{
	elements.push_back(Element(0, 0, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_POSITION));
	elements.push_back(Element(0, 3*4, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_NORMAL));
	elements.push_back(Element(0, 6*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 0));
	elements.push_back(Element(0, 8*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 1));
	elements.push_back(Element(0, 10*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 2));
	elements.push_back(Element(0, 12*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 3));
	createVertexShader("bone_lighting_shader_0light_reflection");
}

void VertexShader::createBoneLightingShader_1light_noreflection()
{
	elements.push_back(Element(0, 0, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_POSITION));
	elements.push_back(Element(0, 3*4, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_NORMAL));
	elements.push_back(Element(0, 6*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 0));
	elements.push_back(Element(0, 8*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 1));
	elements.push_back(Element(0, 10*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 2));
	elements.push_back(Element(0, 12*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 3));
	createVertexShader("bone_lighting_shader_1light_noreflection");
}

void VertexShader::createBoneLightingShader_1light_reflection()
{
	elements.push_back(Element(0, 0, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_POSITION));
	elements.push_back(Element(0, 3*4, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_NORMAL));
	elements.push_back(Element(0, 6*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 0));
	elements.push_back(Element(0, 8*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 1));
	elements.push_back(Element(0, 10*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 2));
	elements.push_back(Element(0, 12*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 3));
	createVertexShader("bone_lighting_shader_1light_reflection");
}

void VertexShader::createBoneLightingShader_2light_noreflection()
{
	elements.push_back(Element(0, 0, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_POSITION));
	elements.push_back(Element(0, 3*4, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_NORMAL));
	elements.push_back(Element(0, 6*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 0));
	elements.push_back(Element(0, 8*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 1));
	elements.push_back(Element(0, 10*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 2));
	elements.push_back(Element(0, 12*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 3));
	createVertexShader("bone_lighting_shader_2light_noreflection");
}

void VertexShader::createBoneLightingShader_2light_reflection()
{
	elements.push_back(Element(0, 0, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_POSITION));
	elements.push_back(Element(0, 3*4, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_NORMAL));
	elements.push_back(Element(0, 6*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 0));
	elements.push_back(Element(0, 8*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 1));
	elements.push_back(Element(0, 10*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 2));
	elements.push_back(Element(0, 12*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 3));
	createVertexShader("bone_lighting_shader_2light_reflection");
}

void VertexShader::createBoneLightingShader_3light_noreflection()
{
	elements.push_back(Element(0, 0, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_POSITION));
	elements.push_back(Element(0, 3*4, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_NORMAL));
	elements.push_back(Element(0, 6*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 0));
	elements.push_back(Element(0, 8*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 1));
	elements.push_back(Element(0, 10*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 2));
	elements.push_back(Element(0, 12*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 3));
	createVertexShader("bone_lighting_shader_3light_noreflection");
}

void VertexShader::createBoneLightingShader_3light_reflection()
{
	elements.push_back(Element(0, 0, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_POSITION));
	elements.push_back(Element(0, 3*4, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_NORMAL));
	elements.push_back(Element(0, 6*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 0));
	elements.push_back(Element(0, 8*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 1));
	elements.push_back(Element(0, 10*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 2));
	elements.push_back(Element(0, 12*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 3));
	createVertexShader("bone_lighting_shader_3light_reflection");
}

void VertexShader::createBoneLightingShader_4light_noreflection()
{
	elements.push_back(Element(0, 0, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_POSITION));
	elements.push_back(Element(0, 3*4, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_NORMAL));
	elements.push_back(Element(0, 6*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 0));
	elements.push_back(Element(0, 8*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 1));
	elements.push_back(Element(0, 10*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 2));
	elements.push_back(Element(0, 12*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 3));
	createVertexShader("bone_lighting_shader_4light_noreflection");
}

void VertexShader::createBoneLightingShader_4light_reflection()
{
	elements.push_back(Element(0, 0, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_POSITION));
	elements.push_back(Element(0, 3*4, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_NORMAL));
	elements.push_back(Element(0, 6*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 0));
	elements.push_back(Element(0, 8*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 1));
	elements.push_back(Element(0, 10*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 2));
	elements.push_back(Element(0, 12*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 3));
	createVertexShader("bone_lighting_shader_4light_reflection");
}

void VertexShader::createBoneLightingShader_5light_noreflection()
{
	elements.push_back(Element(0, 0, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_POSITION));
	elements.push_back(Element(0, 3*4, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_NORMAL));
	elements.push_back(Element(0, 6*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 0));
	elements.push_back(Element(0, 8*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 1));
	elements.push_back(Element(0, 10*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 2));
	elements.push_back(Element(0, 12*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 3));
	createVertexShader("bone_lighting_shader_5light_noreflection");
}

void VertexShader::createBoneLightingShader_5light_reflection()
{
	elements.push_back(Element(0, 0, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_POSITION));
	elements.push_back(Element(0, 3*4, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_NORMAL));
	elements.push_back(Element(0, 6*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 0));
	elements.push_back(Element(0, 8*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 1));
	elements.push_back(Element(0, 10*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 2));
	elements.push_back(Element(0, 12*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 3));
	createVertexShader("bone_lighting_shader_5light_reflection");
}

void VertexShader::createBoneProjectionShaderDirectional()
{
	elements.push_back(Element(0, 0, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_POSITION));
	elements.push_back(Element(0, 3*4, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_NORMAL));
	elements.push_back(Element(0, 6*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 0));
	elements.push_back(Element(0, 8*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 1));
	elements.push_back(Element(0, 10*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 2));
	elements.push_back(Element(0, 12*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 3));
	createVertexShader("bone_projection_shader_dir");
}

void VertexShader::createBoneProjectionShaderPoint()
{
	elements.push_back(Element(0, 0, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_POSITION));
	elements.push_back(Element(0, 3*4, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_NORMAL));
	elements.push_back(Element(0, 6*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 0));
	elements.push_back(Element(0, 8*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 1));
	elements.push_back(Element(0, 10*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 2));
	elements.push_back(Element(0, 12*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 3));
	createVertexShader("bone_projection_shader_point");
}

void VertexShader::createBoneProjectionShaderFlat()
{
	elements.push_back(Element(0, 0, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_POSITION));
	elements.push_back(Element(0, 3*4, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_NORMAL));
	elements.push_back(Element(0, 6*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 0));
	elements.push_back(Element(0, 8*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 1));
	elements.push_back(Element(0, 10*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 2));
	elements.push_back(Element(0, 12*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 3));
	createVertexShader("bone_projection_shader_flat");
}


void VertexShader::createAtiDepthTerrainShader()
{
	elements.clear();
	elements.push_back(Element(0, 0, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_POSITION));
	elements.push_back(Element(0, 3*4, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_NORMAL));
	elements.push_back(Element(1, 0, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 0));
	elements.push_back(Element(1, 2*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 1));
	elements.push_back(Element(1, 4*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 2));
	createVertexShader("ati_depth_default_vertex_shader");
}


void VertexShader::createNvTerrainShadowShaderDirectional()
{
	elements.clear();
	elements.push_back(Element(0, 0, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_POSITION));
	elements.push_back(Element(0, 3*4, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_NORMAL));
	elements.push_back(Element(1, 0, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 0));
	elements.push_back(Element(1, 2*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 1));
	elements.push_back(Element(1, 4*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 2));
	createVertexShader("nv_shadow_terrain_vertex_shader_dir");
}

void VertexShader::createNvTerrainShadowShaderPoint()
{
	elements.clear();
	elements.push_back(Element(0, 0, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_POSITION));
	elements.push_back(Element(0, 3*4, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_NORMAL));
	elements.push_back(Element(1, 0, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 0));
	elements.push_back(Element(1, 2*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 1));
	elements.push_back(Element(1, 4*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 2));
	createVertexShader("nv_shadow_terrain_vertex_shader_point");
}

void VertexShader::createNvTerrainShadowShaderFlat()
{
	elements.clear();
	elements.push_back(Element(0, 0, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_POSITION));
	elements.push_back(Element(0, 3*4, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_NORMAL));
	elements.push_back(Element(1, 0, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 0));
	elements.push_back(Element(1, 2*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 1));
	elements.push_back(Element(1, 4*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 2));
	createVertexShader("nv_shadow_terrain_vertex_shader_flat");
}

void VertexShader::createNvConeShader()
{
	elements.clear();
	elements.push_back(Element(0, 0, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_POSITION));
	elements.push_back(Element(0, 3*4, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_NORMAL));
	elements.push_back(Element(0, 6*4, D3DDECLTYPE_D3DCOLOR, D3DDECLUSAGE_COLOR));
	elements.push_back(Element(0, 7*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 0));
	createVertexShader("cone_vertex_shader_nv");
}

void VertexShader::createDecalShader()
{
	elements.clear();
	elements.push_back(Element(0, 0, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_POSITION));
	elements.push_back(Element(0, 3*4, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_NORMAL));
	elements.push_back(Element(0, 6*4, D3DDECLTYPE_D3DCOLOR, D3DDECLUSAGE_COLOR));
	elements.push_back(Element(0, 7*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 0));
	elements.push_back(Element(0, 9*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 1));
	createVertexShader("decal_vertex_shader");
}

void VertexShader::createDecalPointShader()
{
	elements.clear();
	elements.push_back(Element(0, 0, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_POSITION));
	elements.push_back(Element(0, 3*4, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_NORMAL));
	elements.push_back(Element(0, 6*4, D3DDECLTYPE_D3DCOLOR, D3DDECLUSAGE_COLOR));
	elements.push_back(Element(0, 7*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 0));
	elements.push_back(Element(0, 9*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 1));
	createVertexShader("decal_projection_vertex_shader_point");
}

void VertexShader::createDecalDirShader()
{
	elements.clear();
	elements.push_back(Element(0, 0, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_POSITION));
	elements.push_back(Element(0, 3*4, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_NORMAL));
	elements.push_back(Element(0, 6*4, D3DDECLTYPE_D3DCOLOR, D3DDECLUSAGE_COLOR));
	elements.push_back(Element(0, 7*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 0));
	elements.push_back(Element(0, 9*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 1));
	createVertexShader("decal_projection_vertex_shader_dir");
}

void VertexShader::createDecalFlatShader()
{
	elements.clear();
	elements.push_back(Element(0, 0, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_POSITION));
	elements.push_back(Element(0, 3*4, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_NORMAL));
	elements.push_back(Element(0, 6*4, D3DDECLTYPE_D3DCOLOR, D3DDECLUSAGE_COLOR));
	elements.push_back(Element(0, 7*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 0));
	elements.push_back(Element(0, 9*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 1));
	createVertexShader("decal_projection_vertex_shader_flat");
}

void VertexShader::createFakeDepthShader()
{
	elements.clear();
	elements.push_back(Element(0, 0, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_POSITION));
	elements.push_back(Element(0, 3*4, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_NORMAL));
	elements.push_back(Element(0, 6*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 0));
	elements.push_back(Element(0, 8*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 1));
	createVertexShader("fake_depth_vertex_shader");
}

void VertexShader::createFakeShadowShader()
{
	elements.clear();
	elements.push_back(Element(0, 0, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_POSITION));
	elements.push_back(Element(0, 3*4, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_NORMAL));
	elements.push_back(Element(0, 6*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 0));
	elements.push_back(Element(0, 8*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 1));
	createVertexShader("fake_shadow_vertex_shader");
}

void VertexShader::createFakeDepthBoneShader()
{
	elements.clear();
	elements.push_back(Element(0, 0, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_POSITION));
	elements.push_back(Element(0, 3*4, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_NORMAL));
	elements.push_back(Element(0, 6*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 0));
	elements.push_back(Element(0, 8*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 1));
	elements.push_back(Element(0, 10*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 2));
	elements.push_back(Element(0, 12*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 3));
	createVertexShader("fake_depth_bone_vertex_shader");
}

void VertexShader::createFakeShadowBoneShader()
{
	elements.clear();
	elements.push_back(Element(0, 0, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_POSITION));
	elements.push_back(Element(0, 3*4, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_NORMAL));
	elements.push_back(Element(0, 6*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 0));
	elements.push_back(Element(0, 8*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 1));
	elements.push_back(Element(0, 10*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 2));
	elements.push_back(Element(0, 12*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 3));
	createVertexShader("fake_shadow_bone_vertex_shader");
}

void VertexShader::createFakePlaneShadowShader()
{
	elements.clear();
	elements.push_back(Element(0, 0, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_POSITION));
	elements.push_back(Element(0, 3*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 0));
	createVertexShader("fake_shadow_plane_vertex_shader");
}

void VertexShader::createConeStencilShader()
{
	elements.clear();
	elements.push_back(Element(0, 0, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_POSITION));
	createVertexShader("cone_stencil_vertex_shader");
}

void VertexShader::createProceduralShader()
{
	elements.clear();
	elements.push_back(Element(0, 0, D3DDECLTYPE_FLOAT4, D3DDECLUSAGE_POSITION));
	elements.push_back(Element(0, 4*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 0));
	elements.push_back(Element(0, 6*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 1));
	createVertexShader("procedural_vertex_shader");
}

void VertexShader::applyDeclaration() const
{
	setVertexDeclaration(elements);
}

void VertexShader::apply() const
{
	glEnable(GL_VERTEX_PROGRAM_ARB);
	glBindProgramARB(GL_VERTEX_PROGRAM_ARB, handle);
	activeVertexShader = nm;
	if (tracing) {
		igiosWarning("activated vertex shader %s\n", nm.c_str());
	}
	applyDeclaration();
}

// --

PixelShader::PixelShader()
:	handle(0)
{
}

PixelShader::~PixelShader()
{
	if (handle != 0) {
		glDeleteProgramsARB(1, &handle);
		handle = 0;
	}
}


void PixelShader::createTerrainShader()
{
	createPixelShader("terrain_pixel_shader");
}

void PixelShader::createTerrainLightShader()
{
	createPixelShader("terrain_lighting_pixel_shader");
}

void PixelShader::createGlowShader()
{
	createPixelShader("glow_pixel_shader");
}

void PixelShader::createGlowTex8Shader()
{
	createPixelShader("glow_8tex_pixel_shader");
}

void PixelShader::createGlowPs14Shader()
{
	createPixelShader("glow_ps14_pixel_shader");
}

void PixelShader::createGlowFinalShader()
{
	createPixelShader("glow_pass_pixel_shader");
}

void PixelShader::createLightShader()
{
	createPixelShader("lightmap_pixel_shader");
}

void PixelShader::createNvShadowShader()
{
	createPixelShader("nv_shadow_pixel_shader");
}

void PixelShader::createNvSmoothShadowShader()
{
	createPixelShader("nv_shadow_pixel_shader_smooth");
}

void PixelShader::createNvNoShadowShader()
{
	createPixelShader("nv_noshadow_pixel_shader");
}

void PixelShader::createNvConeShader_Texture()
{
	createPixelShader("nv_cone_pixel_shader_texture");
}

void PixelShader::createNvConeShader_NoTexture()
{
	createPixelShader("nv_cone_pixel_shader_notexture");
}

void PixelShader::createLightingPixelShader_Lightmap()
{
	createPixelShader("lighting_pixel_shader_lmap");
}

void PixelShader::createLightingPixelShader_Lightmap_Reflection()
{
	createPixelShader("lighting_pixel_shader_lmap_reflection");
}

void PixelShader::createLightingPixelShader_Lightmap_LocalReflection()
{
	createPixelShader("lighting_pixel_shader_lmap_localreflection");
}

void PixelShader::createLightingPixelShader_LightmapNoTexture()
{
	createPixelShader("lighting_pixel_shader_lmap_notexture");
}

void PixelShader::createLightingPixelShader_NoLightmap()
{
	createPixelShader("lighting_pixel_shader_no_lmap");
}

void PixelShader::createLightingPixelShader_NoLightmap_Reflection()
{
	createPixelShader("lighting_pixel_shader_no_lmap_reflection");
}

void PixelShader::createLightingPixelShader_NoLightmap_LocalReflection()
{
	createPixelShader("lighting_pixel_shader_no_lmap_localreflection");
}

void PixelShader::createLightingPixelShader_NoLightmapNoTexture()
{
	createPixelShader("lighting_pixel_shader_no_lmap_notexture");
}

void PixelShader::createFakeDepthPixelShader()
{
	createPixelShader("fake_depth_pixel_shader");
}

void PixelShader::createFakeShadowPixelShader()
{
	createPixelShader("fake_shadow_pixel_shader");
}

void PixelShader::createDecalPixelShader()
{
	createPixelShader("decal_pixel_shader");
}

void PixelShader::createColorEffectPixelShader()
{
	createPixelShader("color_effect_pixel_shader");
}

void PixelShader::createColorEffectOffsetPixelShader()
{
	createPixelShader("color_effect_offset_pixel_shader");
}

void PixelShader::createColorEffectOffsetPixelShader_NoGamma()
{
	createPixelShader("color_effect_offset_pixel_shader_nogamma");
}

void PixelShader::createProceduralShader()
{
	createPixelShader("procedural_pixel_shader");
}

void PixelShader::createProceduralOffsetShader()
{
	createPixelShader("procedural_offset_pixel_shader");
}

void PixelShader::createProceduralOffsetBaseShader()
{
	createPixelShader("procedural_offset_base_pixel_shader");
}

void PixelShader::createBlackWhiteShader()
{
	createPixelShader("black_white_effect_pixel_shader");
}

void PixelShader::createOffsetBlendShader()
{
	createPixelShader("offset_blend_pixel_shader");
}

void PixelShader::createSkyboxShader()
{
	createPixelShader("skybox_pixel_shader");
}

void PixelShader::apply() const
{
	glEnable(GL_FRAGMENT_PROGRAM_ARB);
	glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, handle);
	activePixelShader = nm;
	if (tracing) {
		igiosWarning("activated pixel shader %s\n", nm.c_str());
	}
}

bool PixelShader::hasShader() const
{
	if(glIsProgramARB(handle))
		return true;

    return false;
}

// --

//! Constructor
VertexBuffer::VertexBuffer()
:	buffer(0),
	vertexSize(0),
	vertexAmount(0),
	dynamic(false)
{
}

//! Destructor
VertexBuffer::~VertexBuffer()
{
	release();
}

//! Release vertex buffer
void VertexBuffer::release()
{
	if(buffer != 0 && glIsBuffer(buffer))
	{
		glDeleteBuffers(1, &buffer);
		buffer = 0;
	}
}

//! Create vertex buffer
/*!
	\param faceAmount_ number of faces
	\param dynamic_ is index buffer dynamic
*/
void VertexBuffer::create(int vertexAmount_, int vertexSize_, bool dynamic_)
{
	if (buffer == 0) {
		glGenBuffers(1, &buffer);
	}

	vertexSize = vertexSize_;
	vertexAmount = vertexAmount_;
	dynamic = dynamic_;

	glBindBuffer(GL_ARRAY_BUFFER, buffer);

	if(dynamic)
		glBufferData(GL_ARRAY_BUFFER, vertexAmount * vertexSize, NULL, GL_DYNAMIC_DRAW);
	else
		glBufferData(GL_ARRAY_BUFFER, vertexAmount * vertexSize, NULL, GL_STREAM_DRAW);
}

//! Lock vertex buffer
/*!
	\return pointer to vertex buffer
*/
void *VertexBuffer::lock()
{
	void *pointer = 0;

	glBindBuffer(GL_ARRAY_BUFFER, buffer);

	if(dynamic) {  // discard old
		glBufferData(GL_ARRAY_BUFFER, vertexAmount * vertexSize, NULL, GL_DYNAMIC_DRAW);
	}
	pointer = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);

	return pointer;
}

//! Lock vertex buffer
/*!
	\param offset
	\param amount
	\return pointer to vertex buffer
*/
void *VertexBuffer::unsafeLock(int offset, int amount)
{
	void *pointer = 0;

	glBindBuffer(GL_ARRAY_BUFFER, buffer);
	pointer = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);

	igios_unimplemented();
	return pointer;
}

//! Unlock vertex buffer
void VertexBuffer::unlock()
{
	glUnmapBuffer(GL_ARRAY_BUFFER);
}

//! Apply vertex buffer
/*!
	\param stream stream
*/
void VertexBuffer::apply(int stream) const
{
	setStreamSource(stream, buffer, 0, vertexSize);
}

//! Apply vertex buffer
/*!
	\param stream stream
*/
void VertexBuffer::apply(int stream, int offset) const
{
	setStreamSource(stream, buffer, offset, vertexSize);
}

VertexBuffer::operator bool() const
{
	if (buffer != 0 && glIsBuffer(buffer))
		return true;
	return false;
}

// --

//! Constructor
IndexBuffer::IndexBuffer()
:	buffer(0),
	faceAmount(0),
	dynamic(false),
	logger(0)
{
}

//! Destructor
IndexBuffer::~IndexBuffer()
{
}

//! Set logger to index buffer
/*!
	\param logger_ logger
*/
void IndexBuffer::setLogger(IStorm3D_Logger *logger_)
{
	logger = logger_;
}

//! Release index buffer
void IndexBuffer::release()
{
	if(buffer != 0 && glIsBuffer(buffer))
	{
		glDeleteBuffers(1, &buffer);
		buffer = 0;
	}
}

//! Create index buffer
/*!
	\param faceAmount_ number of faces
	\param dynamic_ is index buffer dynamic
*/
void IndexBuffer::create(int faceAmount_, bool dynamic_)
{
	faceAmount = faceAmount_;
	dynamic = dynamic_;
	if (buffer == 0) {
		glGenBuffers(1, &buffer);
	}

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer);

	if(dynamic)
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, faceAmount * 3 * sizeof(GLushort), NULL, GL_DYNAMIC_DRAW);
	else
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, faceAmount * 3 * sizeof(GLushort), NULL, GL_STREAM_DRAW);
}

//! Lock index buffer
/*!
	\return pointer to index buffer
*/
unsigned short *IndexBuffer::lock()
{
	void *pointer = 0;

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer);
	if(dynamic) {  // discard old
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, faceAmount * 3 * sizeof(GLushort), NULL, GL_DYNAMIC_DRAW);
	}
	pointer = glMapBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_WRITE_ONLY);

	return reinterpret_cast<unsigned short *> (pointer);
}

//! Unlock index buffer
void IndexBuffer::unlock()
{
	glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);
}

//! Render index buffer contents
/*!
	\param faceAmount number of faces to render
	\param maxIndex maximum index buffer index
	\param vertexOffset offset to add to each index (FIXME: broken)
	\param startIndex first index buffer index
*/
void IndexBuffer::render(int faceAmount, int maxIndex, int vertexOffset, int startIndex) const
{
	if (vertexOffset != 0) {
		igiosWarning("IndexBuffer::render warning: vertexOffset != 0\n");
		igios_backtrace();
	}
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer);
	glDrawRangeElements(GL_TRIANGLES, startIndex, maxIndex, faceAmount*3, GL_UNSIGNED_SHORT, NULL);

	++storm3d_dip_calls;
}

IndexBuffer::operator bool() const
{
	if (glIsBuffer(buffer))
		return true;
	return false;
}

// --

//! Create shared texture
/*!
	\param texture texture
	\return shared texture
*/
boost::shared_ptr<Storm3D_Texture> createSharedTexture(Storm3D_Texture *texture)
{
	if(texture)
	{
		texture->AddRef();
		return boost::shared_ptr<Storm3D_Texture> (texture, std::mem_fun(&Storm3D_Texture::Release));
	}

	return boost::shared_ptr<Storm3D_Texture> (texture);
}


static bool inverseCulling = false;

//! Set inverse culling
/*!
	\param enable true to set inverse culling
*/
void setInverseCulling(bool enable)
{
	inverseCulling = enable;
}

//! Set cull mode
/*!
	\param type cull mode
*/
void setCulling(CULLMODE type)
{
	if(type == CULL_NONE)
		glDisable(GL_CULL_FACE);
	else
	{
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);
		if(type == CULL_CCW)
		{
			if(!inverseCulling)
				glFrontFace(GL_CCW);
			else
				glFrontFace(GL_CW);
		}
		else if(type == CULL_CW)
		{
			if(!inverseCulling)
				glFrontFace(GL_CW);
			else
				glFrontFace(GL_CCW);
		}
	}
}

} // storm
} // frozenbyte
