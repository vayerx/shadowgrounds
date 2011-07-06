
#ifndef GAME_AREAS_CAMERAAREA_H
#define GAME_AREAS_CAMERAAREA_H

// NOTE: this is not to be confused with camera areas in ui/camera_system!

#include "PrioritizedWeightedQuadArea.h"
#include <vector>

namespace game
{

class CameraArea : public PrioritizedWeightedQuadArea
{
public:
	
	CameraArea(const std::vector<VC3> &corners)
	{
	}

protected:

};

}

#endif
