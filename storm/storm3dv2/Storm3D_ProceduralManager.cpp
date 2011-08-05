#ifdef _MSC_VER
#pragma warning(disable:4103)
#endif

#include <vector>

#include "Storm3D_ProceduralManager.h"
#include "storm3d.h"
#include "storm3d_texture.h"
#include "storm3d_terrain_utils.h"
#include "Storm3D_ShaderManager.h"
#include <IStorm3D_Logger.h>
#include <d3d9.h>
#include <boost/shared_ptr.hpp>

using namespace boost;
using namespace std;

	struct NullDeleter
	{
		void operator() (const void *) const
		{
		}
	};

	struct TexCoord
	{
		VC2 baseFactor;
		VC2 baseOffset;

		VC2 offsetFactor;
		VC2 offsetOffset;

		float timeValue;

		TexCoord()
		:	timeValue(0)
		{
		}
	};

	void clamp(float &value)
	{
		float intval = floorf(value);
		if(fabsf(intval) > 1.f)
			value = fmodf(value, intval);

		/*
		if(value > 0)
		{
			float intval = floorf(value);
			if(intval > 1.f)
				value = fmodf(value, intval);
		}
		else
		{
			float intval = floorf(value);
			if(intval < -1.f)
				value = fmodf(value, intval);
		}
		*/
	}

	void animateSource(TexCoord &coord, const Storm3D_ProceduralManager::Source &source, float timeFactor)
	{
		coord.timeValue += timeFactor;
		coord.baseFactor.x = 1.f / source.texture.scale.x;
		coord.baseFactor.y = 1.f / source.texture.scale.y;
		coord.offsetFactor.x = coord.baseFactor.x;
		coord.offsetFactor.y = coord.baseFactor.y;

		coord.baseOffset.x += timeFactor * source.texture.speed.x;
		coord.baseOffset.y += timeFactor * source.texture.speed.y;
		coord.offsetOffset.x = source.radius.x * sinf(coord.timeValue * source.offset.speed.x);
		coord.offsetOffset.y = source.radius.y * cosf(coord.timeValue * source.offset.speed.y);
		coord.offsetOffset.x += source.linearSpeed.x * coord.timeValue;
		coord.offsetOffset.y += source.linearSpeed.y * coord.timeValue;

		clamp(coord.baseOffset.x);
		clamp(coord.baseOffset.y);
		clamp(coord.offsetOffset.x);
		clamp(coord.offsetOffset.y);
	}

	struct ProceduralEffect
	{
		Storm3D_ProceduralManager::Effect effect;
		shared_ptr<Storm3D_Texture> texture1;
		shared_ptr<Storm3D_Texture> texture2;
		shared_ptr<Storm3D_Texture> offset1;
		shared_ptr<Storm3D_Texture> offset2;
		shared_ptr<Storm3D_Texture> distortion1;
		shared_ptr<Storm3D_Texture> distortion2;
		shared_ptr<Storm3D_Texture> fallback;

		TexCoord coord1;
		TexCoord coord2;
		TexCoord displaceCoord1;
		TexCoord displaceCoord2;

		void init(Storm3D &storm)
		{
			if (!effect.source1.texture.texture.empty())
			{
				Storm3D_Texture *t1 = static_cast<Storm3D_Texture *> (storm.CreateNewTexture(effect.source1.texture.texture.c_str()));
				if(t1)
				{
					texture1.reset(t1, std::mem_fun(&IStorm3D_Texture::Release));
					texture1->setAutoRelease(false);
				}
			}
			if (!effect.source2.texture.texture.empty())
			{
				Storm3D_Texture *t2 = static_cast<Storm3D_Texture *> (storm.CreateNewTexture(effect.source2.texture.texture.c_str()));
				if(t2)
				{
					texture2.reset(t2, std::mem_fun(&IStorm3D_Texture::Release));
					texture2->setAutoRelease(false);
				}
			}

			if (!effect.source1.offset.texture.empty())
			{
				Storm3D_Texture *t3 = static_cast<Storm3D_Texture *> (storm.CreateNewTexture(effect.source1.offset.texture.c_str()));
				if(t3)
				{
					offset1.reset(t3, std::mem_fun(&IStorm3D_Texture::Release));
					offset1->setAutoRelease(false);
				}
			}

			if (!effect.source2.offset.texture.empty())
			{
				Storm3D_Texture *t4 = static_cast<Storm3D_Texture *> (storm.CreateNewTexture(effect.source2.offset.texture.c_str()));
				if(t4)
				{
					offset2.reset(t4, std::mem_fun(&IStorm3D_Texture::Release));
					offset2->setAutoRelease(false);
				}
			}

			if (!effect.distortion1.offset.texture.empty())
			{
				Storm3D_Texture *t5 = static_cast<Storm3D_Texture *> (storm.CreateNewTexture(effect.distortion1.offset.texture.c_str()));
				if(t5)
				{
					distortion1.reset(t5, std::mem_fun(&IStorm3D_Texture::Release));
					distortion1->setAutoRelease(false);
				}
			}
			if (!effect.distortion2.offset.texture.empty())
			{
				Storm3D_Texture *t6 = static_cast<Storm3D_Texture *> (storm.CreateNewTexture(effect.distortion2.offset.texture.c_str()));
				if(t6)
				{
					distortion2.reset(t6, std::mem_fun(&IStorm3D_Texture::Release));
					distortion2->setAutoRelease(false);
				}
			}

			if (!effect.fallback.empty())
			{
				Storm3D_Texture *f = static_cast<Storm3D_Texture *> (storm.CreateNewTexture(effect.fallback.c_str()));
				if(f)
				{
					fallback.reset(f, std::mem_fun(&IStorm3D_Texture::Release));
					fallback->setAutoRelease(false);
				}
			} else {
				storm.getLogger()->warning("ProceduralEffect::init - effect has no fallback texture.");
			}
		}

		void animate(int ms)
		{
			float timeFactor = ms / 1000.f;
			animateSource(coord1, effect.source1, timeFactor);
			animateSource(coord2, effect.source2, timeFactor);

			animateSource(displaceCoord1, effect.distortion1, timeFactor);
			animateSource(displaceCoord2, effect.distortion2, timeFactor);
		}
	};

	typedef map<string, ProceduralEffect> ProceduralEffectList;

struct Storm3D_ProceduralManager::Data
{
	Storm3D &storm;
	CComPtr<IDirect3DTexture9> target;
	CComPtr<IDirect3DTexture9> offsetTarget;

	ProceduralEffectList effects;
	string active;

	IStorm3D_Logger *logger;
	scoped_ptr<frozenbyte::storm::VertexShader> vshader;
	scoped_ptr<frozenbyte::storm::PixelShader> pshader;
	scoped_ptr<frozenbyte::storm::PixelShader> poffsetShader;
	scoped_ptr<frozenbyte::storm::VertexBuffer> vbuffer;
	scoped_ptr<frozenbyte::storm::IndexBuffer> ibuffer;

	bool distortionMode;
	bool hasDistortion;

	bool useFallback;
	shared_ptr<Storm3D_Texture> fallback;

	Data(Storm3D &storm_)
	:	storm(storm_),
		logger(0),
		distortionMode(false),
		hasDistortion(false),
		//useFallback(true)
		useFallback(false)
	{
	}

	void addEffect(const string &name, const Effect &effect)
	{
		ProceduralEffect result;
		result.effect = effect;
		result.init(storm);

		if(result.texture1 && result.texture2)
			effects[name] = result;
		else if(logger)
		{
			string message = "Cannot find both textures for procedural effect: ";
			message += name;
			logger->error(message.c_str());

			logger->info(result.effect.source1.texture.texture.c_str());
			logger->info(result.effect.source2.texture.texture.c_str());
		}
	}

	void init(CComPtr<IDirect3DTexture9> target_, CComPtr<IDirect3DTexture9> offsetTarget_)
	{
		IDirect3DDevice9 &device = *storm.GetD3DDevice();
		target = target_;
		offsetTarget = offsetTarget_;

		if(!target)
			return;

		vshader.reset(new frozenbyte::storm::VertexShader(device));
		vshader->createProceduralShader();
		pshader.reset(new frozenbyte::storm::PixelShader(device));
		pshader->createProceduralShader();

		if(offsetTarget)
		{
			poffsetShader.reset(new frozenbyte::storm::PixelShader(device));
			poffsetShader->createProceduralOffsetShader();
		}

		{
			vbuffer.reset(new frozenbyte::storm::VertexBuffer());
			vbuffer->create(device, 4, 8 * sizeof(float), false);
			float *ptr = reinterpret_cast<float *> (vbuffer->lock());
			if(ptr)
			{
				float width = 1.f;
				float height = 1.f;

				float buffer[] = 
				{
					-width, height,         1.f, 1.f,   0.f, 1.f, 0.f, 1.f,
					-width, -height,  1.f, 1.f,   0.f, 0.f, 0.f, 0.f,
					width, height,                 1.f, 1.f,   1.f, 1.f, 1.f, 1.f,
					width, -height,          1.f, 1.f,   1.f, 0.f, 1.f, 0.f
				};

				memcpy(ptr, buffer, 8 * 4 * sizeof(float));
				vbuffer->unlock();
			}
		}
		{
			ibuffer.reset(new frozenbyte::storm::IndexBuffer());
			ibuffer->create(device, 2, false);

			unsigned short *ptr = ibuffer->lock();
			if(ptr)
			{
				*ptr++ = 0;
				*ptr++ = 2;
				*ptr++ = 1;

				*ptr++ = 1;
				*ptr++ = 2;
				*ptr++ = 3;

				ibuffer->unlock();
			}
		}
	}

	void setActive(const string &name)
	{
		if(name.empty())
		{
			active = name;
			return;
		}

		ProceduralEffectList::iterator it = effects.find(name);
		if(it == effects.end())
			return;

		fallback = it->second.fallback;
		active = name;
	}

	void render(const ProceduralEffect &e, float width, float height, bool offsetTarget)
	{
		IDirect3DDevice9 &device = *storm.GetD3DDevice();
		
		if(e.texture1)
			e.texture1->Apply(1);
		if(e.texture2)
			e.texture2->Apply(3);

		if(offsetTarget)
		{
			if(e.distortion1)
				e.distortion1->Apply(0);
			if(e.distortion2)
				e.distortion2->Apply(2);

			if(!e.distortion1 || !e.distortion2 || !e.effect.enableDistortion)
				return;
		}
		else
		{
			if(e.offset1)
				e.offset1->Apply(0);
			if(e.offset2)
				e.offset2->Apply(2);
		}

		assert(vshader && pshader);
		vshader->apply();

		float constants[8 * 4];
		const TexCoord &c1 = (offsetTarget) ? e.displaceCoord1 : e.coord1;
		const TexCoord &c2 = (offsetTarget) ? e.displaceCoord2 : e.coord2;

		constants[0 + 0] = c1.offsetFactor.x;
		constants[0 + 1] = c1.offsetFactor.y;
		constants[4 + 0] = c1.offsetOffset.x;
		constants[4 + 1] = c1.offsetOffset.y;
		constants[8 + 0] = c1.baseFactor.x;
		constants[8 + 1] = c1.baseFactor.y;
		constants[12 + 0] = c1.baseOffset.x;
		constants[12 + 1] = c1.baseOffset.y;
		constants[16 + 0] = c2.offsetFactor.x;
		constants[16 + 1] = c2.offsetFactor.y;
		constants[20 + 0] = c2.offsetOffset.x;
		constants[20 + 1] = c2.offsetOffset.y;
		constants[24 + 0] = c2.baseFactor.x;
		constants[24 + 1] = c2.baseFactor.y;
		constants[28 + 0] = c2.baseOffset.x;
		constants[28 + 1] = c2.baseOffset.y;

		device.SetVertexShaderConstantF(0, constants, 8);

		float scale1 = (offsetTarget) ? e.effect.distortion1.offset.scale.x : e.effect.source1.offset.scale.x;
		float scale2 = (offsetTarget) ? e.effect.distortion2.offset.scale.x : e.effect.source2.offset.scale.x;
		device.SetTextureStageState(1, D3DTSS_BUMPENVMAT00, F2DW(scale1));
		device.SetTextureStageState(1, D3DTSS_BUMPENVMAT01, F2DW(0.f));
		device.SetTextureStageState(1, D3DTSS_BUMPENVMAT10, F2DW(0.f));
		device.SetTextureStageState(1, D3DTSS_BUMPENVMAT11, F2DW(scale1));
		device.SetTextureStageState(3, D3DTSS_BUMPENVMAT00, F2DW(scale2));
		device.SetTextureStageState(3, D3DTSS_BUMPENVMAT01, F2DW(0.f));
		device.SetTextureStageState(3, D3DTSS_BUMPENVMAT10, F2DW(0.f));
		device.SetTextureStageState(3, D3DTSS_BUMPENVMAT11, F2DW(scale2));

		if(offsetTarget)
		{
			float scale = (scale1 + scale2) * .25f;
			float c2[4] = { scale, scale, scale, scale };
			device.SetPixelShaderConstantF(2, c2, 1);

			poffsetShader->apply();
		}
		else
		{
			pshader->apply();
		}
	
		vbuffer->apply(device, 0);
		ibuffer->render(device, 2, 4, 0);

		device.SetPixelShader(0);
	}

	void update(int ms)
	{
		if(useFallback)
			return;

		IDirect3DDevice9 &device = *storm.GetD3DDevice();
		if(active.empty() || !target)
			return;

		ProceduralEffect &e = effects[active];
		e.animate(ms);

		CComPtr<IDirect3DSurface9> renderSurface;
		CComPtr<IDirect3DSurface9> originalSurface;
		target->GetSurfaceLevel(0, &renderSurface);
		if(!renderSurface)
			return;

		device.SetRenderState(D3DRS_ZENABLE, D3DZB_FALSE);
		device.SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
		device.SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
		device.GetRenderTarget(0, &originalSurface);
		device.SetRenderTarget(0, renderSurface);
		//device.Clear(0, 0, D3DCLEAR_TARGET, 0xFFFFFFFF, 1, 0);

		D3DSURFACE_DESC desc;
		renderSurface->GetDesc(&desc);
		render(e, float(desc.Width), float(desc.Height), false);

		if(distortionMode && offsetTarget)
		{
			CComPtr<IDirect3DSurface9> renderSurface;
			offsetTarget->GetSurfaceLevel(0, &renderSurface);

			device.SetRenderTarget(0, renderSurface);
			if(e.distortion1 && e.distortion2 && e.effect.enableDistortion)
			{
				render(e, float(desc.Width / 2), float(desc.Height / 2), true);
				hasDistortion = true;
			}
			else
				hasDistortion = false;
		}

		device.SetRenderState(D3DRS_ZENABLE, D3DZB_TRUE);
		device.SetRenderTarget(0, originalSurface);

		target->GenerateMipSubLevels();
	}

	void enableDistortion(bool enable)
	{
		distortionMode = enable;
	}

	void reset()
	{
		fallback.reset();
		effects.clear();
		if(target)
			target.Release();
		if(offsetTarget)
			offsetTarget.Release();

		vbuffer.reset();
		ibuffer.reset();
		vshader.reset();
		pshader.reset();
	}
};

Storm3D_ProceduralManager::Storm3D_ProceduralManager(Storm3D &storm)
{
	scoped_ptr<Data> tempData(new Data(storm));
	data.swap(tempData);
}

Storm3D_ProceduralManager::~Storm3D_ProceduralManager()
{
}

void Storm3D_ProceduralManager::setLogger(IStorm3D_Logger *logger)
{
	data->logger = logger;
}

void Storm3D_ProceduralManager::setTarget(CComPtr<IDirect3DTexture9> &target, CComPtr<IDirect3DTexture9> &offsetTarget)
{
	data->init(target, offsetTarget);
}

void Storm3D_ProceduralManager::addEffect(const string &name, const Effect &effect)
{
	data->addEffect(name, effect);
}

void Storm3D_ProceduralManager::enableDistortionMode(bool enable)
{
	data->enableDistortion(enable);
}

void Storm3D_ProceduralManager::useFallback(bool fallback)
{
	data->useFallback = fallback;
}

void Storm3D_ProceduralManager::setActiveEffect(const std::string &name)
{
	data->setActive(name);
}

void Storm3D_ProceduralManager::update(int ms)
{
	data->update(ms);
}

void Storm3D_ProceduralManager::apply(int stage)
{
	if(data->useFallback || !data->target)
	{
		if(data->fallback)
			data->fallback->Apply(stage);
	}
	else
	{
		IDirect3DDevice9 &device = *data->storm.GetD3DDevice();
		if(data->target)
			device.SetTexture(stage, data->target);
	}
}

void Storm3D_ProceduralManager::applyOffset(int stage)
{
	IDirect3DDevice9 &device = *data->storm.GetD3DDevice();
	if(data->offsetTarget)
		device.SetTexture(stage, data->offsetTarget);
}

bool Storm3D_ProceduralManager::hasDistortion() const
{
	if(data->useFallback || !data->target)
		return false;

	return data->hasDistortion;
}

void Storm3D_ProceduralManager::releaseTarget()
{
	data->target.Release();
	data->offsetTarget.Release();
}

void Storm3D_ProceduralManager::reset()
{
	data->reset();
}
