// Copyright 2002-2004 Frozenbyte Ltd.

#ifndef INCLUDED_AMPLITUDE_ARRAY_H
#define INCLUDED_AMPLITUDE_ARRAY_H

#include <vector>

namespace sfx {

class AmplitudeArray
{
	std::vector<unsigned char> data;
	mutable unsigned char maxValue;
	mutable unsigned char avgValue;
	unsigned char originalMax;
	unsigned char originalAvg;

public:
	AmplitudeArray();
	~AmplitudeArray();

	void setSampleAmount(int amount);
	void setSampleAmplitude(int index, unsigned char amplitude);
	void update();

	int getSampleAmount() const;
	unsigned char getMaxAmplitude() const;
	unsigned char getAvgAmplitude() const;
	// Returns normalized amplitude (max amplitude of array is 255)
	unsigned char getAmplitude(int index) const;
};

} // sfx

#endif
