#include "precompiled.h"
#include "Dampers.h"

namespace util {

VectorDamper::VectorDamper()
{
	pos = VC3(0, 0, 0);
	speed = VC3(0, 0, 0);
	target = VC3(0, 0, 0);
	mass = 1.0f;
	k = 0.1f;
	overdampFactor = 1.0f;
	calculateC();
}

VectorDamper::~VectorDamper()
{
}

void VectorDamper::calculateC()
{
	c = sqrt(k*4*overdampFactor);
}

void VectorDamper::update(int ms)
{
	VC3 kterm = (pos-target)*-k;
	VC3 cterm = speed*(-c);
	VC3 force = kterm+cterm;
	speed += force*(ms*0.001f/mass);
	pos += speed*(ms*0.001f);
}

void VectorDamper::setTarget(VC3 target)
{
	this->target = target;
}

void VectorDamper::setK(float k)
{
	this->k = k;
	calculateC();
}

void VectorDamper::setOverdampFactor(float overdampFactor)
{
	this->overdampFactor = overdampFactor;
	calculateC();
}

void VectorDamper::cutToTarget()
{
	pos = target;
	speed = VC3(0, 0, 0);
}

VC3 VectorDamper::getPosition()
{
	return pos;
}

} // util


