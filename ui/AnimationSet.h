
#ifndef ANIMATIONSET_H
#define ANIMATIONSET_H

#include "animdefs.h"

// NOTE: currently only claw utilizes transitions extensively, thus no use point in using 
// much memory for these on other projects...
#ifdef PROJECT_CLAW_PROTO
#define MAX_TRANSITIONS 16
#else
#define MAX_TRANSITIONS 4
#endif

class LinkedList;

namespace util {
	class SimpleParser;
}

namespace ui
{
  class IAnimatable;

  // Ilkka addon: Classes for holding transitions between animations
  class Transition
  {
  public:
		Transition()
		:	fileNumber(-1),
			immediate(false),
			looping(false),
			blend(0),
			blendTime(200),
			speedFactor(false),
			noInterpolate(false),
			staticFactor(false),
			staticFactorValue(1.f),
			from(-1)
		{
		}

		// Animation clip properties
		int fileNumber;
		bool immediate;
		bool looping;
		int blend;
		int blendTime;
		bool speedFactor;
		bool noInterpolate;
		bool staticFactor;
		float staticFactorValue;
		// Animations connected by this transition
		int from;
  private:
  };

  class TransitionList
  {
  public:
		TransitionList();
		~TransitionList();

		void addTransition(Transition trans);
		Transition *getTransition(int anim_from);
  private:
  		Transition *item;
		int items;
  };


  // Ilkka addon end

  class AnimationSet
  {
  public:
    AnimationSet(char *name, char *filename);
    ~AnimationSet();

    void parseValue(char * value, int * fileNumber, bool * looping, bool * immediate, int * blend,
	                  bool * speedFactor, bool * staticFactor, float * staticFactorValue, int * blendTime, bool * noInterpolate, util::SimpleParser *parser);
    
		int getAnimationFileNumber(int anim);
    bool isAnimationInSet(int anim);

    bool isAnimationLooping(int anim);
    bool isAnimationImmediate(int anim);
    int getAnimationBlendNumber(int anim);
    bool isAnimationSpeedFactored(int anim);

    bool isAnimationStaticFactored(int anim);
    float getAnimationStaticFactor(int anim);

    bool isAnimationNonInterpolating(int anim);

    void animate(IAnimatable *animatable, int anim);

    static AnimationSet *getSetByName(char *setName);

	  static const char *getAnimName(int anim);

		static int getAnimNumberByName(const char *animName);

  private:
    char *name;
    int *fileNumberTable;
    bool *loopingTable;
    bool *immediateTable;
    int *blendTable;
    bool *speedFactorTable;
    bool *noInterpolateTable;
    bool *staticFactorTable;
    float *staticFactorValueTable;
		int *blendTimeTable;
	//Ilkka addon: a list (array) for holding the transitions into each animation
	  TransitionList *transitionTable;
	//Ilkka addon end

    static LinkedList *setFilenames;
  };
}

#endif


