// Copyright 2002-2004 Frozenbyte Ltd.

#pragma warning(disable:4103)
#pragma warning(disable:4786)

#include "storm_geometry.h"
#include "storm.h"
#include <istorm3d_model.h>
#include <istorm3d_mesh.h>
#include <istorm3d.h>
#include <vector>

using namespace std;

namespace frozenbyte {
namespace editor {
namespace {

	void copyOld(IStorm3D_Mesh *mesh, vector<Storm3D_Vertex> &vertices, vector<Storm3D_Face> &faces)
	{
		vertices.resize(mesh->GetVertexCount());
		faces.resize(mesh->GetFaceCount());

		const Storm3D_Vertex *v = mesh->GetVertexBufferReadOnly();
		for(unsigned int i = 0; i < vertices.size(); ++i)
			vertices[i] = v[i];

		const Storm3D_Face *f = mesh->GetFaceBufferReadOnly();
		for(unsigned int j = 0; j < faces.size(); ++j)
			faces[j] = f[j];
	}

	void getVertices(Storm3D_Vertex *vertices, const VC3 &start, const VC3 &end, float thickness, const VC3 &normal)
	{
		VC3 dir = (end - start).GetNormalized();
		VC3 side = normal.GetNormalized().GetCrossWith(dir);
		
		thickness *= 0.5f;
		vertices[0].position = start + (side * thickness);
		vertices[0].normal = normal;
		vertices[1].position = start - (side * thickness);
		vertices[1].normal = normal;
		vertices[2].position = end + (side * thickness);
		vertices[2].normal = normal;
		vertices[3].position = end - (side * thickness);
		vertices[3].normal = normal;
	}

	void getFaces(Storm3D_Face *faces, int baseIndex)
	{
		faces[0].vertex_index[0] = 0 + baseIndex;
		faces[0].vertex_index[1] = 1 + baseIndex;
		faces[0].vertex_index[2] = 2 + baseIndex;

		faces[1].vertex_index[0] = 1 + baseIndex;
		faces[1].vertex_index[1] = 3 + baseIndex;
		faces[1].vertex_index[2] = 2 + baseIndex;
	}

	void setBuffers(IStorm3D_Mesh *mesh, const vector<Storm3D_Vertex> &vertices, const vector<Storm3D_Face> &faces)
	{
		mesh->ChangeVertexCount(vertices.size());
		mesh->ChangeFaceCount(faces.size());

		Storm3D_Vertex *vb = mesh->GetVertexBuffer();
		for(unsigned int i = 0; i < vertices.size(); ++i)
			vb[i] = vertices[i];

		Storm3D_Face *fb = mesh->GetFaceBuffer();
		for(unsigned int j = 0; j < faces.size(); ++j)
			fb[j] = faces[j];
	}

	void addPlane(IStorm3D_Mesh *mesh, const VC3 &start, const VC3 &end, float thickness, const VC3 &normal)
	{
		vector<Storm3D_Vertex> vertices;
		vector<Storm3D_Face> faces;

		copyOld(mesh, vertices, faces);
		int baseIndex = vertices.size();

		vertices.resize(vertices.size() + 4);
		getVertices(&vertices[vertices.size() - 4], start, end, thickness, normal);

		faces.resize(faces.size() + 2);
		getFaces(&faces[faces.size() - 2], baseIndex);

		setBuffers(mesh, vertices, faces);
	}

} // unnamed

IStorm3D_Mesh *createWireframeObject(Storm &storm, IStorm3D_Model *model, const COL &color, const char *name)
{
	IStorm3D_Material *m = storm.storm->CreateNewMaterial("..");
	m->SetSpecial(false, true);
	m->SetColor(color);
	m->SetSelfIllumination(COL(.2f, .2f, .2f));

	IStorm3D_Mesh *mesh = storm.storm->CreateNewMesh();
	mesh->UseMaterial(m);

	IStorm3D_Model_Object *o = model->Object_New(name);
	o->SetMesh(mesh);

	return mesh;
}

void addBox(IStorm3D_Mesh *mesh, const VC3 &center, float radius)
{
	VC3 a = center + VC3(0, -radius, -radius);
	VC3 b = center + VC3(0, radius, -radius);
	VC3 c = center + VC3(0, radius, radius);
	VC3 d = center + VC3(0, -radius, radius);

	addPlane(mesh, a, b, 2*radius, VC3(0, 0, -1.f));
	addPlane(mesh, b, c, 2*radius, VC3(0, 1.f, 0));
	addPlane(mesh, c, d, 2*radius, VC3(0, 0, 1.f));
	addPlane(mesh, d, a, 2*radius, VC3(0, -1.f, 0));

	VC3 e = center + VC3(radius, radius, 0);
	VC3 f = center + VC3(radius, -radius, 0);
	addPlane(mesh, e, f, 2*radius, VC3(1.f, 0, 0));

	VC3 g = center + VC3(-radius, radius, 0);
	VC3 h = center + VC3(-radius, -radius, 0);
	addPlane(mesh, g, h, 2*radius, VC3(-1.f, 0, 0));
}

void addLine(IStorm3D_Mesh *mesh, const VC3 &start, const VC3 &end, float thickness, const VC3 &normal)
{
	addPlane(mesh, start, end, thickness, normal);
	addPlane(mesh, end, start, thickness, -normal);
}

void addCone(IStorm3D_Mesh *mesh, const VC3 &origo, float xAngle, float fov, float range, int circleVertices)
{
	vector<Storm3D_Vertex> vertices;
	vector<Storm3D_Face> faces;

	copyOld(mesh, vertices, faces);
	int baseIndex = vertices.size();

	//vertices.resize(vertices.size() + 4);
	//getVertices(&vertices[vertices.size() - 4], start, end, thickness, normal);
	//faces.resize(faces.size() + 2);
	//getFaces(&faces[faces.size() - 2], baseIndex);

	// Vertices
	{
		//float farMul = atanf(fov / 2.f) * range;
		float farMul = tanf(fov / 2.f) * range;

		Storm3D_Vertex origoVertex;
		origoVertex.position = origo;
		origoVertex.normal = VC3(0,0,0);
		vertices.push_back(origoVertex);

		QUAT q;
		q.MakeFromAngles(xAngle, 0, 0);
		MAT m;
		m.CreateRotationMatrix(q);

		for(int i = 0; i < circleVertices; ++i)
		{
			float angle = (float(i) / (circleVertices)) * PI * 2.f;
			float x = sinf(angle);
			float y = cosf(angle);
			float z = range;

			float nx = x;
			float ny = y;
			float nz = 0;

			float u = (x * .5f) + .5f;
			float v = (y * .5f) + .5f;

			VC3 pos(x * farMul, y * farMul, z);
			m.RotateVector(pos);

			Storm3D_Vertex vertex;
			vertex.position = origo + pos;
			vertex.normal = VC3(nx, ny, nz);

			vertices.push_back(vertex);
		}
	}

	// Faces
	{
		for(int i = 0; i < circleVertices; ++i)
		{
			int last = i - 1;
			if(i == 0)
				last = circleVertices - 1;

			Storm3D_Face face1;
			face1.vertex_index[0] = i + 1 + baseIndex;
			face1.vertex_index[1] = 0 + baseIndex;
			face1.vertex_index[2] = last + 1 + baseIndex;

			Storm3D_Face face2;
			face2.vertex_index[0] = face1.vertex_index[1];
			face2.vertex_index[1] = face1.vertex_index[0];
			face2.vertex_index[2] = face1.vertex_index[2];

			faces.push_back(face1);
			faces.push_back(face2);
		}
	}

	setBuffers(mesh, vertices, faces);
}

VC3 getSize(IStorm3D_Model *model)
{
	if(!model)
		return VC3();

	VC3 result;

	boost::scoped_ptr<Iterator<IStorm3D_Model_Object *> > objectIterator(model->ITObject->Begin());
	for(; !objectIterator->IsEnd(); objectIterator->Next())
	{
		IStorm3D_Model_Object *object = objectIterator->GetCurrent();
		if(!object)
			continue;

		if(object->GetNoCollision())
			continue;

		IStorm3D_Mesh *mesh = object->GetMesh();
		if(!mesh)
			continue;

		const Matrix &tm = object->GetMXG();
		const Storm3D_Vertex *buffer = mesh->GetVertexBufferReadOnly();

		for(int i = 0; i < mesh->GetVertexCount(); ++i)
		{
			VC3 position = buffer[i].position;
			tm.TransformVector(position);

			if(position.x < 0)
				position.x *= -1.f;
			if(position.y < 0)
				position.y *= -1.f;
			if(position.z < 0)
				position.z *= -1.f;

			if(position.x > result.x)
				result.x = position.x;
			if(position.y > result.y)
				result.y = position.y;
			if(position.z > result.z)
				result.z = position.z;
		}
	}

	return result;
}

} // editor
} // frozenbyte
