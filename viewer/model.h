// Copyright 2002-2004 Frozenbyte Ltd.

#ifndef INCLUDED_VIEWER_MODEL_H
#define INCLUDED_VIEWER_MODEL_H

#ifndef INCLUDED_BOOST_SCOPED_PTR_HPP
#define INCLUDED_BOOST_SCOPED_PTR_HPP
#include <boost/scoped_ptr.hpp>
#endif
#ifndef INCLUDED_STRING
#define INCLUDED_STRING
#include <string>
#endif

class IStorm3D_Model;

namespace frozenbyte {
namespace editor {
	struct Storm;
	class Dialog;
}
	
namespace viewer {

struct ModelData;

class Model
{
	boost::scoped_ptr<ModelData> data;

public:
	Model(editor::Storm &storm);
	~Model();

	void loadGeometry(const std::string &fileName);
	void addGeometry(const std::string &fileName);
	void loadBones(const std::string &fileName);
	void setScale(float scale);

	void save(const std::string &fileName);
	void load(const std::string &fileName);
	void reload(const editor::Dialog &dialog);
	void freeResources();
	void loadResources();

	void rotateModel(float yAngleDelta, float xAngleDelta);

	void addAnimation(int groupIndex, const std::string &fileName);
	void removeAnimation(int groupIndex, int index);

	void playAnimation(int groupIndex, int index, bool loop);
	void stopAnimation(int groupIndex, int index);
	void setAnimationSpeedFactor(int groupIndex, int index, float speed);
	void setAnimationPaused(bool paused);
	int getAnimationTime(int groupIndex, int index);

	int getAnimationCount(int groupIndex) const;
	std::string getAnimation(int groupIndex, int index) const;

	void attachItems();

	IStorm3D_Model *getModel() const;
	bool hasModel() const;
};

} // end of namespace viewer
} // end of namespace frozenbyte

#endif
