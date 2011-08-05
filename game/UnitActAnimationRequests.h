
#ifndef UNITACTANIMATIONREQUESTS_H
#define UNITACTANIMATIONREQUESTS_H

#define NO_IDLE_ANIMATION_REQUEST 0

#define NO_SPECIAL_ANIMATION_REQUEST 0
#define SPECIAL_ANIMATION_PRONE_UP 1
#define SPECIAL_ANIMATION_PRONE_DOWN 2
#define SPECIAL_ANIMATION_FLIP_SIDE 3
#define SPECIAL_ANIMATION_RISE_UP 4
#define SPECIAL_ANIMATION_ELECTRIFIED 5
#define SPECIAL_ANIMATION_STUNNED 6

#define NO_AIM_ANIMATION_REQUEST 0
#define AIM_ANIMATION_LEFT 1
#define AIM_ANIMATION_RIGHT 2
#define AIM_ANIMATION_BOTH 3
#define AIM_ANIMATION_OTHER 4


namespace game
{
	/**
	 * Latches the animations for UnitActors.
	 *
	 * We don't want to actually change the animation every time the
	 * unit is acting a certain thing, as there may be many other acting
	 * things going on, which may want another animation.
	 *
   * In other word, suppose a unit wishes to walk forward and therefore
	 * set the walk animation, but the unit happens to be dying at the same
	 * time (which therefore requires dying animation). So we cannot just 
	 * set the walk animation and ignore the other things going on, we 
	 * need to mark which animations are requested at that time and 
	 * use the most significant one(s). Which, in this case would be the 
	 * dying animation. The walk animation may not be set.
	 *
   * @author Jukka Kokkonen <jukka@frozenbyte.com>
   * @version 1.0, 9.1.2003 
   * @see UnitActor
   * @see ArmorUnitActor
   * @see AnimationSet
   **/

	class UnitActAnimationRequests
	{
		public:
			bool setStandAnim;
			bool setMoveAnim;
			bool setShootAnim;
			bool endShootAnim;
			int setAimAnim;
			bool endAimAnim;
			bool endTwistAnim;
			int setIdleAnim;
			int setSpecialAnim;
			bool turnLeft;
			bool turnRight;


			UnitActAnimationRequests()
			{
				setStandAnim = false;
				setMoveAnim = false;
				setShootAnim = false;
				setAimAnim = NO_AIM_ANIMATION_REQUEST;
				endShootAnim = false;
				endAimAnim = false;
				endTwistAnim = false;
				setIdleAnim = NO_IDLE_ANIMATION_REQUEST;
				setSpecialAnim = NO_SPECIAL_ANIMATION_REQUEST;
				turnLeft = false;
				turnRight = false;
			}
	};
}

#endif


