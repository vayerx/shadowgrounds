
#include "precompiled.h"

// Copyright 2002-2004 Frozenbyte Ltd.

#include <Storm3D_UI.h>
#include <vector>
#include "track.h"

namespace frozenbyte
{
namespace particle
{

FloatTrack::~FloatTrack() {
	setNumKeys(0);
}

int FloatTrack::getType() {
	return TRACK_FLOAT;
}

void FloatTrack::setValue(void* v, float t) {		
	if(m_keys.empty()) {
		m_keys.push_back(new FloatKey(*((float*)v), t));
		return;
	}
	int i = 0;
	for(i = 0; i < (int)m_keys.size()-1; i++) {
		if(m_keys[i+1]->time > t)
			break;
	}
	m_keys.insert(m_keys.begin()+i, new FloatKey(*((float*)v), t));
}
void FloatTrack::getValue(void* v, float t) const {
	float* f = (float*)v;
	if(m_keys.empty())
		return;
	if(m_keys.size()==1) {
		*f = m_keys.front()->value;
		return;
	}
	if(t <= m_keys.front()->time) {
		*f = m_keys.front()->value;
		return;
	}
	if(t >= m_keys.back()->time) {
		*f = m_keys.back()->value;
		return;
	}

	int i = 0;
	for(i = 1; i < (int)m_keys.size(); ++i) {
		if(m_keys[i]->time > t)
			break;
	}

	if(i == int(m_keys.size()))
	{
		*f = 0.f;
		return;
	}

	FloatKey* k1 = m_keys[i-1];
	FloatKey* k2 = m_keys[i];
	float a = (t - k1->time) / (k2->time - k1->time);
	*f = k1->value * (1.0f - a) + k2->value * a;
}

int FloatTrack::getNumKeys() {
	return m_keys.size();
}
void FloatTrack::setNumKeys(int n) {
	for(int i = 0; i < (int)m_keys.size(); i++)
		delete m_keys[i];
	m_keys.resize(n);
	for(int i = 0; i < n; i++) {
		m_keys[i] = new FloatKey();
	}
}
void FloatTrack::sortKeys() {
	
}
void FloatTrack::setKey(int i, Key* k) {
	assert(i < (int)m_keys.size());
	FloatKey* p = m_keys[i];
	p->time = k->time;
	p->value = ((FloatKey*)k)->value;
}
void FloatTrack::getKey(int i, Key* k) {
	assert(i < (int)m_keys.size());
	FloatKey* p = m_keys[i];
	k->time = p->time;
	((FloatKey*)k)->value = p->value;	
}
void FloatTrack::addKey(Key* k) {
	FloatKey* key = new FloatKey();
	key->time = k->time;
	key->value = ((FloatKey*)k)->value;
	m_keys.push_back(key);
}

KeyControl* FloatTrack::getKeyControl() {
	return (KeyControl*)this;
}
 
} // particle

} // frozenbyte
