// Copyright 2002-2004 Frozenbyte Ltd.

#ifndef INCLUDED_EDITOR_EXPORTER_SCENE_H
#define INCLUDED_EDITOR_EXPORTER_SCENE_H

#ifndef INCLUDED_BOOST_SCOPED_PTR_HPP
#define INCLUDED_BOOST_SCOPED_PTR_HPP
#include <boost/scoped_ptr.hpp>
#endif
#ifndef INCLUDED_VECTOR
#define INCLUDED_VECTOR
#include <vector>
#endif
#ifndef INCLUDED_STRING
#define INCLUDED_STRING
#include <string>
#endif
#ifndef INCLUDED_DATATYPEDEF_H
#define INCLUDED_DATATYPEDEF_H
#include <datatypedef.h>
#endif

namespace frozenbyte {
namespace editor {

struct ExportOptions;
struct ExporterSceneData;

class ExporterScene
{
	boost::scoped_ptr<ExporterSceneData> data;

public:
	ExporterScene();
	~ExporterScene();

	void setHeightmap(const std::vector<unsigned short> &heightMap, const VC2I &mapSize, const VC3 &realSizet);
	void setObstaclemap(const std::vector<unsigned short> &obstacleMap);
	void setTextureRepeat(int value);
	void setAmbient(const TColor<unsigned char> &color);
	void setSun(const TColor<unsigned char> &color, const VC3 &direction);
	//void setFog(bool enabled, const TColor<unsigned char> &color, float start, float end);
	void addFog(const std::string &id, bool enabled, bool cameraCentric, const TColor<unsigned char> &color, float start, float end);
	void setCameraRange(float range);
	void setSun(const VC3 &direction);
	void setBackground(const std::string &modelName);

	void addTexture(const std::string &fileName);
	void setLightmapSize(int size);
	void setBlock(int blockIndex, int textureIndex, const std::vector<unsigned char> &weights);
	void setBlockLightmap(int blockIndex, const std::vector<unsigned char> &weights);
	void setColorMap(const VC2I &size, const std::vector<TColor<unsigned char> > &buffer);
	void addColorMap(const VC2I &size, const std::vector<TColor<unsigned char> > &buffer);

	void save(const ExportOptions &options) const;
};

} // end of namespace editor
} // end of namespace frozenbyte

#endif
