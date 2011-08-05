#pragma warning(disable:4103)
#pragma warning(disable:4786)

#include "editor_object_state.h"
#include <istorm3d_model.h>

using namespace std;
typedef map<IStorm3D_Model_Object *, bool> CollisionMap;

namespace frozenbyte {
namespace editor {

EditorObjectState::EditorObjectState()
{
}

EditorObjectState::~EditorObjectState()
{
}

void EditorObjectState::setCollision(IStorm3D_Model_Object *o)
{
	assert(o);
	collision[o] = !o->GetNoCollision();
}

bool EditorObjectState::hasCollision(IStorm3D_Model_Object *o) const
{
	assert(o);
	CollisionMap::const_iterator it = collision.find(o);
	if(it == collision.end())
		return false;

	return it->second;
}

} // editor
} // frozenbyte
