
#include "precompiled.h"

// Copyright 2002-2004 Frozenbyte Ltd.


#pragma warning( disable : 4800 )

#include <boost/shared_ptr.hpp>
#include <boost/scoped_ptr.hpp>
#include <vector>
#include <string>
#include <map>
#include <list>
#include <fstream>
#include <storm3d_ui.h>
#include "..\editor\string_conversions.h"
#include "..\editor\parser.h"
#include "track.h"
//#include "paramblock.h"
#include "particletiming.h"
#include "parseutil.h"
#include "particlesystem.h"
#include "particleforces.h"

using namespace frozenbyte::editor;

namespace frozenbyte  {
namespace particle {
namespace {

	int dragId = 1;
	int gravityId = 2;
	int windId = 3;
	int sideGravityId = 4;
	int gravityPointId = 5;

} // unnamed

// DRAG-----------------------------------------------------

int DragParticleForce::getType()
{
	return dragId;
}

void DragParticleForce::setFactor(float f) {
	m_factor = f;
}
	
float DragParticleForce::getFactor() {
	return m_factor;
}
	
void DragParticleForce::preCalc(float t) {

}

void DragParticleForce::calcForce(Vector& force, const Vector& pos, const Vector& vel) {
	float len = vel.GetLength();
	Vector newVel = vel * (float)pow(1.0f - m_factor, len);
	force = newVel - vel;
}
	
void DragParticleForce::parseFrom(const editor::ParserGroup& pg) {
	m_factor = convertFromString<float>(pg.getValue("factor", ""), 0.0f);
}

int DragParticleForce::getTypeId() const
{
	return dragId;
}

// GRAVITY -----------------------------------------------------

int GravityParticleForce::getType()
{
	return gravityId;
}

void GravityParticleForce::setGravity(float f) {
	m_gravity = f;
}
	
float GravityParticleForce::getGravity() {
	return m_gravity;
}

void GravityParticleForce::preCalc(float t) {

}
	
void GravityParticleForce::calcForce(Vector& force, const Vector& pos, const Vector& vel) {
#if defined(PROJECT_AOV) && defined(PROJECT_PARTICLE_EDITOR)
	force.x = 0.0f;
	force.y = 0.0f;
	force.z = -m_gravity * PARTICLE_TIME_SCALE;
#else
	force.x = 0.0f;
	force.y = -m_gravity * PARTICLE_TIME_SCALE;
	force.z = 0.0f;
#endif
}
	
void GravityParticleForce::parseFrom(const editor::ParserGroup& pg) {
	m_gravity = convertFromString<float>(pg.getValue("gravity", ""), 0.0f);
}

int GravityParticleForce::getTypeId() const
{
	return gravityId;
}

// SIDE GRAVITY -----------------------------------------------------

int SideGravityParticleForce::getType()
{
	return sideGravityId;
}

void SideGravityParticleForce::setGravity(float f) {
	m_sideGravity = f;
}
	
float SideGravityParticleForce::getGravity() {
	return m_sideGravity;
}

void SideGravityParticleForce::preCalc(float t) {

}
	
void SideGravityParticleForce::calcForce(Vector& force, const Vector& pos, const Vector& vel) {
#if defined(PROJECT_AOV) && defined(PROJECT_PARTICLE_EDITOR)
	force.x = 0.0f;
	force.y = -m_sideGravity * PARTICLE_TIME_SCALE;
	force.z = 0.0f;
#else
	force.x = 0.0f;
	force.y = 0.0f;
	force.z = -m_sideGravity * PARTICLE_TIME_SCALE;
#endif
}
	
void SideGravityParticleForce::parseFrom(const editor::ParserGroup& pg) {
	m_sideGravity = convertFromString<float>(pg.getValue("sidegravity", ""), 0.0f);
}

int SideGravityParticleForce::getTypeId() const
{
	return sideGravityId;
}

// GRAVITY POINT ---------------------------------------------------

int GravityPointParticleForce::getType()
{
	return gravityPointId;
}

void GravityPointParticleForce::setGravity(float f) {
	m_gravity = f;
}
	
float GravityPointParticleForce::getGravity() {
	return m_gravity;
}

void GravityPointParticleForce::setPoint(const VC3 &point) {
	m_point = point;
}
	
VC3 GravityPointParticleForce::getPoint() {
	return m_point;
}

void GravityPointParticleForce::preCalc(float t) {

}
	
void GravityPointParticleForce::calcForce(Vector& force, const Vector& pos, const Vector& vel) {
	VC3 diff = pos - m_point;
	VC3 diffNorm = diff.GetNormalized();
	float diffLenSq = diff.GetSquareLength();
	if (fabsf(diffLenSq) < 0.01f)
	{
		force.x = 0.0f;
		force.y = 0.0f;
		force.z = 0.0f;
	} else {
		// realistic gravity (bad)
		//force = -diffNorm * 0.001f * m_gravity / diffLenSq;
		// unrealisic gravity (better)
		float diffLen = sqrtf(diffLenSq);
		force = -diffNorm * 0.001f * m_gravity / diffLen;
	}
}
	
void GravityPointParticleForce::parseFrom(const editor::ParserGroup& pg) {
	m_gravity = convertFromString<float>(pg.getValue("gravity", ""), 0.0f);
	m_point = convertVectorFromString(pg.getValue("point", "0,0,0"));
}

int GravityPointParticleForce::getTypeId() const
{
	return gravityPointId;
}

// WIND -----------------------------------------------------

int WindParticleForce::getType()
{
	return windId;
}

void WindParticleForce::setWindEffectFactor(float f) 
{
	m_wind_effect_factor = f;
}
	
float WindParticleForce::getWindEffectFactor() 
{
	return m_wind_effect_factor;
}

void WindParticleForce::setSpiralAmount(float f) 
{
	m_spiral_amount = f;
}
	
float WindParticleForce::getSpiralAmount() 
{
	return m_spiral_amount;
}

void WindParticleForce::setSpiralSpeed(float f) 
{
	m_spiral_speed = f;
}
	
float WindParticleForce::getSpiralSpeed() 
{
	return m_spiral_speed;
}

void WindParticleForce::preCalc(float t) 
{
}
	
void WindParticleForce::calcForce(Vector &force, const Vector &pos, const Vector &vel) 
{
	force.x = global_wind_velocity * cosf(global_wind_angle);
	force.y = 0.0f;
	force.z = global_wind_velocity * sinf(global_wind_angle);
	force *= m_wind_effect_factor;

	if(m_spiral_amount != 0.0f)
	{
	    force.x += m_spiral_amount * cosf(wind_timer * m_spiral_speed);
		force.z += m_spiral_amount * sinf(wind_timer * m_spiral_speed);
	}

	force *= PARTICLE_TIME_SCALE;
}
	
void WindParticleForce::parseFrom(const editor::ParserGroup& pg) {
	m_wind_effect_factor = convertFromString<float>(pg.getValue("wind_effect_factor", ""), 0.0f);
	m_spiral_amount = convertFromString<float>(pg.getValue("spiral_amount", ""), 0.0f);
	m_spiral_speed = convertFromString<float>(pg.getValue("spiral_speed", ""), 0.0f);
}

float WindParticleForce::wind_timer = 0.0f;
float WindParticleForce::global_wind_angle = 45.0f*3.1415f/180.0f;
float WindParticleForce::global_wind_velocity = 5.0f;

void WindParticleForce::setGlobalWindAngle(float angleDegrees)
{
  WindParticleForce::global_wind_angle = angleDegrees * 3.1415926f/180.0f;
}

void WindParticleForce::setGlobalWindVelocity(float velocity)
{
  WindParticleForce::global_wind_velocity = velocity;
}

void WindParticleForce::advanceWind(float seconds)
{
  WindParticleForce::wind_timer += seconds;
}

int WindParticleForce::getTypeId() const
{
	return windId;
}

} // particle
} // frozenbyte