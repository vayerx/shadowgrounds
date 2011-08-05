#include <boost/shared_ptr.hpp>
#include <boost/scoped_ptr.hpp>
#include <fstream>
#include <list>
#include <vector>
#include <map>
#include <string>
#include <Storm3D_Ui.h>

#include "particle_typedef.h"
#include "particle.h"
#include "float_track.h"
#include "vector_track.h"
#include "emitter_desc.h"



EmitterDesc::EmitterDesc(std::string name) : mName(name) {

/*
	SharedPtr<IPositionGen> gen(new SphereGen());
	SphereGen* sg = (SphereGen*)gen.get();
	sg->innerRadius = 0.0f;
	sg->outerRadius = 0.1f;
	mGen.swap(gen);

	emitDirectionTrack.setNumKeys(2);
	emitDirectionTrack.setKey(0, 0.0f, Vector(0.0f, 0.0f, 0.0f));
	emitDirectionTrack.setKey(1, 1.0f, Vector(0.0f, 0.0f, 0.0f));
*/	

	emitRateTrack.setNumKeys(2);
	emitRateTrack.setKey(0, 0.0f, 10.0f);
	emitRateTrack.setKey(1, 1.0f, 10.0f);
	
	// dont inherit any velocity from the system
	velocityFactor = 0.0f;

	minEmitTime = 5.0f;
	maxEmitTime = 5.0f;	

	dieAfterEmission = true;

}

const std::string& EmitterDesc::getName() {
	return mName;
}
	
void EmitterDesc::setName(const std::string& name) {
	mName = name;
}

/*	
void EmitterDesc::setPositionGen(SharedPtr<IPositionGen> gen) {
	mGen = gen;
}

SharedPtr<IPositionGen> EmitterDesc::getPositionGen() {
	return mGen;
}
*/

void EmitterDesc::parseIn(ParserGroup& g) {
	
//	ParserGroup& pg = g.getSubGroup("emit_rate_track");
//	emitRateTrack.parseIn(pg);
	
	::parseIn(g, "name", mName);
	::parseIn(g, "min_emission_time", minEmitTime);
	::parseIn(g, "max_emission_time", maxEmitTime);
	::parseIn(g, "position", position);
	::parseIn(g, "velocity_inheritance_factor", velocityFactor);

}


void EmitterDesc::parseOut(ParserGroup& g) {

	//ParserGroup& pg = g.getSubGroup("emit_rate_track");
	//emitRateTrack.parseOut(pg);
	ParserGroup eg;
	emitRateTrack.parseOut(eg);
	g.addSubGroup("emit_rate_track", eg);

	::parseOut(g, "name", mName);
	::parseOut(g, "min_emission_time", minEmitTime);
	::parseOut(g, "max_emission_time", maxEmitTime);
	::parseOut(g, "position", position);
	::parseOut(g, "velocity_inheritance_factor", velocityFactor);

}




float EmitterDesc::fRand(float min, float max) {	
	return min + (max - min) * rand() / (float)RAND_MAX;
}




SprayEmitterDesc::SprayEmitterDesc() {

	position = Vector(0.0f, 0.0f, 0.0f);
	
	spread1 = 0.0f;
	spread2 = 0.0f;

	minSpeed = 1.0f;
	maxSpeed = 1.0f;

}

void SprayEmitterDesc::parseIn(ParserGroup& g) {

	EmitterDesc::parseIn(g);
	
	::parseIn(g, "spread1", spread1);
	::parseIn(g, "spread2", spread2);

	::parseIn(g, "min_speed", minSpeed);
	::parseIn(g, "max_speed", maxSpeed);

}


void SprayEmitterDesc::parseOut(ParserGroup& g) {

	EmitterDesc::parseOut(g);
	
	::parseOut(g, "spread1", spread1);
	::parseOut(g, "spread2", spread2);

	::parseOut(g, "min_speed", minSpeed);
	::parseOut(g, "max_speed", maxSpeed);

}

int SprayEmitterDesc::getType() {
	return ED_SPRAY;
}
	
	
void SprayEmitterDesc::genVelocity(Vector& vel) {

	Vector dir(0.0f, 1.0f, 0.0f);
	dir.Normalize();
	
	Vector up(0.0f, 0.0f, 1.0f);
	Vector v = dir.GetCrossWith(up);
	if(v.GetDotWith(v)<0.0001f) {
		up = Vector(0.0f, 1.0f, 0.0f);	
	}
	
	Vector left;
	left = dir.GetCrossWith(up);
	left.Normalize();
	up = left.GetCrossWith(dir);
	up.Normalize();
		
	float a1 = fRand(-1.0f, 1.0f) * spread1;
	float a2 = fRand(-1.0f, 1.0f) * spread2;
	
	QUAT q1;
	QUAT q2;
	
	q1.MakeFromAxisRotation(left, a1);
	q2.MakeFromAxisRotation(up, a2);
	
	q1.RotateVector(dir);
	q2.RotateVector(dir);
				
	vel = dir * (minSpeed + (maxSpeed - minSpeed) * fRand(0.0f, 1.0f));
	
}
	
void SprayEmitterDesc::genPosition(Vector& pos) {

	pos = position;

}


	
PointArrayEmitterDesc::PointArrayEmitterDesc() {

	rangeStart = 0;
	rangeEnd = 0;

}

void PointArrayEmitterDesc::parseIn(ParserGroup& g) {

	EmitterDesc::parseIn(g);
	
	::parseIn(g, "first_vertex", rangeStart);
	::parseIn(g, "last_vertex", rangeEnd);

	int n;
	::parseIn(g, "num_verts", n);
	mVerts.resize(n);
	mNormals.resize(n);
	for(int i = 0; i < n; i++) {
		std::string str = "v";
//		str.;
		::parseIn(g, str, mVerts[i]);
		str = "n";
//		str << i;
		::parseIn(g, str, mNormals[i]);
	}

}

void PointArrayEmitterDesc::parseOut(ParserGroup& g) {

	EmitterDesc::parseOut(g);

	::parseOut(g, "first_vertex", rangeStart);
	::parseOut(g, "last_vertex", rangeEnd);

	::parseOut(g, "num_verts", (int)mVerts.size());	
	for(int i = 0; i < mVerts.size(); i++) {
		std::string str = "v";
//		str << i;
		::parseOut(g, str, mVerts[i]);
		str = "n";
//		str << i;
		::parseOut(g, str, mNormals[i]);
	}

}




int PointArrayEmitterDesc::getType() {
	return ED_POINT_ARRAY;
}

	
void PointArrayEmitterDesc::setModel(IStorm3D_Mesh* obj) {

}

void PointArrayEmitterDesc::loadModel(IStorm3D* s3d, const std::string& name) {

	IStorm3D_Model* obj = s3d->CreateNewModel();
	if(!obj->LoadS3D(name.c_str())) {
		delete obj;
		return;
	}


}




void PointArrayEmitterDesc::genVelocity(Vector& vel) {

	mIndex = rangeStart + rand() % (rangeEnd - rangeStart);

	vel = mNormals[mIndex] * (minSpeed + (maxSpeed - minSpeed) * fRand(0.0f, 1.0f));

}
	
void PointArrayEmitterDesc::genPosition(Vector& pos) {

	pos = mVerts[mIndex];	

}



CloudEmitterDesc::CloudEmitterDesc() {

	shapeType = SHAPE_BOX;
	
	minSpeed = 1.0f;
	maxSpeed = 1.0f;

	randomDirection = true;

	direction = Vector(0.0f, 1.0f, 0.0f);

	bmin = Vector(-0.5f, -0.5f, -0.5f);
	bmax = Vector(0.5f, 0.5f, 0.5f);

	cRadius = 0.5f;
	cHeight = 2.0f;

	sInnerRadius = 0.0f;
	sOuterRadius = 1.0f;
}
	
int CloudEmitterDesc::getType() {
	return ED_CLOUD;
}

	

void CloudEmitterDesc::genVelocity(Vector& vel) {
	if(randomDirection) {
		vel = Vector(fRand(-1.0f, 1.0f), fRand(-1.0f, 1.0f), fRand(-1.0f, 1.0f));
		vel.Normalize();
	} else {
		vel = direction;
	}
	vel *= (minSpeed + (maxSpeed - minSpeed) * fRand(0.0f, 1.0f));
}
	

void CloudEmitterDesc::genPosition(Vector& pos) {
	
	if(shapeType == SHAPE_BOX) {
		pos.x = position.x + bmin.x + (bmax.x - bmin.x) * fRand(0.0f, 1.0f);
		pos.y = position.y + bmin.y + (bmax.y - bmin.y) * fRand(0.0f, 1.0f);
		pos.z = position.z + bmin.z + (bmax.z - bmin.z) * fRand(0.0f, 1.0f);
	}
	if(shapeType == SHAPE_SPHERE) {
		Vector v = Vector(fRand(-1.0f, 1.0f), fRand(-1.0f, 1.0f), fRand(-1.0f, 1.0f));
		v.Normalize();
		pos = position + v * (sInnerRadius + (sOuterRadius - sInnerRadius) * fRand(0.0f, 1.0f));
	}
	if(shapeType == SHAPE_CYLINDER) {
		float r2 = cRadius;//cRadius * cRadius;
		pos.y = fRand(-1.0f, 1.0f) * cHeight * 0.5f;
		float len = r2;
		while((float)sqrt(len) > r2) {
			pos.x = fRand(-0.5f, 0.5f) * cRadius; 
			pos.z = fRand(-0.5f, 0.5f) * cRadius; 
			len = pos.x * pos.x + pos.z * pos.z;
		}
	}
}

void CloudEmitterDesc::parseIn(ParserGroup& g) {

	EmitterDesc::parseIn(g);

	::parseIn(g, "shape", shapeType);
	if(shapeType == SHAPE_BOX) {
		::parseIn(g, "box_min", bmin);
		::parseIn(g, "box_max", bmax);
	}
	if(shapeType == SHAPE_SPHERE) {
		::parseIn(g, "inner_radius", sInnerRadius);
		::parseIn(g, "outer_radius", sOuterRadius);
	}
	if(shapeType == SHAPE_CYLINDER) {
		::parseIn(g, "radius", cRadius);
		::parseIn(g, "height", cHeight);
	}

}

void CloudEmitterDesc::parseOut(ParserGroup& g) {

	EmitterDesc::parseOut(g);

	::parseOut(g, "shape", shapeType);
	if(shapeType == SHAPE_BOX) {
		::parseOut(g, "box_min", bmin);
		::parseOut(g, "box_max", bmax);
	}
	if(shapeType == SHAPE_SPHERE) {
		::parseOut(g, "inner_radius", sInnerRadius);
		::parseOut(g, "outer_radius", sOuterRadius);
	}
	if(shapeType == SHAPE_CYLINDER) {
		::parseOut(g, "radius", cRadius);
		::parseOut(g, "height", cHeight);
	}
	
}	


