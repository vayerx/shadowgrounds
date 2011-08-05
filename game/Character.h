
#ifndef CHARACTER_H
#define CHARACTER_H

#include "GameObject.h"
#include "../ui/Visual2D.h"

#define CHAR_SKILL_AIMING 0
#define CHAR_SKILL_STEALTH 1
#define CHAR_SKILL_SPEED 2
#define CHAR_SKILL_TECHNICAL 3
#define CHAR_SKILL_MEDICAL 4
#define CHAR_SKILL_STRENGTH 5
#define CHAR_SKILL_STAMINA 6
#define CHAR_SKILL_MOTIVATION 7
#define CHAR_SKILL_LEADERSHIP 8

#define CHAR_SKILLS_AMOUNT 9

#define CHAR_GENDER_NONE 0
#define CHAR_GENDER_MALE 1
#define CHAR_GENDER_FEMALE 2


namespace game
{

  class Unit;

  /**
   * A game character class.
   * An instance of this class defines a game person that can be hired by
   * the player. Each of human player's units (armors) must be controlled
   * by a character.
   * 
   * @version 1.1, 7.7.2002
   * @author Jukka Kokkonen <jukka@frozenbyte.com>
   * @see Unit
   */

  class Character : public GameObject
  {
  public:
    virtual SaveData *getSaveData() const;

		virtual const char *getStatusInfo() const;

    Character(const char *filename);
    ~Character();

    /**
     * Returns the shortened name of the character ("F. Lastname").
     * @return  char*, short version of the character's name.
     * The returned pointer points to objects internal data, do not
     * delete it.
     */
    char *getName(); 

    /**
     * return the full name of the character ("Firstname Lastname").
     * @return  char*, character's full name.
     * The returned pointer points to objects internal data, do not
     * delete it.
     */
    char *getFullname();

    char *getScript();

    char *getLipsyncId();

    int getSkillAmount(int skill);

    void setSkillAmount(int skill, int amount);

    char *getBio();
    ui::Visual2D *getImage();
    ui::Visual2D *getMessageImage();
    ui::Visual2D *getIconImage();
    Unit *getUnit();

		int getGender();

    static const char *getSkillName(int skill);

		static ui::Visual2D *conversationFace1;
		static ui::Visual2D *conversationFace2;

  private:
    char *name;
    char *fullname;
    char *bio;
    char *script;
    char *lipsyncId;
    ui::Visual2D *image;
    ui::Visual2D *messageImage;
    ui::Visual2D *iconImage;
    Unit *armor;
		int gender;

    int skillValue[CHAR_SKILLS_AMOUNT];

    void setUnit(Unit *unit);

    friend class Unit;
  };

}

#endif

