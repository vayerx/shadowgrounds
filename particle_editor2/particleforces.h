// Copyright 2002-2004 Frozenbyte Ltd.

#ifndef PARTICLE_FORCES_H
#define PARTICLE_FORCES_H

namespace frozenbyte
{
namespace particle
{

	class DragParticleForce : public IParticleForce {
	  float m_factor;
  public:
	  static int getType();

	  void setFactor(float f);
	  float getFactor();
	  void preCalc(float t);
	  void calcForce(Vector& force, const Vector& pos, const Vector& vel);
	  void parseFrom(const editor::ParserGroup& pg);
	  int getTypeId() const;
  };

  class GravityParticleForce : public IParticleForce {
	  float m_gravity;
  public:
	  static int getType();

	  void setGravity(float f);
	  float getGravity();
	  void preCalc(float t);
	  void calcForce(Vector& force, const Vector& pos, const Vector& vel);
	  void parseFrom(const editor::ParserGroup& pg);
	  int getTypeId() const;
  };

  class SideGravityParticleForce : public IParticleForce {
	  float m_sideGravity;
  public:
	  static int getType();

	  void setGravity(float f);
	  float getGravity();
	  void preCalc(float t);
	  void calcForce(Vector& force, const Vector& pos, const Vector& vel);
	  void parseFrom(const editor::ParserGroup& pg);
	  int getTypeId() const;
  };

  class WindParticleForce : public IParticleForce {
	  float m_wind_effect_factor;
	  float m_spiral_amount;
	  float m_spiral_speed;

		static float wind_timer;
		static float global_wind_angle;
		static float global_wind_velocity;
    /*
    static VC3 global_wind_sink_position;
    static VC3 global_wind_source_position;
    static VC3 global_wind_vortex_position;
    static float global_wind_sink_speed;
    static float global_wind_source_speed;
    static float global_wind_vortex_speed;
    static float global_wind_sink_attenuation;
    static float global_wind_source_attenuation;
    static float global_wind_vortex_attenuation;
    */

  public:
    static void setGlobalWindAngle(float angleDegrees);
    static void setGlobalWindVelocity(float velocity);
    static void advanceWind(float seconds);
	static int getType();

	  void setWindEffectFactor(float f);
	  float getWindEffectFactor();
	  void setSpiralAmount(float f);
	  float getSpiralAmount();
	  void setSpiralSpeed(float f);
	  float getSpiralSpeed();
	  void preCalc(float t);
	  void calcForce(Vector& force, const Vector& pos, const Vector& vel);
	  void parseFrom(const editor::ParserGroup& pg);

	  int getTypeId() const;
  };

} // particle
} // frozenbyte

#endif
