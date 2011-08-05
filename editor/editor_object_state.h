#ifndef INCLUDED_EDITOR_OBJECT_STATE_H
#define INCLUDED_EDITOR_OBJECT_STATE_H

#include <map>
class IStorm3D_Model_Object;

namespace frozenbyte {
namespace editor {

class EditorObjectState
{
	std::map<IStorm3D_Model_Object *, bool> collision;

public:
	EditorObjectState();
	~EditorObjectState();

	void setCollision(IStorm3D_Model_Object *o);
	bool hasCollision(IStorm3D_Model_Object *o) const;
};

} // editor
} // frozenbyte

#endif