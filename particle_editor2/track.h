// Copyright 2002-2004 Frozenbyte Ltd.

#ifndef PARTICLE_TRACK_H
#define PARTICLE_TRACK_H

namespace frozenbyte
{
namespace particle
{

enum TRACK_TYPE
{
	TRACK_INT,
	TRACK_FLOAT,
	TRACK_VECTOR
};

class Key {
public:
	virtual ~Key() {}
	float time;
};

class FloatKey : public Key {
public:
	FloatKey() {}
	FloatKey(float v, float t) { value = v; time = t; }
	float value;
};

class VectorKey : public Key {
public:
	VectorKey() {}
	VectorKey(const Vector& v, float t) { value = v; time = t; }
	Vector value;
};

class KeyControl {
public:
	virtual ~KeyControl() {}
	virtual int getNumKeys()=0;
	virtual void setNumKeys(int n)=0;
	virtual void sortKeys()=0;
	virtual void setKey(int i, Key* k)=0;
	virtual void getKey(int i, Key* k)=0;
	virtual void addKey(Key* k)=0;
};

class Track {
public:
	virtual ~Track() {};
	virtual int getType()=0;
	virtual void setValue(void* v, float t=0)=0;
	virtual void getValue(void* v, float t=0) const=0;
	virtual KeyControl* getKeyControl()=0;
};

class FloatTrack : public Track, public KeyControl {
	std::vector<FloatKey*> m_keys;
public:
	~FloatTrack();
	
	KeyControl* getKeyControl();

	int getType();
	void setValue(void* v, float t);
	void getValue(void* v, float t) const;

	int getNumKeys();
	void setNumKeys(int n);
	void sortKeys();
	void setKey(int i, Key* k);
	void getKey(int i, Key* k);
	void addKey(Key* k);
};

class VectorTrack : public Track, public KeyControl {
	std::vector<VectorKey*> m_keys;
public:
	~VectorTrack();

	KeyControl* getKeyControl();

	int getType();
	void setValue(void* v, float t);
	void getValue(void* v, float t) const;

	int getNumKeys();
	void setNumKeys(int n);
	void sortKeys();
	void setKey(int i, Key* k);
	void getKey(int i, Key* k);
	void addKey(Key* k);

};



} // particle

} // frozenbyte

#endif
