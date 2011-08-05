#include <boost/shared_ptr.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/lexical_cast.hpp>
#include <list>
#include <vector>
#include <map>
#include <string>
#include <Storm3D_Ui.h>
#include "particle_typedef.h"
#include "vector_track.h"





struct VectorTrackData {

	struct Key {
		Key(float _val, float _t) : val(_val), time(_t) {}
		Key() {}
		Vector val;
		float time;
	};

	std::vector<Key*> mKeys;

	VectorTrackData() {

	}

	~VectorTrackData() {
		clearKeys();
	}
	
	void copy(VectorTrackData& other) {
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

	Vector eval(float time) {
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
		clearKeys();
		::parseIn(g, "num_keys", n);
		for(int i = 0; i < n; i++) {
			Key* k = new Key;
			std::string str = "key";
			str += boost::lexical_cast<std::string>(i);;
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
			str += boost::lexical_cast<std::string>(i);;
			::parseOut(g, (str + ".time"), k->time);
			::parseOut(g, (str + ".value"), k->val);
		}
	}

};


VectorTrack::VectorTrack() {
	ScopedPtr<VectorTrackData> temp(new VectorTrackData);
	m.swap(temp);
}

VectorTrack::~VectorTrack() {

}
	
void VectorTrack::operator=(VectorTrack& other) {
	m->copy(*other.m);
}

void VectorTrack::setNumKeys(int n) {
	m->setNumKeys(n);
}

int VectorTrack::getNumKeys() {
	return m->getNumKeys();
}

const Vector& VectorTrack::getKeyValue(int i) {
	return m->getKey(i)->val;
}

float VectorTrack::getKeyTime(int i) {
	return m->getKey(i)->time;
}

void VectorTrack::setKey(int i, float t, const Vector& val) {
	m->getKey(i)->time = t;
	m->getKey(i)->val = val;
}

Vector VectorTrack::eval(float t) {
	return m->eval(t);
}

void VectorTrack::parseIn(ParserGroup& g) {
	m->parseIn(g);
}

void VectorTrack::parseOut(ParserGroup& g) {
	m->parseOut(g);
}

