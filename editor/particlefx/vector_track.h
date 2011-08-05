#ifndef VECTOR_TRACK_H
#define VECTOR_TRACK_H

struct VectorTrackData;
class VectorTrack {
	ScopedPtr<VectorTrackData> m; 
public:
	VectorTrack();
	~VectorTrack();
	void operator=(VectorTrack& other);
	void setNumKeys(int n);
	int getNumKeys();
	const Vector& getKeyValue(int i);
	float getKeyTime(int i);
	void setKey(int i, float t, const Vector& v);
	Vector eval(float t);	
	void parseIn(ParserGroup& g);
	void parseOut(ParserGroup& g);
};


#endif
