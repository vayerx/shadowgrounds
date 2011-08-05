// Copyright 2002-2004 Frozenbyte Ltd.

#ifndef LIPSYNC_MANAGER_H
#define LIPSYNC_MANAGER_H

#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>
#include <string>

class IStorm3D;
class IStorm3D_Model;

namespace sfx {

class LipsyncProperties;
class AmplitudeArray;

class LipsyncManager
{
	struct Data;
	boost::scoped_ptr<Data> data;

public:
	LipsyncManager(IStorm3D *storm);
	~LipsyncManager();

	const LipsyncProperties &getProperties() const;
	boost::shared_ptr<AmplitudeArray> getAmplitudeBuffer(const std::string &file) const;

	void setIdle(IStorm3D_Model *model, const std::string &value, int fadeTime = -1);
	void setExpression(IStorm3D_Model *model, const std::string &value, int fadeTime = -1);
	void play(IStorm3D_Model *model, const boost::shared_ptr<AmplitudeArray> &array, int startTime);

	void update(int ms, int currentTime);
	void reset();
};

} // sfx

#endif

