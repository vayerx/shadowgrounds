#include <boost/shared_ptr.hpp>
#include <boost/scoped_ptr.hpp>
#include <list>
#include <vector>
#include <map>
#include <string>
#include <Storm3D_Ui.h>
#include "particle_typedef.h"
#include "particle.h"
#include "float_track.h"
#include "vector_track.h"
#include "particle_desc.h"


ParticleDesc::TextureInfo::TextureInfo() {

	nFrames = 0;
	columns = 1;
	rows = 1;
	alphaType = 0;
	animType = 0;
	startFrame = 0.0f;
	fps = 18.0f;


}

void ParticleDesc::TextureInfo::operator=(const TextureInfo& other) {

	name = other.name;
	path = other.path;
	nFrames = other.nFrames;
	columns = other.columns;
	rows = other.rows;
	alphaType = other.alphaType;
	animType = other.animType;
	startFrame = other.startFrame;
	fps = other.fps;
	
}

void ParticleDesc::TextureInfo::parseIn(ParserGroup& prev) {

	ParserGroup& g = prev.getSubGroup("texture_info");

	::parseIn(g, "name", name);
	::parseIn(g, "path", path);
	::parseIn(g, "frames", nFrames);
	::parseIn(g, "frame_width", columns);
	::parseIn(g, "frame_height", rows);
	::parseIn(g, "alpha_type", alphaType);
	::parseIn(g, "start_frame", startFrame);
	::parseIn(g, "frames_per_sec", fps);

}

void ParticleDesc::TextureInfo::parseOut(ParserGroup& prev) {

	ParserGroup g;

	::parseOut(g, "name", name);
	::parseOut(g, "path", path);
	::parseOut(g, "frames", nFrames);
	::parseOut(g, "frame_width", columns);
	::parseOut(g, "frame_height", rows);
	::parseOut(g, "alpha_type", alphaType);
	::parseOut(g, "start_frame", startFrame);
	::parseOut(g, "frames_per_sec", fps);

	prev.addSubGroup("texture_info", g);	

}


ParticleDesc::ParticleDesc() : mMaterial(0) {

	colorTrack.setNumKeys(2);
	colorTrack.setKey(0, 0, Vector(1.0f, 1.0f, 1.0f));
	colorTrack.setKey(1, 1.0f, Vector(0.0f, 0.0f, 0.0f));
	
	alphaTrack.setNumKeys(2);
	alphaTrack.setKey(0, 0, 1.0f);
	alphaTrack.setKey(1, 1.0f, 0.0f);

	sizeTrack.setNumKeys(2);
	sizeTrack.setKey(0, 0.0f, 0.2f);
	sizeTrack.setKey(1, 1.0f, 0.2f);
	
	minAngle = 0.0f;
	maxAngle = 0.0f;

	minSpin = 0.0f;
	maxSpin = 0.0f;

	minLife = 0.5f;
	maxLife = 1.0f;

	drawStyle = DSTYLE_QUAD;

	gravityMultiplier = 1.0f;

	dragFactor = 0.1f;

	dragFuncByVelocity = DRAG_LINEAR;
	
	dragFuncBySize = DRAG_NONE;

	collisionType = CTYPE_NONE;

	bounce = 1.0f;

}
	
	
const std::string& ParticleDesc::getName() {
	return mName;
}
	
void ParticleDesc::setName(const std::string& name) {
	mName = name;
}

void ParticleDesc::loadTexture(IStorm3D* s3d, const std::string& path, const std::string& name) {
	if(mMaterial == NULL) {
		std::string str = mName;
		mMaterial = s3d->CreateNewMaterial(str.c_str());
	}
	if(name.empty())
		return;
	
	IStorm3D_Texture* tex = s3d->CreateNewTexture(name.c_str());
	if(tex == NULL) {
		return;
	}
	mMaterial->SetBaseTexture(tex);

	texInfo.name = name;
	texInfo.path = path;
	
	setAlphaType((IStorm3D_Material::ATYPE)texInfo.alphaType);
}	

void ParticleDesc::setAlphaType(IStorm3D_Material::ATYPE type) {
	texInfo.alphaType = type;
	if(mMaterial)
		mMaterial->SetAlphaType(type);
}
	
IStorm3D_Material* ParticleDesc::getMaterial() {
	return mMaterial;
}


void ParticleDesc::parseIn(IStorm3D* s3d, ParserGroup& g) {
/*
	ParserGroup& cg = g.getSubGroup("color_track");
	colorTrack.parseIn(cg);
	
	ParserGroup& ag = g.getSubGroup("alpha_track");
	alphaTrack.parseIn(ag);
	
	ParserGroup& sg = g.getSubGroup("size_track");
	sizeTrack.parseIn(sg);
*/
	texInfo.parseIn(g);

	::parseIn(g, "name", mName);
	::parseIn(g, "min_angle", minAngle);
	::parseIn(g, "max_angle", maxAngle);
	::parseIn(g, "min_spin", minSpin);
	::parseIn(g, "max_spin", maxSpin);
	::parseIn(g, "min_life", minLife);
	::parseIn(g, "max_life", maxLife);
	::parseIn(g, "draw_style", drawStyle);
	::parseIn(g, "gravity_multiplier", gravityMultiplier);
	::parseIn(g, "drag_factor", dragFactor);
	::parseIn(g, "drag_function_by_velocity", dragFuncByVelocity);
	::parseIn(g, "drag_function_by_size", dragFuncBySize);
	::parseIn(g, "collision_type", collisionType);
	::parseIn(g, "bounce", bounce);
	
	loadTexture(s3d, "data/particles/", texInfo.name);

}

void ParticleDesc::parseOut(ParserGroup& g) {

	ParserGroup cg;
	colorTrack.parseOut(cg);
	g.addSubGroup("color_track", cg);

	ParserGroup ag;
	alphaTrack.parseOut(ag);
	g.addSubGroup("alpha_track", ag);

	ParserGroup sg;
	sizeTrack.parseOut(sg);
	g.addSubGroup("size_track", sg);

	texInfo.parseOut(g);

	::parseOut(g, "name", mName);
	::parseOut(g, "min_angle", minAngle);
	::parseOut(g, "max_angle", maxAngle);
	::parseOut(g, "min_spin", minSpin);
	::parseOut(g, "max_spin", maxSpin);
	::parseOut(g, "min_life", minLife);
	::parseOut(g, "max_life", maxLife);
	::parseOut(g, "draw_style", drawStyle);
	::parseOut(g, "gravity_multiplier", gravityMultiplier);
	::parseOut(g, "drag_factor", dragFactor);
	::parseOut(g, "drag_function_by_velocity", dragFuncByVelocity);
	::parseOut(g, "drag_function_by_size", dragFuncBySize);
	::parseOut(g, "collition_type", collisionType);
	::parseOut(g, "bounce", bounce);

}


/*
void ParticleDesc::save(std::ofstream& os) {

	colorTrack.save(os);
	alphaTrack.save(os);
	sizeTrack.save(os);
	
	texInfo.save(os);
	
	os << minAngle;
	os << maxAngle;
	os << minAngleChange;
	os << maxAngleChange;
	os << minLife;
	os << maxLife;
	os << drawStyle;
	

}

void ParticleDesc::load(IStorm3D* s3d, std::ifstream& is) {

	colorTrack.load(is);
	alphaTrack.load(is);
	sizeTrack.load(is);
	
	texInfo.load(is);
	
	loadTexture(s3d, texInfo.path, texInfo.name);
	setAlphaType(texInfo.alphaType);

	is >> minAngle;
	is >> maxAngle;
	is >> minAngleChange;
	is >> maxAngleChange;
	is >> minLife;
	is >> maxLife;
	is >> drawStyle;

}
*/