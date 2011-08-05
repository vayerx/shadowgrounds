#ifndef INCLUDED_HELPER_POSITION_CALCULATOR_H
#define INCLUDED_HELPER_POSITION_CALCULATOR_H

#include <DatatypeDef.h>
class IStorm3D_Model;

namespace util {

bool getHelperPosition(IStorm3D_Model *model, const char *name, VC3 &result);
bool getHelperPositionOffset(IStorm3D_Model *model, const char *name, const VC3 &relativeOffset, VC3 &result);

} // util

#endif
