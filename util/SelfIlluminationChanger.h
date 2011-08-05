#ifndef INCLUDED_UTIL_SELF_ILLUMINATION_CHANGER_H
#define INCLUDED_UTIL_SELF_ILLUMINATION_CHANGER_H

#include <boost/scoped_ptr.hpp>
#include <DatatypeDef.h>

class IStorm3D_Model;
class IStorm3D_Scene;

namespace util {

class SelfIlluminationChanger
{
	struct Data;
	boost::scoped_ptr<Data> data;

public:
	SelfIlluminationChanger();
	~SelfIlluminationChanger();

	void addModel(IStorm3D_Model *model);
	void addAllModels(IStorm3D_Scene *scene);
	void clearModels();

	void setFactor(const COL &color);
};

} // utils

#endif
