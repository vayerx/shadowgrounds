#ifndef INCLUDED_STORM3D_VIDEOSTREAMER_H
#define INCLUDED_STORM3D_VIDEOSTREAMER_H

#include <boost/scoped_ptr.hpp>
#include <istorm3d_videostreamer.h>

class Storm3D;
class IStorm3D_StreamBuilder;

class Storm3D_VideoStreamer: public IStorm3D_VideoStreamer
{
	struct Data;
	boost::scoped_ptr<Data> data;

public:
	Storm3D_VideoStreamer(Storm3D &storm, const char *fileName, IStorm3D_StreamBuilder *streamBuilder, bool loop, bool downscale, bool higherColorRange);
	~Storm3D_VideoStreamer();

	bool hasVideo() const;
	bool hasEnded() const;
	int getTime() const;
	bool isPlaying() const;
	
	IStorm3D_Material *getMaterial();
	void setPosition(const VC2 &position, const VC2 &size);
	void setAlpha(float alpha);
	void update();
	void render(IStorm3D_Scene *scene);
	virtual void getTextureCoords(float &x, float &y);
};

#endif

