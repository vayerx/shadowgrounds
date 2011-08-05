// Copyright 2002-2004 Frozenbyte Ltd.

#undef WIN32_LEAN_AND_MEAN

#include "Export_Lod.h"
#include "nvtristrip.h"
#include <cassert>
#include <boost/scoped_array.hpp>
#include <windows.h>
#include <atlbase.h>
#include <d3d9.h>
#include <d3dx9mesh.h>

#include "Export_Types.h"

#pragma comment(lib, "d3d9.lib")

#ifndef NDEBUG
#pragma comment(lib, "d3dx9d.lib")
#else
#pragma comment(lib, "d3dx9.lib")
#endif

namespace frozenbyte {
namespace exporter {
namespace {
	static const int LOD_AMOUNT = 3;

	class DX
	{
		HWND windowHandle;
	
		CComPtr<IDirect3DDevice9> device;
		CComPtr<IDirect3D9> d3dHandle;

		void createWindow()
		{
			WNDCLASSEX windowClass = { 0 };
			windowClass.cbSize = sizeof(WNDCLASSEX);
			windowClass.lpfnWndProc = DefWindowProc;
			windowClass.hInstance = GetModuleHandle(0);
			windowClass.lpszClassName = "hidden_window_class";

			// Hidden window
			RegisterClassEx(&windowClass);
			windowHandle = CreateWindowEx(0, "hidden_window_class", "hidden window", WS_CAPTION, 0, 0, 10, 10, 0, 0, GetModuleHandle(0), 0);
		}

		void createDevice()
		{
			d3dHandle.Attach(Direct3DCreate9(D3D_SDK_VERSION));

			D3DPRESENT_PARAMETERS presentInfo; 
			ZeroMemory(&presentInfo, sizeof(D3DPRESENT_PARAMETERS));
			presentInfo.Windowed = TRUE;
			presentInfo.SwapEffect = D3DSWAPEFFECT_DISCARD;
			presentInfo.BackBufferFormat = D3DFMT_UNKNOWN;	
			presentInfo.BackBufferCount = 1;

			d3dHandle->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, windowHandle, D3DCREATE_HARDWARE_VERTEXPROCESSING, &presentInfo, &device);
		}

	public:
		DX()
		{
			createWindow();
			createDevice();
		}

		~DX()
		{
			DestroyWindow(windowHandle);
		}

		IDirect3DDevice9 &getDevice()
		{
			return *device;
		}
	};

	class Mesh
	{
		IDirect3DDevice9 &device;
		CComPtr<ID3DXMesh> cleanedMesh;
		CComPtr<ID3DXPMesh> lodMesh;

		D3DVERTEXELEMENT9 declaration[MAX_FVF_DECL_SIZE];
		boost::scoped_array<DWORD> adjacency;

	public:
		Mesh(IDirect3DDevice9 &device_)
		:	device(device_)
		{
		}

		bool generateMeshes(const std::vector<Vertex> &vertices, const std::vector<Face> &faces)
		{
			CComPtr<ID3DXMesh> basicMesh;
			
			D3DXDeclaratorFromFVF(D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX4, declaration);
			D3DXCreateMesh(faces.size(), vertices.size(), D3DXMESH_SYSTEMMEM, declaration, &device, &basicMesh);

			{
				unsigned short int *indexBuffer = 0;
				basicMesh->LockIndexBuffer(0, reinterpret_cast<void **> (&indexBuffer));
				
				for(unsigned int i = 0; i < faces.size(); ++i)
				for(int j = 0; j < 3; ++j)
				{
					int vertexIndex = faces[i].getVertexIndex(j);

					assert(vertexIndex < int(vertices.size()));
					*indexBuffer++ = vertexIndex;
				}

				basicMesh->UnlockIndexBuffer();
			}

			{
				float *vertexBuffer = 0;
				basicMesh->LockVertexBuffer(0, reinterpret_cast<void **> (&vertexBuffer));

				for(unsigned int i = 0; i < vertices.size(); ++i)
				{
					const Vertex &v = vertices[i];

					*vertexBuffer++ = float(v.getPosition().x);
					*vertexBuffer++ = float(v.getPosition().y);
					*vertexBuffer++ = float(v.getPosition().z);

					*vertexBuffer++ = float(v.getNormal().x);
					*vertexBuffer++ = float(v.getNormal().y);
					*vertexBuffer++ = float(v.getNormal().z);

					*vertexBuffer++ = float(v.getUv().x);
					*vertexBuffer++ = float(v.getUv().y);
					*vertexBuffer++ = float(v.getUv2().x);
					*vertexBuffer++ = float(v.getUv2().y);

					*vertexBuffer++ = float(v.getBoneWeight(0));
					*vertexBuffer++ = float(v.getBoneIndex(0));
					*vertexBuffer++ = float(v.getBoneWeight(1));
					*vertexBuffer++ = float(v.getBoneIndex(1));
				}

				basicMesh->UnlockVertexBuffer();
			}

			boost::scoped_array<DWORD> adjacency(new DWORD[3 * faces.size()]);
			basicMesh->GenerateAdjacency(0.001f, adjacency.get());

			cleanedMesh = basicMesh;

			D3DXATTRIBUTEWEIGHTS attributeWeights = { 0 };
			attributeWeights.Position = 1.f;
			attributeWeights.Boundary = 1.f;
			attributeWeights.Normal = .5f;

			D3DXGeneratePMesh(cleanedMesh, adjacency.get(), &attributeWeights, 0, cleanedMesh->GetNumVertices() / 3, D3DXMESHSIMP_VERTEX, &lodMesh);
			if(!lodMesh)
			{
				CComPtr<ID3DXBuffer> info;
				D3DXValidMesh(basicMesh, adjacency.get(), &info);

				cleanedMesh = 0;
				//D3DXCleanMesh(D3DXCLEAN_SIMPLIFICATION, basicMesh, adjacency.get(), &cleanedMesh, adjacency.get(), 0);
				D3DXCleanMesh(basicMesh, adjacency.get(), &cleanedMesh, adjacency.get(), 0);
				D3DXGeneratePMesh(cleanedMesh, adjacency.get(), &attributeWeights, 0, cleanedMesh->GetNumVertices() / 3, D3DXMESHSIMP_VERTEX, &lodMesh);

				int vertexAdd = cleanedMesh->GetNumVertices() - basicMesh->GetNumVertices();
				int facedd = cleanedMesh->GetNumFaces() - basicMesh->GetNumFaces();

				if(!lodMesh)
					return false;
			}

			return true;
		}

		void getVertices(std::vector<Vertex> &vertices)
		{
			lodMesh->SetNumVertices(lodMesh->GetMaxVertices());

			vertices.clear();
			vertices.resize(lodMesh->GetNumVertices());

			float *vertexBuffer = 0;
			lodMesh->LockVertexBuffer(D3DLOCK_READONLY, reinterpret_cast<void **> (&vertexBuffer));

			for(unsigned int i = 0; i < vertices.size(); ++i)
			{
				Vertex &v = vertices[i];

				FBVector position(*vertexBuffer++, *vertexBuffer++, *vertexBuffer++);
				v.setPosition(position);

				FBVector normal(*vertexBuffer++, *vertexBuffer++, *vertexBuffer++);
				v.setNormal(normal);
			
				FBVector2 uv(*vertexBuffer++, *vertexBuffer++);
				v.setUv(uv);

				FBVector2 uv2(*vertexBuffer++, *vertexBuffer++);
				v.setUv2(uv2);

				for(int j = 0; j < 2; ++j)
				{
					float weight = *vertexBuffer++;
					int index = int(*vertexBuffer++ + .5f);

					v.addWeight(index, weight);
				}
			}

			lodMesh->UnlockVertexBuffer();
		}

		void getFaces(std::vector<Face> &faces, int lodLevel, int lodDetail)
		{
			int maxVertex = lodMesh->GetMaxVertices();
			int minVertex = lodMesh->GetMinVertices();
			minVertex = lodDetail * (maxVertex - minVertex) / 4 + minVertex;

			int vertexAmount = (LOD_AMOUNT - lodLevel - 1) * (maxVertex - minVertex) / (LOD_AMOUNT - 1) + minVertex;
			lodMesh->SetNumVertices(vertexAmount);

			unsigned short int *indexBuffer = 0;
			lodMesh->LockIndexBuffer(D3DLOCK_READONLY, reinterpret_cast<void **> (&indexBuffer));

			unsigned short indices[20] = { 0 };
			for(int k = 0; k < 20; ++k)
				indices[k] = indexBuffer[k];

			faces.clear();
			faces.resize(lodMesh->GetNumFaces());

			for(unsigned int i = 0; i < lodMesh->GetNumFaces(); ++i)
			for(int j = 0; j < 3; ++j)
			{
				assert(*indexBuffer < lodMesh->GetNumVertices());
				faces[i].setVertexIndex(j, *indexBuffer++);
			}

			lodMesh->UnlockIndexBuffer();
		}
	};

} // unnamed

struct LodData
{
	std::vector<Vertex> vertices;
	const std::vector<Face> &originalFaces;

	std::vector<Face> faces[LOD_AMOUNT];
	bool hasLods;

	LodData(const std::vector<Vertex> &vertices_, const std::vector<Face> &faces_, bool optimize)
	:	vertices(vertices_),
		originalFaces(faces_),
		hasLods(false)
	{
		faces[0] = originalFaces;

		if(optimize)
			optimizeFaces(0);
	}

	void optimizeFaces(int index)
	{
		assert(index >= 0 && index < LOD_AMOUNT);
		int faceAmount = faces[index].size();

		unsigned short *oldIndices = new unsigned short[faceAmount * 3];

		for(int i = 0; i < faceAmount; ++i)
		for(int j = 0; j < 3; ++j)
		{
			int oldIndex = faces[index][i].getVertexIndex(j);
			assert(oldIndex >= 0 && oldIndex < int(vertices.size()));

			oldIndices[i *  3 + j] = oldIndex;
		}

		PrimitiveGroup *primitiveGroup = 0;
		unsigned short groupAmount = 0;

		SetCacheSize(CACHESIZE_GEFORCE3);
		SetListsOnly(true);

		GenerateStrips(oldIndices, faceAmount * 3, &primitiveGroup, &groupAmount);
		faces[index].resize(primitiveGroup->numIndices / 3);

		{
			for(unsigned int i = 0; i < primitiveGroup->numIndices / 3; ++i)
			for(int j = 0; j < 3; ++j)
			{
				int newIndex = primitiveGroup->indices[i * 3 + j];
				assert(newIndex >= 0 && newIndex < int(vertices.size()));

				faces[index][i].setVertexIndex(j, newIndex);
			}
		}

		delete[] oldIndices;
		delete[] primitiveGroup;
	}

	void generateLods(int lodDetail)
	{
		static DX dx;
		Mesh mesh(dx.getDevice());

		if(!mesh.generateMeshes(vertices, faces[0]))
			return;

		mesh.getVertices(vertices);

		for(int i = 0; i < LOD_AMOUNT; ++i)
		{
			mesh.getFaces(faces[i], i, lodDetail);
			optimizeFaces(i);
		}

		hasLods = true;
	}
};

Lod::Lod(const std::vector<Vertex> &vertices, const std::vector<Face> &faces, bool optimizeVcache)
{
	boost::scoped_ptr<LodData> tempData(new LodData(vertices, faces, optimizeVcache));
	data.swap(tempData);
}

Lod::~Lod()
{
}

void Lod::generateLods(int lodDetail)
{
	data->generateLods(lodDetail);
}

int Lod::getFaceBufferCount() const
{
	if(data->hasLods)
		return LOD_AMOUNT;
	
	return 1;
}

const std::vector<Face> &Lod::getFaceBuffer(int index) const
{
	assert(index >= 0 && index < getFaceBufferCount());
	return data->faces[index];
}

const std::vector<Vertex> &Lod::getVertices() const
{
	return data->vertices;
}

} // end of namespace export
} // end of namespace frozenbyte
