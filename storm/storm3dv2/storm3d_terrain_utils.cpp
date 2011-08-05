// Copyright 2002-2004 Frozenbyte Ltd.

#ifdef _MSC_VER
#pragma warning(disable:4103)
#endif

#include <fstream>
#include <vector>
#include <d3d9.h>
#include <stdio.h>

#include "storm3d_terrain_utils.h"
#include "storm3d_texture.h"
#include "IStorm3D_Logger.h"

extern int storm3d_dip_calls;

#include "../../filesystem/input_stream.h"
#include "../../filesystem/file_package_manager.h"
#include "../../util/Debug_MemoryManager.h"

#ifndef NDEBUG
static std::string activePixelShader;
static std::string activeVertexShader;
static bool tracing = false;

void activeShaderNames() {
	fprintf(stderr, "vertex: %s\npixel: %s\n", activeVertexShader.c_str(), activePixelShader.c_str());
}

void setTracing(bool tracing_) {
	if (tracing != tracing_) {
		tracing = tracing_;
		fprintf(stderr, "Tracing %s\nCurrent shaders:", tracing_ ? "enabled" : "disabled");
		activeShaderNames();
	}
}

#endif

namespace frozenbyte {
namespace storm {

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

		/*
		std::ifstream stream(fileName.c_str());
		while(stream)
		{
			std::string line;
			std::getline(stream, line);

			result += line;
			result += "\r\n";
		}
		*/
	}

	CComPtr<IDirect3DVertexShader9> VertexShader::createVertexShader(IDirect3DDevice9 &device, const std::string &fileName)
	{
		std::string shaderString;
		name = fileName;
		readFile(shaderString, fileName);

		ID3DXBuffer *assembledShader = 0;
		ID3DXBuffer *compilationErrors = 0;

		HRESULT hr = 0;

		if(!fileName.empty())
		{
			hr = D3DXAssembleShader(shaderString.c_str(), shaderString.size(), 0, 0, 0, &assembledShader, &compilationErrors);
			if(FAILED(hr))
			{
				char *foo = (char *) compilationErrors->GetBufferPointer();
				OutputDebugString(foo);

#ifdef LEGACY_FILES
				FILE *fp = fopen("vertex_shader_dbg.txt", "w");
#else
				FILE *fp = fopen("logs/vertex_shader_dbg.txt", "w");
#endif
				if (fp != NULL)
				{
					fprintf(fp, "File: %s\r\n", fileName.c_str());
					fprintf(fp, "Output: %s\r\n", foo);
					fclose(fp);
				}

				assert(!MessageBox(NULL, "D3DXAssembleShader failed!", "Shit happens", MB_OK));
			}
		}

		if(!assembledShader)
			return 0;

		CComPtr<IDirect3DVertexShader9> result;
		DWORD *buffer = (assembledShader) ? (DWORD*)assembledShader->GetBufferPointer() : 0;

		hr = device.CreateVertexShader(buffer, &result);
		if(FAILED(hr))
		{
			assert(MessageBox(NULL, "CreateVertexShader failed!", "Shit happens", MB_OK));
		}

		return result;
	}

	CComPtr<IDirect3DPixelShader9> PixelShader::createPixelShader(IDirect3DDevice9 &device, const std::string &fileName)
	{
		std::string shaderString;
		readFile(shaderString, fileName);

		name = fileName;

		ID3DXBuffer *assembledShader = 0;
		ID3DXBuffer *compilationErrors = 0;

		HRESULT hr = D3DXAssembleShader(shaderString.c_str(), shaderString.size(), 0, 0, 0, &assembledShader, &compilationErrors);
		if(FAILED(hr))
		{
			char *foo = (char *) compilationErrors->GetBufferPointer();

#ifdef LEGACY_FILES
			FILE *fp = fopen("pixel_shader_dbg.txt", "w");
#else
			FILE *fp = fopen("logs/pixel_shader_dbg.txt", "w");
#endif
			if (fp != NULL)
			{
				fprintf(fp, "File: %s\r\n", fileName.c_str());
				fprintf(fp, "Output: %s\r\n", foo);
				fclose(fp);
			}

			assert(!MessageBox(NULL, "D3DXAssembleShader failed!", "Shit happens", MB_OK));
		}

		if(!assembledShader)
			return 0;

		CComPtr<IDirect3DPixelShader9> result;
		hr = device.CreatePixelShader((DWORD*)assembledShader->GetBufferPointer(), &result);

		if(FAILED(hr))
		{
			assert(MessageBox(NULL, "CreatePixelShader failed!", "Shit happens", MB_OK));
		}

		return result;
	}

	D3DVERTEXELEMENT9 createElement(int stream, int offset, int type, int usage, int index = 0)
	{
		D3DVERTEXELEMENT9 result;
		result.Stream = stream;
		result.Offset = offset;
		result.Type = type;
		result.Usage = usage;
		result.Method = 0;
		result.UsageIndex = index;

		return result;
	}

	static D3DVERTEXELEMENT9 end = D3DDECL_END();

VertexShader::VertexShader(IDirect3DDevice9 &device_)
:	handle(0),
	device(device_)
{
}

VertexShader::~VertexShader()
{
}

void VertexShader::create2DShader2Tex()
{
	elements.clear();
	elements.push_back(createElement(0, 0, D3DDECLTYPE_FLOAT4, D3DDECLUSAGE_POSITION));
	elements.push_back(createElement(0, 4*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 0));
	elements.push_back(createElement(0, 6*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 1));
	elements.push_back(end);

#ifdef LEGACY_FILES
	handle = createVertexShader(device, "Data\\Shaders\\2d_vertex_shader_2tex.txt");
#else
	handle = createVertexShader(device, "data\\shader\\2d_vertex_shader_2tex.fvs");
#endif
	declaration = 0;
	device.CreateVertexDeclaration(&elements[0], &declaration);
}

void VertexShader::createTerrainShader()
{
	elements.clear();
	elements.push_back(createElement(0, 0, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_POSITION));
	elements.push_back(createElement(0, 3*4, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_NORMAL));
	elements.push_back(createElement(1, 0, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 0));
	elements.push_back(createElement(1, 2*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 1));
	elements.push_back(createElement(1, 4*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 2));
	elements.push_back(end);

	//handle = createVertexShader(device, "");
	declaration = 0;
	device.CreateVertexDeclaration(&elements[0], &declaration);
}

void VertexShader::createAtiTerrainShader()
{
	elements.clear();
	elements.push_back(createElement(0, 0, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_POSITION));
	elements.push_back(createElement(0, 3*4, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_NORMAL));
	elements.push_back(createElement(1, 0, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 0));
	elements.push_back(createElement(1, 2*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 1));
	elements.push_back(createElement(1, 4*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 2));
	elements.push_back(end);

#ifdef LEGACY_FILES
	handle = createVertexShader(device, "Data\\Shaders\\ati_terrain_vertex_shader.txt");
#else
	handle = createVertexShader(device, "data\\shader\\ati_terrain_vertex_shader.fvs");
#endif
	declaration = 0;
	device.CreateVertexDeclaration(&elements[0], &declaration);
}

void VertexShader::createAtiLightingShader()
{
	elements.clear();
	elements.push_back(createElement(0, 0, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_POSITION));
	elements.push_back(createElement(0, 3*4, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_NORMAL));
	elements.push_back(createElement(1, 0, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 0));
	elements.push_back(createElement(1, 2*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 1));
	elements.push_back(createElement(1, 4*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 2));
	elements.push_back(end);

#ifdef LEGACY_FILES
	handle = createVertexShader(device, "Data\\Shaders\\ati_terrain_lighting_vertex_shader.txt");
#else
	handle = createVertexShader(device, "data\\shader\\ati_terrain_lighting_vertex_shader.fvs");
#endif
	declaration = 0;
	device.CreateVertexDeclaration(&elements[0], &declaration);
}

void VertexShader::createNvTerrainShader()
{
	elements.clear();
	elements.push_back(createElement(0, 0, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_POSITION));
	elements.push_back(createElement(0, 3*4, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_NORMAL));
	elements.push_back(createElement(1, 0, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 0));
	elements.push_back(createElement(1, 2*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 1));
	elements.push_back(createElement(1, 4*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 2));
	elements.push_back(end);

#ifdef LEGACY_FILES
	handle = createVertexShader(device, "Data\\Shaders\\nv_terrain_vertex_shader.txt");
#else
	handle = createVertexShader(device, "data\\shader\\nv_terrain_vertex_shader.fvs");
#endif
	declaration = 0;
	device.CreateVertexDeclaration(&elements[0], &declaration);
}

void VertexShader::createNvLightingShader()
{
	elements.clear();
	elements.push_back(createElement(0, 0, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_POSITION));
	elements.push_back(createElement(0, 3*4, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_NORMAL));
	elements.push_back(createElement(1, 0, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 0));
	elements.push_back(createElement(1, 2*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 1));
	elements.push_back(createElement(1, 4*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 2));
	elements.push_back(end);

#ifdef LEGACY_FILES
	handle = createVertexShader(device, "Data\\Shaders\\nv_terrain_lighting_vertex_shader.txt");
#else
	handle = createVertexShader(device, "data\\shader\\nv_terrain_lighting_vertex_shader.fvs");
#endif
	declaration = 0;
	device.CreateVertexDeclaration(&elements[0], &declaration);
}

void VertexShader::createDefaultShader()
{
	elements.clear();
	elements.push_back(createElement(0, 0, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_POSITION));
	elements.push_back(createElement(0, 3*4, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_NORMAL));
	elements.push_back(createElement(0, 6*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 0));
	elements.push_back(createElement(0, 8*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 1));
	elements.push_back(end);

#ifdef LEGACY_FILES
	handle = createVertexShader(device, "Data\\Shaders\\default_shader.txt");
#else
	handle = createVertexShader(device, "data\\shader\\default_shader.fvs");
#endif
	declaration = 0;
	device.CreateVertexDeclaration(&elements[0], &declaration);
}

/*
void VertexShader::createLightingShader()
{
	elements.clear();
	elements.push_back(createElement(0, 0, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_POSITION));
	elements.push_back(createElement(0, 3*4, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_NORMAL));
	elements.push_back(createElement(0, 6*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 0));
	elements.push_back(createElement(0, 8*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 1));
	elements.push_back(end);

	handle = createVertexShader(device, "Data\\Shaders\\lighting_shader.txt");
	declaration = 0;
	device.CreateVertexDeclaration(&elements[0], &declaration);
}
*/

void VertexShader::createLightingShader_0light_noreflection()
{
	elements.clear();
	elements.push_back(createElement(0, 0, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_POSITION));
	elements.push_back(createElement(0, 3*4, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_NORMAL));
	elements.push_back(createElement(0, 6*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 0));
	elements.push_back(createElement(0, 8*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 1));
	elements.push_back(end);

#ifdef LEGACY_FILES
	handle = createVertexShader(device, "Data\\Shaders\\lighting_shader_0light_noreflection.txt");
#else
	handle = createVertexShader(device, "data\\shader\\lighting_shader_0light_noreflection.fvs");
#endif
	declaration = 0;
	device.CreateVertexDeclaration(&elements[0], &declaration);
}

void VertexShader::createLightingShader_0light_localreflection()
{
	elements.clear();
	elements.push_back(createElement(0, 0, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_POSITION));
	elements.push_back(createElement(0, 3*4, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_NORMAL));
	elements.push_back(createElement(0, 6*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 0));
	elements.push_back(createElement(0, 8*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 1));
	elements.push_back(end);

#ifdef LEGACY_FILES
	handle = createVertexShader(device, "Data\\Shaders\\lighting_shader_0light_localreflection.txt");
#else
	handle = createVertexShader(device, "data\\shader\\lighting_shader_0light_localreflection.fvs");
#endif
	declaration = 0;
	device.CreateVertexDeclaration(&elements[0], &declaration);
}

void VertexShader::createLightingShader_0light_reflection()
{
	elements.clear();
	elements.push_back(createElement(0, 0, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_POSITION));
	elements.push_back(createElement(0, 3*4, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_NORMAL));
	elements.push_back(createElement(0, 6*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 0));
	elements.push_back(createElement(0, 8*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 1));
	elements.push_back(end);

#ifdef LEGACY_FILES
	handle = createVertexShader(device, "Data\\Shaders\\lighting_shader_0light_reflection.txt");
#else
	handle = createVertexShader(device, "data\\shader\\lighting_shader_0light_reflection.fvs");
#endif
	declaration = 0;
	device.CreateVertexDeclaration(&elements[0], &declaration);
}

void VertexShader::createLightingShader_1light_noreflection()
{
	elements.clear();
	elements.push_back(createElement(0, 0, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_POSITION));
	elements.push_back(createElement(0, 3*4, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_NORMAL));
	elements.push_back(createElement(0, 6*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 0));
	elements.push_back(createElement(0, 8*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 1));
	elements.push_back(end);

#ifdef LEGACY_FILES
	handle = createVertexShader(device, "Data\\Shaders\\lighting_shader_1light_noreflection.txt");
#else
	handle = createVertexShader(device, "data\\shader\\lighting_shader_1light_noreflection.fvs");
#endif
	declaration = 0;
	device.CreateVertexDeclaration(&elements[0], &declaration);
}

void VertexShader::createLightingShader_1light_localreflection()
{
	elements.clear();
	elements.push_back(createElement(0, 0, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_POSITION));
	elements.push_back(createElement(0, 3*4, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_NORMAL));
	elements.push_back(createElement(0, 6*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 0));
	elements.push_back(createElement(0, 8*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 1));
	elements.push_back(end);

#ifdef LEGACY_FILES
	handle = createVertexShader(device, "Data\\Shaders\\lighting_shader_1light_localreflection.txt");
#else
	handle = createVertexShader(device, "data\\shader\\lighting_shader_1light_localreflection.fvs");
#endif
	declaration = 0;
	device.CreateVertexDeclaration(&elements[0], &declaration);
}

void VertexShader::createLightingShader_1light_reflection()
{
	elements.clear();
	elements.push_back(createElement(0, 0, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_POSITION));
	elements.push_back(createElement(0, 3*4, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_NORMAL));
	elements.push_back(createElement(0, 6*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 0));
	elements.push_back(createElement(0, 8*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 1));
	elements.push_back(end);

#ifdef LEGACY_FILES
	handle = createVertexShader(device, "Data\\Shaders\\lighting_shader_1light_reflection.txt");
#else
	handle = createVertexShader(device, "data\\shader\\lighting_shader_1light_reflection.fvs");
#endif
	declaration = 0;
	device.CreateVertexDeclaration(&elements[0], &declaration);
}

void VertexShader::createLightingShader_2light_noreflection()
{
	elements.clear();
	elements.push_back(createElement(0, 0, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_POSITION));
	elements.push_back(createElement(0, 3*4, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_NORMAL));
	elements.push_back(createElement(0, 6*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 0));
	elements.push_back(createElement(0, 8*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 1));
	elements.push_back(end);

#ifdef LEGACY_FILES
	handle = createVertexShader(device, "Data\\Shaders\\lighting_shader_2light_noreflection.txt");
#else
	handle = createVertexShader(device, "data\\shader\\lighting_shader_2light_noreflection.fvs");
#endif
	declaration = 0;
	device.CreateVertexDeclaration(&elements[0], &declaration);
}

void VertexShader::createLightingShader_2light_localreflection()
{
	elements.clear();
	elements.push_back(createElement(0, 0, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_POSITION));
	elements.push_back(createElement(0, 3*4, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_NORMAL));
	elements.push_back(createElement(0, 6*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 0));
	elements.push_back(createElement(0, 8*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 1));
	elements.push_back(end);

#ifdef LEGACY_FILES
	handle = createVertexShader(device, "Data\\Shaders\\lighting_shader_2light_localreflection.txt");
#else
	handle = createVertexShader(device, "data\\shader\\lighting_shader_2light_localreflection.fvs");
#endif
	declaration = 0;
	device.CreateVertexDeclaration(&elements[0], &declaration);
}

void VertexShader::createLightingShader_2light_reflection()
{
	elements.clear();
	elements.push_back(createElement(0, 0, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_POSITION));
	elements.push_back(createElement(0, 3*4, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_NORMAL));
	elements.push_back(createElement(0, 6*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 0));
	elements.push_back(createElement(0, 8*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 1));
	elements.push_back(end);

#ifdef LEGACY_FILES
	handle = createVertexShader(device, "Data\\Shaders\\lighting_shader_2light_reflection.txt");
#else
	handle = createVertexShader(device, "data\\shader\\lighting_shader_2light_reflection.fvs");
#endif
	declaration = 0;
	device.CreateVertexDeclaration(&elements[0], &declaration);
}

void VertexShader::createLightingShader_3light_noreflection()
{
	elements.clear();
	elements.push_back(createElement(0, 0, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_POSITION));
	elements.push_back(createElement(0, 3*4, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_NORMAL));
	elements.push_back(createElement(0, 6*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 0));
	elements.push_back(createElement(0, 8*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 1));
	elements.push_back(end);

#ifdef LEGACY_FILES
	handle = createVertexShader(device, "Data\\Shaders\\lighting_shader_3light_noreflection.txt");
#else
	handle = createVertexShader(device, "data\\shader\\lighting_shader_3light_noreflection.fvs");
#endif
	declaration = 0;
	device.CreateVertexDeclaration(&elements[0], &declaration);
}

void VertexShader::createLightingShader_3light_localreflection()
{
	elements.clear();
	elements.push_back(createElement(0, 0, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_POSITION));
	elements.push_back(createElement(0, 3*4, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_NORMAL));
	elements.push_back(createElement(0, 6*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 0));
	elements.push_back(createElement(0, 8*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 1));
	elements.push_back(end);

#ifdef LEGACY_FILES
	handle = createVertexShader(device, "Data\\Shaders\\lighting_shader_3light_localreflection.txt");
#else
	handle = createVertexShader(device, "data\\shader\\lighting_shader_3light_localreflection.fvs");
#endif
	declaration = 0;
	device.CreateVertexDeclaration(&elements[0], &declaration);
}

void VertexShader::createLightingShader_3light_reflection()
{
	elements.clear();
	elements.push_back(createElement(0, 0, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_POSITION));
	elements.push_back(createElement(0, 3*4, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_NORMAL));
	elements.push_back(createElement(0, 6*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 0));
	elements.push_back(createElement(0, 8*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 1));
	elements.push_back(end);

#ifdef LEGACY_FILES
	handle = createVertexShader(device, "Data\\Shaders\\lighting_shader_3light_reflection.txt");
#else
	handle = createVertexShader(device, "data\\shader\\lighting_shader_3light_reflection.fvs");
#endif
	declaration = 0;
	device.CreateVertexDeclaration(&elements[0], &declaration);
}

void VertexShader::createLightingShader_4light_noreflection()
{
	elements.clear();
	elements.push_back(createElement(0, 0, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_POSITION));
	elements.push_back(createElement(0, 3*4, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_NORMAL));
	elements.push_back(createElement(0, 6*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 0));
	elements.push_back(createElement(0, 8*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 1));
	elements.push_back(end);

#ifdef LEGACY_FILES
	handle = createVertexShader(device, "Data\\Shaders\\lighting_shader_4light_noreflection.txt");
#else
	handle = createVertexShader(device, "data\\shader\\lighting_shader_4light_noreflection.fvs");
#endif
	declaration = 0;
	device.CreateVertexDeclaration(&elements[0], &declaration);
}

void VertexShader::createLightingShader_4light_localreflection()
{
	elements.clear();
	elements.push_back(createElement(0, 0, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_POSITION));
	elements.push_back(createElement(0, 3*4, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_NORMAL));
	elements.push_back(createElement(0, 6*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 0));
	elements.push_back(createElement(0, 8*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 1));
	elements.push_back(end);

#ifdef LEGACY_FILES
	handle = createVertexShader(device, "Data\\Shaders\\lighting_shader_4light_localreflection.txt");
#else
	handle = createVertexShader(device, "data\\shader\\lighting_shader_4light_localreflection.fvs");
#endif
	declaration = 0;
	device.CreateVertexDeclaration(&elements[0], &declaration);
}

void VertexShader::createLightingShader_4light_reflection()
{
	elements.clear();
	elements.push_back(createElement(0, 0, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_POSITION));
	elements.push_back(createElement(0, 3*4, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_NORMAL));
	elements.push_back(createElement(0, 6*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 0));
	elements.push_back(createElement(0, 8*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 1));
	elements.push_back(end);

#ifdef LEGACY_FILES
	handle = createVertexShader(device, "Data\\Shaders\\lighting_shader_4light_reflection.txt");
#else
	handle = createVertexShader(device, "data\\shader\\lighting_shader_4light_reflection.fvs");
#endif
	declaration = 0;
	device.CreateVertexDeclaration(&elements[0], &declaration);
}

void VertexShader::createLightingShader_5light_noreflection()
{
	elements.clear();
	elements.push_back(createElement(0, 0, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_POSITION));
	elements.push_back(createElement(0, 3*4, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_NORMAL));
	elements.push_back(createElement(0, 6*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 0));
	elements.push_back(createElement(0, 8*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 1));
	elements.push_back(end);

#ifdef LEGACY_FILES
	handle = createVertexShader(device, "Data\\Shaders\\lighting_shader_5light_noreflection.txt");
#else
	handle = createVertexShader(device, "data\\shader\\lighting_shader_5light_noreflection.fvs");
#endif
	declaration = 0;
	device.CreateVertexDeclaration(&elements[0], &declaration);
}

void VertexShader::createLightingShader_5light_localreflection()
{
	elements.clear();
	elements.push_back(createElement(0, 0, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_POSITION));
	elements.push_back(createElement(0, 3*4, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_NORMAL));
	elements.push_back(createElement(0, 6*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 0));
	elements.push_back(createElement(0, 8*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 1));
	elements.push_back(end);

#ifdef LEGACY_FILES
	handle = createVertexShader(device, "Data\\Shaders\\lighting_shader_5light_localreflection.txt");
#else
	handle = createVertexShader(device, "data\\shader\\lighting_shader_5light_localreflection.fvs");
#endif
	declaration = 0;
	device.CreateVertexDeclaration(&elements[0], &declaration);
}

void VertexShader::createLightingShader_5light_reflection()
{
	elements.clear();
	elements.push_back(createElement(0, 0, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_POSITION));
	elements.push_back(createElement(0, 3*4, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_NORMAL));
	elements.push_back(createElement(0, 6*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 0));
	elements.push_back(createElement(0, 8*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 1));
	elements.push_back(end);

#ifdef LEGACY_FILES
	handle = createVertexShader(device, "Data\\Shaders\\lighting_shader_5light_reflection.txt");
#else
	handle = createVertexShader(device, "data\\shader\\lighting_shader_5light_reflection.fvs");
#endif
	declaration = 0;
	device.CreateVertexDeclaration(&elements[0], &declaration);
}

void VertexShader::createSkyboxShader()
{
	elements.clear();
	elements.push_back(createElement(0, 0, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_POSITION));
	elements.push_back(createElement(0, 3*4, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_NORMAL));
	elements.push_back(createElement(0, 6*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 0));
	elements.push_back(createElement(0, 8*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 1));
	elements.push_back(end);

#ifdef LEGACY_FILES
	handle = createVertexShader(device, "Data\\Shaders\\skybox_shader.txt");
#else
	handle = createVertexShader(device, "data\\shader\\skybox_shader.fvs");
#endif
	declaration = 0;
	device.CreateVertexDeclaration(&elements[0], &declaration);
}

void VertexShader::createDefaultProjectionShaderDirectional()
{
	elements.clear();
	elements.push_back(createElement(0, 0, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_POSITION));
	elements.push_back(createElement(0, 3*4, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_NORMAL));
	elements.push_back(createElement(0, 6*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 0));
	elements.push_back(createElement(0, 8*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 1));
	elements.push_back(end);

#ifdef LEGACY_FILES
	handle = createVertexShader(device, "Data\\Shaders\\default_projection_shader_dir.txt");
#else
	handle = createVertexShader(device, "data\\shader\\default_projection_shader_dir.fvs");
#endif
	declaration = 0;
	device.CreateVertexDeclaration(&elements[0], &declaration);
}

void VertexShader::createDefaultProjectionShaderPoint()
{
	elements.clear();
	elements.push_back(createElement(0, 0, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_POSITION));
	elements.push_back(createElement(0, 3*4, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_NORMAL));
	elements.push_back(createElement(0, 6*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 0));
	elements.push_back(createElement(0, 8*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 1));
	elements.push_back(end);

#ifdef LEGACY_FILES
	handle = createVertexShader(device, "Data\\Shaders\\default_projection_shader_point.txt");
#else
	handle = createVertexShader(device, "data\\shader\\default_projection_shader_point.fvs");
#endif
	declaration = 0;
	device.CreateVertexDeclaration(&elements[0], &declaration);
}

void VertexShader::createDefaultProjectionShaderFlat()
{
	elements.clear();
	elements.push_back(createElement(0, 0, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_POSITION));
	elements.push_back(createElement(0, 3*4, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_NORMAL));
	elements.push_back(createElement(0, 6*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 0));
	elements.push_back(createElement(0, 8*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 1));
	elements.push_back(end);

#ifdef LEGACY_FILES
	handle = createVertexShader(device, "Data\\Shaders\\default_projection_shader_flat.txt");
#else
	handle = createVertexShader(device, "data\\shader\\default_projection_shader_flat.fvs");
#endif
	declaration = 0;
	device.CreateVertexDeclaration(&elements[0], &declaration);
}

void VertexShader::createBoneShader()
{
	elements.push_back(createElement(0, 0, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_POSITION));
	elements.push_back(createElement(0, 3*4, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_NORMAL));
	elements.push_back(createElement(0, 6*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 0));
	elements.push_back(createElement(0, 8*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 1));
	elements.push_back(createElement(0, 10*4, D3DDECLTYPE_FLOAT4, D3DDECLUSAGE_TEXCOORD, 2));
	elements.push_back(end);

#ifdef LEGACY_FILES
	handle = createVertexShader(device, "Data\\Shaders\\bone_shader_1.txt");
#else
	handle = createVertexShader(device, "data\\shader\\bone_shader_1.fvs");
#endif
	declaration = 0;
	device.CreateVertexDeclaration(&elements[0], &declaration);
}


void VertexShader::createBasicBoneLightingShader()
{
	elements.push_back(createElement(0, 0, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_POSITION));
	elements.push_back(createElement(0, 3*4, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_NORMAL));
	elements.push_back(createElement(0, 6*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 0));
	elements.push_back(createElement(0, 8*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 1));
	elements.push_back(createElement(0, 10*4, D3DDECLTYPE_FLOAT4, D3DDECLUSAGE_TEXCOORD, 2));
	elements.push_back(end);

#ifdef LEGACY_FILES
	handle = createVertexShader(device, "Data\\Shaders\\basic_bone_lighting_shader_1.txt");
#else
	handle = createVertexShader(device, "data\\shader\\basic_bone_lighting_shader_1.fvs");
#endif
	declaration = 0;
	device.CreateVertexDeclaration(&elements[0], &declaration);
}

/*
void VertexShader::createBoneLightingShader()
{
	elements.push_back(createElement(0, 0, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_POSITION));
	elements.push_back(createElement(0, 3*4, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_NORMAL));
	elements.push_back(createElement(0, 6*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 0));
	elements.push_back(createElement(0, 8*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 1));
	elements.push_back(createElement(0, 10*4, D3DDECLTYPE_FLOAT4, D3DDECLUSAGE_TEXCOORD, 2));
	elements.push_back(end);

	handle = createVertexShader(device, "Data\\Shaders\\bone_lighting_shader_1.txt");
	declaration = 0;
	device.CreateVertexDeclaration(&elements[0], &declaration);
}
*/

void VertexShader::createBoneLightingShader_0light_noreflection()
{
	elements.push_back(createElement(0, 0, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_POSITION));
	elements.push_back(createElement(0, 3*4, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_NORMAL));
	elements.push_back(createElement(0, 6*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 0));
	elements.push_back(createElement(0, 8*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 1));
	elements.push_back(createElement(0, 10*4, D3DDECLTYPE_FLOAT4, D3DDECLUSAGE_TEXCOORD, 2));
	elements.push_back(end);

#ifdef LEGACY_FILES
	handle = createVertexShader(device, "Data\\Shaders\\bone_lighting_shader_0light_noreflection.txt");
#else
	handle = createVertexShader(device, "data\\shader\\bone_lighting_shader_0light_noreflection.fvs");
#endif
	declaration = 0;
	device.CreateVertexDeclaration(&elements[0], &declaration);
}

void VertexShader::createBoneLightingShader_0light_reflection()
{
	elements.push_back(createElement(0, 0, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_POSITION));
	elements.push_back(createElement(0, 3*4, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_NORMAL));
	elements.push_back(createElement(0, 6*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 0));
	elements.push_back(createElement(0, 8*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 1));
	elements.push_back(createElement(0, 10*4, D3DDECLTYPE_FLOAT4, D3DDECLUSAGE_TEXCOORD, 2));
	elements.push_back(end);

#ifdef LEGACY_FILES
	handle = createVertexShader(device, "Data\\Shaders\\bone_lighting_shader_0light_reflection.txt");
#else
	handle = createVertexShader(device, "data\\shader\\bone_lighting_shader_0light_reflection.fvs");
#endif
	declaration = 0;
	device.CreateVertexDeclaration(&elements[0], &declaration);
}

void VertexShader::createBoneLightingShader_1light_noreflection()
{
	elements.push_back(createElement(0, 0, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_POSITION));
	elements.push_back(createElement(0, 3*4, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_NORMAL));
	elements.push_back(createElement(0, 6*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 0));
	elements.push_back(createElement(0, 8*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 1));
	elements.push_back(createElement(0, 10*4, D3DDECLTYPE_FLOAT4, D3DDECLUSAGE_TEXCOORD, 2));
	elements.push_back(end);

#ifdef LEGACY_FILES
	handle = createVertexShader(device, "Data\\Shaders\\bone_lighting_shader_1light_noreflection.txt");
#else
	handle = createVertexShader(device, "data\\shader\\bone_lighting_shader_1light_noreflection.fvs");
#endif
	declaration = 0;
	device.CreateVertexDeclaration(&elements[0], &declaration);
}

void VertexShader::createBoneLightingShader_1light_reflection()
{
	elements.push_back(createElement(0, 0, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_POSITION));
	elements.push_back(createElement(0, 3*4, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_NORMAL));
	elements.push_back(createElement(0, 6*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 0));
	elements.push_back(createElement(0, 8*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 1));
	elements.push_back(createElement(0, 10*4, D3DDECLTYPE_FLOAT4, D3DDECLUSAGE_TEXCOORD, 2));
	elements.push_back(end);

#ifdef LEGACY_FILES
	handle = createVertexShader(device, "Data\\Shaders\\bone_lighting_shader_1light_reflection.txt");
#else
	handle = createVertexShader(device, "data\\shader\\bone_lighting_shader_1light_reflection.fvs");
#endif
	declaration = 0;
	device.CreateVertexDeclaration(&elements[0], &declaration);
}

void VertexShader::createBoneLightingShader_2light_noreflection()
{
	elements.push_back(createElement(0, 0, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_POSITION));
	elements.push_back(createElement(0, 3*4, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_NORMAL));
	elements.push_back(createElement(0, 6*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 0));
	elements.push_back(createElement(0, 8*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 1));
	elements.push_back(createElement(0, 10*4, D3DDECLTYPE_FLOAT4, D3DDECLUSAGE_TEXCOORD, 2));
	elements.push_back(end);

#ifdef LEGACY_FILES
	handle = createVertexShader(device, "Data\\Shaders\\bone_lighting_shader_2light_noreflection.txt");
#else
	handle = createVertexShader(device, "data\\shader\\bone_lighting_shader_2light_noreflection.fvs");
#endif

	declaration = 0;
	device.CreateVertexDeclaration(&elements[0], &declaration);
}

void VertexShader::createBoneLightingShader_2light_reflection()
{
	elements.push_back(createElement(0, 0, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_POSITION));
	elements.push_back(createElement(0, 3*4, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_NORMAL));
	elements.push_back(createElement(0, 6*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 0));
	elements.push_back(createElement(0, 8*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 1));
	elements.push_back(createElement(0, 10*4, D3DDECLTYPE_FLOAT4, D3DDECLUSAGE_TEXCOORD, 2));
	elements.push_back(end);

#ifdef LEGACY_FILES
	handle = createVertexShader(device, "Data\\Shaders\\bone_lighting_shader_2light_reflection.txt");
#else
	handle = createVertexShader(device, "data\\shader\\bone_lighting_shader_2light_reflection.fvs");
#endif
	declaration = 0;
	device.CreateVertexDeclaration(&elements[0], &declaration);
}

void VertexShader::createBoneLightingShader_3light_noreflection()
{
	elements.push_back(createElement(0, 0, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_POSITION));
	elements.push_back(createElement(0, 3*4, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_NORMAL));
	elements.push_back(createElement(0, 6*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 0));
	elements.push_back(createElement(0, 8*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 1));
	elements.push_back(createElement(0, 10*4, D3DDECLTYPE_FLOAT4, D3DDECLUSAGE_TEXCOORD, 2));
	elements.push_back(end);

#ifdef LEGACY_FILES
	handle = createVertexShader(device, "Data\\Shaders\\bone_lighting_shader_3light_noreflection.txt");
#else
	handle = createVertexShader(device, "data\\shader\\bone_lighting_shader_3light_noreflection.fvs");
#endif

	declaration = 0;
	device.CreateVertexDeclaration(&elements[0], &declaration);
}

void VertexShader::createBoneLightingShader_3light_reflection()
{
	elements.push_back(createElement(0, 0, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_POSITION));
	elements.push_back(createElement(0, 3*4, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_NORMAL));
	elements.push_back(createElement(0, 6*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 0));
	elements.push_back(createElement(0, 8*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 1));
	elements.push_back(createElement(0, 10*4, D3DDECLTYPE_FLOAT4, D3DDECLUSAGE_TEXCOORD, 2));
	elements.push_back(end);

#ifdef LEGACY_FILES
	handle = createVertexShader(device, "Data\\Shaders\\bone_lighting_shader_3light_reflection.txt");
#else
	handle = createVertexShader(device, "data\\shader\\bone_lighting_shader_3light_reflection.fvs");
#endif
	declaration = 0;
	device.CreateVertexDeclaration(&elements[0], &declaration);
}

void VertexShader::createBoneLightingShader_4light_noreflection()
{
	elements.push_back(createElement(0, 0, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_POSITION));
	elements.push_back(createElement(0, 3*4, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_NORMAL));
	elements.push_back(createElement(0, 6*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 0));
	elements.push_back(createElement(0, 8*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 1));
	elements.push_back(createElement(0, 10*4, D3DDECLTYPE_FLOAT4, D3DDECLUSAGE_TEXCOORD, 2));
	elements.push_back(end);

#ifdef LEGACY_FILES
	handle = createVertexShader(device, "Data\\Shaders\\bone_lighting_shader_4light_noreflection.txt");
#else
	handle = createVertexShader(device, "data\\shader\\bone_lighting_shader_4light_noreflection.fvs");
#endif

	declaration = 0;
	device.CreateVertexDeclaration(&elements[0], &declaration);
}

void VertexShader::createBoneLightingShader_4light_reflection()
{
	elements.push_back(createElement(0, 0, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_POSITION));
	elements.push_back(createElement(0, 3*4, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_NORMAL));
	elements.push_back(createElement(0, 6*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 0));
	elements.push_back(createElement(0, 8*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 1));
	elements.push_back(createElement(0, 10*4, D3DDECLTYPE_FLOAT4, D3DDECLUSAGE_TEXCOORD, 2));
	elements.push_back(end);

#ifdef LEGACY_FILES
	handle = createVertexShader(device, "Data\\Shaders\\bone_lighting_shader_4light_reflection.txt");
#else
	handle = createVertexShader(device, "data\\shader\\bone_lighting_shader_4light_reflection.fvs");
#endif
	declaration = 0;
	device.CreateVertexDeclaration(&elements[0], &declaration);
}

void VertexShader::createBoneLightingShader_5light_noreflection()
{
	elements.push_back(createElement(0, 0, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_POSITION));
	elements.push_back(createElement(0, 3*4, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_NORMAL));
	elements.push_back(createElement(0, 6*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 0));
	elements.push_back(createElement(0, 8*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 1));
	elements.push_back(createElement(0, 10*4, D3DDECLTYPE_FLOAT4, D3DDECLUSAGE_TEXCOORD, 2));
	elements.push_back(end);

#ifdef LEGACY_FILES
	handle = createVertexShader(device, "Data\\Shaders\\bone_lighting_shader_5light_noreflection.txt");
#else
	handle = createVertexShader(device, "data\\shader\\bone_lighting_shader_5light_noreflection.fvs");
#endif

	declaration = 0;
	device.CreateVertexDeclaration(&elements[0], &declaration);
}

void VertexShader::createBoneLightingShader_5light_reflection()
{
	elements.push_back(createElement(0, 0, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_POSITION));
	elements.push_back(createElement(0, 3*4, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_NORMAL));
	elements.push_back(createElement(0, 6*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 0));
	elements.push_back(createElement(0, 8*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 1));
	elements.push_back(createElement(0, 10*4, D3DDECLTYPE_FLOAT4, D3DDECLUSAGE_TEXCOORD, 2));
	elements.push_back(end);

#ifdef LEGACY_FILES
	handle = createVertexShader(device, "Data\\Shaders\\bone_lighting_shader_5light_reflection.txt");
#else
	handle = createVertexShader(device, "data\\shader\\bone_lighting_shader_5light_reflection.fvs");
#endif
	declaration = 0;
	device.CreateVertexDeclaration(&elements[0], &declaration);
}

void VertexShader::createBoneProjectionShaderDirectional()
{
	elements.push_back(createElement(0, 0, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_POSITION));
	elements.push_back(createElement(0, 3*4, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_NORMAL));
	elements.push_back(createElement(0, 6*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 0));
	elements.push_back(createElement(0, 8*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 1));
	elements.push_back(createElement(0, 10*4, D3DDECLTYPE_FLOAT4, D3DDECLUSAGE_TEXCOORD, 2));
	elements.push_back(end);

#ifdef LEGACY_FILES
	handle = createVertexShader(device, "Data\\Shaders\\bone_projection_shader_dir.txt");
#else
	handle = createVertexShader(device, "data\\shader\\bone_projection_shader_dir.fvs");
#endif
	declaration = 0;
	device.CreateVertexDeclaration(&elements[0], &declaration);
}

void VertexShader::createBoneProjectionShaderPoint()
{
	elements.push_back(createElement(0, 0, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_POSITION));
	elements.push_back(createElement(0, 3*4, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_NORMAL));
	elements.push_back(createElement(0, 6*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 0));
	elements.push_back(createElement(0, 8*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 1));
	elements.push_back(createElement(0, 10*4, D3DDECLTYPE_FLOAT4, D3DDECLUSAGE_TEXCOORD, 2));
	elements.push_back(end);

#ifdef LEGACY_FILES
	handle = createVertexShader(device, "Data\\Shaders\\bone_projection_shader_point.txt");
#else
	handle = createVertexShader(device, "data\\shader\\bone_projection_shader_point.fvs");
#endif
	declaration = 0;
	device.CreateVertexDeclaration(&elements[0], &declaration);
}

void VertexShader::createBoneProjectionShaderFlat()
{
	elements.push_back(createElement(0, 0, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_POSITION));
	elements.push_back(createElement(0, 3*4, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_NORMAL));
	elements.push_back(createElement(0, 6*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 0));
	elements.push_back(createElement(0, 8*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 1));
	elements.push_back(createElement(0, 10*4, D3DDECLTYPE_FLOAT4, D3DDECLUSAGE_TEXCOORD, 2));
	elements.push_back(end);

#ifdef LEGACY_FILES
	handle = createVertexShader(device, "Data\\Shaders\\bone_projection_shader_flat.txt");
#else
	handle = createVertexShader(device, "data\\shader\\bone_projection_shader_flat.fvs");
#endif
	declaration = 0;
	device.CreateVertexDeclaration(&elements[0], &declaration);
}

void VertexShader::createAtiDepthShader()
{
	elements.clear();
	elements.push_back(createElement(0, 0, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_POSITION));
	elements.push_back(createElement(0, 3*4, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_NORMAL));
	elements.push_back(createElement(0, 6*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 0));
	elements.push_back(createElement(0, 8*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 1));
	elements.push_back(end);

#ifdef LEGACY_FILES
	handle = createVertexShader(device, "Data\\Shaders\\ati_depth_default_vertex_shader.txt");
#else
	handle = createVertexShader(device, "data\\shader\\ati_depth_default_vertex_shader.fvs");
#endif
	declaration = 0;
	device.CreateVertexDeclaration(&elements[0], &declaration);
}

void VertexShader::createAtiDepthTerrainShader()
{
	elements.clear();
	elements.push_back(createElement(0, 0, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_POSITION));
	elements.push_back(createElement(0, 3*4, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_NORMAL));
	elements.push_back(createElement(1, 0, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 0));
	elements.push_back(createElement(1, 2*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 1));
	elements.push_back(createElement(1, 4*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 2));
	elements.push_back(end);

#ifdef LEGACY_FILES
	handle = createVertexShader(device, "Data\\Shaders\\ati_depth_default_vertex_shader.txt");
#else
	handle = createVertexShader(device, "data\\shader\\ati_depth_default_vertex_shader.fvs");
#endif
	declaration = 0;
	device.CreateVertexDeclaration(&elements[0], &declaration);
}

void VertexShader::createAtiBoneDepthShader()
{
	elements.clear();
	elements.push_back(createElement(0, 0, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_POSITION));
	elements.push_back(createElement(0, 3*4, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_NORMAL));
	elements.push_back(createElement(0, 6*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 0));
	elements.push_back(createElement(0, 8*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 1));
	elements.push_back(createElement(0, 10*4, D3DDECLTYPE_FLOAT4, D3DDECLUSAGE_TEXCOORD, 2));
	elements.push_back(end);

#ifdef LEGACY_FILES
	handle = createVertexShader(device, "Data\\Shaders\\ati_depth_bone_vertex_shader.txt");
#else
	handle = createVertexShader(device, "data\\shader\\ati_depth_bone_vertex_shader.fvs");
#endif
	declaration = 0;
	device.CreateVertexDeclaration(&elements[0], &declaration);
}

void VertexShader::createAtiShadowShaderDirectional()
{
	elements.clear();
	elements.push_back(createElement(0, 0, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_POSITION));
	elements.push_back(createElement(0, 3*4, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_NORMAL));
	elements.push_back(createElement(0, 6*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 0));
	elements.push_back(createElement(0, 8*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 1));
	elements.push_back(end);

#ifdef LEGACY_FILES
	handle = createVertexShader(device, "Data\\Shaders\\ati_shadow_default_vertex_shader_dir.txt");
#else
	handle = createVertexShader(device, "data\\shader\\ati_shadow_default_vertex_shader_dir.fvs");
#endif
	declaration = 0;
	device.CreateVertexDeclaration(&elements[0], &declaration);
}

void VertexShader::createAtiShadowShaderPoint()
{
	elements.clear();
	elements.push_back(createElement(0, 0, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_POSITION));
	elements.push_back(createElement(0, 3*4, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_NORMAL));
	elements.push_back(createElement(0, 6*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 0));
	elements.push_back(createElement(0, 8*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 1));
	elements.push_back(end);

#ifdef LEGACY_FILES
	handle = createVertexShader(device, "Data\\Shaders\\ati_shadow_default_vertex_shader_point.txt");
#else
	handle = createVertexShader(device, "data\\shader\\ati_shadow_default_vertex_shader_point.fvs");
#endif
	declaration = 0;
	device.CreateVertexDeclaration(&elements[0], &declaration);
}

void VertexShader::createAtiShadowShaderFlat()
{
	elements.clear();
	elements.push_back(createElement(0, 0, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_POSITION));
	elements.push_back(createElement(0, 3*4, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_NORMAL));
	elements.push_back(createElement(0, 6*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 0));
	elements.push_back(createElement(0, 8*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 1));
	elements.push_back(end);

#ifdef LEGACY_FILES
	handle = createVertexShader(device, "Data\\Shaders\\ati_shadow_default_vertex_shader_flat.txt");
#else
	handle = createVertexShader(device, "data\\shader\\ati_shadow_default_vertex_shader_flat.fvs");
#endif
	declaration = 0;
	device.CreateVertexDeclaration(&elements[0], &declaration);
}

void VertexShader::createAtiBoneShadowShaderDirectional()
{
	elements.clear();
	elements.push_back(createElement(0, 0, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_POSITION));
	elements.push_back(createElement(0, 3*4, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_NORMAL));
	elements.push_back(createElement(0, 6*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 0));
	elements.push_back(createElement(0, 8*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 1));
	elements.push_back(createElement(0, 10*4, D3DDECLTYPE_FLOAT4, D3DDECLUSAGE_TEXCOORD, 2));
	elements.push_back(end);

#ifdef LEGACY_FILES
	handle = createVertexShader(device, "Data\\Shaders\\ati_shadow_bone_vertex_shader_dir.txt");
#else
	handle = createVertexShader(device, "data\\shader\\ati_shadow_bone_vertex_shader_dir.fvs");
#endif
	declaration = 0;
	device.CreateVertexDeclaration(&elements[0], &declaration);
}

void VertexShader::createAtiBoneShadowShaderPoint()
{
	elements.clear();
	elements.push_back(createElement(0, 0, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_POSITION));
	elements.push_back(createElement(0, 3*4, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_NORMAL));
	elements.push_back(createElement(0, 6*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 0));
	elements.push_back(createElement(0, 8*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 1));
	elements.push_back(createElement(0, 10*4, D3DDECLTYPE_FLOAT4, D3DDECLUSAGE_TEXCOORD, 2));
	elements.push_back(end);

#ifdef LEGACY_FILES
	handle = createVertexShader(device, "Data\\Shaders\\ati_shadow_bone_vertex_shader_point.txt");
#else
	handle = createVertexShader(device, "data\\shader\\ati_shadow_bone_vertex_shader_point.fvs");
#endif
	declaration = 0;
	device.CreateVertexDeclaration(&elements[0], &declaration);
}

void VertexShader::createAtiBoneShadowShaderFlat()
{
	elements.clear();
	elements.push_back(createElement(0, 0, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_POSITION));
	elements.push_back(createElement(0, 3*4, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_NORMAL));
	elements.push_back(createElement(0, 6*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 0));
	elements.push_back(createElement(0, 8*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 1));
	elements.push_back(createElement(0, 10*4, D3DDECLTYPE_FLOAT4, D3DDECLUSAGE_TEXCOORD, 2));
	elements.push_back(end);

#ifdef LEGACY_FILES
	handle = createVertexShader(device, "Data\\Shaders\\ati_shadow_bone_vertex_shader_flat.txt");
#else
	handle = createVertexShader(device, "data\\shader\\ati_shadow_bone_vertex_shader_flat.fvs");
#endif
	declaration = 0;
	device.CreateVertexDeclaration(&elements[0], &declaration);
}

void VertexShader::createAtiTerrainShadowShaderDirectional()
{
	elements.clear();
	elements.push_back(createElement(0, 0, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_POSITION));
	elements.push_back(createElement(0, 3*4, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_NORMAL));
	elements.push_back(createElement(1, 0, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 0));
	elements.push_back(createElement(1, 2*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 1));
	elements.push_back(createElement(1, 4*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 2));
	elements.push_back(end);

#ifdef LEGACY_FILES
	handle = createVertexShader(device, "Data\\Shaders\\ati_shadow_terrain_vertex_shader_dir.txt");
#else
	handle = createVertexShader(device, "data\\shader\\ati_shadow_terrain_vertex_shader_dir.fvs");
#endif
	declaration = 0;
	device.CreateVertexDeclaration(&elements[0], &declaration);
}

void VertexShader::createAtiTerrainShadowShaderPoint()
{
	elements.clear();
	elements.push_back(createElement(0, 0, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_POSITION));
	elements.push_back(createElement(0, 3*4, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_NORMAL));
	elements.push_back(createElement(1, 0, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 0));
	elements.push_back(createElement(1, 2*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 1));
	elements.push_back(createElement(1, 4*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 2));
	elements.push_back(end);

#ifdef LEGACY_FILES
	handle = createVertexShader(device, "Data\\Shaders\\ati_shadow_terrain_vertex_shader_point.txt");
#else
	handle = createVertexShader(device, "data\\shader\\ati_shadow_terrain_vertex_shader_point.fvs");
#endif
	declaration = 0;
	device.CreateVertexDeclaration(&elements[0], &declaration);
}

void VertexShader::createAtiTerrainShadowShaderFlat()
{
	elements.clear();
	elements.push_back(createElement(0, 0, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_POSITION));
	elements.push_back(createElement(0, 3*4, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_NORMAL));
	elements.push_back(createElement(1, 0, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 0));
	elements.push_back(createElement(1, 2*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 1));
	elements.push_back(createElement(1, 4*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 2));
	elements.push_back(end);

#ifdef LEGACY_FILES
	handle = createVertexShader(device, "Data\\Shaders\\ati_shadow_terrain_vertex_shader_flat.txt");
#else
	handle = createVertexShader(device, "data\\shader\\ati_shadow_terrain_vertex_shader_flat.fvs");
#endif
	declaration = 0;
	device.CreateVertexDeclaration(&elements[0], &declaration);
}

void VertexShader::createNvTerrainShadowShaderDirectional()
{
	elements.clear();
	elements.push_back(createElement(0, 0, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_POSITION));
	elements.push_back(createElement(0, 3*4, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_NORMAL));
	elements.push_back(createElement(1, 0, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 0));
	elements.push_back(createElement(1, 2*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 1));
	elements.push_back(createElement(1, 4*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 2));
	elements.push_back(end);

#ifdef LEGACY_FILES
	handle = createVertexShader(device, "Data\\Shaders\\nv_shadow_terrain_vertex_shader_dir.txt");
#else
	handle = createVertexShader(device, "data\\shader\\nv_shadow_terrain_vertex_shader_dir.fvs");
#endif
	declaration = 0;
	device.CreateVertexDeclaration(&elements[0], &declaration);
}

void VertexShader::createNvTerrainShadowShaderPoint()
{
	elements.clear();
	elements.push_back(createElement(0, 0, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_POSITION));
	elements.push_back(createElement(0, 3*4, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_NORMAL));
	elements.push_back(createElement(1, 0, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 0));
	elements.push_back(createElement(1, 2*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 1));
	elements.push_back(createElement(1, 4*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 2));
	elements.push_back(end);

#ifdef LEGACY_FILES
	handle = createVertexShader(device, "Data\\Shaders\\nv_shadow_terrain_vertex_shader_point.txt");
#else
	handle = createVertexShader(device, "data\\shader\\nv_shadow_terrain_vertex_shader_point.fvs");
#endif
	declaration = 0;
	device.CreateVertexDeclaration(&elements[0], &declaration);
}

void VertexShader::createNvTerrainShadowShaderFlat()
{
	elements.clear();
	elements.push_back(createElement(0, 0, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_POSITION));
	elements.push_back(createElement(0, 3*4, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_NORMAL));
	elements.push_back(createElement(1, 0, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 0));
	elements.push_back(createElement(1, 2*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 1));
	elements.push_back(createElement(1, 4*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 2));
	elements.push_back(end);

#ifdef LEGACY_FILES
	handle = createVertexShader(device, "Data\\Shaders\\nv_shadow_terrain_vertex_shader_flat.txt");
#else
	handle = createVertexShader(device, "data\\shader\\nv_shadow_terrain_vertex_shader_flat.fvs");
#endif
	declaration = 0;
	device.CreateVertexDeclaration(&elements[0], &declaration);
}

void VertexShader::createAtiConeShader()
{
	elements.clear();
	elements.push_back(createElement(0, 0, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_POSITION));
	elements.push_back(createElement(0, 3*4, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_NORMAL));
	elements.push_back(createElement(0, 6*4, D3DDECLTYPE_D3DCOLOR, D3DDECLUSAGE_COLOR));
	elements.push_back(createElement(0, 7*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 0));
	elements.push_back(end);

#ifdef LEGACY_FILES
	handle = createVertexShader(device, "Data\\Shaders\\cone_vertex_shader_ati.txt");
#else
	handle = createVertexShader(device, "data\\shader\\cone_vertex_shader_ati.fvs");
#endif
	declaration = 0;
	device.CreateVertexDeclaration(&elements[0], &declaration);
}

void VertexShader::createNvConeShader()
{
	elements.clear();
	elements.push_back(createElement(0, 0, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_POSITION));
	elements.push_back(createElement(0, 3*4, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_NORMAL));
	elements.push_back(createElement(0, 6*4, D3DDECLTYPE_D3DCOLOR, D3DDECLUSAGE_COLOR));
	elements.push_back(createElement(0, 7*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 0));
	elements.push_back(end);

#ifdef LEGACY_FILES
	handle = createVertexShader(device, "Data\\Shaders\\cone_vertex_shader_nv.txt");
#else
	handle = createVertexShader(device, "data\\shader\\cone_vertex_shader_nv.fvs");
#endif
	declaration = 0;
	device.CreateVertexDeclaration(&elements[0], &declaration);
}

void VertexShader::createDecalShader()
{
	elements.clear();
	elements.push_back(createElement(0, 0, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_POSITION));
	elements.push_back(createElement(0, 3*4, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_NORMAL));
	elements.push_back(createElement(0, 6*4, D3DDECLTYPE_D3DCOLOR, D3DDECLUSAGE_COLOR));
	elements.push_back(createElement(0, 7*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 0));
	elements.push_back(createElement(0, 9*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 1));
	elements.push_back(end);

#ifdef LEGACY_FILES
	handle = createVertexShader(device, "Data\\Shaders\\decal_vertex_shader.txt");
#else
	handle = createVertexShader(device, "data\\shader\\decal_vertex_shader.fvs");
#endif
	declaration = 0;
	device.CreateVertexDeclaration(&elements[0], &declaration);
}

void VertexShader::createDecalPointShader()
{
	elements.clear();
	elements.push_back(createElement(0, 0, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_POSITION));
	elements.push_back(createElement(0, 3*4, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_NORMAL));
	elements.push_back(createElement(0, 6*4, D3DDECLTYPE_D3DCOLOR, D3DDECLUSAGE_COLOR));
	elements.push_back(createElement(0, 7*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 0));
	elements.push_back(createElement(0, 9*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 1));
	elements.push_back(end);

#ifdef LEGACY_FILES
	handle = createVertexShader(device, "Data\\Shaders\\decal_projection_vertex_shader_point.txt");
#else
	handle = createVertexShader(device, "data\\shader\\decal_projection_vertex_shader_point.fvs");
#endif
	declaration = 0;
	device.CreateVertexDeclaration(&elements[0], &declaration);
}

void VertexShader::createDecalDirShader()
{
	elements.clear();
	elements.push_back(createElement(0, 0, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_POSITION));
	elements.push_back(createElement(0, 3*4, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_NORMAL));
	elements.push_back(createElement(0, 6*4, D3DDECLTYPE_D3DCOLOR, D3DDECLUSAGE_COLOR));
	elements.push_back(createElement(0, 7*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 0));
	elements.push_back(createElement(0, 9*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 1));
	elements.push_back(end);

#ifdef LEGACY_FILES
	handle = createVertexShader(device, "Data\\Shaders\\decal_projection_vertex_shader_dir.txt");
#else
	handle = createVertexShader(device, "data\\shader\\decal_projection_vertex_shader_dir.fvs");
#endif
	declaration = 0;
	device.CreateVertexDeclaration(&elements[0], &declaration);
}

void VertexShader::createDecalFlatShader()
{
	elements.clear();
	elements.push_back(createElement(0, 0, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_POSITION));
	elements.push_back(createElement(0, 3*4, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_NORMAL));
	elements.push_back(createElement(0, 6*4, D3DDECLTYPE_D3DCOLOR, D3DDECLUSAGE_COLOR));
	elements.push_back(createElement(0, 7*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 0));
	elements.push_back(createElement(0, 9*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 1));
	elements.push_back(end);

#ifdef LEGACY_FILES
	handle = createVertexShader(device, "Data\\Shaders\\decal_projection_vertex_shader_flat.txt");
#else
	handle = createVertexShader(device, "data\\shader\\decal_projection_vertex_shader_flat.fvs");
#endif
	declaration = 0;
	device.CreateVertexDeclaration(&elements[0], &declaration);
}

void VertexShader::createFakeDepthShader()
{
	elements.clear();
	elements.push_back(createElement(0, 0, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_POSITION));
	elements.push_back(createElement(0, 3*4, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_NORMAL));
	elements.push_back(createElement(0, 6*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 0));
	elements.push_back(createElement(0, 8*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 1));
	elements.push_back(end);

#ifdef LEGACY_FILES
	handle = createVertexShader(device, "Data\\Shaders\\fake_depth_vertex_shader.txt");
#else
	handle = createVertexShader(device, "data\\shader\\fake_depth_vertex_shader.fvs");
#endif
	declaration = 0;
	device.CreateVertexDeclaration(&elements[0], &declaration);
}

void VertexShader::createFakeShadowShader()
{
	elements.clear();
	elements.push_back(createElement(0, 0, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_POSITION));
	elements.push_back(createElement(0, 3*4, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_NORMAL));
	elements.push_back(createElement(0, 6*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 0));
	elements.push_back(createElement(0, 8*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 1));
	elements.push_back(end);

#ifdef LEGACY_FILES
	handle = createVertexShader(device, "Data\\Shaders\\fake_shadow_vertex_shader.txt");
#else
	handle = createVertexShader(device, "data\\shader\\fake_shadow_vertex_shader.fvs");
#endif
	declaration = 0;
	device.CreateVertexDeclaration(&elements[0], &declaration);
}

void VertexShader::createFakeDepthBoneShader()
{
	elements.clear();
	elements.push_back(createElement(0, 0, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_POSITION));
	elements.push_back(createElement(0, 3*4, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_NORMAL));
	elements.push_back(createElement(0, 6*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 0));
	elements.push_back(createElement(0, 8*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 1));
	elements.push_back(createElement(0, 10*4, D3DDECLTYPE_FLOAT4, D3DDECLUSAGE_TEXCOORD, 2));
	elements.push_back(end);

#ifdef LEGACY_FILES
	handle = createVertexShader(device, "Data\\Shaders\\fake_depth_bone_vertex_shader.txt");
#else
	handle = createVertexShader(device, "data\\shader\\fake_depth_bone_vertex_shader.fvs");
#endif
	declaration = 0;
	device.CreateVertexDeclaration(&elements[0], &declaration);
}

void VertexShader::createFakeShadowBoneShader()
{
	elements.clear();
	elements.push_back(createElement(0, 0, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_POSITION));
	elements.push_back(createElement(0, 3*4, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_NORMAL));
	elements.push_back(createElement(0, 6*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 0));
	elements.push_back(createElement(0, 8*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 1));
	elements.push_back(createElement(0, 10*4, D3DDECLTYPE_FLOAT4, D3DDECLUSAGE_TEXCOORD, 2));
	elements.push_back(end);

#ifdef LEGACY_FILES
	handle = createVertexShader(device, "Data\\Shaders\\fake_shadow_bone_vertex_shader.txt");
#else
	handle = createVertexShader(device, "data\\shader\\fake_shadow_bone_vertex_shader.fvs");
#endif
	declaration = 0;
	device.CreateVertexDeclaration(&elements[0], &declaration);
}

void VertexShader::createFakePlaneShadowShader()
{
	elements.clear();
	elements.push_back(createElement(0, 0, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_POSITION));
	elements.push_back(createElement(0, 3*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 0));
	elements.push_back(end);

#ifdef LEGACY_FILES
	handle = createVertexShader(device, "Data\\Shaders\\fake_shadow_plane_vertex_shader.txt");
#else
	handle = createVertexShader(device, "data\\shader\\fake_shadow_plane_vertex_shader.fvs");
#endif
	declaration = 0;
	device.CreateVertexDeclaration(&elements[0], &declaration);
}

void VertexShader::createConeStencilShader()
{
	elements.clear();
	elements.push_back(createElement(0, 0, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_POSITION));
	elements.push_back(end);

#ifdef LEGACY_FILES
	handle = createVertexShader(device, "Data\\Shaders\\cone_stencil_vertex_shader.txt");
#else
	handle = createVertexShader(device, "data\\shader\\cone_stencil_vertex_shader.fvs");
#endif
	declaration = 0;
	device.CreateVertexDeclaration(&elements[0], &declaration);
}

void VertexShader::createProceduralShader()
{
	elements.clear();
	elements.push_back(createElement(0, 0, D3DDECLTYPE_FLOAT4, D3DDECLUSAGE_POSITION));
	elements.push_back(createElement(0, 4*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 0));
	elements.push_back(createElement(0, 6*4, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, 1));
	elements.push_back(end);

#ifdef LEGACY_FILES
	handle = createVertexShader(device, "Data\\Shaders\\procedural_vertex_shader.txt");
#else
	handle = createVertexShader(device, "data\\shader\\procedural_vertex_shader.fvs");
#endif
	declaration = 0;
	device.CreateVertexDeclaration(&elements[0], &declaration);
}

void VertexShader::applyDeclaration() const
{
	device.SetVertexDeclaration(declaration);
}

void VertexShader::apply() const
{
#ifndef NDEBUG
	activeVertexShader = name;
	if (tracing) {
		fprintf(stderr, "activated vertex shader %s\n", name.c_str());
	}
#endif
	device.SetVertexDeclaration(declaration);
	device.SetVertexShader(handle);
}

// --

PixelShader::PixelShader(IDirect3DDevice9 &device_)
:	handle(0),
	device(device_)
{
}

PixelShader::~PixelShader()
{
}

#ifdef LEGACY_FILES

void PixelShader::createTerrainShader()
{
	handle = createPixelShader(device, "Data\\Shaders\\terrain_pixel_shader.txt");
}

void PixelShader::createTerrainLightShader()
{
	handle = createPixelShader(device, "Data\\Shaders\\terrain_lighting_pixel_shader.txt");
}

void PixelShader::createGlowShader()
{
	handle = createPixelShader(device, "Data\\Shaders\\glow_pixel_shader.txt");
}

void PixelShader::createGlowTex8Shader()
{
	handle = createPixelShader(device, "Data\\Shaders\\glow_8tex_pixel_shader.txt");
}

void PixelShader::createGlowPs14Shader()
{
	handle = createPixelShader(device, "Data\\Shaders\\glow_ps14_pixel_shader.txt");
}

void PixelShader::createGlowFinalShader()
{
	handle = createPixelShader(device, "Data\\Shaders\\glow_pass_pixel_shader.txt");
}

void PixelShader::createLightShader()
{
	handle = createPixelShader(device, "Data\\Shaders\\lightmap_pixel_shader.txt");
}

void PixelShader::createAtiLightConeShader_Texture()
{
	handle = createPixelShader(device, "Data\\Shaders\\ati_cone_pixel_shader_conetex.txt");
}

void PixelShader::createAtiLightConeShader_NoTexture()
{
	handle = createPixelShader(device, "Data\\Shaders\\ati_cone_pixel_shader_no_conetex.txt");
}

void PixelShader::createAtiFloatLightConeShader_Texture()
{
	handle = createPixelShader(device, "Data\\Shaders\\ati_float_cone_pixel_shader_conetex.txt");
}

void PixelShader::createAtiFloatLightConeShader_NoTexture()
{
	handle = createPixelShader(device, "Data\\Shaders\\ati_float_cone_pixel_shader_no_conetex.txt");
}

void PixelShader::createAtiDepthPixelShader()
{
	handle = createPixelShader(device, "Data\\Shaders\\ati_depth_pixel_shader.txt");
}

void PixelShader::createAtiShadowPixelShader()
{
	handle = createPixelShader(device, "Data\\Shaders\\ati_shadow_pixel_shader.txt");
}

void PixelShader::createAtiShadowSolidPixelShader()
{
	handle = createPixelShader(device, "Data\\Shaders\\ati_shadow_solid_pixel_shader.txt");
}

void PixelShader::createAtiShadowTerrainPixelShader()
{
	handle = createPixelShader(device, "Data\\Shaders\\ati_shadow_terrain_pixel_shader.txt");
}

void PixelShader::createAtiNoShadowPixelShader()
{
	handle = createPixelShader(device, "Data\\Shaders\\ati_noshadow_pixel_shader.txt");
}

void PixelShader::createAtiNoShadowTerrainPixelShader()
{
	handle = createPixelShader(device, "Data\\Shaders\\ati_noshadow_terrain_pixel_shader.txt");
}

void PixelShader::createAtiFloatDepthPixelShader()
{
	handle = createPixelShader(device, "Data\\Shaders\\ati_float_depth_pixel_shader.txt");
}

void PixelShader::createAtiFloatShadowPixelShader()
{
	handle = createPixelShader(device, "Data\\Shaders\\ati_float_shadow_pixel_shader.txt");
}

void PixelShader::createAtiFloatShadowTerrainPixelShader()
{
	handle = createPixelShader(device, "Data\\Shaders\\ati_float_shadow_terrain_pixel_shader.txt");
}

void PixelShader::createAtiFloatNoShadowPixelShader()
{
	handle = createPixelShader(device, "Data\\Shaders\\ati_noshadow_pixel_shader.txt");
}

void PixelShader::createAtiFloatNoShadowTerrainPixelShader()
{
	handle = createPixelShader(device, "Data\\Shaders\\ati_noshadow_terrain_pixel_shader.txt");
}

void PixelShader::createNvShadowShader()
{
	handle = createPixelShader(device, "Data\\Shaders\\nv_shadow_pixel_shader.txt");
}

void PixelShader::createNvSmoothShadowShader()
{
	handle = createPixelShader(device, "Data\\Shaders\\nv_shadow_pixel_shader_smooth.txt");
}

void PixelShader::createNvNoShadowShader()
{
	handle = createPixelShader(device, "Data\\Shaders\\nv_noshadow_pixel_shader.txt");
}

void PixelShader::createNvConeShader_Texture()
{
	handle = createPixelShader(device, "Data\\Shaders\\nv_cone_pixel_shader_texture.txt");
}

void PixelShader::createNvConeShader_NoTexture()
{
	handle = createPixelShader(device, "Data\\Shaders\\nv_cone_pixel_shader_notexture.txt");
}

void PixelShader::createLightingPixelShader_Lightmap()
{
	handle = createPixelShader(device, "Data\\Shaders\\lighting_pixel_shader_lmap.txt");
}

void PixelShader::createLightingPixelShader_Lightmap_Reflection()
{
	handle = createPixelShader(device, "Data\\Shaders\\lighting_pixel_shader_lmap_reflection.txt");
}

void PixelShader::createLightingPixelShader_Lightmap_LocalReflection()
{
	handle = createPixelShader(device, "Data\\Shaders\\lighting_pixel_shader_lmap_localreflection.txt");
}

void PixelShader::createLightingPixelShader_LightmapNoTexture()
{
	handle = createPixelShader(device, "Data\\Shaders\\lighting_pixel_shader_lmap_notexture.txt");
}

void PixelShader::createLightingPixelShader_NoLightmap()
{
	handle = createPixelShader(device, "Data\\Shaders\\lighting_pixel_shader_no_lmap.txt");
}

void PixelShader::createLightingPixelShader_NoLightmap_Reflection()
{
	handle = createPixelShader(device, "Data\\Shaders\\lighting_pixel_shader_no_lmap_reflection.txt");
}

void PixelShader::createLightingPixelShader_NoLightmap_LocalReflection()
{
	handle = createPixelShader(device, "Data\\Shaders\\lighting_pixel_shader_no_lmap_localreflection.txt");
}

void PixelShader::createLightingPixelShader_NoLightmapNoTexture()
{
	handle = createPixelShader(device, "Data\\Shaders\\lighting_pixel_shader_no_lmap_notexture.txt");
}

void PixelShader::createFakeDepthPixelShader()
{
	handle = createPixelShader(device, "Data\\Shaders\\fake_depth_pixel_shader.txt");
}

void PixelShader::createFakeShadowPixelShader()
{
	handle = createPixelShader(device, "Data\\Shaders\\fake_shadow_pixel_shader.txt");
}

void PixelShader::createDecalPixelShader()
{
	handle = createPixelShader(device, "Data\\Shaders\\decal_pixel_shader.txt");
}

void PixelShader::createColorEffectPixelShader()
{
	handle = createPixelShader(device, "Data\\Shaders\\color_effect_pixel_shader.txt");
}

void PixelShader::createColorEffectOffsetPixelShader()
{
	handle = createPixelShader(device, "Data\\Shaders\\color_effect_offset_pixel_shader.txt");
}

void PixelShader::createColorEffectOffsetPixelShader_NoGamma()
{
	handle = createPixelShader(device, "Data\\Shaders\\color_effect_offset_pixel_shader_nogamma.txt");
}

void PixelShader::createProceduralShader()
{
	handle = createPixelShader(device, "Data\\Shaders\\procedural_pixel_shader.txt");
}

void PixelShader::createProceduralOffsetShader()
{
	handle = createPixelShader(device, "Data\\Shaders\\procedural_offset_pixel_shader.txt");
}

void PixelShader::createProceduralOffsetBaseShader()
{
	handle = createPixelShader(device, "Data\\Shaders\\procedural_offset_base_pixel_shader.txt");
}

void PixelShader::createBlackWhiteShader()
{
	handle = createPixelShader(device, "Data\\Shaders\\black_white_effect_pixel_shader.txt");
}

void PixelShader::createOffsetBlendShader()
{
	handle = createPixelShader(device, "Data\\Shaders\\offset_blend_pixel_shader.txt");
}



#else



void PixelShader::createTerrainShader()
{
	handle = createPixelShader(device, "data\\shader\\terrain_pixel_shader.fps");
}

void PixelShader::createTerrainLightShader()
{
	handle = createPixelShader(device, "data\\shader\\terrain_lighting_pixel_shader.fps");
}

void PixelShader::createGlowShader()
{
	handle = createPixelShader(device, "data\\shader\\glow_pixel_shader.fps");
}

void PixelShader::createGlowTex8Shader()
{
	handle = createPixelShader(device, "data\\shader\\glow_8tex_pixel_shader.fps");
}

void PixelShader::createGlowPs14Shader()
{
	handle = createPixelShader(device, "data\\shader\\glow_ps14_pixel_shader.fps");
}

void PixelShader::createGlowFinalShader()
{
	handle = createPixelShader(device, "data\\shader\\glow_pass_pixel_shader.fps");
}

void PixelShader::createLightShader()
{
	handle = createPixelShader(device, "data\\shader\\lightmap_pixel_shader.fps");
}

void PixelShader::createAtiLightConeShader_Texture()
{
	handle = createPixelShader(device, "data\\shader\\ati_cone_pixel_shader_conetex.fps");
}

void PixelShader::createAtiLightConeShader_NoTexture()
{
	handle = createPixelShader(device, "data\\shader\\ati_cone_pixel_shader_no_conetex.fps");
}

void PixelShader::createAtiFloatLightConeShader_Texture()
{
	handle = createPixelShader(device, "data\\shader\\ati_float_cone_pixel_shader_conetex.fps");
}

void PixelShader::createAtiFloatLightConeShader_NoTexture()
{
	handle = createPixelShader(device, "data\\shader\\ati_float_cone_pixel_shader_no_conetex.fps");
}

void PixelShader::createAtiDepthPixelShader()
{
	handle = createPixelShader(device, "data\\shader\\ati_depth_pixel_shader.fps");
}

void PixelShader::createAtiShadowPixelShader()
{
	handle = createPixelShader(device, "data\\shader\\ati_shadow_pixel_shader.fps");
}

void PixelShader::createAtiShadowSolidPixelShader()
{
	handle = createPixelShader(device, "data\\shader\\ati_shadow_solid_pixel_shader.fps");
}

void PixelShader::createAtiShadowTerrainPixelShader()
{
	handle = createPixelShader(device, "data\\shader\\ati_shadow_terrain_pixel_shader.fps");
}

void PixelShader::createAtiNoShadowPixelShader()
{
	handle = createPixelShader(device, "data\\shader\\ati_noshadow_pixel_shader.fps");
}

void PixelShader::createAtiNoShadowTerrainPixelShader()
{
	handle = createPixelShader(device, "data\\shader\\ati_noshadow_terrain_pixel_shader.fps");
}

void PixelShader::createAtiFloatDepthPixelShader()
{
	handle = createPixelShader(device, "data\\shader\\ati_float_depth_pixel_shader.fps");
}

void PixelShader::createAtiFloatShadowPixelShader()
{
	handle = createPixelShader(device, "data\\shader\\ati_float_shadow_pixel_shader.fps");
}

void PixelShader::createAtiFloatShadowTerrainPixelShader()
{
	handle = createPixelShader(device, "data\\shader\\ati_float_shadow_terrain_pixel_shader.fps");
}

void PixelShader::createAtiFloatNoShadowPixelShader()
{
	handle = createPixelShader(device, "data\\shader\\ati_noshadow_pixel_shader.fps");
}

void PixelShader::createAtiFloatNoShadowTerrainPixelShader()
{
	handle = createPixelShader(device, "data\\shader\\ati_noshadow_terrain_pixel_shader.fps");
}

void PixelShader::createNvShadowShader()
{
	handle = createPixelShader(device, "data\\shader\\nv_shadow_pixel_shader.fps");
}

void PixelShader::createNvSmoothShadowShader()
{
	handle = createPixelShader(device, "data\\shader\\nv_shadow_pixel_shader_smooth.fps");
}

void PixelShader::createNvNoShadowShader()
{
	handle = createPixelShader(device, "data\\shader\\nv_noshadow_pixel_shader.fps");
}

void PixelShader::createNvConeShader_Texture()
{
	handle = createPixelShader(device, "data\\shader\\nv_cone_pixel_shader_texture.fps");
}

void PixelShader::createNvConeShader_NoTexture()
{
	handle = createPixelShader(device, "data\\shader\\nv_cone_pixel_shader_notexture.fps");
}

void PixelShader::createLightingPixelShader_Lightmap()
{
	handle = createPixelShader(device, "data\\shader\\lighting_pixel_shader_lmap.fps");
}

void PixelShader::createLightingPixelShader_Lightmap_Reflection()
{
	handle = createPixelShader(device, "data\\shader\\lighting_pixel_shader_lmap_reflection.fps");
}

void PixelShader::createLightingPixelShader_Lightmap_LocalReflection()
{
	handle = createPixelShader(device, "data\\shader\\lighting_pixel_shader_lmap_localreflection.fps");
}

void PixelShader::createLightingPixelShader_LightmapNoTexture()
{
	handle = createPixelShader(device, "data\\shader\\lighting_pixel_shader_lmap_notexture.fps");
}

void PixelShader::createLightingPixelShader_NoLightmap()
{
	handle = createPixelShader(device, "data\\shader\\lighting_pixel_shader_no_lmap.fps");
}

void PixelShader::createLightingPixelShader_NoLightmap_Reflection()
{
	handle = createPixelShader(device, "data\\shader\\lighting_pixel_shader_no_lmap_reflection.fps");
}

void PixelShader::createLightingPixelShader_NoLightmap_LocalReflection()
{
	handle = createPixelShader(device, "data\\shader\\lighting_pixel_shader_no_lmap_localreflection.fps");
}

void PixelShader::createLightingPixelShader_NoLightmapNoTexture()
{
	handle = createPixelShader(device, "data\\shader\\lighting_pixel_shader_no_lmap_notexture.fps");
}

void PixelShader::createFakeDepthPixelShader()
{
	handle = createPixelShader(device, "data\\shader\\fake_depth_pixel_shader.fps");
}

void PixelShader::createFakeShadowPixelShader()
{
	handle = createPixelShader(device, "data\\shader\\fake_shadow_pixel_shader.fps");
}

void PixelShader::createDecalPixelShader()
{
	handle = createPixelShader(device, "data\\shader\\decal_pixel_shader.fps");
}

void PixelShader::createColorEffectPixelShader()
{
	handle = createPixelShader(device, "data\\shader\\color_effect_pixel_shader.fps");
}

void PixelShader::createColorEffectOffsetPixelShader()
{
	handle = createPixelShader(device, "data\\shader\\color_effect_offset_pixel_shader.fps");
}

void PixelShader::createColorEffectOffsetPixelShader_NoGamma()
{
	handle = createPixelShader(device, "data\\shader\\color_effect_offset_pixel_shader_nogamma.fps");
}

void PixelShader::createProceduralShader()
{
	handle = createPixelShader(device, "data\\shader\\procedural_pixel_shader.fps");
}

void PixelShader::createProceduralOffsetShader()
{
	handle = createPixelShader(device, "data\\shader\\procedural_offset_pixel_shader.fps");
}

void PixelShader::createProceduralOffsetBaseShader()
{
	handle = createPixelShader(device, "data\\shader\\procedural_offset_base_pixel_shader.fps");
}

void PixelShader::createBlackWhiteShader()
{
	handle = createPixelShader(device, "data\\shader\\black_white_effect_pixel_shader.fps");
}

void PixelShader::createOffsetBlendShader()
{
	handle = createPixelShader(device, "data\\shader\\offset_blend_pixel_shader.fps");
}


#endif

void PixelShader::apply() const
{
#ifndef NDEBUG
	activePixelShader = name;
	if (tracing) {
		fprintf(stderr, "activated pixel shader %s\n", name.c_str());
	}
#endif
	device.SetPixelShader(handle);
}

bool PixelShader::hasShader() const
{
	if(handle)
		return true;

	return false;
}

// ---
/*
StateBlock::StateBlock()
:	handle(0),
	device(0)
{
}

StateBlock::~StateBlock()
{
	if(handle)
		device->DeleteStateBlock(handle);
}

void StateBlock::setDevice(IDirect3DDevice9 &device_)
{
	device = &device_;
}

void StateBlock::startRecording()
{
	assert(device);
	device->BeginStateBlock();
}

void StateBlock::endRecording()
{
	assert(device);
	device->EndStateBlock(&handle);
}

void StateBlock::apply() const
{
	assert(device);
	assert(handle);

	device->ApplyStateBlock(handle);
}
*/
// --

VertexBuffer::VertexBuffer()
:	vertexSize(0),
	vertexAmount(0),
	dynamic(false)
{
}

VertexBuffer::~VertexBuffer()
{
}

void VertexBuffer::release()
{
	buffer.Release();
}

void VertexBuffer::create(IDirect3DDevice9 &device, int vertexAmount_, int vertexSize_, bool dynamic_)
{
	if(vertexSize_ == vertexSize)
	if(vertexAmount >= vertexAmount_)
	if(buffer)
	if(dynamic == dynamic_)
		return;

	buffer.Release();
	vertexSize = vertexSize_;
	vertexAmount = vertexAmount_;
	dynamic = dynamic_;

	DWORD usage = D3DUSAGE_WRITEONLY;
	D3DPOOL pool = D3DPOOL_MANAGED;

	if(dynamic)
	{
		usage |= D3DUSAGE_DYNAMIC;
		pool = D3DPOOL_DEFAULT;

		//device.EvictManagedResources();
	}

	device.CreateVertexBuffer(vertexAmount * vertexSize, usage, 0, pool, &buffer, 0);
}

void *VertexBuffer::lock()
{
	void *pointer = 0;
	DWORD flags = dynamic ? D3DLOCK_DISCARD | D3DLOCK_NOSYSLOCK : 0;

	buffer->Lock(0, 0, &pointer, flags);
	return pointer;
}

void *VertexBuffer::unsafeLock(int offset, int amount)
{
	void *pointer = 0;
	DWORD flags = dynamic ? D3DLOCK_NOOVERWRITE | D3DLOCK_NOSYSLOCK : 0;

	buffer->Lock(offset * vertexSize, amount * vertexSize, &pointer, flags);
	return pointer;
}

void VertexBuffer::unlock()
{
	buffer->Unlock();
}

void VertexBuffer::apply(IDirect3DDevice9 &device, int stream) const
{
	device.SetStreamSource(stream, buffer, 0, vertexSize);
}

VertexBuffer::operator bool() const
{
	return buffer != 0;
}

// --

IndexBuffer::IndexBuffer()
:	faceAmount(0),
	dynamic(false),
	logger(0)
{
}

IndexBuffer::~IndexBuffer()
{
}

void IndexBuffer::setLogger(IStorm3D_Logger *logger_)
{
	logger = logger_;
}

void IndexBuffer::release()
{
	buffer.Release();
}

void IndexBuffer::create(IDirect3DDevice9 &device, int faceAmount_, bool dynamic_)
{
	if(faceAmount >= faceAmount_)
	if(buffer)
	if(dynamic == dynamic_)
		return;

	faceAmount = faceAmount_;
	dynamic = dynamic_;
	buffer.Release();

	DWORD usage = D3DUSAGE_WRITEONLY;
	D3DPOOL pool = D3DPOOL_MANAGED;

	if(dynamic)
	{
		usage = D3DUSAGE_DYNAMIC;
		pool = D3DPOOL_DEFAULT;

		//device.EvictManagedResources();
	}

	device.CreateIndexBuffer(faceAmount * 3 * sizeof(unsigned short), usage, D3DFMT_INDEX16, pool, &buffer, 0);
}

unsigned short *IndexBuffer::lock()
{
	void *pointer = 0;
	//DWORD flags = dynamic ? D3DLOCK_DISCARD : 0;
	DWORD flags = dynamic ? D3DLOCK_DISCARD | D3DLOCK_NOSYSLOCK : 0;

	buffer->Lock(0, 0, &pointer, flags);
	return reinterpret_cast<unsigned short *> (pointer);
}

void IndexBuffer::unlock()
{
	buffer->Unlock();
}

void IndexBuffer::render(IDirect3DDevice9 &device, int faceAmount, int maxIndex, int vertexOffset, int startIndex) const
{
	device.SetIndices(buffer);
	
	validateDevice(device, logger);
	device.DrawIndexedPrimitive(D3DPT_TRIANGLELIST, vertexOffset, 0, maxIndex, startIndex, faceAmount);

	++storm3d_dip_calls;
}

IndexBuffer::operator bool() const
{
	return buffer != 0;
}

// --

boost::shared_ptr<Storm3D_Texture> createSharedTexture(Storm3D_Texture *texture)
{
	if(texture)
	{
		texture->AddRef();
		return boost::shared_ptr<Storm3D_Texture> (texture, std::mem_fun(&Storm3D_Texture::Release));
	}

	return boost::shared_ptr<Storm3D_Texture> (texture);
}

void validateDevice(IDirect3DDevice9 &device, IStorm3D_Logger *logger)
{
//#ifdef NDEBUG
	return;
//#endif
/*
	HRESULT deviceState = device.TestCooperativeLevel();
    if(deviceState == D3DERR_DEVICELOST || deviceState == D3DERR_DEVICENOTRESET)
		return;

	DWORD value = 1;
	HRESULT hr = device.ValidateDevice(&value);
	if(SUCCEEDED(hr))
		return;

	const char *msg = "ValidateDevice() failed for unknown reason";
	if(hr == D3DERR_CONFLICTINGRENDERSTATE)
		msg = "ValidateDevice() failed -- D3DERR_CONFLICTINGRENDERSTATE";
	else if(hr == D3DERR_CONFLICTINGTEXTUREFILTER)
		msg = "ValidateDevice() failed -- D3DERR_CONFLICTINGTEXTUREFILTER";
	else if(hr == D3DERR_DRIVERINTERNALERROR)
		msg = "ValidateDevice() failed -- D3DERR_DRIVERINTERNALERROR";
	else if(hr == D3DERR_TOOMANYOPERATIONS)
		msg = "ValidateDevice() failed -- D3DERR_TOOMANYOPERATIONS";
	else if(hr == D3DERR_UNSUPPORTEDALPHAARG)
		msg = "ValidateDevice() failed -- D3DERR_UNSUPPORTEDALPHAARG";
	else if(hr == D3DERR_UNSUPPORTEDALPHAOPERATION)
		msg = "ValidateDevice() failed -- D3DERR_UNSUPPORTEDALPHAOPERATION";
	else if(hr == D3DERR_UNSUPPORTEDCOLORARG)
		msg = "ValidateDevice() failed -- D3DERR_UNSUPPORTEDCOLORARG";
	else if(hr == D3DERR_UNSUPPORTEDCOLOROPERATION)
		msg = "ValidateDevice() failed -- D3DERR_UNSUPPORTEDCOLOROPERATION";
	else if(hr == D3DERR_UNSUPPORTEDFACTORVALUE)
		msg = "ValidateDevice() failed -- D3DERR_UNSUPPORTEDFACTORVALUE";
	else if(hr == D3DERR_UNSUPPORTEDTEXTUREFILTER)
		msg = "ValidateDevice() failed -- D3DERR_UNSUPPORTEDTEXTUREFILTER";
	else if(hr == D3DERR_WRONGTEXTUREFORMAT)
		msg = "ValidateDevice() failed -- D3DERR_WRONGTEXTUREFORMAT";
	else if(hr == D3DERR_DRIVERINTERNALERROR)
		msg = "ValidateDevice() failed -- D3DERR_DRIVERINTERNALERROR";

	if(logger)
		logger->error(msg);

	assert(!"Whoopsie, validate() failed");
*/
}

// ...

static int currentAnisotrophy = 0;
void setCurrentAnisotrophy(int max)
{
	currentAnisotrophy = max;
}

void applyMaxAnisotrophy(IDirect3DDevice9 &device, int stageAmount)
{
	if(currentAnisotrophy)
	{
		for(int i = 0; i < stageAmount; ++i)
			device.SetSamplerState(i, D3DSAMP_MAXANISOTROPY, currentAnisotrophy);
	}
}

void enableMinMagFiltering(IDirect3DDevice9 &device, int startStage, int endStage, bool enable)
{
	DWORD minFilter = D3DTEXF_POINT;
	DWORD magFilter = D3DTEXF_POINT;

	if(enable)
	{
		magFilter = D3DTEXF_LINEAR;

		if(currentAnisotrophy)
			minFilter = D3DTEXF_ANISOTROPIC;
		else
			minFilter = D3DTEXF_LINEAR;
	}

	for(int i = startStage; i <= endStage; ++i)
	{
		device.SetSamplerState(i, D3DSAMP_MINFILTER, minFilter);
		device.SetSamplerState(i, D3DSAMP_MAGFILTER, magFilter);
	}
}

void enableMipFiltering(IDirect3DDevice9 &device, int startStage, int endStage, bool enable)
{
	DWORD filter = D3DTEXF_NONE;
	if(enable)
		filter = D3DTEXF_LINEAR;

	for(int i = startStage; i <= endStage; ++i)
		device.SetSamplerState(i, D3DSAMP_MIPFILTER, filter);
}

static bool inverseCulling = false;
void setInverseCulling(bool enable)
{
	inverseCulling = enable;
}

void setCulling(IDirect3DDevice9 &device, DWORD type)
{
	if(type == D3DCULL_NONE)
		device.SetRenderState(D3DRS_CULLMODE, type);
	else
	{
		if(type == D3DCULL_CCW)
		{
			if(!inverseCulling)
				device.SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
			else
				device.SetRenderState(D3DRS_CULLMODE, D3DCULL_CW);
		}
		else if(type == D3DCULL_CW)
		{
			if(!inverseCulling)
				device.SetRenderState(D3DRS_CULLMODE, D3DCULL_CW);
			else
				device.SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
		}
	}
}

void dumpD3DXMatrix(const D3DXMATRIX &mat) {
	for (unsigned int i = 0; i < 4; i++) {
        fprintf(stderr, "%f\t%f\t%f\t%f\n", mat.m[i][0], mat.m[i][1], mat.m[i][2], mat.m[i][3]);
	}
}

} // storm
} // frozenbyte

