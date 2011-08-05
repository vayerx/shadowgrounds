
#include "precompiled.h"

#include "HelperPositionCalculator.h"
#include "../system/Logger.h"
#include <IStorm3D_Model.h>
#include <IStorm3D_Bone.h>
#include <boost/scoped_ptr.hpp>
#include <cstdio>

using namespace std;
using namespace boost;

namespace util {

bool getHelperPositionOffset(IStorm3D_Model *model, const char *name, const VC3 &relativeOffset, VC3 &result)
{
	if(!model || !name)
	{
		assert(!"Null pointers given to getHelperPosition");
		return false;
	}

	IStorm3D_Helper *helper = model->SearchHelper(name);
	if(helper && helper->GetParentBone())
	{
		result = relativeOffset;
		helper->GetParentBone()->GetMXG().RotateVector(result);
		result += helper->GetGlobalPosition();
		return true;
	}
	
	return false;
}

bool getHelperPosition(IStorm3D_Model *model, const char *name, VC3 &result)
{
	if(!model || !name)
	{
		assert(!"Null pointers given to getHelperPosition");
		return false;
	}

	IStorm3D_Helper *helper = model->SearchHelper(name);
	if(helper)
	{
		result = helper->GetGlobalPosition();
		return true;
	}

	/*
	string message = "getHelperPosition - Given helper not found: ";
	message += name;
	Logger::getInstance()->error(message.c_str());
	*/

	return false;
}

} // util
