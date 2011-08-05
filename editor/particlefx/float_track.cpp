#include <boost/shared_ptr.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/lexical_cast.hpp>
#include <list>
#include <vector>
#include <map>
#include <string>
#include <Storm3D_Ui.h>
#include "particle_typedef.h"
#include "float_track.h"


struct FloatTrackData {

	struct Key {
		Key(float _val, float _t) : val(_val), time(_t) {}
		Key() {}
		float val;
		float time;
	};

	std::vector<Key*> mKeys;

	FloatTrackData() {

	}

	~FloatTrackData() {
		clearKeys();
	}
	
	void copy(FloatTrackData& other) {
		setNumKeys(other.getNumKeys());
		for(int i = 0; i < other.getNumKeys(); i++) {
			memcpy(mKeys[i], other.getKey(i), sizeof(Key));
		}
	}

	void clearKeys() {
		for(int i = 0; i < mKeys.size(); i++) {
			delete mKeys[i];
		}
		mKeys.clear();
	}

	void setNumKeys(int n) {
		clearKeys();
		mKeys.resize(n);
		for(int i = 0; i < n; i++) {
			mKeys[i] = new Key();
		}
	}

	int getNumKeys() {
		return mKeys.size();
	}

	Key* getKey(int i) {
		return mKeys[i];
	}

	float eval(float time) {
		if(mKeys.empty())
			return 0;
		if(time <= mKeys.front()->time) {
			return mKeys.front()->val;
		}
		if(time >= mKeys.back()->time) {
			return mKeys.back()->val;
		}
		for(int i = 0; i < (mKeys.size()-1); i++) {
			if(mKeys[i+1]->time > time)
				break;
		}
		Key* k1 = mKeys[i];
		Key* k2 = mKeys[i+1];
		float t = (time - k1->time) / (k2->time - k1->time);
		return (k1->val * (1.0f - t) + k2->val * t);
	}

	void parseIn(ParserGroup& g) {
		int n;
		::parseIn(g, "num_keys", n);
		if(n < 2)
			return;		
		clearKeys();
		for(int i = 0; i < n; i++) {
			Key* k = new Key;
			std::string str = "key";
			str += boost::lexical_cast<std::string>(i);
			::parseIn(g, (str + ".time"), k->time);
			::parseIn(g, (str + ".value"), k->val);
			mKeys.push_back(k);
		}
	}
		
	void parseOut(ParserGroup& g) {
		::parseOut(g, "num_keys", (int)mKeys.size());
		for(int i = 0; i < mKeys.size(); i++) {
			Key* k = mKeys[i];
			std::string str = "key";
			str += boost::lexical_cast<std::string>(i);
			::parseOut(g, (str + ".time"), k->time);
			::parseOut(g, (str + ".value"), k->val);
		}
	}

};


FloatTrack::FloatTrack() {
	ScopedPtr<FloatTrackData> temp(new FloatTrackData);
	m.swap(temp);
}

FloatTrack::~FloatTrack() {

}
	
void FloatTrack::operator=(FloatTrack& other) {
	m->copy(*other.m);
}

void FloatTrack::setNumKeys(int n) {
	m->setNumKeys(n);
}

int FloatTrack::getNumKeys() {
	return m->getNumKeys();
}

float FloatTrack::getKeyValue(int i) {
	return m->getKey(i)->val;
}

float FloatTrack::getKeyTime(int i) {
	return m->getKey(i)->time;
}

void FloatTrack::setKey(int i, float t, float val) {
	m->getKey(i)->time = t;
	m->getKey(i)->val = val;
}

float FloatTrack::eval(float t) {
	return m->eval(t);
}


void FloatTrack::parseIn(ParserGroup& g) {
	m->parseIn(g);
}

void FloatTrack::parseOut(ParserGroup& g) {
	m->parseOut(g);
}
