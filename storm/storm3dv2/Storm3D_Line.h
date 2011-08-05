#pragma once

#include <IStorm3D_Line.h>
#include <d3d9.h>
#include <vector>
#include <atlbase.h>

class Storm3D;

class Storm3D_Line: public IStorm3D_Line
{
	Storm3D *storm;
	IDirect3DVertexBuffer9 *vertex_buffer;
	IDirect3DIndexBuffer9 *index_buffer;

	IDirect3DVertexBuffer9 *vertex_buffer2;
	bool pixel_line;

	std::vector<Vector> points;
	float thickness;
	int color;

	bool rebuild_indices;
	bool rebuild_vertices;

public:
	explicit Storm3D_Line(Storm3D *storm_);
	~Storm3D_Line();

	// Add as many as you like (>= 2)
	void AddPoint(const Vector &position);
	int GetPointCount();
	void RemovePoint(int index);
	
	// Units in world space
	void SetThickness(float thickness);
	void SetColor(int color);

	// Storm-stuff (expose this and remove that cast?)
	void Render(IDirect3DDevice9 *device);

	void releaseDynamicResources();
	void recreateDynamicResources();
};
