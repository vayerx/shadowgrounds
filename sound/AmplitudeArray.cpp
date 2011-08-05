#include "precompiled.h"

// Copyright 2002-2004 Frozenbyte Ltd.

#include "AmplitudeArray.h"
#include "../util/assert.h"

using namespace std;

namespace sfx {

typedef vector<unsigned char> AmpSampleList;

AmplitudeArray::AmplitudeArray()
:	maxValue(0),
	avgValue(0),
	originalMax(0),
	originalAvg(0)
{
}

AmplitudeArray::~AmplitudeArray()
{
}

void AmplitudeArray::setSampleAmount(int amount)
{
	FB_ASSERT(amount >= 0);
	data.resize(amount);
}

void AmplitudeArray::setSampleAmplitude(int index, unsigned char amplitude)
{
	FB_ASSERT(index >= 0 && index < int(data.size()));
	data[index] = amplitude;
}

void AmplitudeArray::update()
{
	if(data.empty())
		return;

	originalAvg = 0;
	originalMax = 0;
	
	int addAvg = 0;
	for(AmpSampleList::iterator it = data.begin(); it != data.end(); ++it)
	{
		unsigned char value = *it;

		addAvg += value;
		if(value > maxValue)
			originalMax = value;
	}

	if(originalMax < 10)
		originalMax = 10;
	originalAvg = (unsigned char) (addAvg / data.size());

	maxValue = originalMax;
	avgValue = originalAvg;
}

int AmplitudeArray::getSampleAmount() const
{
	return int(data.size());
}

unsigned char AmplitudeArray::getMaxAmplitude() const
{
	return maxValue;
}

unsigned char AmplitudeArray::getAvgAmplitude() const
{
	return avgValue;
}

unsigned char AmplitudeArray::getAmplitude(int index) const
{
	if(index >= int(data.size()))
		return 0;

	FB_ASSERT(index >= 0 && index < int(data.size()));
	FB_ASSERT(maxValue);

	/*
	int sampleAhead = 30;
	int dataSize = data.size();
	if(index + sampleAhead < dataSize)
	{
		unsigned char newMax = 0;
		unsigned char newAvg = 0;

		int addAvg = 0;
		for(int i = index; i < index + sampleAhead; ++i)
		{
			unsigned char value = 0;
			if(i >= dataSize)
				value = data[dataSize - 1];
			else
				value = data[i];

			addAvg += value;
			if(value > newMax)
				newMax = value;
		}

		if(newMax < 10)
			newMax = 10;
		newAvg = unsigned char(addAvg / sampleAhead);

		//max = (3 * originalMax / 4) + (1 * newMax / 4);
		//avg = (3 * originalAvg / 4) + (1 * newAvg / 4);
		max = (3 * originalMax / 4) + (1 * newMax / 4);
		avg = (2 * originalAvg / 4) + (2 * newAvg / 4);
	}
	*/

	int sampleAhead = 10;
	int dataSize = data.size();
	if(index - sampleAhead >= 0 && index + sampleAhead < dataSize)
	{
		unsigned char newMax = 0;
		unsigned char newAvg = 0;

		int addAvg = 0;
		for(int i = index - sampleAhead; i < index + sampleAhead; ++i)
		{
			unsigned char value = 0;
			if(i >= dataSize)
				value = data[dataSize - 1];
			else
				value = data[i];

			addAvg += value;
			if(value > newMax)
				newMax = value;
		}

		if(newMax < 10)
			newMax = 10;
		newAvg = (unsigned char) (addAvg / (sampleAhead * 2));

		//max = (3 * originalMax / 4) + (1 * newMax / 4);
		//avg = (3 * originalAvg / 4) + (1 * newAvg / 4);
		maxValue = (3 * originalMax / 4) + (1 * newMax / 4);
		avgValue = (2 * originalAvg / 4) + (2 * newAvg / 4);
	}

	return (unsigned char)(255 * data[index] / maxValue);
}

} // sfx
