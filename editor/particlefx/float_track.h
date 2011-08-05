#ifndef FLOAT_TRACK_H
#define FLOAT_TRACK_H

struct FloatTrackData;
class FloatTrack {
	ScopedPtr<FloatTrackData> m; 
public:
	FloatTrack();
	~FloatTrack();
	void operator=(FloatTrack& other);
	void setNumKeys(int n);
	int getNumKeys();
	float getKeyValue(int i);
	float getKeyTime(int i);
	void setKey(int i, float t, float val);
	float eval(float t);
	void parseIn(ParserGroup& g);
	void parseOut(ParserGroup& g);
};

#endif
