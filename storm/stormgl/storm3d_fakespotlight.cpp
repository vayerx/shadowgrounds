// Copyright 2002-2004 Frozenbyte Ltd.

#ifdef _MSC_VER
#pragma warning(disable:4103)
#endif

#include <string>
#include <vector>

#include <GL/glew.h>
#include "storm3d_fakespotlight.h"
#include "storm3d_spotlight_shared.h"
#include "Storm3D_ShaderManager.h"
#include "storm3d_camera.h"
#include "storm3d_terrain_utils.h"
#include "storm3d.h"
#include <IStorm3D_Logger.h>

#include "../../util/Debug_MemoryManager.h"

using namespace boost;

// FIXME since using shared color texture
// - SetViewport
// - Scale texcoords for reading


	int BUFFER_WIDTH = 512;
	int BUFFER_HEIGHT = 512;

	struct RenderTarget
	{
		boost::shared_ptr<glTexWrapper> color;
		boost::shared_ptr<glTexWrapper> depth;

		Framebuffer *fbo;

		/*int width;
		int height;*/

		bool hasInitialized() const
		{
			if(color && depth)
				return true;
			return false;
		}

		RenderTarget()
		{
			color = glTexWrapper::rgbaTexture(BUFFER_WIDTH, BUFFER_HEIGHT);
			depth = glTexWrapper::depthStencilTexture(BUFFER_WIDTH, BUFFER_HEIGHT);

			// TODO: cache
			fbo = new Framebuffer();
			fbo->setRenderTarget(color, depth);
			fbo->disable();

			if(!hasInitialized())
			{
				color.reset();
				depth.reset();
			}
		}

		~RenderTarget() {
			// TODO: put back in cache
			if (fbo != NULL) {
				delete fbo; fbo = NULL;
			}
		}

		bool set()
		{
			if(!color || !depth)
				return false;

			if (BUFFER_WIDTH <= 0 || BUFFER_HEIGHT <= 0)
				return false;

			// FIXME?
			fbo->activate();
			if(!fbo->validate()) {
				fbo->disable();
				return false;
			}

			glDisable(GL_SCISSOR_TEST);

			// White, alpha tested (Normal)
			glClearColor(1.0f, 1.0f, 1.0f, 0.0f);
			glClearDepth(1.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

			glViewport(0, 0, BUFFER_WIDTH, BUFFER_HEIGHT);

			return true;
		}

		void applyColor(int stage)
		{
			glActiveTexture(GL_TEXTURE0 + stage);
			glClientActiveTexture(GL_TEXTURE0 + stage);
			color->bind();
		}
	};


	struct ProjectionPlane
	{
		float height;

		VC2 min;
		VC2 max;

		ProjectionPlane()
		:	height(0)
		{
		}
	};


struct Storm3D_FakeSpotlight::Data
{
	Storm3D &storm; 

	shared_ptr<RenderTarget> renderTarget;
	Storm3D_SpotlightShared properties;
	Storm3D_Camera camera;
	ProjectionPlane plane;

	float fadeFactor;
	float fogFactor;
	bool enabled;
	bool visible;
	bool renderObjects;

	COL renderedColor;

	static frozenbyte::storm::VertexShader *shadowVertexShader;
	static frozenbyte::storm::PixelShader *depthPixelShader;
	static frozenbyte::storm::PixelShader *shadowPixelShader;
	static frozenbyte::storm::IndexBuffer *indexBuffer;
	frozenbyte::storm::VertexBuffer vertexBuffer;

	Data(Storm3D &storm_)
	:	storm(storm_),
		camera(&storm),
		fadeFactor(0),
		fogFactor(0.f),
		enabled(false),
		visible(false),
		renderObjects(true)
	{
		if(BUFFER_WIDTH <= 0 || BUFFER_HEIGHT <= 0)
			return;

		properties.color = COL(.5f, .5f, .5f);
		createVertexBuffer();

		renderTarget.reset(new RenderTarget());
	}

	void createVertexBuffer()
	{
		vertexBuffer.create(5, 5 * sizeof(float), true);
	}

	void updateMatrices(const D3DXMATRIX &cameraView)
	{
		properties.resolutionX = BUFFER_WIDTH;
		properties.resolutionY = BUFFER_HEIGHT;
		properties.updateMatrices(cameraView, 0);
	}
};

frozenbyte::storm::IndexBuffer *Storm3D_FakeSpotlight::Data::indexBuffer = 0;
frozenbyte::storm::VertexShader *Storm3D_FakeSpotlight::Data::shadowVertexShader = 0;
frozenbyte::storm::PixelShader *Storm3D_FakeSpotlight::Data::depthPixelShader = 0;
frozenbyte::storm::PixelShader *Storm3D_FakeSpotlight::Data::shadowPixelShader = 0;

//! Constructor
Storm3D_FakeSpotlight::Storm3D_FakeSpotlight(Storm3D &storm)
{
	scoped_ptr<Data> tempData(new Data(storm));
	data.swap(tempData);
}

//! Destructor
Storm3D_FakeSpotlight::~Storm3D_FakeSpotlight()
{
}

//! Test visibility of fake spotlight
/*!
	\param camera camera to which test visibility
*/
void Storm3D_FakeSpotlight::testVisibility(Storm3D_Camera &camera)
{
	// Frustum vs. AABB
	VC2 min, max;
	float planeY = data->properties.position.y - data->plane.height;
	getPlane(min, max);
	AABB clipBox ( VC3( min.x, planeY - 0.1f, min.y ), VC3( max.x, planeY + 0.1f, max.y ) );
	data->visible = camera.TestBoxVisibility ( clipBox.mmin, clipBox.mmax );

}

//! Disable visibility
void Storm3D_FakeSpotlight::disableVisibility()
{
	data->visible = false;
}

//! Enable of disable fake spotlight
/*!
	\param enable true to enable
*/
void Storm3D_FakeSpotlight::enable(bool enable)
{
	if(BUFFER_WIDTH <= 0 || BUFFER_HEIGHT <= 0)
		return;

	data->enabled = enable;
}

//! Is fake spotlight enabled?
/*!
	\return true if enabled
*/
bool Storm3D_FakeSpotlight::enabled() const
{
	return data->enabled && data->visible;
}

//! Set position of fake spotlight
/*!
	\param position position
*/
void Storm3D_FakeSpotlight::setPosition(const VC3 &position)
{
	data->properties.position = position;
}

//! Set direction of fake spotlight
/*!
	\param direction direction
*/
void Storm3D_FakeSpotlight::setDirection(const VC3 &direction)
{
	data->properties.direction = direction;
}

//! Set field of view of fake spotlight
/*!
	\param fov field of view
*/
void Storm3D_FakeSpotlight::setFov(float fov)
{
	data->properties.fov = fov;
}

//! Set range of fake spotlight
/*!
	\param range range
*/
void Storm3D_FakeSpotlight::setRange(float range)
{
	data->properties.range = range;
}

//! Set color of fake spotlight
/*!
	\param color color
	\param fadeFactor fade factor
*/
void Storm3D_FakeSpotlight::setColor(const COL &color, float fadeFactor)
{
	data->properties.color = color;
	data->fadeFactor = fadeFactor;
}

//! Set plane of fake spotlight
/*!
	\param height plane height
	\param minCorner
	\param maxCorner
*/
void Storm3D_FakeSpotlight::setPlane(float height, const VC2 &minCorner, const VC2 &maxCorner)
{
	data->plane.height = height;
	data->plane.min = minCorner;
	data->plane.max = maxCorner;
}

//! Set rendering of object shadows
/*!
	\param render true to enable rendering
*/
void Storm3D_FakeSpotlight::renderObjectShadows(bool render)
{
	data->renderObjects = render;
}

//! Set fog factor of fake spotlight
/*!
	\param factor fog factor
*/
void Storm3D_FakeSpotlight::setFogFactor(float factor)
{
	data->fogFactor = factor;
}

//! Get plane min and max
/*!
	\param min reference to a vector taking the min position
	\param max reference to a vector taking the max position
*/
void Storm3D_FakeSpotlight::getPlane(VC2 &min, VC2 &max) const
{
	min = data->plane.min;
	min.x += data->properties.position.x;
	min.y += data->properties.position.z;

	max = data->plane.max;
	max.x += data->properties.position.x;
	max.y += data->properties.position.z;
}

//! Get plane height
/*!
	\return height
*/
float Storm3D_FakeSpotlight::getPlaneHeight() const
{
	return data->properties.position.y - data->plane.height;
}

//! Should object shadows be rendered?
/*!
	\return true if rendered
*/
bool Storm3D_FakeSpotlight::shouldRenderObjectShadows() const
{
	return data->renderObjects;
}

//! Get camera
/*!
	\return camera
*/
Storm3D_Camera &Storm3D_FakeSpotlight::getCamera()
{
	return data->camera;
}

//! Set up clip planes
/*!
	\param cameraView
*/
void Storm3D_FakeSpotlight::setClipPlanes(const float *cameraView)
{
	if(BUFFER_WIDTH <= 0 || BUFFER_HEIGHT <= 0)
		return;

	data->properties.setClipPlanes(cameraView);
}

//! Set up scissor rectangle
/*!
	\param camera camera
	\param screenSize screen size
*/
void Storm3D_FakeSpotlight::setScissorRect(Storm3D_Camera &camera, const VC2I &screenSize)
{
	if(BUFFER_WIDTH <= 0 || BUFFER_HEIGHT <= 0)
		return;

	data->properties.setScissorRect(camera, screenSize);
}

//! Set as render target
/*!
	\param cameraView
	\return true if success
*/
bool Storm3D_FakeSpotlight::setAsRenderTarget(const D3DXMATRIX &cameraView)
{
	if(BUFFER_WIDTH <= 0 || BUFFER_HEIGHT <= 0)
		return false;

	data->depthPixelShader->apply();
	data->updateMatrices(cameraView);

	if(data->renderTarget && data->renderTarget->set())
	{
		Storm3D_ShaderManager::GetSingleton()->setTextureTm(data->properties.shaderProjection);
		Storm3D_ShaderManager::GetSingleton()->setSpot(COL(), data->properties.position, data->properties.direction, data->properties.range, .1f);
		// D3DXMatrixMultiply(lightViewProjection[i], lightView[i], lightProjection);
		Storm3D_ShaderManager::GetSingleton()->SetViewMatrix(data->properties.lightView);
		Storm3D_ShaderManager::GetSingleton()->SetProjectionMatrix(data->properties.lightProjection);
		//Storm3D_ShaderManager::GetSingleton()->SetViewProjectionMatrix(data->properties.lightProjection, data->properties.lightView);
	}

	COL c = data->properties.color;
	float colorData[4] = { c.r, c.g, c.b, 0 };
	glProgramEnvParameter4fvARB(GL_VERTEX_PROGRAM_ARB, 10, colorData);
	data->renderedColor = c;

	Storm3D_ShaderManager::GetSingleton()->setFakeProperties(data->properties.position.y - data->plane.height, .1f, .3137f);

	return true;
}

//! Apply textures
/*!
	\param cameraView
*/
void Storm3D_FakeSpotlight::applyTextures(const D3DXMATRIX &cameraView)
{
	if(BUFFER_WIDTH <= 0 || BUFFER_HEIGHT <= 0)
		return;

	data->shadowPixelShader->apply();
	data->updateMatrices(cameraView);

	if(data->renderTarget)
	{
		data->renderTarget->applyColor(0);
		data->renderTarget->applyColor(1);
		data->renderTarget->applyColor(2);
		data->renderTarget->applyColor(3);
		Storm3D_ShaderManager::GetSingleton()->setSpotTarget(data->properties.targetProjection);
	}

	Storm3D_ShaderManager::GetSingleton()->setTextureTm(data->properties.shaderProjection);
	Storm3D_ShaderManager::GetSingleton()->setSpot(COL(), data->properties.position, data->properties.direction, data->properties.range, .1f);
}

//! Render projection
void Storm3D_FakeSpotlight::renderProjection()
{
	if(BUFFER_WIDTH <= 0 || BUFFER_HEIGHT <= 0)
		return;

	if(!data->renderTarget)
		return;

	VC3 position = data->properties.position;
	position.y -= data->plane.height;

	const VC2 &min = data->plane.min;
	const VC2 &max = data->plane.max;
	float range = data->properties.range * sqrtf(2.f);

	VC3 a = position;
	a.x += min.x;
	a.z += min.y;
	float ad = a.GetRangeTo(position) / range;
	VC3 b = position;
	b.x += min.x;
	b.z += max.y;
	float bd = b.GetRangeTo(position) / range;
	VC3 c = position;
	c.x += max.x;
	c.z += min.y;
	float cd = c.GetRangeTo(position) / range;
	VC3 d = position;
	d.x += max.x;
	d.z += max.y;
	float dd = d.GetRangeTo(position) / range;
	VC3 e = position;
	float ed = 0.f;

	ad = bd = cd = dd = 1.f;

	float buffer[] = 
	{
		a.x, a.y, a.z,  ad, .5f,
		b.x, b.y, b.z,  bd, .5f,
		c.x, c.y, c.z,  cd, .5f,
		d.x, d.y, d.z,  dd, .5f,
		e.x, e.y, e.z,  ed, .5f
	};

	data->shadowVertexShader->apply(); // fake_shadow_plane_vertex_shader.txt

	memcpy(data->vertexBuffer.lock(), buffer, 5 * 5 * sizeof(float));
	data->vertexBuffer.unlock();
	data->vertexBuffer.apply(0);

	Storm3D_ShaderManager *manager = Storm3D_ShaderManager::GetSingleton();
	manager->setTextureTm(data->properties.shaderProjection);
	manager->setSpot(COL(), data->properties.position, data->properties.direction, data->properties.range, .1f);

	D3DXMATRIX identity;

	manager->setSpotTarget(data->properties.targetProjection);
	manager->SetWorldTransform(identity);

	glEnable(GL_ALPHA_TEST);
	glAlphaFunc(GL_GREATER, 1.0f/255.0f);

	{
		float factor = (1.f - data->fadeFactor) + data->fogFactor * (data->fadeFactor);
		float c0[4] = { factor, factor, factor, 1.f };
		glProgramEnvParameter4fvARB(GL_FRAGMENT_PROGRAM_ARB, 0, c0);

		float fogFactor = data->fogFactor;
		float c2[4] = { fogFactor, fogFactor, fogFactor, 1.f };
		glProgramEnvParameter4fvARB(GL_FRAGMENT_PROGRAM_ARB, 2, c2);
	}

	{
		float xd = 1.f / float(data->renderTarget->color->getWidth());
		float yd = 1.f / float(data->renderTarget->color->getHeight());
		float xd1 = xd * 1.5f;
		float yd1 = yd * 1.5f;
		float xd2 = xd * 2.5f;
		float yd2 = yd * 2.5f;

		float deltas1[4] = { -xd2, -yd2, 0, 0 };
		float deltas2[4] = { -xd1,  yd2, 0, 0 };
		float deltas3[4] = {  xd1, -yd1, 0, 0 };
		float deltas4[4] = {  xd2,  yd1, 0, 0 };

		glProgramEnvParameter4fvARB(GL_VERTEX_PROGRAM_ARB, 5, deltas1);
		glProgramEnvParameter4fvARB(GL_VERTEX_PROGRAM_ARB, 6, deltas2);
		glProgramEnvParameter4fvARB(GL_VERTEX_PROGRAM_ARB, 7, deltas3);
		glProgramEnvParameter4fvARB(GL_VERTEX_PROGRAM_ARB, 8, deltas4);
	}

	data->indexBuffer->render(4, 5);
}

void Storm3D_FakeSpotlight::debugRender()
{
	igios_unimplemented();
}

//! Release dynamic resources
void Storm3D_FakeSpotlight::releaseDynamicResources()
{
	if(BUFFER_WIDTH <= 0 || BUFFER_HEIGHT <= 0)
		return;

	data->vertexBuffer.release();
}

//! Recreate dynamic resources
void Storm3D_FakeSpotlight::recreateDynamicResources()
{
	if(BUFFER_WIDTH <= 0 || BUFFER_HEIGHT <= 0)
		return;

	data->createVertexBuffer();
}

//! Filter buffers
/*!
	\param storm Storm3D
*/
void Storm3D_FakeSpotlight::filterBuffers(Storm3D &storm)
{
	if(BUFFER_WIDTH <= 0 || BUFFER_HEIGHT <= 0)
		return;
}

//! Set up buffer sizes
/*!
	\param storm Storm3D
	\param shadowQuality shadow quality
*/
void Storm3D_FakeSpotlight::querySizes(Storm3D &storm, int shadowQuality)
{
	if(shadowQuality >= 100)
	{
		BUFFER_WIDTH = 1024;
		BUFFER_HEIGHT = 1024;
	}
	/*else if(shadowQuality >= 75)
	{
		BUFFER_WIDTH = 768;
		BUFFER_HEIGHT = 768;
	}*/
	else if(shadowQuality >= 50)
	{
		BUFFER_WIDTH = 512;
		BUFFER_HEIGHT = 512;
	}
	/*else if(shadowQuality >= 25)
	{
		BUFFER_WIDTH = 384;
		BUFFER_HEIGHT = 384;
	}*/
	else if(shadowQuality >= 0)
	{
		BUFFER_WIDTH = 256;
		BUFFER_HEIGHT = 256;
	}
	else if(shadowQuality == -1)
	{
		BUFFER_WIDTH = -1;
		BUFFER_HEIGHT = -1;
	}

	if(BUFFER_WIDTH > 0 && BUFFER_HEIGHT > 0)
	{
		storm.setNeededColorTarget(VC2I(BUFFER_WIDTH * 2, BUFFER_HEIGHT * 2));
		storm.setNeededDepthTarget(VC2I(BUFFER_WIDTH * 2, BUFFER_HEIGHT * 2));
	}
}

//! Create buffers
/*!
	\param storm Storm3D
	\param shadowQuality shadow quality
*/
void Storm3D_FakeSpotlight::createBuffers(Storm3D &storm, int shadowQuality)
{
	if(BUFFER_WIDTH <= 0 || BUFFER_HEIGHT <= 0)
		return;

	Storm3D_FakeSpotlight::Data::indexBuffer = new frozenbyte::storm::IndexBuffer();
	{
		Storm3D_FakeSpotlight::Data::indexBuffer->create(4, false);
		WORD *pointer = Storm3D_FakeSpotlight::Data::indexBuffer->lock();
		
		*pointer++ = 4;
		*pointer++ = 0;
		*pointer++ = 1;

		*pointer++ = 2;
		*pointer++ = 0;
		*pointer++ = 4;

		*pointer++ = 4;
		*pointer++ = 3;
		*pointer++ = 2;

		*pointer++ = 3;
		*pointer++ = 4;
		*pointer++ = 1;

		Storm3D_FakeSpotlight::Data::indexBuffer->unlock();
	}

	Storm3D_FakeSpotlight::Data::shadowVertexShader = new frozenbyte::storm::VertexShader();
	Storm3D_FakeSpotlight::Data::shadowVertexShader->createFakePlaneShadowShader();

	Storm3D_FakeSpotlight::Data::depthPixelShader = new frozenbyte::storm::PixelShader();
	Storm3D_FakeSpotlight::Data::depthPixelShader->createFakeDepthPixelShader();
	Storm3D_FakeSpotlight::Data::shadowPixelShader = new frozenbyte::storm::PixelShader();
	Storm3D_FakeSpotlight::Data::shadowPixelShader->createFakeShadowPixelShader();
}

//! Free buffers
void Storm3D_FakeSpotlight::freeBuffers()
{
	if(BUFFER_WIDTH <= 0 || BUFFER_HEIGHT <= 0)
		return;

	delete Storm3D_FakeSpotlight::Data::indexBuffer; Storm3D_FakeSpotlight::Data::indexBuffer = 0;
	delete Storm3D_FakeSpotlight::Data::shadowVertexShader; Storm3D_FakeSpotlight::Data::shadowVertexShader = 0;
	delete Storm3D_FakeSpotlight::Data::depthPixelShader; Storm3D_FakeSpotlight::Data::depthPixelShader = 0;
	delete Storm3D_FakeSpotlight::Data::shadowPixelShader; Storm3D_FakeSpotlight::Data::shadowPixelShader = 0;
}
