// Copyright 2002-2004 Frozenbyte Ltd.

#ifdef _MSC_VER
#pragma warning(disable:4103)
#endif

#include <math.h>
#include <algorithm>
#include <GL/glew.h>
#include "storm3d_spotlight_shared.h"
#include "storm3d_camera.h"
#include "storm3d_scene.h"
#include "../../util/Debug_MemoryManager.h"
#include <c2_frustum.h>
#include "igios.h"

#ifdef _MSC_VER
#define MIN(a,b) ( min( (a), (b) ) )
#define MAX(a,b) ( max( (a), (b) ) )
#else
#define MIN(a,b) (std::max( (a), (b) ) )
#define MAX(a,b) (std::min( (a), (b) ) )
#endif

//! Constructor
Storm3D_SpotlightShared::Storm3D_SpotlightShared()
:	range(20.f),
	fov(60.f)
{
	resolutionX = 1;
	resolutionY = 1;
}

//! Destructor
Storm3D_SpotlightShared::~Storm3D_SpotlightShared()
{
}

//! Update matrices
/*!
	\param cameraView camera view matrix
	\param bias camera bias
*/
void Storm3D_SpotlightShared::updateMatrices(const D3DXMATRIX &cameraView, float bias)
{
	VC3 lightPosition(position.x, position.y, position.z);
	VC3 up(0, 1.f, 0);
	VC3 lookAt = lightPosition;
	lookAt += VC3(direction.x, direction.y, direction.z);

	D3DXMatrixPerspectiveFovLH(lightProjection, (float)D3DXToRadian(fov), 1.f, .2f, range);
	D3DXMATRIX cameraMatrix;
	D3DXMatrixInverse(cameraMatrix, NULL, cameraView);

	D3DXMatrixLookAtLH(lightView, lightPosition, lookAt, up);

	D3DXMatrixMultiply(shadowProjection, lightView, lightProjection);

	shaderProjection = shadowProjection;
	D3DXMatrixMultiply(shadowProjection, cameraMatrix, shadowProjection);

	{
		//float scaleX = 1.0f / SHADOW_WIDTH, scaleY = 1.0f / SHADOW_HEIGHT;
		//D3DXMATRIX shadowTweak;
		//D3DXMatrixIdentity(shadowTweak);

		D3DXMATRIX shadowTweak( 0.5f,	0.0f,		0.0f,	0.0f,
								0.0f,	0.5f,		0.0f,	0.0f,
								0.0f,	0.0f,		0.5f,	0.0f,
								0.5f,	0.5f,		0.5f,	1.0f );

		D3DXMatrixMultiply(targetProjection, lightView, lightProjection);
		D3DXMatrixMultiply(targetProjection, targetProjection, shadowTweak);
	}
}


//! Set clip planes
/*!
	\param cameraView camera view matrix
*/
void Storm3D_SpotlightShared::setClipPlanes(const float *cameraView)
{
	// Not used anywhere

	D3DXMATRIX m(cameraView);
	float determinant = D3DXMatrixDeterminant(m);
	D3DXMatrixInverse(m, &determinant, m);
	D3DXMatrixTranspose(m, m);

	VC3 d(direction.x, direction.y, direction.z);
	VC2 bd(d.x, d.z);
	bd.Normalize();		
	VC3 p1(position.x - 8*bd.x, position.y, position.z - 8*bd.y);
	VC3 p2(p1.x, p1.y + 5.f, p1.z);

	float angle = DegToRadian(fov) * .55f;

	D3DXPLANE leftPlane;
	D3DXMATRIX leftTransform;
	D3DXMatrixRotationY(leftTransform, -angle);
	VC3 leftPoint(direction.x, 0, direction.z);
	VC4 leftPoint2;
	D3DXVec3Transform(leftPoint2, leftPoint, leftTransform);
	leftPoint = p1;
	leftPoint.x += leftPoint2.x;
	leftPoint.z += leftPoint2.z;
	D3DXPlaneFromPoints(leftPlane, p1, p2, leftPoint);
	D3DXPlaneNormalize(leftPlane, leftPlane);
	D3DXPlaneTransform(leftPlane, leftPlane, m);

	D3DXPLANE rightPlane;
	D3DXMATRIX rightTransform;
	D3DXMatrixRotationY(rightTransform, angle);
	VC3 rightPoint(direction.x, 0, direction.z);
	VC4 rightPoint2;
	D3DXVec3Transform(rightPoint2, rightPoint, rightTransform);
	rightPoint = p1;
	rightPoint.x += rightPoint2.x;
	rightPoint.z += rightPoint2.z;
	D3DXPlaneFromPoints(rightPlane, rightPoint, p2, p1);
	D3DXPlaneNormalize(rightPlane, rightPlane);
	D3DXPlaneTransform(rightPlane, rightPlane, m);

	D3DXPLANE backPlane;
	VC3 pb(p1.x, p1.y, p1.z);
	D3DXPlaneFromPointNormal(backPlane, pb, d);
	D3DXPlaneNormalize(backPlane, backPlane);
	D3DXPlaneTransform(backPlane, backPlane, m);

	glClipPlane(0, leftPlane);
	glEnable(GL_CLIP_PLANE0);
	glClipPlane(1, rightPlane);
	glEnable(GL_CLIP_PLANE1);
	glClipPlane(2, backPlane);
	glEnable(GL_CLIP_PLANE2);
}

namespace {

	void calculateLineToScissor(const VC3 &v1, const VC3 &v2, const Frustum &frustum, Storm3D_Camera &camera, const VC2I &screenSize, int &minX, int &minY, int &maxX, int &maxY)
	{
		Plane plane[5];
		{
			for(int i = 0; i < 5; ++i)
				plane[i].MakeFromNormalAndPosition(frustum.planeNormals[i], frustum.position);
		}

		VC3 v[2] = { v1, v2 };
		int i = 0;
		for(i = 0; i < 5; ++i)
		{
			float r1 = plane[i].GetPointRange(v[0]);
			float r2 = plane[i].GetPointRange(v[1]);
			if(r1 < 0 && r2 < 0)
				return;

			if(r1 < 0 || r2 < 0)
			{
				VC3 ab = v[1] - v[0];
				float t = (plane[i].range_to_origin - plane[i].planenormal.GetDotWith(v[0])) / plane[i].planenormal.GetDotWith(ab);
				if(t >= 0.f && t <= 1.f)
				{
					VC3 clamp = v[0] + (ab * t);

					if(r1 < 0)
						v[0] = clamp;
					else
						v[1] = clamp;
				}
				else
				{
					assert(!"..");
				}
			}
		}

		for(i = 0; i < 2; ++i)
		{
			VC3 result;

			int x = int(result.x * screenSize.x);
			int y = int(result.y * screenSize.y);

			maxX = max(x, maxX);
			maxY = max(y, maxY);
			minX = min(x, minX);
			minY = min(y, minY);
		}
	}

}


namespace {

	struct ClipPolygon
	{
		VC3 vertices[12];
		int vertexAmount;

		ClipPolygon()
		:	vertexAmount(0)
		{
		}

		void addVertex(const VC3 &vertex)
		{
			// We only have space for triangle clipped against frustum
			assert(vertexAmount <= 11);

			vertices[vertexAmount] = vertex;
			++vertexAmount;
		}

		void reset()
		{
			vertexAmount = 0;
		}
	};

	// Sutherland-Hodgman algorithm for clipping

	void clipLine(const PLANE &plane, const VC3 &p1, const VC3 &p2, VC3 &result)
	{
		float d1 = plane.GetPointRange(p1);
		float d2 = plane.GetPointRange(p2);
		if(d1 < 0 && d2 < 0)
			return;
		if(d1 > 0 && d2 > 0)
			return;

		float s = (d1 / (d1 - d2));
		result = p1 + ((p2 - p1) * s);
	}

	void clipPolygonToPlane(const ClipPolygon &in, ClipPolygon &out, PLANE &plane)
	{
		if(!in.vertexAmount)
			return;

		VC3 v1 = in.vertices[in.vertexAmount - 1];
		for(int i = 0; i < in.vertexAmount; ++i)
		{
			VC3 v2 = in.vertices[i];
			
			bool inside1 = plane.GetPointRange(v1) > 0.0f;
			bool inside2 = plane.GetPointRange(v2) > 0.0f;

			if(inside1 && inside2) // Both inside
			{
				out.addVertex(v2);
			}
			else if(inside1 && !inside2) // Leaving
			{
				VC3 newVert;
				clipLine(plane, v1, v2, newVert);

				out.addVertex(newVert);
			}
			else if(!inside1 && inside2) // Entering
			{			
				VC3 newVert;
				clipLine(plane, v2, v1, newVert);

				out.addVertex(newVert);
				out.addVertex(v2);
			}

			v1 = v2;
		}
	}

	const ClipPolygon &clipTriangleToFrustum(const VC3 &v1, const VC3 &v2, const VC3 &v3, const Frustum &frustum)
	{
		static ClipPolygon polygon1;
		static ClipPolygon polygon2;

		ClipPolygon *in = &polygon1;
		in->reset();
		in->addVertex(v1);
		in->addVertex(v2);
		in->addVertex(v3);

		ClipPolygon *out = &polygon2;
		out->reset();

		// We can handle frustum by clipping to each plane in turn.

		for(int i = 0; i < 5; ++i)
		{
			const VC3 &planeNormal = frustum.planeNormals[i];
			PLANE plane;

			if(i == 0)
				plane.MakeFromNormalAndPosition(planeNormal, frustum.position + planeNormal);
			else
				plane.MakeFromNormalAndPosition(planeNormal, frustum.position);

			clipPolygonToPlane(*in, *out, plane);

			in->reset();
			std::swap(in, out);
		}

		return *in;
	}

} // unnamed


bool Storm3D_SpotlightShared::setScissorRect(Storm3D_Camera &camera, const VC2I &screenSize, Storm3D_Scene *scene)
{
    /*
	igiosWarning("setScissorRect: fov %f\n", fov);
	igiosWarning("direction: (%f,%f,%f)\n", direction.x, direction.y, direction.z);
	igiosWarning("dirlen: %f\n", direction.GetLength());
    */
	igios_unimplemented();
	// FIXME: broken
	return true;

	D3DXMATRIX light;
	VC3 lightPosition(position.x, position.y, position.z);
	VC3 up(0, 1.f, 0);
	VC3 lookAt = lightPosition;
	lookAt += VC3(direction.x, direction.y, direction.z);
	D3DXMatrixLookAtLH(light, lightPosition, lookAt, up);

	// Create frustum vertices

	VC3 v[5];
	v[0] = VC3(0, 0, 0);
	v[1] = VC3(0, 0, 1.f);
	v[2] = VC3(0, 0, 1.f);
	v[3] = VC3(0, 0, 1.f);
	v[4] = VC3(0, 0, 1.f);

	float det = D3DXMatrixDeterminant(light);
	D3DXMatrixInverse(light, &det, light);
	float angle = DegToRadian(fov) * .5f;
	for(int i = 0; i <= 4; ++i)
	{
		if(i > 0)
		{
			float z = v[i].z;
			if(i == 1 || i == 2)
			{
				v[i].x = z * sinf(angle);
				v[i].z = z * cosf(angle);
			}
			else
			{
				v[i].x = z * sinf(-angle);
				v[i].z = z * cosf(-angle);
			}

			if(i == 1 || i == 3)
				v[i].y = z * sinf(angle);
			else
				v[i].y = z * sinf(-angle);

			float scale = range / cosf(angle);
			v[i] *= scale;
		}

		D3DXVec3TransformCoord(v[i], v[i], light);
	}

	// Create area

	const Frustum &frustum = camera.getFrustum();
	int minX = screenSize.x;
	int minY = screenSize.y;
	int maxX = 0;
	int maxY = 0;

	for(int i = 0; i < 6; ++i)
	{
		VC3 v1;
		VC3 v2;
		VC3 v3;

		if(i == 0)
		{
			v1 = v[0];
			v2 = v[1];
			v3 = v[2];
		}
		else if(i == 1)
		{
			v1 = v[0];
			v2 = v[2];
			v3 = v[4];
		}
		else if(i == 2)
		{
			v1 = v[0];
			v2 = v[3];
			v3 = v[4];
		}
		else if(i == 3)
		{
			v1 = v[0];
			v2 = v[1];
			v3 = v[3];
		}
		else if(i == 4)
		{
			v1 = v[1];
			v2 = v[2];
			v3 = v[3];
		}
		else if(i == 5)
		{
			v1 = v[4];
			v2 = v[2];
			v3 = v[3];
		}

		const ClipPolygon &clipPolygon = clipTriangleToFrustum(v1, v2, v3, frustum);
		for(int j = 0; j < clipPolygon.vertexAmount; ++j)
		{
			VC3 result;
			float rhw = 0.f;
			float real_z = 0.f;
			camera.GetTransformedToScreen(clipPolygon.vertices[j], result, rhw, real_z);

			int x = int(result.x * screenSize.x);
			int y = int(result.y * screenSize.y);

			x = max(x, 0);
			y = max(y, 0);
			x = min(x, screenSize.x - 1);
			y = min(y, screenSize.y - 1);

			maxX = max(x, maxX);
			maxY = max(y, maxY);
			minX = min(x, minX);
			minY = min(y, minY);

			/*
			// Visualize clipped polygons
			if(scene)
			{
				VC3 p1 = clipPolygon.vertices[j];
				VC3 p2 = clipPolygon.vertices[(j + 1) % clipPolygon.vertexAmount];


				for(int k = 0; k < 5; ++k)
				{
					const VC3 &planeNormal = frustum.planeNormals[k];
					PLANE plane;

					if(k == 0)
						plane.MakeFromNormalAndPosition(planeNormal, frustum.position + planeNormal);
					else
						plane.MakeFromNormalAndPosition(planeNormal, frustum.position);
	
					float d1 = plane.GetPointRange(p1);
					float d2 = plane.GetPointRange(p2);

					if(d1 < .25f)
						p1 += planeNormal * (.25f - d1);
					if(d2 < .25f)
						p2 += planeNormal * (.25f - d2);
				}

				scene->AddLine(p1, p2, COL(1.f, 1.f, 1.f));
			}
			*/
		}
	}

	SDL_Rect rc;
	bool visible = false;

	if(maxX > minX && maxY > minY)
	{
		visible = true;
		rc.x = minX;
		rc.y = minY;
		rc.w = maxX - minX;
		rc.h = maxY - minY;
	}
	else
	{
		visible = false;
		rc.x = 0;
		rc.y = 0;
		rc.w = 1;
		rc.h = 1;
	}

/*
	// Visualize scissor area
	if(scene && visible)
	{
		static DWORD foo = GetTickCount();
		int dif = (GetTickCount() - foo) % 2000;
		if(dif < 1000)
			scene->Render2D_Picture(0, VC2(float(minX), float(minY)), VC2(float(maxX - minX), float(maxY - minY)), 0.5f, 0.f, 0, 0, 0, 0, false);
	}
*/

	glScissor(rc.x, rc.y, rc.w, rc.h);
	glEnable(GL_SCISSOR_TEST);

	return visible;
}
