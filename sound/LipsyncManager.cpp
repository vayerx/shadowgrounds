

// note, add a dummy precompiled.h to viewer or whatever projects that include this.
// (instead of commenting this out)
#include "precompiled.h"


// Copyright 2002-2004 Frozenbyte Ltd.

#include "LipsyncManager.h"
#include "LipsyncProperties.h"
#include "WaveReader.h"
#include "AmplitudeArray.h"
#include "../util/assert.h"
#include <IStorm3D.h>
#include <IStorm3D_Bone.h>
#include <boost/shared_ptr.hpp>
#include <string>
#include <map>

namespace sfx {

using namespace boost;
using namespace std;
typedef map<string, shared_ptr<IStorm3D_BoneAnimation> > AnimationMap;


	struct Releaser
	{
		void operator () (IStorm3D_BoneAnimation *b)
		{
			if(b)
				b->Release();
		}
	};

	struct Fader
	{
		struct Info
		{
			IStorm3D_Model *model;
			IStorm3D_BoneAnimation *animation;
			int fadeTime;

			int waitTime;
			int currentTime;

			Info()
			:	model(0),
				animation(0),
				fadeTime(0),
				waitTime(0),
				currentTime(0)
			{
			}
		};

		typedef vector<Info> Fades;
		Fades fades;

		void addFadeOut(IStorm3D_BoneAnimation *a, IStorm3D_Model *m, int fadeTime, int waitTime)
		{
			FB_ASSERT(a && m && fadeTime >= 0 && waitTime >= 0);

			Info i;
			i.model = m;
			i.animation = a;
			i.fadeTime = fadeTime;
			i.waitTime = waitTime;

			fades.push_back(i);
		}

		void update(int delta)
		{
			Fades::iterator it = fades.begin();
			for(; it != fades.end(); )
			{
				Info &i = *it;
				i.currentTime += delta;

				if(i.currentTime >= i.waitTime)
				{
					i.model->BlendWithAnimationOut(0, i.animation, i.fadeTime);
					it = fades.erase(it);
				}
				else
					++it;
			}
		}

		void reset()
		{
			fades.clear();
		}
	};

	struct ModelInfo
	{
		IStorm3D_BoneAnimation *idle;
		IStorm3D_BoneAnimation *expression;

		ModelInfo()
		:	idle(0),
			expression(0)
		{
		}

		void setIdle(Fader &fader, IStorm3D_Model *model, IStorm3D_BoneAnimation *a, int time)
		{
			if(idle == a)
				return;

			if(a)
			{
				model->BlendToAnimation(0, a, time, true);
				idle = a;
			}
		}

		void setExpression(Fader &fader, IStorm3D_Model *model, IStorm3D_BoneAnimation *a, int time)
		{
			if(a == expression)
				return;

			if(expression)
				fader.addFadeOut(expression, model, time, 2 * time);

			if(a)
			{
				model->BlendWithAnimationIn(0, a, time, false);
				expression = a;
			}
		}
	};

	struct Amplitude
	{
		unsigned char limit;
		shared_ptr<IStorm3D_BoneAnimation> animation;

		Amplitude()
		:	limit(0)
		{
		}
	};

	struct AmplitudeSorter
	{
		bool operator () (const Amplitude &a, const Amplitude &b) const
		{
			return a.limit < b.limit;
		}
	};

	struct PlayData
	{
		IStorm3D_Model *model;
		boost::shared_ptr<AmplitudeArray> array;
		IStorm3D_BoneAnimation *previous;
		int currentSample;

		int startTime;

		explicit PlayData(const boost::shared_ptr<AmplitudeArray> &array_)
		:	model(0),
			array(array_),
			previous(0),
			currentSample(0),
			startTime(0)
		{
			//WaveReader reader(file);
			//reader.readAmplitudeArray(sampleRate, array);
			//array.update();
		}

		void set(Fader &fader, IStorm3D_BoneAnimation *a, int time)
		{
			if(a == previous)
				return;

			if(previous)
				fader.addFadeOut(previous, model, time, 2 * time);

			if(a)
			{
				float blendFactor = 0.95f + (array->getMaxAmplitude() / 1550.f);
				if(blendFactor > 1.f)
					blendFactor = 1.f;

				model->BlendWithAnimationIn(0, a, time, false, blendFactor);
				previous = a;
			}
		}

		void setCurrentTime(int currentTime, int sampleRate)
		{
			int time = currentTime - startTime;
			int newSample = (time / sampleRate);

			if(newSample > currentSample)
				currentSample = newSample;
		}

		unsigned char getAmplitude() const
		{
			return array->getAmplitude(currentSample);
		}

		bool hasEnded() const
		{
			return currentSample >= array->getSampleAmount() - 1;
		}
	};

	typedef map<IStorm3D_Model *, ModelInfo> ModelInfos;
	typedef vector<shared_ptr<PlayData> > PlayDatas;
	typedef vector<Amplitude> Amplitudes;


struct LipsyncManager::Data
{
	IStorm3D *storm;
	AnimationMap idles;
	AnimationMap expressions;
	ModelInfos modelInfo;

	int idleFadeTime;
	int expressionFadeTime;
	int sampleRate;

	LipsyncProperties properties;
	Fader fader;

	PlayDatas playDatas;
	Amplitudes amplitudes;

	Data(IStorm3D *storm_)
	:	storm(storm_),
		idleFadeTime(0),
		expressionFadeTime(0),
		sampleRate(0)
	{
	}

	void addAnimation(AnimationMap &map, const string &name, const string &file)
	{
		IStorm3D_BoneAnimation *a = storm->CreateNewBoneAnimation(file.c_str());
		map[name].reset(a, Releaser());
	}

	void buildAmplitudeArray()
	{
		for(int i = 0; i < properties.getPhonemAmount(); ++i)
		{
			const LipsyncProperties::Phonem &phonem = properties.getPhonem(i);
			Amplitude amplitude;
			amplitude.limit = phonem.limit;
			amplitude.animation.reset(storm->CreateNewBoneAnimation(phonem.file.c_str()), Releaser());

			amplitudes.push_back(amplitude);
			std::sort(amplitudes.begin(), amplitudes.end(), AmplitudeSorter());
		}
	}

	void initialize()
	{
		int i = 0;
		for(i = 0; i < properties.getIdleAnimationAmount(); ++i)
			addAnimation(idles, properties.getIdleAnimationName(i), properties.getIdleAnimation(i));

		for(i = 0; i < properties.getExpressionAnimationAmount(); ++i)
			addAnimation(expressions, properties.getExpressionAnimationName(i), properties.getExpressionAnimation(i));

		buildAmplitudeArray();

		idleFadeTime = properties.getPropertyValue(LipsyncProperties::IdleFadeTime);
		expressionFadeTime = properties.getPropertyValue(LipsyncProperties::ExpressionFadeTime);
		sampleRate = properties.getPropertyValue(LipsyncProperties::SampleRate);
	}

	void setIdle(IStorm3D_Model *model, const string &value, int fadeTime)
	{
		FB_ASSERT(model);

		AnimationMap::iterator it = idles.find(value);
		if(it == idles.end())
			return;

		if(fadeTime < 0)
		{
			fadeTime = idleFadeTime;
		}
		modelInfo[model].setIdle(fader, model, it->second.get(), fadeTime);
	}

	void setExpression(IStorm3D_Model *model, const string &value, int fadeTime)
	{
		FB_ASSERT(model);

		AnimationMap::iterator it = expressions.find(value);
		if(it == expressions.end())
			return;

		if(fadeTime < 0)
		{
			fadeTime = expressionFadeTime;
		}

		modelInfo[model].setExpression(fader, model, it->second.get(), fadeTime);
	}

	void play(IStorm3D_Model *model, const boost::shared_ptr<AmplitudeArray> &array, int currentTime)
	{
		boost::shared_ptr<PlayData> playInfo(new PlayData(array));
		playInfo->model = model;
		playInfo->startTime = currentTime;

		playDatas.push_back(playInfo);
	}

	IStorm3D_BoneAnimation *getPhonem(PlayData *data)
	{
		unsigned char amplitude = data->getAmplitude();

		Amplitudes::iterator it = amplitudes.begin();
		for(; it != amplitudes.end(); ++it)
		{
			if(amplitude <= it->limit)
				return it->animation.get();
		}

		return 0;
	}
		
	void update(int ms, int currentTime)
	{
		fader.update(ms);

		PlayDatas::iterator it = playDatas.begin();
		for(; it != playDatas.end(); )
		{
			boost::shared_ptr<PlayData> &playInfo = *it;
			PlayData *ptr = playInfo.get();

			ptr->setCurrentTime(currentTime, sampleRate);
			ptr->set(fader, getPhonem(ptr), sampleRate);

			if(ptr->hasEnded())
			{
				ptr->set(fader, 0, sampleRate * 3);
				it = playDatas.erase(it);
			}
			else
				++it;
		}
	}
};

LipsyncManager::LipsyncManager(IStorm3D *storm)
{
	boost::scoped_ptr<Data> tempData(new Data(storm));
	tempData->initialize();

	data.swap(tempData);
}

LipsyncManager::~LipsyncManager()
{
}

const LipsyncProperties &LipsyncManager::getProperties() const
{
	return data->properties;
}

boost::shared_ptr<AmplitudeArray> LipsyncManager::getAmplitudeBuffer(const std::string &file) const
{
	boost::shared_ptr<AmplitudeArray> array(new AmplitudeArray());

	WaveReader reader(file);
	reader.readAmplitudeArray(data->sampleRate, *array);
	array->update();

	return array;
}

void LipsyncManager::setIdle(IStorm3D_Model *model, const string &value, int fadeTime)
{
	data->setIdle(model, value, fadeTime);
}

void LipsyncManager::setExpression(IStorm3D_Model *model, const string &value, int fadeTime)
{
	data->setExpression(model, value, fadeTime);
}

void LipsyncManager::play(IStorm3D_Model *model, const boost::shared_ptr<AmplitudeArray> &array, int currentTime)
{
	data->play(model, array, currentTime);
}

void LipsyncManager::update(int ms, int currentTime)
{
	data->update(ms, currentTime);
}

void LipsyncManager::reset()
{
	data->fader.reset();
	data->playDatas.clear();
}

} // sfx
