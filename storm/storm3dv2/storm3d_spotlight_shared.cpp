// Copyright 2002-2004 Frozenbyte Ltd.

#ifdef _MSC_VER
#pragma warning(disable:4103)
#endif

#include <math.h>
#include <algorithm>
#include "storm3d_spotlight_shared.h"
#include "storm3d_camera.h"
#include "storm3d_scene.h"
#include "../../util/Debug_MemoryManager.h"
#include <c2_frustum.h>

#ifdef _MSC_VER
#define MIN(a,b) ( min( (a), (b) ) )
#define MAX(a,b) ( max( (a), (b) ) )
#else
#define MIN(a,b) (std::max( (a), (b) ) )
#define MAX(a,b) (std::min( (a), (b) ) )
#endif

Storm3D_SpotlightShared::Storm3D_SpotlightShared(IDirect3DDevice9 &device_)
:	device(device_),
	range(20.f),
	fov(60.f)
{
	soffsetX = 0.25f;
	soffsetY = 0.25f;
	scaleX = 0.25f;
	scaleY = 0.25f;

	resolutionX = 1;
	resolutionY = 1;
}

Storm3D_SpotlightShared::~Storm3D_SpotlightShared()
{
}

void Storm3D_SpotlightShared::updateMatrices(const D3DXMATRIX &cameraView, float bias)
{
	D3DXVECTOR3 lightPosition(position.x, position.y, position.z);
	D3DXVECTOR3 up(0, 1.f, 0);
	D3DXVECTOR3 lookAt = lightPosition;
	lookAt += D3DXVECTOR3(direction.x, direction.y, direction.z);

	D3DXMatrixPerspectiveFovLH(&lightProjection, D3DXToRadian(fov), 1.f, .2f, range);
	D3DXMATRIX cameraMatrix(cameraView);
	float det = D3DXMatrixDeterminant(&cameraMatrix);
	D3DXMatrixInverse(&cameraMatrix, &det, &cameraMatrix);

	float currentBias = bias;
	for(int i = 0; i < 2; ++i)
	{	
		D3DXMatrixLookAtLH(&lightView[i], &lightPosition, &lookAt, &up);
		//if(i == 1)
		//	currentBias = 0;
		if(i == 1)
			currentBias = 1.f;

		// Tweak matrix
		float soffsetX = 0.5f;
		float soffsetY = 0.5f;
		float scale = 0.5f;

		/*
		D3DXMATRIX shadowTweak( scale,    0.0f,     0.0f,				0.0f,
								0.0f,     -scale,   0.0f,				0.0f,
								0.0f,      0.0f,     float(tweakRange),	0.0f,
								soffsetX,  soffsetY, currentBias,		1.0f );
		*/
		D3DXMATRIX shadowTweak( scale,    0.0f,      0.0f,				0.0f,
								0.0f,     -scale,    0.0f,				0.0f,
								0.0f,      0.0f,	 currentBias,		0.0f,
								soffsetX,  soffsetY, 0.f,				1.0f );

		D3DXMatrixMultiply(&shadowProjection[i], &lightProjection, &shadowTweak);
		D3DXMatrixMultiply(&shadowProjection[i], &lightView[i], &shadowProjection[i]);
		D3DXMatrixMultiply(&lightViewProjection[i], &lightView[i], &lightProjection);

		shaderProjection[i] = shadowProjection[i];
		D3DXMatrixMultiply(&shadowProjection[i], &cameraMatrix, &shadowProjection[i]);
	}

	{
		float xf = (1.f / resolutionX * .5f);
		float yf = (1.f / resolutionY * .5f);
		float sX = soffsetX + (2 * targetPos.x * soffsetX) - xf;
		float sY = soffsetY + (2 * targetPos.y * soffsetY) - yf;

		/*
		D3DXMATRIX shadowTweak( scaleX,    0.0f,	0.0f,				0.0f,
								0.0f,    -scaleY,	0.0f,				0.0f,
								0.0f,     0.0f,     float(tweakRange),	0.0f,
								sX,       sY,		bias,				1.0f );
		*/

		D3DXMATRIX shadowTweak( scaleX,    0.0f,	0.0f,				0.0f,
								0.0f,    -scaleY,	0.0f,				0.0f,
								0.0f,     0.0f,     bias,				0.0f,
								sX,       sY,		0.f,				1.0f );

		D3DXMatrixMultiply(&targetProjection, &lightProjection, &shadowTweak);
		D3DXMatrixMultiply(&targetProjection, &lightView[0], &targetProjection);
	}
}

void Storm3D_SpotlightShared::updateMatricesOffCenter(const D3DXMATRIX &cameraView, const VC2 &min, const VC2 &max, float height, Storm3D_Camera &camera)
{
	// Position of the light in global coordinates
	// Y-axis is height
	D3DXVECTOR3 lightPosition(position.x, position.y, position.z);
	// Up vector (z-axis)
	D3DXVECTOR3 up(0.f, 0.f, 1.f);
	// Look direction
	D3DXVECTOR3 lookAt = lightPosition;
	lookAt.y -= 1.f;

	{
		// max and min define the extents of light area in local coordinates
		// Z-axis is height
		float zmin = 0.2f;
		//float zmax = std::max(range, height) * 1.4f;
		// height is light height from light properties
		float zmax = height;
		float factor = 1.5f * zmin / height;
		float xmin = min.x * factor;
		float xmax = max.x * factor;
		float ymin = min.y * factor;
		float ymax = max.y * factor;
		D3DXMatrixPerspectiveOffCenterLH(&lightProjection, xmin, xmax, ymin, ymax, zmin, zmax);

		// Calculate the extents of light area in global coordinates
		VC2 worldMin = min;
		worldMin.x += position.x;
		worldMin.y += position.z;
		VC2 worldMax = max;
		worldMax.x += position.x;
		worldMax.y += position.z;

		// Generate approximate camera for culling.

		// Calculate range of the camera.
		// Y-axis is height
		float planeY = position.y - height;
		// Calculate distances from light position to light plane edges
		VC3 p1 = VC3( worldMin.x, planeY, worldMin.y ) - position;
		VC3 p2 = VC3( worldMax.x, planeY, worldMin.y ) - position;
		VC3 p3 = VC3( worldMax.x, planeY, worldMax.y ) - position;
		VC3 p4 = VC3( worldMin.x, planeY, worldMax.y ) - position;
		float d1 = p1.GetLength();
		float d2 = p2.GetLength();
		float d3 = p3.GetLength();
		float d4 = p4.GetLength();
		float maxRange = 0.0f;
		maxRange = MAX( maxRange, d1 );
		maxRange = MAX( maxRange, d2 );
		maxRange = MAX( maxRange, d3 );
		maxRange = MAX( maxRange, d4 );
		//maxRange = sqrtf(maxRange);

		// Calculate FOV of the camera.
		VC3 planeCenter = VC3( (worldMin.x + worldMax.x) * 0.5f, planeY, (worldMin.y + worldMax.y) * 0.5f );
		VC3 camVec = planeCenter - position;
		camVec.Normalize();
		float minDot = 10000.0f;
		float t1 = camVec.GetDotWith( p1 ) / d1;
		float t2 = camVec.GetDotWith( p2 ) / d2;
		float t3 = camVec.GetDotWith( p3 ) / d3;
		float t4 = camVec.GetDotWith( p4 ) / d4;
		minDot = MIN( minDot, t1 );
		minDot = MIN( minDot, t2 );
		minDot = MIN( minDot, t3 );
		minDot = MIN( minDot, t4 );
		float maxAngle = acosf( minDot );

		// Place camera to light position
		camera.SetPosition(position);
		camera.SetUpVec(VC3(0.f, 0.f, 1.f));
		// Point camera at light plane center
		camera.SetTarget(planeCenter);
		camera.SetFieldOfView( maxAngle );
		camera.SetVisibilityRange( maxRange );
	}
	
	D3DXMATRIX cameraMatrix(cameraView);
	float det = D3DXMatrixDeterminant(&cameraMatrix);
	D3DXMatrixInverse(&cameraMatrix, &det, &cameraMatrix);

	unsigned int tweakRange = 1;
	float bias = 0.f;
	float currentBias = 0.f;
	for(int i = 0; i < 2; ++i)
	{	
		D3DXMatrixLookAtLH(&lightView[i], &lightPosition, &lookAt, &up);
		if(i == 1)
			currentBias = 0;

		// Tweak matrix
		float soffsetX = 0.5f;
		float soffsetY = 0.5f;
		float scale = 0.5f;

		D3DXMATRIX shadowTweak( scale,    0.0f,     0.0f,				0.0f,
								0.0f,     -scale,   0.0f,				0.0f,
								0.0f,      0.0f,     float(tweakRange),	0.0f,
								soffsetX,  soffsetY, currentBias,		1.0f );

		D3DXMatrixMultiply(&shadowProjection[i], &lightProjection, &shadowTweak);
		D3DXMatrixMultiply(&shadowProjection[i], &lightView[i], &shadowProjection[i]);
		D3DXMatrixMultiply(&lightViewProjection[i], &lightView[i], &lightProjection);

		shaderProjection[i] = shadowProjection[i];
		D3DXMatrixMultiply(&shadowProjection[i], &cameraMatrix, &shadowProjection[i]);
	}

	{
		float xf = (1.f / resolutionX * .5f);
		float yf = (1.f / resolutionY * .5f);
		float sX = soffsetX + (2 * targetPos.x * soffsetX) - xf;
		float sY = soffsetY + (2 * targetPos.y * soffsetY) - yf;

		D3DXMATRIX shadowTweak( scaleX,    0.0f,	0.0f,				0.0f,
								0.0f,    -scaleY,	0.0f,				0.0f,
								0.0f,     0.0f,     float(tweakRange),	0.0f,
								sX,       sY,		bias,				1.0f );

		D3DXMatrixMultiply(&targetProjection, &lightProjection, &shadowTweak);
		D3DXMatrixMultiply(&targetProjection, &lightView[0], &targetProjection);
	}
}

void Storm3D_SpotlightShared::setClipPlanes(const float *cameraView)
{

	D3DXMATRIX m(cameraView);
	float determinant = D3DXMatrixDeterminant(&m);
	D3DXMatrixInverse(&m, &determinant, &m);
	D3DXMatrixTranspose(&m, &m);

	D3DXVECTOR3 d(direction.x, direction.y, direction.z);
	VC2 bd(d.x, d.z);
	bd.Normalize();		
	D3DXVECTOR3 p1(position.x - 8*bd.x, position.y, position.z - 8*bd.y);
	//D3DXVECTOR3 p1(position.x - 1*bd.x, position.y, position.z - 1*bd.y);
	D3DXVECTOR3 p2(p1.x, p1.y + 5.f, p1.z);

	float angle = D3DXToRadian(fov) * .55f;

	D3DXPLANE leftPlane;
	D3DXMATRIX leftTransform;
	D3DXMatrixRotationY(&leftTransform, -angle);
	D3DXVECTOR3 leftPoint(direction.x, 0, direction.z);
	D3DXVECTOR4 leftPoint2;
	D3DXVec3Transform(&leftPoint2, &leftPoint, &leftTransform);
	leftPoint = p1;
	leftPoint.x += leftPoint2.x;
	leftPoint.z += leftPoint2.z;
	D3DXPlaneFromPoints(&leftPlane, &p1, &p2, &leftPoint);
	D3DXPlaneNormalize(&leftPlane, &leftPlane);
	D3DXPlaneTransform(&leftPlane, &leftPlane, &m);

	D3DXPLANE rightPlane;
	D3DXMATRIX rightTransform;
	D3DXMatrixRotationY(&rightTransform, angle);
	D3DXVECTOR3 rightPoint(direction.x, 0, direction.z);
	D3DXVECTOR4 rightPoint2;
	D3DXVec3Transform(&rightPoint2, &rightPoint, &rightTransform);
	rightPoint = p1;
	rightPoint.x += rightPoint2.x;
	rightPoint.z += rightPoint2.z;
	D3DXPlaneFromPoints(&rightPlane, &rightPoint, &p2, &p1);
	D3DXPlaneNormalize(&rightPlane, &rightPlane);
	D3DXPlaneTransform(&rightPlane, &rightPlane, &m);

	D3DXPLANE backPlane;
	D3DXVECTOR3 pb(p1.x, p1.y, p1.z);
	D3DXPlaneFromPointNormal(&backPlane, &pb, &d);
	D3DXPlaneNormalize(&backPlane, &backPlane);
	D3DXPlaneTransform(&backPlane, &backPlane, &m);

	device.SetClipPlane(0, leftPlane);
	device.SetClipPlane(1, rightPlane);
	device.SetClipPlane(2, backPlane);
	device.SetRenderState(D3DRS_CLIPPLANEENABLE, D3DCLIPPLANE0 | D3DCLIPPLANE1 | D3DCLIPPLANE2);
}

namespace {

	VC3 toVC3(const D3DXVECTOR3 &v)
	{
		return VC3(v.x, v.y, v.z);
	}

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
			/*
			float rhw = 0, realZ = 0;

			result.x = std::max(0.f, result.x);
			result.y = std::max(0.f, result.y);
			result.x = std::min(1.f, result.x);
			result.y = std::min(1.f, result.y);

			//if(fabsf(rhw) < 0.0001f)
			//	continue;

			bool flip = false;
			if(realZ < 0)
				flip = true;

			if(flip)
			{
				result.x = 1.f - result.x;
				result.y = 1.f - result.y;
			}
			*/

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
	D3DXMATRIX light;
	D3DXVECTOR3 lightPosition(position.x, position.y, position.z);
	D3DXVECTOR3 up(0, 1.f, 0);
	D3DXVECTOR3 lookAt = lightPosition;
	lookAt += D3DXVECTOR3(direction.x, direction.y, direction.z);
	D3DXMatrixLookAtLH(&light, &lightPosition, &lookAt, &up);

	// Create frustum vertices

	D3DXVECTOR3 v[5];
	v[0] = D3DXVECTOR3(0, 0, 0);
	v[1] = D3DXVECTOR3(0, 0, 1.f);
	v[2] = D3DXVECTOR3(0, 0, 1.f);
	v[3] = D3DXVECTOR3(0, 0, 1.f);
	v[4] = D3DXVECTOR3(0, 0, 1.f);

	float det = D3DXMatrixDeterminant(&light);
	D3DXMatrixInverse(&light, &det, &light);
	float angle = D3DXToRadian(fov) * .5f;
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

		D3DXVec3TransformCoord(&v[i], &v[i], &light);
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
			v1 = toVC3(v[0]);
			v2 = toVC3(v[1]);
			v3 = toVC3(v[2]);
		}
		else if(i == 1)
		{
			v1 = toVC3(v[0]);
			v2 = toVC3(v[2]);
			v3 = toVC3(v[4]);
		}
		else if(i == 2)
		{
			v1 = toVC3(v[0]);
			v2 = toVC3(v[3]);
			v3 = toVC3(v[4]);
		}
		else if(i == 3)
		{
			v1 = toVC3(v[0]);
			v2 = toVC3(v[1]);
			v3 = toVC3(v[3]);
		}
		else if(i == 4)
		{
			v1 = toVC3(v[1]);
			v2 = toVC3(v[2]);
			v3 = toVC3(v[3]);
		}
		else if(i == 5)
		{
			v1 = toVC3(v[4]);
			v2 = toVC3(v[2]);
			v3 = toVC3(v[3]);
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
			//if(x < -1 || x > screenSize.x)
			//	continue;
			//if(y < -1 || x > screenSize.y)
			//	continue;

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

	RECT rc;
	bool visible = false;

	if(maxX > minX && maxY > minY)
	{
		visible = true;
		rc.left = minX;
		rc.top = minY;
		rc.right = maxX;
		rc.bottom = maxY;
	}
	else
	{
		visible = false;
		rc.left = 0;
		rc.top = 0;
		rc.right = 1;
		rc.bottom = 1;
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
	device.SetScissorRect(&rc);
	device.SetRenderState(D3DRS_SCISSORTESTENABLE, TRUE);

	return visible;
}
