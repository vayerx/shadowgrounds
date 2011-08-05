// Copyright 2002-2004 Frozenbyte Ltd.

#ifndef INCLUDED_LIPSYNC_MANAGER_H
#define INCLUDED_LIPSYNC_MANAGER_H

#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>
#include <string>

class IStorm3D;
class IStorm3D_Terrain;
class IStorm3D_Texture;

namespace sfx {
	class AmplitudeArray;
} // sfx

namespace util {

class LipsyncManager
{
	struct Data;
	boost::scoped_ptr<Data> data;

public:
	LipsyncManager(IStorm3D *storm, IStorm3D_Terrain *terrain);
	~LipsyncManager();

	enum CharPosition
	{
		Left = 0,
		Right = 1,
		Middle = 2
	};

	const std::string &getCharacter(CharPosition position) const;
	boost::shared_ptr<sfx::AmplitudeArray> getAmplitudeBuffer(const std::string &file) const;
	bool isActive() const;


	void setCharacter(CharPosition position, const std::string &id);
	void setIdle(const std::string &character, const std::string &idleAnimation, int fadeTime = -1);
	void setExpression(const std::string &character, const std::string &idleAnimation, int fadeTime = -1);
	void playSpeech(const std::string &character, const boost::shared_ptr<sfx::AmplitudeArray> &amplitudes, int time);

	// some rendering settings
	void setCamera(CharPosition pos, const VC3 &cameraPos, const VC3 &cameraTarget, float aspectRatio = 0.0f);
	void setBackground(CharPosition pos, COL color);
	void resetCamera(CharPosition pos);	
	void resetBackground(CharPosition pos);

	void setActive(bool active, int numChars = 2);
	void update();
};

} // util

#endif
