#ifndef INCLUDED_DAMPERS_H
#define INCLUDED_DAMPERS_H

#include <DatatypeDef.h>

namespace util {

class VectorDamper
{
protected:
	VC3 pos;
	VC3 speed;
	VC3 target;
	float k;
	float c;
	float mass;
	float overdampFactor;

	void calculateC();
public:
	VectorDamper();
	~VectorDamper();

	void update(int ms);
	void setTarget(VC3 target);

	void setK(float k);
	void setOverdampFactor(float overdampFactor);

	void cutToTarget();

	VC3 getPosition();
};

} // util

#endif
