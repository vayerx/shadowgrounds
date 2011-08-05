// Copyright 2002-2004 Frozenbyte Ltd.

#ifndef INCLUDED_WAVEREADER_H
#define INCLUDED_WAVEREADER_H

#include <boost/scoped_ptr.hpp>
#include <string>

namespace sfx {

class AmplitudeArray;

class WaveReader
{
	struct Data;
	boost::scoped_ptr<Data> data;

public:
	WaveReader(const std::string &file);
	~WaveReader();

	void readAmplitudeArray(int tickTime, AmplitudeArray &array);
};

} // sfx

#endif

