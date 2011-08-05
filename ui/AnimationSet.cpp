
#include "precompiled.h"

#include "AnimationSet.h"

#include <assert.h>
#include <string.h>

#include "../game/SimpleOptions.h"
#include "../game/options/options_precalc.h"

#include "IAnimatable.h"
#include "Animator.h"
#include "../convert/str2int.h"
#include "../container/LinkedList.h"
#include "../util/SimpleParser.h"
#include "../util/Preprocessor.h"
#include "../util/StringUtil.h"
#include "../system/FileTimestampChecker.h"
#include "../system/Logger.h"


namespace ui
{
  const char *anim_names[ANIM_AMOUNT+1] =
  {
    "none",
    "stand",
    "walk",
    "run",
    "die_back",
    "die_front",
    "die_impact_back",
    "die_impact_front",
    "shoot_left_arm",
    "shoot_right_arm",
    "walk_damaged",
    "run_damaged",
    "stagger_backward",
    "stagger_forward",
    "aim_left_arm",
    "aim_right_arm",
    "idle1",
    "idle2",
    "idle3",
    "idle4",
    "idle5",
    "idle6",
    "idle7",
    "idle8",
    "shoot_heavy_left",
    "shoot_heavy_right",
    "shoot_medium_left",
    "shoot_medium_right",
    "shoot_rapid_left",
    "shoot_rapid_right",
    "shoot_ultrarapid_left",
    "shoot_ultrarapid_right",
    "shoot",
    "crouch",
    "shoot_crouch",
    "prone",
    "shoot_prone",
    "crawl",
    "go_prone",
    "rise_prone",
    "drive",
    "die",
		"sprint",
		"stagger_left",
		"stagger_right",
    "aim_prone",
    "get_up_backdown",
    "get_up_facedown",
    "flip_side_left",
    "flip_side_right",
    "aim_both_arms",
    "torsotwist_left",
    "torsotwist_right",
    "aim",
		"die_prone",
		"strafe_left",
		"strafe_right",
		"run_backward",
		"give",
		"aim_type0",
		"aim_type1",
		"aim_type2",
		"aim_type3",
		"aim_type4",
		"aim_type5",
		"aim_type6",
		"aim_type7",
		"shoot_type0",
		"shoot_type1",
		"shoot_type2",
		"shoot_type3",
		"shoot_type4",
		"shoot_type5",
		"shoot_type6",
		"shoot_type7",
		"stand_type0",
		"stand_type1",
		"stand_type2",
		"stand_type3",
		"stand_type4",
		"stand_type5",
		"stand_type6",
		"stand_type7",
		"walk_type0",
		"walk_type1",
		"walk_type2",
		"walk_type3",
		"walk_type4",
		"walk_type5",
		"walk_type6",
		"walk_type7",
		"run_type0",
		"run_type1",
		"run_type2",
		"run_type3",
		"run_type4",
		"run_type5",
		"run_type6",
		"run_type7",
		"sprint_type0",
		"sprint_type1",
		"sprint_type2",
		"sprint_type3",
		"sprint_type4",
		"sprint_type5",
		"sprint_type6",
		"sprint_type7",
		"run_backward_type0",
		"run_backward_type1",
		"run_backward_type2",
		"run_backward_type3",
		"run_backward_type4",
		"run_backward_type5",
		"run_backward_type6",
		"run_backward_type7",
    "jump",
    "special1",
    "special2",
    "special3",
    "special4",
    "special5",
    "special6",
    "special7",
    "special8",
    "special9",
    "special10",
    "special11",
    "special12",
    "special13",
    "special14",
    "special15",
    "special16",
    "special17",
    "special18",
    "special19",
    "special20",
    "special21",
    "special22",
    "special23",
    "special24",
    "special25",
    "special26",
    "special27",
    "special28",
    "special29",
    "special30",
    "special31",
    "special32",
    "special33",
    "special34",
    "special35",
    "special36",
    "special37",
    "special38",
    "special39",
    "special40",
    "special41",
    "special42",
    "special43",
    "special44",
    "special45",
		"strafe_left_type0",
		"strafe_left_type1",
		"strafe_left_type2",
		"strafe_left_type3",
		"strafe_left_type4",
		"strafe_left_type5",
		"strafe_left_type6",
		"strafe_left_type7",
		"strafe_right_type0",
		"strafe_right_type1",
		"strafe_right_type2",
		"strafe_right_type3",
		"strafe_right_type4",
		"strafe_right_type5",
		"strafe_right_type6",
		"strafe_right_type7",
    "jump_backward",
    "jump_left",
    "jump_right",
		"push_button",
		"electrified",
		"stunned",
		"turn_left",
		"turn_right",
		"turn_left_type0",
		"turn_left_type1",
		"turn_left_type2",
		"turn_left_type3",
		"turn_left_type4",
		"turn_left_type5",
		"turn_left_type6",
		"turn_left_type7",
		"turn_right_type0",
		"turn_right_type1",
		"turn_right_type2",
		"turn_right_type3",
		"turn_right_type4",
		"turn_right_type5",
		"turn_right_type6",
		"turn_right_type7",
		"die_poison",
    "die_back2",
    "die_front2",
    "die_impact_back2",
    "die_impact_front2",
		"shoot_type0_standing",
		"shoot_type1_standing",
		"shoot_type2_standing",
		"shoot_type3_standing",
		"shoot_type4_standing",
		"shoot_type5_standing",
		"shoot_type6_standing",
		"shoot_type7_standing",
		"clip_reload_type0",
		"clip_reload_type1",
		"clip_reload_type2",
		"clip_reload_type3",
		"clip_reload_type4",
		"clip_reload_type5",
		"clip_reload_type6",
		"clip_reload_type7",
		"die_heat",
		"die_heat2",
		"***"
  };

  LinkedList *AnimationSet::setFilenames = NULL;
  
  AnimationSet *AnimationSet::getSetByName(char *setName)
  {
    if (setFilenames == NULL)
    {
			if (anim_names[ANIM_AMOUNT] == NULL 
				|| strcmp(anim_names[ANIM_AMOUNT], "***") != 0)
			{
				assert(!"AnimationSet - Bad amount of animation names.");
				return NULL;
			}

      Logger::getInstance()->debug("AnimationSet::getSetByName - Reading the animation set list.");
      setFilenames = new LinkedList();

      util::SimpleParser parser = util::SimpleParser();
#ifdef LEGACY_FILES
			bool loadok = parser.loadFile("Data/Animations/animation_list.txt");
#else
			bool loadok = parser.loadFile("data/animation/animation_list.txt");
#endif
      if (!loadok)
      {
        Logger::getInstance()->error("AnimationSet::getSetByName - Failed to read the animation set list.");
        return NULL;
      }
      while (parser.next())
      {
        char *key = parser.getKey();
        if (key == NULL)
        {
          parser.error("AnimationSet::getSetByName - Animation set name and filename expected.");
        } else {
          char *value = parser.getValue();
          assert(value != NULL);
          setFilenames->append(new AnimationSet(key, value));
        }
      }
    }

    // TODO, replace with a more efficient structure (hashmap maybe).
    LinkedListIterator iter = LinkedListIterator(setFilenames);
    while (iter.iterateAvailable())
    {
      AnimationSet *tmp = (AnimationSet *)iter.iterateNext();
      if (strcmp(tmp->name, setName) == 0)
      {
        return tmp;
      }
    }

    Logger::getInstance()->error("AnimationSet::getSetByName - No set found by given name.");
    Logger::getInstance()->debug(setName);
    return NULL;
  }


  AnimationSet::AnimationSet(char *name, char *filename)
  {
    assert(name != NULL);

    this->name = new char[strlen(name) + 1];
    strcpy(this->name, name);

    fileNumberTable = new int[ANIM_AMOUNT];
    loopingTable = new bool[ANIM_AMOUNT];
    immediateTable = new bool[ANIM_AMOUNT];
    blendTable = new int[ANIM_AMOUNT];
    speedFactorTable = new bool[ANIM_AMOUNT];
    noInterpolateTable = new bool[ANIM_AMOUNT];
    staticFactorTable = new bool[ANIM_AMOUNT];
    staticFactorValueTable = new float[ANIM_AMOUNT];
		blendTimeTable = new int[ANIM_AMOUNT];
	//Ilkka addon:
    transitionTable = new TransitionList[ANIM_AMOUNT];
    //Ilkka addon end

    for (int i = 0; i < ANIM_AMOUNT; i++)
    {
      fileNumberTable[i] = -1;
      loopingTable[i] = false;
      immediateTable[i] = false;
      blendTable[i] = 0;
      speedFactorTable[i] = false;
      noInterpolateTable[i] = false;
      staticFactorTable[i] = false;
      staticFactorValueTable[i] = 1.0f;
			blendTimeTable[i] = 200;
  	  //Ilkka addon:
			//transitionTable = new TransitionList();
      //Ilkka addon end
    }

    // load the set from file

		std::string pp_filename = filename;
		if (pp_filename.length() > 4
			&& pp_filename.substr(pp_filename.length() - 4, 4) == ".ast")
		{
			pp_filename = pp_filename.substr(0, pp_filename.length() - 4) + ".past";

			bool upToDate = FileTimestampChecker::isFileNewerOrSameThanFile(pp_filename.c_str(), filename);
			bool ppWarning = false;

			if (!game::SimpleOptions::getBool(DH_OPT_B_AUTO_AST_PREPROCESS))
			{
				if (!upToDate)
				{
					ppWarning = true;
				}
				upToDate = true;
			}

			if (game::SimpleOptions::getBool(DH_OPT_B_FORCE_AST_PREPROCESS))
			{
				Logger::getInstance()->warning("AnimationSet - Preprocessed animation set is recreated (force preprocess on).");
				Logger::getInstance()->debug(filename);
				upToDate = false;
			}

			if (!upToDate)
			{
				if (ppWarning)
				{
					Logger::getInstance()->warning("AnimationSet - Preprocessed animation set is not up to date, but it will not be recreated (auto preprocess off).");
					Logger::getInstance()->debug(filename);
				}
			}

			if (!upToDate)
			{
				const char *preproscheck = game::SimpleOptions::getString(DH_OPT_S_AST_PREPROCESSOR_CHECK);
				const char *prepros = game::SimpleOptions::getString(DH_OPT_S_AST_PREPROCESSOR);
				util::Preprocessor::preprocess(preproscheck, prepros, filename, pp_filename.c_str());
			}
		}

    util::SimpleParser parser = util::SimpleParser();
    bool loadok = parser.loadFile(pp_filename.c_str());
    if (!loadok)
    {
      Logger::getInstance()->error("AnimationSet - Failed to read the animation set.");
    } else {
      while (parser.next())
      {
        char *key = parser.getKey();
        if (key == NULL)
        {
          parser.error("AnimationSet - Anim name and number expected.");
        } else {
          char *value = parser.getValue();
          assert(value != NULL);
          bool keyok = false;
					// Ilkka addon: If the key begins with "transition:", handle it as such. Somebody better check this one...
					// (Checked and modified. ;) --jpk
					if (strncmp(key, "transition:", 11) == 0) 
					{
							Transition trans = Transition();
							// these would do a buffer overflow to strings over 64 chars (since no check for that)
							//char from[64] = { 0 };
							//char to[64] = { 0 };
							std::string from;
							std::string to;
							int pos = strcspn(&key[11], ":");
							int animTo = -1;
							//strncpy(from, &key[11], pos);
							// this will probably crash if no colon char found. (goes 1 char past null terminator)
							//strcpy(to, &(&key[11])[pos+1]);
							from = &key[11];
							to = &key[11];
							from = from.substr(0, pos);
							if (pos < (int)to.length())
							{
								to = to.substr(pos+1, (to.length() - (pos+1)));
							} else {
								to = "";
							}
							from = util::StringRemoveWhitespace(from);
							to = util::StringRemoveWhitespace(to);
							for (int j = 0; j < ANIM_AMOUNT; j++)
							{
								if (strcmp(from.c_str(), anim_names[j]) == 0) {
										trans.from = this->getAnimationFileNumber(j);
										break;
								}
							}							
							for (int j = 0; j < ANIM_AMOUNT; j++)
							{
								if (strcmp(to.c_str(), anim_names[j]) == 0) {
										animTo = j;
										break;
								}
							}
							if ((animTo==-1) || (trans.from==-1)) 
							{
								LOG_ERROR("AnimationSet - Invalid transition definition.");
								LOG_DEBUG(from.c_str());
								LOG_DEBUG(to.c_str());
							} else {
									parseValue(value, &trans.fileNumber, &trans.looping, &trans.immediate, &trans.blend,
										&trans.speedFactor, &trans.staticFactor, &trans.staticFactorValue, &trans.blendTime, &trans.noInterpolate, &parser);
									transitionTable[animTo].addTransition(trans);
							}
							keyok = true;
					} else {
						for (int j = 0; j < ANIM_AMOUNT; j++)
						{
								if (strcmp(key, anim_names[j]) == 0) {
										parseValue(value, &fileNumberTable[j], &loopingTable[j], &immediateTable[j], &blendTable[j],
															 &speedFactorTable[j], &staticFactorTable[j], &staticFactorValueTable[j], &blendTimeTable[j], &noInterpolateTable[j], &parser);
									keyok = true;
									break;
								}
						}
					}
          if (!keyok)
          {
            parser.error("AnimationSet - Unknown anim name.");
						LOG_DEBUG(key);
          }
        }
      }
    }
  }


  AnimationSet::~AnimationSet()
  {
    delete [] fileNumberTable;
    delete [] immediateTable;
    delete [] loopingTable;
    delete [] blendTable;
    delete [] speedFactorTable;
    delete [] noInterpolateTable;
    delete [] staticFactorTable;
    delete [] staticFactorValueTable;
		delete[] blendTimeTable;
		delete [] transitionTable;
  }

	void AnimationSet::parseValue(char * value, int * fileNumber, bool * looping, bool * immediate, int * blend,
	                              bool * speedFactor, bool * staticFactor, float * staticFactorValue, int * blendTime, bool * noInterpolate, util::SimpleParser *parser)
	{
              int numpos = 0;
              *looping = true;
              *immediate = false;
              *blend = 0;
              *speedFactor = false;
              *noInterpolate = false;
              *staticFactor = false;
              *staticFactorValue = 0.0f;
							*blendTime = 200;
              int vallen = strlen(value);
              bool errored = false;
							bool firstColonFound = false;
							bool wasComma = false;
              for (int k = 0; k < vallen; k++)
              {
                if (value[k] == ':')
                {
									if (firstColonFound)
									{
                    parser->error("AnimationSet - Multiple colons encountered.");
									}
                  numpos = k + 1;
									firstColonFound = true;
                } else {
									if (value[k] == ',')
										wasComma = true;
								}
              }
              if (numpos != 0)
              {
                for (int k = 0; k < vallen; k++)
                {
                  if (value[k] == ':')
                  {
                    break;
                  } else {
                    if (strncmp(&value[k], "no_loop", 7) == 0)
                    {
                      *looping = false;
                      k += 7 - 1;
                    }
                    if (strncmp(&value[k], "immediate", 9) == 0)
                    {
                      *immediate = true;
                      k += 9 - 1;
                    }
                    if (strncmp(&value[k], "speedfactor", 11) == 0)
                    {
                      *speedFactor = true;
                      k += 11 - 1;
                    }
                    if (strncmp(&value[k], "no_interpolate", 14) == 0)
                    {
                      *noInterpolate = true;
                      k += 14 - 1;
                    }
                    if (strncmp(&value[k], "blend_", 6) == 0
                      && value[k + 6] >= '1' && value[k + 6] <= '9')
                    {
                      *blend = 1 + (value[k + 6] - '1');
                      k += 7 - 1;
                    }
                    if (strncmp(&value[k], "factor_", 7) == 0
											&& strlen(&value[k]) >= 9
                      && value[k + 7] >= '0' && value[k + 7] <= '9'
											&& value[k + 8] == '.'
                      && value[k + 9] >= '0' && value[k + 9] <= '9')
                    {
											*staticFactor = true;
                      *staticFactorValue = 
												(value[k + 7] - '0') + 0.1f * (value[k + 9] - '0');
											if (value[k + 10] >= '0' && value[k + 10] <= '9')
											{
												*staticFactorValue += 0.01f * (value[k + 10] - '0');
	                      k += 11 - 1;
											} else {
	                      k += 10 - 1;
											}
                    }

                    if (strncmp(&value[k], "blendtime_", 10) == 0
											&& strlen(&value[k]) >= 9
                      && value[k + 10] >= '0' && value[k + 10] <= '9'
											&& value[k + 11] == '.'
                      && value[k + 12] >= '0' && value[k + 12] <= '9')
                    {
                      *blendTime = int(1000.f * (value[k + 10] - '0') + 1000.0f * 0.1f * (value[k + 12] - '0'));
											if (value[k + 13] >= '0' && value[k + 13] <= '9')
											{
												*blendTime += int(1000.f * 0.01f * (value[k + 13] - '0'));
	                      k += 14 - 1;
											} else {
	                      k += 13 - 1;
											}
                    }

                    if (value[k + 1] == ',')
                    {
                      k++;
                    } else {
                      if (value[k + 1] != ':' && !errored)
                      {
                        errored = true;
                        parser->error("AnimationSet - Unexpected flag value.");
                      }
                    }
                  }
                }
              } else {
								if (wasComma)
								{
                  parser->error("AnimationSet - Unexpected comma encountered when no colon, probably an error.");
								}
							}
							if (value[numpos] >= '0' && value[numpos] <= '9')
							{
								*fileNumber = str2int(&value[numpos]);
							} else {
								*fileNumber = Animator::getAnimationNumberByFilename(&value[numpos]);
								if (*fileNumber == -1)
								{
									*fileNumber = Animator::addAnimationFilename(&value[numpos]);
									if (*fileNumber == -1)
									{
										parser->error("AnimationSet - Failed to add animation.");
									}
								}
							}
	}

  int AnimationSet::getAnimationFileNumber(int anim)
  {
    assert(anim >= 0 && anim < ANIM_AMOUNT);
    return fileNumberTable[anim];
  }


  bool AnimationSet::isAnimationInSet(int anim)
  {
    assert(anim >= 0 && anim < ANIM_AMOUNT);
    if (fileNumberTable[anim] != -1)
    {
      return true;
    } else {
      return false;
    }
  }


  bool AnimationSet::isAnimationImmediate(int anim)
  {
    return immediateTable[anim];
  }


  bool AnimationSet::isAnimationLooping(int anim)
  {
    return loopingTable[anim];
  }


  bool AnimationSet::isAnimationSpeedFactored(int anim)
  {
    return speedFactorTable[anim];
  }

  bool AnimationSet::isAnimationStaticFactored(int anim)
	{
    return staticFactorTable[anim];
	}

  float AnimationSet::getAnimationStaticFactor(int anim)
	{
		assert(staticFactorTable[anim]);
    return staticFactorValueTable[anim];
	}
  
  bool AnimationSet::isAnimationNonInterpolating(int anim)
  {
    return noInterpolateTable[anim];
  }

  int AnimationSet::getAnimationBlendNumber(int anim)
  {
    return blendTable[anim];

  }

  void AnimationSet::animate(IAnimatable *animatable, int anim)
  {
    if (isAnimationInSet(anim))
    {
			int from = animatable->getAnimation();
			int to = anim;
			Transition *transition = transitionTable[to].getTransition(from);
			int blendTime = blendTimeTable[to];
			if(transition)
				blendTime = transition->blendTime;

			int blend = getAnimationBlendNumber(anim);
      if (blend == 0)
      {
        Animator::setAnimation(animatable,
					transition ? transition->fileNumber : -1,
          getAnimationFileNumber(anim), 
          !isAnimationImmediate(anim), 
					blendTime,
          isAnimationLooping(anim),
					isAnimationNonInterpolating(anim));
      } else {
        Animator::setBlendAnimation(animatable, blend - 1, 
					transition ? transition->fileNumber : -1,
          getAnimationFileNumber(anim), 
          !isAnimationImmediate(anim),
					blendTime,
          isAnimationLooping(anim),
					isAnimationNonInterpolating(anim));
      }
    }
  }

  int AnimationSet::getAnimNumberByName(const char *animName)
  {
    assert(animName != NULL);
    for (int i = 0; i < ANIM_AMOUNT; i++)
    {
      if (strcmp(anim_names[i], animName) == 0)
      {
        return i;
      }
    }
    return -1;
	}
 
  const char *AnimationSet::getAnimName(int anim)
  {
		assert(anim >= 0 && anim < ANIM_AMOUNT);
		return anim_names[anim];
	}

  TransitionList::TransitionList()
  {
	item = new Transition[MAX_TRANSITIONS];
	items = 0;
  }

  TransitionList::~TransitionList()
  {
	delete [] item;
  }

  void TransitionList::addTransition(Transition trans)
  {
	  if (items==MAX_TRANSITIONS) {
		  Logger::getInstance()->error("AnimationSet - Tried to add more transitions than supported.");
		  return;
	  }
	  item[items++]=trans;
  }

  Transition *TransitionList::getTransition(int anim_from)
  {
	  for (int i=0;i<items; i++)
	  {
		  if(item[i].from==anim_from) {
			  return &item[i];
		  }
	  }

	  return 0;
  }

 
  //void AnimationSet::stopBlendedAnimation()

}


