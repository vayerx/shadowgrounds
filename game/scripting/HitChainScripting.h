
#ifndef HITCHAINSCRIPTING_H
#define HITCHAINSCRIPTING_H

namespace util
{
	class ScriptProcess;
}

#include <DatatypeDef.h>

namespace game
{
	class Game;
	class GameScriptData;
	class Unit;
	class Projectile;
	class Bullet;

	extern VC3 projs_originPosition;
	extern VC3 projs_originalPosition;
	extern VC3 projs_chainedPosition;

	extern VC3 projs_originDirection;
	extern VC3 projs_originalDirection;
	extern VC3 projs_chainedDirection;

	extern VC3 projs_hitPlaneNormal;

	extern float projs_chainedRange;
	extern float projs_chainedVelocityFactor;

	extern float projs_skipRaytraceDistance;

	extern int projs_hitChain;

	extern bool projs_chainedRotationRandom;
	extern bool projs_chainedOriginToHitUnit;
	
	extern int projs_chainedCustomValue;
	extern int projs_lifeTime;

	extern Unit *projs_hitUnit;
	extern Unit *projs_shooter;

  extern Bullet *projs_chainedBulletType;
  extern Bullet *projs_originBulletType;


	/** 
	 * Hit chain script commands
	 */
	class HitChainScripting
	{
		public:			
			/** 
			 * Just processes one command...
			 */
			static void process(util::ScriptProcess *sp, 
				int command, floatint intFloat, char *stringData, ScriptLastValueType *lastValue, 
				GameScriptData *gsd, Game *game);
	};
}

#endif



