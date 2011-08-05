#include <storm3d_ui.h>
#include <vector>
#include "track.h"

namespace frozenbyte
{
namespace particle
{

VectorTrack::~VectorTrack() {
	setNumKeys(0);
}

int VectorTrack::getType() {
	return TRACK_VECTOR;
}

void VectorTrack::setValue(void* v, float t) {			
	if(m_keys.empty()) {
		m_keys.push_back(new VectorKey(*((Vector*)v), t));
		return;
	}
	for(int i = 0; i < m_keys.size()-1; i++) {
		if(m_keys[i+1]->time > t)
			break;
	}
	m_keys.insert(m_keys.begin()+i, new VectorKey(*((float*)v), t));
}
void VectorTrack::getValue(void* v, float t) {
	Vector* f = (Vector*)v;
	if(m_keys.empty())
		return;
	if(m_keys.size()==1) {
		*f = m_keys.front()->value;
		return;
	}
	if(t < m_keys.front()->time) {
		*f = m_keys.front()->value;
		return;
	}
	if(t > m_keys.back()->time) {
		*f = m_keys.back()->value;
		return;
	}
	for(int i = 0; i < m_keys.size()-1; i++) {
		if(m_keys[i+1]->time > t)
			break;
	}
	VectorKey* k1 = m_keys[i];
	VectorKey* k2 = m_keys[i+1];
	float a = (t - k1->time) / (k2->time - k1->time);
	*f = k1->value * (1.0f - a) + k2->value * a;
}

int VectorTrack::getNumKeys() {
	return m_keys.size();
}
void VectorTrack::setNumKeys(int n) {
	for(int i = 0; i < m_keys.size(); i++)
		delete m_keys[i];
	m_keys.resize(n);
	for(i = 0; i < n; i++) {
		m_keys[i] = new VectorKey();
	}
}
void VectorTrack::sortKeys() {
	
}
void VectorTrack::setKey(int i, Key* k) {
	assert(i < m_keys.size());
	VectorKey* p = m_keys[i];
	p->time = k->time;
	p->value = ((VectorKey*)k)->value;
}
void VectorTrack::getKey(int i, Key* k) {
	assert(i < m_keys.size());
	VectorKey* p = m_keys[i];
	k->time = p->time;
	((VectorKey*)k)->value = p->value;	
}
void VectorTrack::addKey(Key* k) {
	VectorKey* key = new VectorKey();
	key->time = k->time;
	key->value = ((VectorKey*)k)->value;
	m_keys.push_back(key);
}

KeyControl* VectorTrack::getKeyControl() {
	return (KeyControl*)this;
}


} // particle

} // frozenbyte