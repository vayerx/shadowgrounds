#include <storm3d_ui.h>
#include <boost/shared_ptr.hpp>
#include <boost/scoped_ptr.hpp>
#include <vector>
#include <string>
#include <map>
#include <list>
#include "..\editor\string_conversions.h"
#include "..\editor\parser.h"
#include "track.h"
#include "paramblock.h"
#include "parseutil.h"
#include "particletiming.h"
#include "particlesystem.h"
#include "particlesystemmanager.h"


namespace frozenbyte {
namespace particle {

using namespace frozenbyte::editor;

void GenParticleForce::parseFrom(const editor::ParserGroup& pg) {
	parseParamBlockFrom(pg, *m_pb);
}

void GenParticleForce::parseTo(ParserGroup& pg) {
//	parseParamBlockTo()
}



static ParamDesc genParticleSystemParamDesc[] = {
	ParamDesc(PB_EMIT_RATE, "emit_rate", PARAM_FLOAT, false),
	ParamDesc(PB_EMIT_START_TIME, "emit_start", PARAM_FLOAT, false),
	ParamDesc(PB_EMIT_STOP_TIME, "emit_stop", PARAM_FLOAT, false),
	ParamDesc(PB_MAX_PARTICLES, "max_particles", PARAM_INT, false),
	ParamDesc(PB_DIE_AFTER_EMISSION, "die_after_emission", PARAM_INT, false),
	ParamDesc(PB_VELOCITY_INHERITANCE_FACTOR, "velocity_inheritance_factor", PARAM_FLOAT, false),
	ParamDesc(PB_EMITTER_POSITION, "emitter_position", PARAM_VECTOR, false),
	ParamDesc(PB_LAUNCH_SPEED, "launch_speed", PARAM_FLOAT, false),
	ParamDesc(PB_LAUNCH_SPEED_VAR, "launch_speed_var", PARAM_FLOAT, false),
	ParamDesc(PB_PARTICLE_COLOR, "particle_color", PARAM_VECTOR, true),
	ParamDesc(PB_PARTICLE_ALPHA, "particle_alpha", PARAM_FLOAT, true),
	ParamDesc(PB_PARTICLE_SIZE, "particle_size", PARAM_FLOAT, true),
	ParamDesc(PB_PARTICLE_LIFE, "particle_life", PARAM_FLOAT, false),
	ParamDesc(PB_PARTICLE_LIFE_VAR, "particle_life_var", PARAM_FLOAT, false),	
	ParamDesc(PB_PARTICLE_START_ANGLE, "particle_start_angle", PARAM_FLOAT, false),
	ParamDesc(PB_PARTICLE_START_ANGLE_VAR, "particle_start_angle_var", PARAM_FLOAT, false),
	ParamDesc(PB_PARTICLE_SPIN, "particle_spin", PARAM_FLOAT, false),
	ParamDesc(PB_PARTICLE_SPIN_VAR, "particle_spin_var", PARAM_FLOAT, false),
	ParamDesc(PB_PARTICLE_TEXTURE, "texture", PARAM_STRING, false),
	ParamDesc(PB_PARTICLE_TEXTURE_SUB_DIV_U, "texture_sub_div_u", PARAM_INT, false),
	ParamDesc(PB_PARTICLE_TEXTURE_SUB_DIV_V, "texture_sub_div_v", PARAM_INT, false),
	ParamDesc(PB_PARTICLE_TEXTURE_ALPHA_TYPE, "texture_alpha_type", PARAM_INT, false),
	ParamDesc(PB_PARTICLE_ANIMATION_FRAME_COUNT, "animation_frame_count", PARAM_INT, false),
	ParamDesc(PB_PARTICLE_ANIMATION_START_FRAME, "animation_start_frame", PARAM_FLOAT, false),
	ParamDesc(PB_PARTICLE_ANIMATION_START_FRAME_VAR, "animation_start_frame_var", PARAM_FLOAT, false),
	ParamDesc(PB_PARTICLE_ANIMATION_TYPE, "animation_type", PARAM_STRING, false),
	ParamDesc(PB_PARTICLE_ANIMATION_FPS, "animation_fps", PARAM_FLOAT, false)
};

GenParticleSystem::GenParticleSystem() {

}
	
GenParticleSystem::~GenParticleSystem() {
}

void GenParticleSystem::baseCopy(GenParticleSystem* ps) {
	ps->m_pb = m_pb;
	ps->m_mtl = m_mtl;
	for(int i = 0; i < m_forces.size(); i++)
		ps->m_forces.push_back(m_forces[i]);	
}

const char* GenParticleSystem::getSuperClassName() {
	return "system";
}

const char* GenParticleSystem::getClassName() {
	return "gen_system";
}
	
void GenParticleSystem::setParticlePosition(Vector& v) {

}

void GenParticleSystem::setParticleVelocity(Vector& v, float speed) {

}
	
int GenParticleSystem::getNumParticles() {
	return m_numParts;
}
 		
void GenParticleSystem::setTarget(const Vector& target) {
	m_trackTarget = true;
	m_target = target;
}
	
void GenParticleSystem::setTM(const Matrix& tm) {
	m_tm = tm;
}
	
void GenParticleSystem::setVelocity(const Vector& vel) {
	m_velocity = vel;
}
	
void GenParticleSystem::parseFrom(const ParserGroup& pg) {
	parseParamBlockFrom(pg, *m_pb);
	int nForces;
	nForces = convertFromString<int>(pg.getValue("num_forces", "0"), 0);	
	for(int i = 0; i < nForces; i++) {
		std::string str = "force" + boost::lexical_cast<std::string>(i);
		const ParserGroup& fg = pg.getSubGroup(str);
		std::string className = fg.getValue("class", "");
		if(className.empty())
			continue;
		ParticleForce* force = ParticleSystemManager::getSingleton().createForce(className);
		force->parseFrom(fg);
		boost::shared_ptr<ParticleForce> ptr(force);
		m_forces.push_back(ptr);
	}
}
	
void GenParticleSystem::parseTo(ParserGroup& pg) {
/*
	parseParamBlockTo(pg, *m_pb);
	pg.setValue("num_forces", convertToString<int>((int)m_forces.size()));
	for(int i = 0; i < m_forces.size(); i++) {
		ParserGroup fg;
		fg.setValue("className", m_forces[i]->getClassName());
		m_forces[i]->parseTo(fg);
		std::string str = "force" + boost::lexical_cast<std::string>(i);
		pg.addSubGroup(str, fg);
	}*/
}
	
void GenParticleSystem::kill() {
	m_shutdown = true;
}

bool GenParticleSystem::isDead() {
	return !m_alive;
}

void GenParticleSystem::create() {
	
	boost::shared_ptr<ParamBlock> pb(new ParamBlock());
	m_pb.swap(pb);
	m_pb->addParams(genParticleSystemParamDesc, GEN_PARTICLE_SYSTEM_PARAM_COUNT);
	m_pb->setValue(PB_EMIT_RATE, 10.0f);
	m_pb->setValue(PB_EMIT_START_TIME, 3.0f);
	m_pb->setValue(PB_EMIT_STOP_TIME, 5.0f);
	m_pb->setValue(PB_MAX_PARTICLES, 100);	
	m_pb->setValue(PB_EMITTER_POSITION, Vector(0.0f, 0.0f, 0.0f));	
	m_pb->setValue(PB_LAUNCH_SPEED, 1.0f);
	m_pb->setValue(PB_LAUNCH_SPEED_VAR, 1.0f);
//	m_pb->setTrack(PB_PARTICLE_COLOR, m_col);
	m_pb->setValue(PB_PARTICLE_COLOR, Vector(1.0f, 1.0f, 1.0f), 0.0f);
	m_pb->setValue(PB_PARTICLE_COLOR, Vector(1.0f, 1.0f, 1.0f), 1.0f);
//	m_pb->setTrack(PB_PARTICLE_ALPHA, m_alphaTrack.get());
	m_pb->setValue(PB_PARTICLE_ALPHA, 1.0f, 0.0f);
	m_pb->setValue(PB_PARTICLE_ALPHA, 0.0f, 1.0f);
//	m_pb->setTrack(PB_PARTICLE_SIZE, m_sizeTrack.get());
	m_pb->setValue(PB_PARTICLE_SIZE, 1.0f, 0.0f);
	m_pb->setValue(PB_PARTICLE_SIZE, 1.0f, 1.0f);
	m_pb->setValue(PB_PARTICLE_LIFE, 1.0f);
	m_pb->setValue(PB_PARTICLE_LIFE_VAR, 1.0f);
	m_pb->setValue(PB_PARTICLE_START_ANGLE, 0.0f);
	m_pb->setValue(PB_PARTICLE_START_ANGLE_VAR, 0.0f);
	m_pb->setValue(PB_PARTICLE_SPIN, 0.0f);
	m_pb->setValue(PB_PARTICLE_SPIN_VAR, 0.0f);
	m_pb->setValue(PB_PARTICLE_TEXTURE_SUB_DIV_U, 1);
	m_pb->setValue(PB_PARTICLE_TEXTURE_SUB_DIV_V, 1);
	m_pb->setValue(PB_PARTICLE_TEXTURE_ALPHA_TYPE, (int)IStorm3D_Material::ATYPE_ADD);
	m_pb->setValue(PB_PARTICLE_ANIMATION_FRAME_COUNT, 0);
	m_pb->setValue(PB_PARTICLE_ANIMATION_START_FRAME, 0.f);
	m_pb->setValue(PB_PARTICLE_ANIMATION_START_FRAME_VAR, 0.f);
	m_pb->setValue(PB_PARTICLE_ANIMATION_TYPE, "loop");
	m_pb->setValue(PB_PARTICLE_ANIMATION_FPS, 18.0f);	
	m_pb->setValue(PB_PARTICLE_TEXTURE, "data/particles/flame_anim.jpg");	

}
	
void GenParticleSystem::init(IStorm3D* s3d, IStorm3D_Scene* scene) {

	std::string textureName;
	m_pb->getValue(PB_PARTICLE_TEXTURE, textureName);
	m_mtl = ParticleSystemManager::getSingleton().getMaterial(textureName);

}

void GenParticleSystem::prepareForLaunch(IStorm3D* s3d, IStorm3D_Scene* scene) {

	m_pb->getValue(PB_MAX_PARTICLES, m_maxParticles);
	m_parts.resize(m_maxParticles);
	m_mesh.resize(m_maxParticles);
	for(int i = 0; i < m_parts.size(); i++)
		m_parts[i].alive = false;

	m_pb->getValue(PB_PARTICLE_TEXTURE_SUB_DIV_U, m_animInfo.columns);
	m_pb->getValue(PB_PARTICLE_TEXTURE_SUB_DIV_V, m_animInfo.rows);
	m_pb->getValue(PB_PARTICLE_ANIMATION_FRAME_COUNT, m_animInfo.frames);
	
	m_time = 0;
	m_particleResidue = 0;
	m_numParts = 0;
	m_alive = true;
	m_shutdown = false;


}

	
bool GenParticleSystem::tick(IStorm3D_Scene* scene) {

	float emitStartTime;
	float emitStopTime;
	m_pb->getValue(PB_EMIT_START_TIME, emitStartTime);
	m_pb->getValue(PB_EMIT_STOP_TIME, emitStopTime);
	float emitTime = emitStopTime - emitStartTime;

	// tick inner timer	
	m_time += PARTICLE_TIME_SCALE;
	if(m_time < emitStartTime)
		return true;
	
	if(m_time > emitStopTime) {
		m_shutdown = true;
		if(m_numParts==0) {
			m_alive = false;
			return false;
		}
	}

	int dieAfterEmission;
	int i;
	
	float launchSpeed, launchSpeedVar;
	m_pb->getValue(PB_LAUNCH_SPEED, launchSpeed);
	m_pb->getValue(PB_LAUNCH_SPEED_VAR, launchSpeedVar);
	m_pb->getValue(PB_DIE_AFTER_EMISSION, dieAfterEmission);
	launchSpeed *= PARTICLE_TIME_SCALE;
	launchSpeedVar *= PARTICLE_TIME_SCALE;
	
	float life, lifeVar;
	m_pb->getValue(PB_PARTICLE_LIFE, life);
	m_pb->getValue(PB_PARTICLE_LIFE_VAR, lifeVar);

	float angle, angleVar;
	m_pb->getValue(PB_PARTICLE_START_ANGLE, angle);
	m_pb->getValue(PB_PARTICLE_START_ANGLE_VAR, angleVar);

	float angleSpeed, angleSpeedVar;
	m_pb->getValue(PB_PARTICLE_SPIN, angleSpeed);
	m_pb->getValue(PB_PARTICLE_SPIN_VAR, angleSpeedVar);
	angleSpeed *= PARTICLE_TIME_SCALE;
	angleSpeedVar *= PARTICLE_TIME_SCALE;

	float startFrame, frameVar;
	m_pb->getValue(PB_PARTICLE_ANIMATION_START_FRAME, startFrame);
	m_pb->getValue(PB_PARTICLE_ANIMATION_START_FRAME_VAR, frameVar);

	int animType;
	std::string strAnimType;
	m_pb->getValue(PB_PARTICLE_ANIMATION_TYPE, strAnimType);
	if(strAnimType == "loop")
		animType = 0;
	else
		animType = 1;

	float fps = 0.0f;
	if(animType == 0) {
		m_pb->getValue(PB_PARTICLE_ANIMATION_FPS, fps);
		fps *= PARTICLE_TIME_SCALE;
	}

	Vector emitterPosition;
	m_pb->getValue(PB_EMITTER_POSITION, emitterPosition);

		
	// move and expire particles
	for(i = 0; i < m_parts.size(); i++) {
		Particle& p = m_parts[i];
		if(p.alive) {
			p.age += PARTICLE_TIME_SCALE;
			if(p.age >= p.life) {
				p.alive = false;
				m_numParts--;
			} else {
				p.position += p.velocity;
				p.angle += p.angleSpeed;
				p.frame += p.frameSpeed;
			}
		}
	}
	
	// emit particles
	if(!m_shutdown) {
	
		float t = m_time / emitTime;

		int seed1 = rand() % RAND_MAX;
		int seed2 = rand() % RAND_MAX;
		int seed3 = rand() % RAND_MAX;

		float emitRate;
		m_pb->getValue(PB_EMIT_RATE, emitRate, t);
		emitRate *= PARTICLE_TIME_SCALE;
		float needed = emitRate + m_particleResidue;
		int a = 0;
		float velocityFactor;
		m_pb->getValue(PB_VELOCITY_INHERITANCE_FACTOR, velocityFactor);
		while(needed >= 1.0f) {
			for(a; a < m_parts.size(); a++) {
				Particle& p = m_parts[a];
				if(!p.alive) {
					p.alive = true;
					p.age = 0;
					p.life = life + lifeVar * (float)seed1 / (float)RAND_MAX;
					p.angle = angle + angleVar * (float)seed2 / (float)RAND_MAX;
					p.angleSpeed = angleSpeed + angleSpeedVar * (float)seed2 / (float)RAND_MAX;
					p.frame = startFrame + frameVar * (float)seed3 / (float)RAND_MAX;
					if(animType == 0) {
						p.frameSpeed = fps;
					} else {
						p.frameSpeed = ((float)m_animInfo.frames - p.frame) / p.life;
						p.frameSpeed *= PARTICLE_TIME_SCALE;
					}
					float speed = launchSpeed + launchSpeedVar * (float)seed1 / (float)RAND_MAX;
					setParticlePosition(p.position);
					setParticleVelocity(p.velocity, speed);
					p.position += emitterPosition;
					m_tm.RotateVector(p.velocity);
					m_tm.TransformVector(p.position);
					p.velocity += m_velocity * velocityFactor;
					m_numParts++;
					needed -= 1.0f;
					break;
				}
			}
			if(a >= m_parts.size())
				break;			
		}
		m_particleResidue = needed;
	}
	
	// apply forces
	for(i = 0; i < m_forces.size(); i++) {
		Vector f;
		
		m_forces[i]->preCalculate(m_time);

		for(int j = 0; j < m_parts.size(); j++) {
			if(m_parts[j].alive) {
				m_forces[i]->calcForce(f, m_parts[j].position, m_parts[j].velocity);
				m_parts[j].velocity += f;
			}
		}
	}

	return true;
}

void GenParticleSystem::render(IStorm3D_Scene* scene) {
	
	if(!m_alive)
		return;

	int j = 0;
	
	for(int i = 0; i < m_parts.size(); i++) {
		Particle& p = m_parts[i];
		if(p.alive) {
			Vector color;
			float alpha;
			float size;
			float t = p.age / p.life;
			Storm3D_PointParticle& shape = m_mesh[j++];
			m_pb->getValue(PB_PARTICLE_COLOR, color, t);
			m_pb->getValue(PB_PARTICLE_ALPHA, alpha, t);
			m_pb->getValue(PB_PARTICLE_SIZE, size, t);
			shape.center.position = p.position;
			shape.center.size = size;
			shape.center.color = COL(color.x, color.y, color.z);
			shape.center.alpha = alpha;
			shape.alive = true;
			shape.angle = p.angle;
			shape.frame = p.frame;
		}
	}

	int alphaType;
	m_pb->getValue(PB_PARTICLE_TEXTURE_ALPHA_TYPE, alphaType);

	if(m_mtl)
		m_mtl->SetAlphaType((IStorm3D_Material::ATYPE)alphaType);
	
	if(m_animInfo.frames > 0)
		scene->GetParticleSystem()->RenderParticles(m_mtl, &m_mesh[0], j, &m_animInfo);
	else
		scene->GetParticleSystem()->RenderParticles(m_mtl, &m_mesh[0], j, NULL);

}


} // particle
} // frozenbyte