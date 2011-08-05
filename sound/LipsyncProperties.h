// Copyright 2002-2004 Frozenbyte Ltd.

#ifndef INCLUDEDE_LIPSYNC_PROPERTIES_H
#define INCLUDEDE_LIPSYNC_PROPERTIES_H

#include <boost/scoped_ptr.hpp>
#include <string>

namespace sfx {

class LipsyncProperties
{
	struct Data;
	boost::scoped_ptr<Data> data;

public:
	LipsyncProperties();
	~LipsyncProperties();

	enum Property
	{
		IdleFadeTime,
		ExpressionFadeTime,
		SampleRate
	};

	int getPropertyValue(Property property) const;

	int getIdleAnimationAmount() const;
	const std::string &getIdleAnimation(int index) const;
	const std::string &getIdleAnimationName(int index) const;
	int getExpressionAnimationAmount() const;
	const std::string &getExpressionAnimation(int index) const;
	const std::string &getExpressionAnimationName(int index) const;

	struct Phonem
	{
		unsigned char limit;
		std::string file;

		Phonem()
		:	limit(0)
		{
		}
	};

	int getPhonemAmount() const;
	const Phonem &getPhonem(int index) const;
};

} // sfx

#endif
