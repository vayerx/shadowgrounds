// Copyright 2002-2004 Frozenbyte Ltd.

#ifdef _MSC_VER
#pragma warning(disable:4103)
#endif

#include <vector>
#include <string>

#include "storm3d_spotlight.h"
#include "storm3d_spotlight_shared.h"
#include "storm3d_terrain_utils.h"
#include "storm3d_texture.h"
#include "Storm3D_ShaderManager.h"
#include "storm3d_adapter.h"
#include "storm3d_camera.h"
#include "storm3d.h"
#include "igios3D.h"
#include <IStorm3D_Logger.h>
#include "c2_sphere.h"
#include <math.h>

#include "../../util/Debug_MemoryManager.h"

namespace {
	int SHADOW_WIDTH = 512;
	int SHADOW_HEIGHT = 512;
	static const bool SHARE_BUFFERS = true;

	static const int CONE_CIRCLE_VERTICES = 100;
	static const int CONE_BASE_VERTICES = CONE_CIRCLE_VERTICES;
	static const int CONE_VERTICES = CONE_CIRCLE_VERTICES + CONE_BASE_VERTICES + 1;
	static const int CONE_FACES = (CONE_CIRCLE_VERTICES);

	bool targetActive = false;


} // unnamed

struct Storm3D_SpotlightData
{
	Storm3D &storm;

	boost::shared_ptr<glTexWrapper> shadowMap;
	boost::shared_ptr<glTexWrapper> tempColor;
	Framebuffer *fbo;
	boost::shared_ptr<Storm3D_Texture> projectionTexture;
	boost::shared_ptr<Storm3D_Texture> coneTexture;

	Storm3D_SpotlightShared properties;
	float coneColorMultiplier;
	float smoothness;

	bool smoothing;
	Storm3D_Camera camera;

	bool hasFade;
	bool hasCone;
	bool hasShadows;
	bool enabled;
	bool visible;
	bool coneUpdated;

	frozenbyte::storm::VertexBuffer coneVertexBuffer;
	frozenbyte::storm::IndexBuffer coneIndexBuffer;
	bool updateCone;
	bool scissorRect;

	frozenbyte::storm::VertexBuffer coneStencilVertexBuffer;
	frozenbyte::storm::IndexBuffer coneStencilIndexBuffer;

	const IStorm3D_Model *noShadowModel;
	IStorm3D_Spotlight::Type light_type;

	float coneFov;
	float angle[2];
	float speed[2];

	bool spotlightAlwaysVisible;

	static frozenbyte::storm::PixelShader *nvShadowPixelShader;
	static frozenbyte::storm::PixelShader *nvSmoothShadowPixelShader;
	static frozenbyte::storm::PixelShader *nvNoShadowPixelShader;
	static frozenbyte::storm::PixelShader *coneNvPixelShader_Texture;
	static frozenbyte::storm::PixelShader *coneNvPixelShader_NoTexture;
	static frozenbyte::storm::VertexShader *coneStencilVertexShader;

	Storm3D_SpotlightData(Storm3D &storm_)
	:	storm(storm_),
		properties(),
		coneColorMultiplier(0.3f),
		smoothness(5.f),
		smoothing(false),
		camera(&storm),
		hasFade(false),
		hasCone(false),
		hasShadows(false),
		enabled(true),
		visible(false),
		coneUpdated(false),

		updateCone(true),
		scissorRect(true),
		noShadowModel(0),
		light_type(IStorm3D_Spotlight::Directional),
		coneFov(0),
		spotlightAlwaysVisible(false)
	{
		properties.direction = VC3(0, 0, 1.f);
		properties.color = COL(1.f, 1.f, 1.f);

		shadowMap = glTexWrapper::depthStencilTexture(SHADOW_WIDTH, SHADOW_HEIGHT);
		tempColor = glTexWrapper::rgbaTexture(SHADOW_WIDTH, SHADOW_HEIGHT);

		// TODO: cache
		fbo = new Framebuffer();
		fbo->setRenderTarget(tempColor, shadowMap);
		fbo->disable();

		angle[0] = angle[1] = 0.f;
		speed[0] = 0.07f;
		speed[1] = -0.11f;
	}

	~Storm3D_SpotlightData() {
		// TODO: put back in cache
		if (fbo != NULL) {
			delete fbo; fbo = NULL;
		}
	}

	void updateMatrices(const D3DXMATRIX &cameraView)
	{
		properties.resolutionX = SHADOW_WIDTH;
		properties.resolutionY = SHADOW_HEIGHT;
		properties.updateMatrices(cameraView, 0.9989f);

		camera.SetPosition(properties.position);
		camera.SetTarget(properties.position + properties.direction);
		camera.SetFieldOfView(DegToRadian(properties.fov));
		camera.SetVisibilityRange(properties.range);
	}

	bool setScissorRect(Storm3D_Camera &camera, const VC2I &screenSize, Storm3D_Scene &scene)
	{
		return properties.setScissorRect(camera, screenSize, &scene);
	}

	void setClipPlanes(const float *cameraView)
	{
		properties.setClipPlanes(cameraView);
	}

	void createCone()
	{
		if(!hasCone || !updateCone)
			return;

		// Vertices
		{
			// Uh oh. Undefined casting etc, should clean up sometime
			// -- psd
			struct VertexType
			{
				float position[3];
				float normal[3];
				unsigned int color;
				float uv[2];

				void setPosition(float x, float y, float z)
				{
					position[0] = x;
					position[1] = y;
					position[2] = z;
				}

				void setNormal(float x, float y, float z)
				{
					normal[0] = x;
					normal[1] = y;
					normal[2] = z;
				}

				void setColor(unsigned int color_)
				{
					color = color_;
				}

				void setUv(float u, float v)
				{
					uv[0] = u;
					uv[1] = v;
				}
			};

			static const unsigned char color = 255;
			coneVertexBuffer.create(CONE_VERTICES, 3 * sizeof(float) + 3 * sizeof(float) + sizeof(unsigned int) + 2 * sizeof(float), false);
			VertexType *buffer = reinterpret_cast<VertexType *> (coneVertexBuffer.lock());

			for(int i = 0; i < CONE_BASE_VERTICES; ++i)
			{
				float angle = (float(i) / (CONE_BASE_VERTICES)) * PI * 2.f;

				float nx = sinf(angle);
				float ny = cosf(angle);
				float nz = 0;
				float u = .5f;
				float v = .5f;

				buffer->setPosition(0, 0, 0);
				buffer->setNormal(nx, ny, nz);
				buffer->setColor(COLOR_RGBA(color, color, color, color));
				buffer->setUv(u, v);
				++buffer;
			}

			float coneRange = properties.range * .5f;
			float farMul = tanf(DegToRadian(coneFov / 2)) * coneRange * .9f;

			// Create circle
			for(int i = 0; i < CONE_CIRCLE_VERTICES; ++i)
			{
				float angle = (float(i) / (CONE_CIRCLE_VERTICES)) * PI * 2.f;
				float x = sinf(angle);
				float y = cosf(angle);
				float z = coneRange;

				float nx = x;
				float ny = y;
				float nz = 0;

				float u = (x * .5f) + .5f;
				float v = (y * .5f) + .5f;

				buffer->setPosition(x * farMul, y * farMul, z);
				buffer->setNormal(nx, ny, nz);
				buffer->setColor(COLOR_RGBA(color, color, color, color));
				buffer->setUv(u, v);
				++buffer;
			}

			coneVertexBuffer.unlock();
		}

		// Faces
		if(!coneIndexBuffer)
		{
			coneIndexBuffer.create(CONE_FACES, false);
			unsigned short *indexBuffer = coneIndexBuffer.lock();

			for(int i = 0; i < CONE_CIRCLE_VERTICES; ++i)
			{
				int base = CONE_BASE_VERTICES;
				int last = i - 1;
				if(i == 0)
					last = CONE_CIRCLE_VERTICES - 1;

				*indexBuffer++ = i + base;
				*indexBuffer++ = i;
				*indexBuffer++ = last + base;
			}

			coneIndexBuffer.unlock();
		}
	}

	void createStencilCone()
	{
		// Vertices
		{
			// Uh oh. Undefined casting etc, should clean up sometime
			// -- psd
			struct VertexType
			{
				float position[3];

				void setPosition(float x, float y, float z)
				{
					position[0] = x;
					position[1] = y;
					position[2] = z;
				}
			};

			coneStencilVertexBuffer.create(CONE_VERTICES, 3 * sizeof(float), false);
			VertexType *buffer = reinterpret_cast<VertexType *> (coneStencilVertexBuffer.lock());

			for(int i = 0; i < CONE_BASE_VERTICES; ++i)
			{
				buffer->setPosition(0, 0, 0.1f);
				++buffer;
			}

			float coneRange = properties.range;
			float farMul = tanf(DegToRadian(properties.fov / 2)) * coneRange * .9f;

			// Create circle
			for(int i = 0; i < CONE_CIRCLE_VERTICES; ++i)
			{
				float angle = (float(i) / (CONE_CIRCLE_VERTICES)) * PI * 2.f;
				float x = sinf(angle);
				float y = cosf(angle);
				float z = coneRange;

				buffer->setPosition(x * farMul, y * farMul, z);
				++buffer;
			}

			coneStencilVertexBuffer.unlock();
		}

		// Faces
		{
			coneStencilIndexBuffer.create(CONE_FACES, false);
			unsigned short *indexBuffer = coneStencilIndexBuffer.lock();

			for(int i = 0; i < CONE_CIRCLE_VERTICES; ++i)
			{
				int base = CONE_BASE_VERTICES;
				int last = i - 1;
				if(i == 0)
					last = CONE_CIRCLE_VERTICES - 1;

				*indexBuffer++ = i + base;
				*indexBuffer++ = i;
				*indexBuffer++ = last + base;
			}

			coneStencilIndexBuffer.unlock();
		}
	}
};

frozenbyte::storm::PixelShader *Storm3D_SpotlightData::nvShadowPixelShader = 0;
frozenbyte::storm::PixelShader *Storm3D_SpotlightData::nvSmoothShadowPixelShader = 0;
frozenbyte::storm::PixelShader *Storm3D_SpotlightData::nvNoShadowPixelShader = 0;
frozenbyte::storm::PixelShader *Storm3D_SpotlightData::coneNvPixelShader_Texture = 0;
frozenbyte::storm::PixelShader *Storm3D_SpotlightData::coneNvPixelShader_NoTexture = 0;
frozenbyte::storm::VertexShader *Storm3D_SpotlightData::coneStencilVertexShader = 0;

//! Constructor
Storm3D_Spotlight::Storm3D_Spotlight(Storm3D &storm)
{
	boost::scoped_ptr<Storm3D_SpotlightData> tempData(new Storm3D_SpotlightData(storm));
	data.swap(tempData);
}

//! Destructor
Storm3D_Spotlight::~Storm3D_Spotlight()
{
}

//! Test visibility of spotlight
/*!
	\param camera camera to which test visibility
*/
void Storm3D_Spotlight::testVisibility(Storm3D_Camera &camera)
{
	// Simple hack, skip visibility check if camera has orthogonal projection on (practically when mapview is enabled)
	if(!(data->spotlightAlwaysVisible || camera.GetForcedOrthogonalProjectionEnabled() ) )
	{
		data->visible = camera.TestSphereVisibility(data->properties.position, data->properties.range);
		data->coneUpdated = false;
	}
	else
	{
		data->visible = true;
		data->coneUpdated = false;
	}
}

//! Enable of disable spotlight
/*!
	\param enable true to enable
*/
void Storm3D_Spotlight::enable(bool enable)
{
	data->enabled = enable;
}

//! Is spotlight enabled?
/*!
	\return true if enabled
*/
bool Storm3D_Spotlight::enabled() const
{
	if(!data->enabled || !data->visible)
		return false;

	return true;
}

//! Set field of view of spotlight
/*!
	\param fov field of view
*/
void Storm3D_Spotlight::setFov(float fov)
{
	data->properties.fov = fov;
	data->updateCone = true;
	data->coneFov = fov;
}

//! Set cone field of view of spotlight
/*!
	\param fov field of view
*/
void Storm3D_Spotlight::setConeFov(float fov)
{
	data->coneFov = fov;
}

//! Set range of spotlight
/*!
	\param range range
*/
void Storm3D_Spotlight::setRange(float range)
{
	data->properties.range = range;
	data->updateCone = true;
}

void Storm3D_Spotlight::setClipRange(float range)
{
}

//! Set position of spotlight
/*!
	\param position position
*/
void Storm3D_Spotlight::setPosition(const VC3 &position)
{
	data->properties.position = position;
}

//! Set direction of spotlight
/*!
	\param direction direction
*/
void Storm3D_Spotlight::setDirection(const VC3 &direction)
{
	data->properties.direction = direction;
}

//! Enable or disable feature
/*!
	\param feature feature to enable or disable
	\param enable true to enable
*/
void Storm3D_Spotlight::enableFeature(IStorm3D_Spotlight::Feature feature, bool enable)
{
	if(feature == IStorm3D_Spotlight::Fade)
		data->hasFade = enable;
	else if(feature == IStorm3D_Spotlight::ConeVisualization)
	{
		data->hasCone = enable;
	}
	else if(feature == IStorm3D_Spotlight::Shadows)
	{
		data->hasShadows = enable;
	}
	else if(feature == IStorm3D_Spotlight::ScissorRect)
	{
		data->scissorRect = enable;
	}
	else
	{
		assert(!"whoops");
	}
}

//! Is feature enabled?
/*!
	\param feature feature to check
	\return true if enabled
*/
bool Storm3D_Spotlight::featureEnabled(Feature feature) const
{
	if(feature == IStorm3D_Spotlight::Fade)
		return data->hasFade;
	else if(feature == IStorm3D_Spotlight::ConeVisualization)
	{
		if(!featureEnabled(IStorm3D_Spotlight::Shadows))
			return false;

		return data->hasCone;
	}
	else if(feature == IStorm3D_Spotlight::Shadows)
	{
		if(!data->shadowMap)
			return false;

		return data->hasShadows;
	}
	else
	{
		assert(!"whoops");
		return false;
	}
}

//! Set type of light
/*!
	\param type type
*/
void Storm3D_Spotlight::setType(Type type)
{
	data->light_type = type;
}

//! Get type of light
/*!
	\return type
*/
IStorm3D_Spotlight::Type Storm3D_Spotlight::getType() const
{
	return data->light_type;
}

//! Set no shadow model
/*!
	\param model model
*/
void Storm3D_Spotlight::setNoShadowModel(const IStorm3D_Model *model)
{
	data->noShadowModel = model;
}

//! Get no shadow model
/*!
	\return model
*/
const IStorm3D_Model *Storm3D_Spotlight::getNoShadowModel() const
{
	return data->noShadowModel;
}

//! Set up clip planes
/*!
	\param cameraView
*/
void Storm3D_Spotlight::setClipPlanes(const float *cameraView)
{
	data->setClipPlanes(cameraView);
}

//! Enable smoothing
/*!
	\param enable true to enable
*/
void Storm3D_Spotlight::enableSmoothing(bool enable)
{
	data->smoothing = enable;
}

//! Set up scissor rectangle
/*!
	\param camera camera
	\param screenSize screen size
*/
bool Storm3D_Spotlight::setScissorRect(Storm3D_Camera &camera, const VC2I &screenSize, Storm3D_Scene &scene)
{
	return data->setScissorRect(camera, screenSize, scene);
}

//! Set projection texture
/*!
	\param texture texture
*/
void Storm3D_Spotlight::setProjectionTexture(boost::shared_ptr<IStorm3D_Texture> texture)
{
	data->projectionTexture = boost::static_pointer_cast<Storm3D_Texture> (texture);
}

//! Set cone texture
/*!
	\param texture texture
*/
void Storm3D_Spotlight::setConeTexture(boost::shared_ptr<IStorm3D_Texture> texture)
{
	data->coneTexture = boost::static_pointer_cast<Storm3D_Texture> (texture);
}

//! Does the spotlight have a cone texture?
/*!
	\return true if cone texture exists
*/
bool Storm3D_Spotlight::hasConeTexture() const
{
	return data->coneTexture;
}

//! Set color multiplier
/*!
	\param color multiplier
*/
void Storm3D_Spotlight::setColorMultiplier(const COL &color)
{
	data->properties.color = color;
}

//! Set cone multiplier
/*!
	\param scalar multiplier
*/
void Storm3D_Spotlight::setConeMultiplier(float scalar)
{
	data->coneColorMultiplier = scalar;
}

//! Set smoothness
/*!
	\param smoothness smoothness
*/
void Storm3D_Spotlight::setSmoothness(float smoothness)
{
	data->smoothness = smoothness;
}

//! Get color multiplier
/*!
	\return color multiplier
*/
const COL &Storm3D_Spotlight::getColorMultiplier() const
{
	return data->properties.color;
}

//! Get orientation of spotlight
/*!
	\return orientation
*/
QUAT Storm3D_Spotlight::getOrientation() const
{
	VC3 lightPosition(data->properties.position.x, data->properties.position.y, data->properties.position.z);
	VC3 up(0, 1.f, 0);
	VC3 lookAt = lightPosition;
	lookAt += VC3(data->properties.direction.x, data->properties.direction.y, data->properties.direction.z);

	D3DXMATRIX tm;
	D3DXMatrixLookAtLH(tm, lightPosition, lookAt, up);

	float det = D3DXMatrixDeterminant(tm);
	D3DXMatrixInverse(tm, &det, tm);

	MAT m;

	for(int j = 0; j < 4; ++j)
		for(int i = 0; i < 4; ++i)
			m.Set(j*4 + i, tm[j*4 + i]);

	return m.GetRotation();
}

//! Set as render target
/*!
	\param cameraView
	\return true if success
*/
bool Storm3D_Spotlight::setAsRenderTarget(const D3DXMATRIX &cameraView)
{
	data->updateMatrices(cameraView);

	if(data->shadowMap)
	{
		glErrors();
		igios_unimplemented();
		// FIXME: something goes wrong here and dev driver complains
        // mismatched depth/stencil
        /*
		igiosWarning("tempColor (%dx%d) %x\n", data->tempColor->getWidth(), data->tempColor->getHeight(), data->tempColor->getFmt());
		igiosWarning("shadowMap (%dx%d) %x\n", data->shadowMap->getWidth(), data->shadowMap->getHeight(), data->shadowMap->getFmt());
        */
		for (unsigned int i = 0; i < 9; i++) {
			glActiveTexture(GL_TEXTURE0 + i);
			glBindTexture(GL_TEXTURE_2D, 0);
			glDisable(GL_TEXTURE_2D);
			glDisable(GL_TEXTURE_3D);
			glDisable(GL_TEXTURE_CUBE_MAP);
		}
		data->fbo->activate();

		glErrors();
		if (data->fbo->validate()) {
			glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
			glClearDepth(1.0);
			glClearStencil(0);
			glClearColor(0, 0, 0, 0);
			glClear(GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

			D3DXMATRIX dm;
			D3DXMatrixIdentity(dm);

			frozenbyte::storm::PixelShader::disable();
			glActiveTexture(GL_TEXTURE0);
			glClientActiveTexture(GL_TEXTURE0);
			glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
			glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_RGB, GL_TEXTURE);
			glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_ALPHA, GL_TEXTURE);
			glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_REPLACE);
			glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_REPLACE);

			//data->properties.lightProjection._22 = - data->properties.lightProjection._22;
			Storm3D_ShaderManager::GetSingleton()->SetProjectionMatrix(data->properties.lightProjection);
			Storm3D_ShaderManager::GetSingleton()->SetViewMatrix(data->properties.lightView);
			//Storm3D_ShaderManager::GetSingleton()->SetViewProjectionMatrix(data->properties.lightProjection, data->properties.lightView);
			Storm3D_ShaderManager::GetSingleton()->setSpot(data->properties.color, data->properties.position, data->properties.direction, data->properties.range, .1f);
			Storm3D_ShaderManager::GetSingleton()->SetWorldTransform(dm);

			return true;
		} else {
			igiosWarning("Storm3D_Spotlight::setAsRenderTarget: renderTarget validate failed\n");
			data->fbo->disable();
			return false;
		}

		IStorm3D_Logger *logger = data->storm.logger;
		if(logger)
		{
			logger->error("Failed to set render target");
		}
	}

	return false;
}

//! Get camera
/*!
	\return camera
*/
Storm3D_Camera &Storm3D_Spotlight::getCamera()
{
	return data->camera;
}

//! Render stencil cone to camera
/*!
	\param camera camera to use
*/
void Storm3D_Spotlight::renderStencilCone(Storm3D_Camera &camera)
{
	data->createStencilCone();
	data->coneStencilVertexShader->apply();

	VC3 lightPosition(data->properties.position.x, data->properties.position.y, data->properties.position.z);
	VC3 up(0, 1.f, 0);
	VC3 lookAt = lightPosition;
	lookAt += VC3(data->properties.direction.x, data->properties.direction.y, data->properties.direction.z);

	D3DXMATRIX tm;
	D3DXMatrixLookAtLH(tm, lightPosition, lookAt, up);

	float det = D3DXMatrixDeterminant(tm);
	D3DXMatrixInverse(tm, &det, tm);
	Storm3D_ShaderManager::GetSingleton()->SetWorldTransform(tm, true);

	data->coneStencilVertexBuffer.apply(0);
	data->coneStencilIndexBuffer.render(CONE_FACES, CONE_VERTICES);
}

//! Apply textures
/*!
	\param cameraView
	\param cameraViewProjection
	\param storm Storm3D
	\param renderShadows
*/
void Storm3D_Spotlight::applyTextures(const D3DXMATRIX &cameraView, Storm3D &storm, bool renderShadows)
{
	data->updateMatrices(cameraView);

	if(data->projectionTexture)
	{
		data->projectionTexture->AnimateVideo();
		data->projectionTexture->Apply(0);
	}

	if(renderShadows && data->shadowMap) {
		glActiveTexture(GL_TEXTURE1);
		data->shadowMap->bind();
		glErrors();
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE_ARB, GL_COMPARE_R_TO_TEXTURE_ARB);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC_ARB, GL_LEQUAL);
		glTexParameteri(GL_TEXTURE_2D, GL_DEPTH_TEXTURE_MODE_ARB, GL_LUMINANCE);
		glErrors();
	}

	if(data->hasShadows && renderShadows && data->shadowMap)
	{
		if(data->smoothing)
			data->nvSmoothShadowPixelShader->apply();
		else
			data->nvShadowPixelShader->apply();
	}
	else
		data->nvNoShadowPixelShader->apply();

	Storm3D_ShaderManager::GetSingleton()->setSpot(data->properties.color, data->properties.position, data->properties.direction, data->properties.range, .1f);
	// data->properties.shaderProjection is world -> light projection space  (lightView * lightProjection)
	Storm3D_ShaderManager::GetSingleton()->setTextureTm(data->properties.shaderProjection);
	// data->properties.targetProjection is world -> shadow map space (lightView * lightProjection * shadowTweak ([-1,1] -> [0,1]))
	Storm3D_ShaderManager::GetSingleton()->setSpotTarget(data->properties.targetProjection);

	if(data->light_type == Directional)
		Storm3D_ShaderManager::GetSingleton()->setSpotType(Storm3D_ShaderManager::Directional);
	if(data->light_type == Point)
		Storm3D_ShaderManager::GetSingleton()->setSpotType(Storm3D_ShaderManager::Point);
	if(data->light_type == Flat)
		Storm3D_ShaderManager::GetSingleton()->setSpotType(Storm3D_ShaderManager::Flat);

	if(data->smoothing)
	{
		float c[8][4] = 
		{
			{ -2.5f,   3.f, 0, 0 },
			{  -1.f,   1.f, 0, 0 },
			{ -0.5f,   4.f, 0, 0 },
			{  2.5f,   2.f, 0, 0 },
			{ -1.5f,  -2.f, 0, 0 },
			{ -0.5f,  -4.f, 0, 0 },
			{  0.5f, -0.5f, 0, 0 },
			{  1.5f,  -3.f, 0, 0 }
		};

		for(int i = 0; i < 8; ++i)
		{
			c[i][0] *= data->smoothness * 0.0009765625f;
			c[i][1] *= data->smoothness * 1.5f * 0.0013020833f;
		}

		for (int i = 0; i < 8; i++) {
			glProgramEnvParameter4fvARB(GL_FRAGMENT_PROGRAM_ARB, 5 + i, c[i]);
		}
	}
}

//! Apply terrain shader
/*!
	\param renderShadows true to render shadows
*/
void Storm3D_Spotlight::applyTerrainShader(bool renderShadows)
{
	const COL &color = data->properties.color;
	float c[4] = { color.r, color.g, color.b, 1.f };
	glProgramEnvParameter4fvARB(GL_VERTEX_PROGRAM_ARB, 17, c);
}


//! Render cone
/*!
	\param camera camera
	\param timeFactor time factor
	\param renderGlows true to render glows
*/
void Storm3D_Spotlight::renderCone(Storm3D_Camera &camera, float timeFactor, bool renderGlows)
{
	if(!data->hasCone || !data->hasShadows || !data->shadowMap)
		return;

	bool normalPass = !data->coneUpdated;
	if(data->hasCone && data->updateCone && !data->coneUpdated)
	{
		data->createCone();
		data->coneUpdated = true;
	}

	VC3 lightPosition(data->properties.position.x, data->properties.position.y, data->properties.position.z);
	VC3 up(0, 1.f, 0);
	VC3 lookAt = lightPosition;
	lookAt += VC3(data->properties.direction.x, data->properties.direction.y, data->properties.direction.z);

	D3DXMATRIX tm;
	D3DXMatrixLookAtLH(tm, lightPosition, lookAt, up);

	VC3 cameraDir = camera.GetDirection();
	cameraDir.Normalize();
	VC3 direction(cameraDir.x, cameraDir.y, cameraDir.z);
	D3DXVec3TransformNormal(direction, direction, tm);

	if(data->shadowMap) {
		glActiveTexture(GL_TEXTURE0);
		data->shadowMap->bind();
		glErrors();
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE_ARB, GL_COMPARE_R_TO_TEXTURE_ARB);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC_ARB, GL_LEQUAL);
		glTexParameteri(GL_TEXTURE_2D, GL_DEPTH_TEXTURE_MODE_ARB, GL_LUMINANCE);
		glErrors();
	}
	if(data->coneTexture)
	{
		data->coneTexture->AnimateVideo();
		data->coneTexture->Apply(3);

		data->coneTexture->Apply(1);

	}

	if(data->coneTexture)
		data->coneNvPixelShader_Texture->apply();
	else
		data->coneNvPixelShader_NoTexture->apply();

	Storm3D_ShaderManager::GetSingleton()->setSpot(data->properties.color, data->properties.position, data->properties.direction, data->properties.range, .1f);
	Storm3D_ShaderManager::GetSingleton()->setTextureTm(data->properties.shaderProjection);
	Storm3D_ShaderManager::GetSingleton()->setSpotTarget(data->properties.targetProjection);

	float det = D3DXMatrixDeterminant(tm);
	D3DXMatrixInverse(tm, &det, tm);
	Storm3D_ShaderManager::GetSingleton()->SetWorldTransform(tm, true);

	float colorMul = data->coneColorMultiplier;
	float colorData[4] = { data->properties.color.r * colorMul, data->properties.color.g * colorMul, data->properties.color.b * colorMul, 1.f };
	if(renderGlows)
	{
		if(normalPass)
		{
			colorData[0] *= 0.2f;
			colorData[1] *= 0.2f;
			colorData[2] *= 0.2f;
			colorData[3] *= 0.2f;
		}
		else
		{
			colorData[0] *= 0.4f;
			colorData[1] *= 0.4f;
			colorData[2] *= 0.4f;
			colorData[3] *= 0.4f;
		}
	}

	glProgramEnvParameter4fvARB(GL_VERTEX_PROGRAM_ARB, 9, colorData);

	float bias = 0.005f;
	float directionData[4] = { -direction.x, -direction.y, -direction.z, bias };
	glProgramEnvParameter4fvARB(GL_VERTEX_PROGRAM_ARB, 10, directionData);

	for(int i = 0; i < 2; ++i)
	{
		data->angle[i] += data->speed[i] * timeFactor;
		VC3 center(0.5f, 0.5f, 0.f);
		QUAT quat1;
		quat1.MakeFromAngles(0, 0, data->angle[i]);

		D3DXMATRIX rot1;
		D3DXMatrixAffineTransformation(rot1, 1.f, center, quat1, 0.0f);
		
		if(i == 0)
			glActiveTexture(GL_TEXTURE3);
		else
			glActiveTexture(GL_TEXTURE1);

		glMatrixMode(GL_TEXTURE);
		glLoadMatrixf(rot1.raw);
	}

	data->coneVertexBuffer.apply(0);
	data->coneIndexBuffer.render(CONE_FACES, CONE_VERTICES);
	glMatrixMode(GL_TEXTURE);
	glActiveTexture(GL_TEXTURE1);
	glLoadIdentity();
	glActiveTexture(GL_TEXTURE3);
	glLoadIdentity();

}

void Storm3D_Spotlight::debugRender()
{
	igios_unimplemented();
}

//! Set up buffer sizes
/*!
	\param storm Storm3D
	\param shadowQuality shadow quality
*/
void Storm3D_Spotlight::querySizes(Storm3D &storm, int shadowQuality)
{
	if(shadowQuality >= 100)
	{
		SHADOW_WIDTH = 1024;
		SHADOW_HEIGHT = 1024;
	}
	else if(shadowQuality >= 75)
	{
		SHADOW_WIDTH = 1024;
		SHADOW_HEIGHT = 512;
	}
	else if(shadowQuality >= 50)
	{
		SHADOW_WIDTH = 512;
		SHADOW_HEIGHT = 512;
	}
	else if(shadowQuality >= 25)
	{
		SHADOW_WIDTH = 512;
		SHADOW_HEIGHT = 256;
	}
	else
	{
		SHADOW_WIDTH = 256;
		SHADOW_HEIGHT = 256;
	}
}

//! Create shadow buffers
/*!
	\param storm Storm3D
	\param shadowQuality shadow quality
*/
void Storm3D_Spotlight::createShadowBuffers(Storm3D &storm, int shadowQuality)
{
	Storm3D_SpotlightData::nvSmoothShadowPixelShader = new frozenbyte::storm::PixelShader();
	Storm3D_SpotlightData::nvSmoothShadowPixelShader->createNvSmoothShadowShader();

	Storm3D_SpotlightData::nvShadowPixelShader = new frozenbyte::storm::PixelShader();
	Storm3D_SpotlightData::nvShadowPixelShader->createNvShadowShader();
	Storm3D_SpotlightData::nvNoShadowPixelShader = new frozenbyte::storm::PixelShader();
	Storm3D_SpotlightData::nvNoShadowPixelShader->createNvNoShadowShader();

	Storm3D_SpotlightData::coneNvPixelShader_Texture = new frozenbyte::storm::PixelShader();
	Storm3D_SpotlightData::coneNvPixelShader_Texture->createNvConeShader_Texture();
	Storm3D_SpotlightData::coneNvPixelShader_NoTexture = new frozenbyte::storm::PixelShader();
	Storm3D_SpotlightData::coneNvPixelShader_NoTexture->createNvConeShader_NoTexture();

	Storm3D_SpotlightData::coneStencilVertexShader = new frozenbyte::storm::VertexShader();
	Storm3D_SpotlightData::coneStencilVertexShader->createConeStencilShader();
}

//! Free shadow buffers
void Storm3D_Spotlight::freeShadowBuffers()
{
	delete Storm3D_SpotlightData::nvShadowPixelShader; Storm3D_SpotlightData::nvShadowPixelShader = 0;
	delete Storm3D_SpotlightData::nvSmoothShadowPixelShader; Storm3D_SpotlightData::nvSmoothShadowPixelShader = 0;
	delete Storm3D_SpotlightData::nvNoShadowPixelShader; Storm3D_SpotlightData::nvNoShadowPixelShader = 0;

	delete Storm3D_SpotlightData::coneNvPixelShader_Texture; Storm3D_SpotlightData::coneNvPixelShader_Texture = 0;
	delete Storm3D_SpotlightData::coneNvPixelShader_NoTexture; Storm3D_SpotlightData::coneNvPixelShader_NoTexture = 0;

	delete Storm3D_SpotlightData::coneStencilVertexShader; Storm3D_SpotlightData::coneStencilVertexShader = 0;
}

//! Clear cache
void Storm3D_Spotlight::clearCache()
{
	targetActive = false;
}

void Storm3D_Spotlight::setEnableClip(bool enableClip)
{
	data->spotlightAlwaysVisible = !enableClip;
}

void Storm3D_Spotlight::setDebug(void) {
	// set depth texture as stage 0 texture

	glActiveTexture(GL_TEXTURE0);
	// FIXME: using color buffer for testing
	data->shadowMap->bind();
	//data->tempColor->bind();
	/*
	if (data->projectionTexture)
		data->projectionTexture->Apply(0);
	*/
}

