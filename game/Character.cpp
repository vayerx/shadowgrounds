
#include "precompiled.h"

#include <string.h>
#include <assert.h>

#include "Character.h"

#include "../system/Logger.h"
#include "../convert/str2int.h"

#include "../util/Debug_MemoryManager.h"
#include "../filesystem/input_stream_wrapper.h"

using namespace frozenbyte;

namespace {

	struct Tracker
	{
		Tracker()
		{
		}

		~Tracker()
		{
			delete game::Character::conversationFace1;
			delete game::Character::conversationFace2;
			game::Character::conversationFace1 = 0;
			game::Character::conversationFace2 = 0;
		}
	};

	Tracker tracker;
}

namespace game
{
  const char *charSkillNames[CHAR_SKILLS_AMOUNT] =
  { 
    "aiming",
    "stealth",
    "speed",
    "technical",
    "medical",
    "strength",
    "stamina",
    "motivation",
    "leadership"
  };

  const char *Character::getSkillName(int skill)
  {
    assert(skill >= 0 && skill < CHAR_SKILLS_AMOUNT);
    return charSkillNames[skill];
  }

	ui::Visual2D *Character::conversationFace1 = NULL;
	ui::Visual2D *Character::conversationFace2 = NULL;
	// TODO: clean up the above conversationFace visuals on uninit


  SaveData *Character::getSaveData() const
  {
    // TODO
    return NULL;
  }
	
	const char *Character::getStatusInfo() const
	{
		return "Character";
	}

  Character::Character(const char *filename)
  {
    name = NULL;
    fullname = NULL;
    bio = NULL;
    image = NULL;
    messageImage = NULL;
		iconImage = NULL;
    armor = NULL;
    script = NULL;
    lipsyncId = NULL;
		gender = CHAR_GENDER_NONE;

    for (int ski = 0; ski < CHAR_SKILLS_AMOUNT; ski++)
    {
      skillValue[ski] = 0;
    }

	filesystem::FB_FILE *f = filesystem::fb_fopen(filename, "rb");
    if (f == NULL)
    {
      Logger::getInstance()->error("Character - Could not open file.");
      Logger::getInstance()->debug(filename);
      return;
    }

	/*
    fseek(f, 0, SEEK_END);
    int flen = ftell(f);
    fseek(f, 0, SEEK_SET);
	*/
	int flen = filesystem::fb_fsize(f);

    char *buf = new char[flen + 1];
    int got = filesystem::fb_fread(buf, sizeof(char), flen, f);
    buf[got] = '\0';

    int lastpos = 0;
    for (int i = 0; i < got; i++)
    {
      if (buf[i] == '\r') buf[i] = '\0';
      if (buf[i] == '\n') 
      {
        buf[i] = '\0';

        // remove ending quote
        // wo/ cr
        if (i >= 1 && buf[i - 1] == '\"')
        {
          buf[i - 1] = '\0';
        } else {
          // w/ cr
          if (i >= 2 && buf[i - 1] == '\0' && buf[i - 2] == '\"')
          {
            buf[i - 2] = '\0';
          }
        }

        bool lineok = false;

        if (buf[lastpos] == '\0'
          || strncmp(&buf[lastpos], "//", 2) == 0)
        {
          // empty line or comment
        } else {
          if (strncmp(&buf[lastpos], "name=\"", 6) == 0)
          {
            if (name != NULL) delete [] name;
            name = new char[strlen(&buf[lastpos + 6]) + 1];
            strcpy(name, &buf[lastpos + 6]);
            lineok = true;
          }
          if (strncmp(&buf[lastpos], "script=\"", 8) == 0)
          {
            if (script != NULL) delete [] script;
            script = new char[strlen(&buf[lastpos + 8]) + 1];
            strcpy(script, &buf[lastpos + 8]);
            lineok = true;
          }
          if (strncmp(&buf[lastpos], "lipsyncid=\"", 11) == 0)
          {
            if (lipsyncId != NULL) delete [] lipsyncId;
            lipsyncId = new char[strlen(&buf[lastpos + 11]) + 1];
            strcpy(lipsyncId, &buf[lastpos + 11]);
            lineok = true;
          }
          if (strncmp(&buf[lastpos], "fullname=\"", 10) == 0)
          {
            if (fullname != NULL) delete [] fullname;
            fullname = new char[strlen(&buf[lastpos + 10]) + 1];
            strcpy(fullname, &buf[lastpos + 10]);
            lineok = true;
          }
          if (strncmp(&buf[lastpos], "image=\"", 7) == 0)
          {
            image = new ui::Visual2D(&buf[lastpos + 7]);
            lineok = true;
          }
          if (strncmp(&buf[lastpos], "messageimage=\"", 14) == 0)
          {
            messageImage = new ui::Visual2D(&buf[lastpos + 14]);
            lineok = true;
          }
          if (strncmp(&buf[lastpos], "iconimage=\"", 11) == 0)
          {
            iconImage = new ui::Visual2D(&buf[lastpos + 11]);
            lineok = true;
          }
          if (strncmp(&buf[lastpos], "gender=", 7) == 0)
          {
						if (strcmp(&buf[lastpos + 7], "female") == 0)
	            gender = CHAR_GENDER_FEMALE;
						if (strcmp(&buf[lastpos + 7], "male") == 0)
	            gender = CHAR_GENDER_MALE;
						if (strcmp(&buf[lastpos + 7], "none") == 0)
	            gender = CHAR_GENDER_NONE;
            lineok = true;
          }
          if (strncmp(&buf[lastpos], "bio=\"", 5) == 0)
          {
            if (bio != NULL) delete [] bio;
            bio = new char[strlen(&buf[lastpos + 5]) + 1];
            strcpy(bio, &buf[lastpos + 5]);
            lineok = true;
          }
          if (strncmp(&buf[lastpos], "bio+=\"", 6) == 0)
          {
            int biolen = 0;
            if (bio != NULL) biolen = strlen(bio);
            char *newbio = new char[biolen + strlen(&buf[lastpos + 6]) + 1];
            if (bio != NULL) 
            {
              strcpy(newbio, bio);
              delete [] bio;
            }
            strcpy(&newbio[biolen], &buf[lastpos + 6]);
            bio = newbio;
            lineok = true;
          }
          if (!lineok)
          {
            // none of above ones, maybe a skill...
            for (int sk = 0; sk < CHAR_SKILLS_AMOUNT; sk++)
            {
              int sklen = strlen(charSkillNames[sk]);
              if (strncmp(&buf[lastpos], charSkillNames[sk], sklen) == 0
                && strncmp(&buf[lastpos + sklen], "=", 1) == 0)
              {
                int val = str2int(&buf[lastpos + sklen + 1]);
                skillValue[sk] = val;
                lineok = true;
                break;
              }
            }
          }
          if (!lineok)
          {
            Logger::getInstance()->warning("Character - Unknown data key.");
          }
        }
        lastpos = i + 1;
      }
    }

    if (name == NULL || fullname == NULL) 
    {
      Logger::getInstance()->error("Character - No name or fullname defined.");
      assert(0);
    }

    delete [] buf;

    filesystem::fb_fclose(f);
  }

  Character::~Character()
  {
    if (name != NULL) delete [] name;
    if (script != NULL) delete [] script;
    if (fullname != NULL) delete [] fullname;
    if (bio != NULL) delete [] bio;
    if (image != NULL) delete image;
    if (messageImage != NULL) delete messageImage;
    if (iconImage != NULL) delete iconImage;
    if (lipsyncId != NULL) delete[] lipsyncId;
  }

  char *Character::getName()
  {
    return name;
  }

  char *Character::getScript()
  {
    return script;
  }

  char *Character::getLipsyncId()
  {
    return lipsyncId;
  }

  char *Character::getFullname()
  {
    return fullname;
  }

  int Character::getSkillAmount(int skill)
  {
    assert(skill >= 0 && skill < CHAR_SKILLS_AMOUNT);
    return this->skillValue[skill];
  }

  void Character::setSkillAmount(int skill, int amount)
  {
    assert(skill >= 0 && skill < CHAR_SKILLS_AMOUNT);
    this->skillValue[skill] = amount;
  }

  char *Character::getBio()
  {
    return bio;
  }

  ui::Visual2D *Character::getImage()
  {
    return image;
  }

  ui::Visual2D *Character::getMessageImage()
  {
    return messageImage;
  }

  ui::Visual2D *Character::getIconImage()
  {
    return iconImage;
  }

  Unit *Character::getUnit()
  {
    return armor;
  }

  void Character::setUnit(Unit *unit)
  {
    armor = unit;
  }

  int Character::getGender()
  {
    return gender;
  }

}
