
#include "precompiled.h"

#include <string.h>

#include "MissionParser.h"

#include "../convert/str2int.h"
#include "../system/Logger.h"
#include "gamedefs.h"
#include "Unit.h"
#include "Part.h"
#include "PartList.h"
#include "PartType.h"
#include "PartTypeAvailabilityList.h"
#include "Game.h"
#include "Building.h"
#include "BuildingList.h"
#include "Unit.h"
#include "UnitList.h"
#include "UnitType.h"
#include "Character.h"
#include "scripting/GameScripting.h"

#include "../filesystem/input_stream_wrapper.h"
#include "../util/Debug_MemoryManager.h"


#define DEFAULT_UNIT_TYPE_NAME "Armor"
using namespace frozenbyte;

namespace game
{
  const char *mp_currentfile = "";

  MissionParser::MissionParser()
  {
    // nop
  }

  MissionParser::~MissionParser()
  {
    // nop
  }

  void MissionParser::error(const char *err, int linenum, bool isError)
  {
    char *buf = new char[strlen(err) + 1 + 60 + strlen(mp_currentfile)];
    strcpy(buf, err);
    strcat(buf, " (file ");
    strcat(buf, mp_currentfile);
    strcat(buf, ", line ");
    strcat(buf, int2str(linenum));
    strcat(buf, ")");
    if (isError)
      (Logger::getInstance())->error(buf);
    else
      (Logger::getInstance())->warning(buf);
    delete [] buf;
  }

  void MissionParser::parseMission(Game *game, char *filename, int section)
  {
		if (filename == NULL)
			return;

    mp_currentfile = filename;

		if (section == MISSIONPARSER_SECTION_BEFORE)
		{
			game->setMissionId(NULL);
		}

    filesystem::FB_FILE *f = filesystem::fb_fopen(filename, "rb");

    if (f == NULL)
    {
      error("Could not open mission file.", 0);
      return;
    }

    //fseek(f, 0, SEEK_END);
    //int flen = ftell(f);
    //fseek(f, 0, SEEK_SET);
    int flen = filesystem::fb_fsize(f);

    char *buf = new char[flen + 1];

    int datalen = filesystem::fb_fread(buf, sizeof(char), flen, f);
    buf[datalen] = '\0';

    int lineNumber = 1;
    int lastpos = 0;
    int i;
    int atSection = 0;
    bool okSection = false;
    for (i = 0; i < datalen; i++)
    {
      if (buf[i] == '\r' || buf[i] == '\n')
      {
        // TODO: linenumbering goes wrong if carriage return missing 
        // so if non-windows text format, it's real line number + 1 
        if (buf[i] == '\n') lineNumber++;

        buf[i] = '\0';

        // remove trailing spaces
        for (int j = i - 1; j >= lastpos; j--)
        {
          if (buf[j] == ' ') 
            buf[j] = '\0';
          else 
            break;
        }

        // process if not empty and not start with comments
        if (buf[lastpos] != '\0' 
          && strncmp(&buf[lastpos], "//", 2) != 0)
        {
          int tokenSep = -1;
          for (int k = lastpos; k < i; k++)
          {
            if (buf[k] == ' ')
            {
              buf[k] = '\0';
              tokenSep = k;
              break;
            }
          }
          char *cmd = &buf[lastpos];
          char *data = NULL;
          if (tokenSep != -1) 
            data = &buf[tokenSep + 1];

          bool lineok = false;

          if (strcmp(cmd, "section") == 0)
          {
            if (strcmp(data, "before") == 0)
            {
              atSection = MISSIONPARSER_SECTION_BEFORE;
            }
            if (strcmp(data, "combat") == 0)
            {
              atSection = MISSIONPARSER_SECTION_COMBAT;
            }
            if (strcmp(data, "after") == 0)
            {
              atSection = MISSIONPARSER_SECTION_AFTER;
            }
            if (section == atSection)
              okSection = true;
            else 
              okSection = false;
            lineok = true;
          }

          if (strcmp(cmd, "includeScript") == 0)
          {
            if (data != NULL)
            { 
							if (data[0] == '\"')
							{
								char *stringedData = new char[strlen(&data[1]) + 1];
								strcpy(stringedData, &data[1]);
								for (int stfix = strlen(stringedData) - 1; stfix >= 0; stfix--)
								{
									if (stringedData[stfix] == '\"')
									{
										stringedData[stfix] = '\0';
										break;
									}
								}
	              game->gameScripting->loadScripts(stringedData, filename);
								delete [] stringedData;
							} else {
	              game->gameScripting->loadScripts(data, filename);
							}
            } else {
              error("Missing script filename.", lineNumber);
            }
            lineok = true;
          }

          if (okSection)
          {
/*
            if (strcmp(cmd, "setPlayer") == 0)
            {
              int val = str2int(data);
              if (val >= 0 && val < ABS_MAX_PLAYERS)
              {
                player = val;
              } else {
                error("Player number out of range.", lineNumber);
              }
              lineok = true;
            }
*/
/*
            if (strcmp(cmd, "addMoney") == 0)
            {
              int val = str2int(data);
              game->money[player] += val;
              lineok = true;
            }
            if (strcmp(cmd, "setFriendly") == 0)
            {
              int val = str2int(data);
              if (val >= 0 && val < ABS_MAX_PLAYERS)
              {
                game->hostile[player][val] = false;
              } else {
                error("Player number out of range.", lineNumber);
              }
              lineok = true;
            }
            if (strcmp(cmd, "setHostile") == 0)
            {
              int val = str2int(data);
              if (val >= 0 && val < ABS_MAX_PLAYERS)
              {
                game->hostile[player][val] = true;
              } else {
                error("Player number out of range.", lineNumber);
              }
              lineok = true;
            }
            if (strcmp(cmd, "setSpawn") == 0)
            {
              if (strlen(data) < 16)
              {
                char splitbuf[16];
                strcpy(splitbuf, data);
                int spawnx = 0;
                int spawny = 0;
                int buflen = strlen(splitbuf);
                for (int splitp = 0; splitp < buflen; splitp++)
                {
                  if (splitbuf[splitp] == ',')
                  {
                    splitbuf[splitp] = '\0';
                    spawnx = str2int(splitbuf);
                    spawny = str2int(&splitbuf[splitp + 1]);
                    break;
                  }
                }
                if (spawnx <= 0 || spawny <= 0)
                {
                  error("Bad spawn coordinates.", lineNumber);
                } else {
                  game->spawnX[player] = spawnx;
                  game->spawnY[player] = spawny;
                }
              } else {
                error("setSpawn parameter format bad.", lineNumber);
              }
              lineok = true;
            }
*/
/*
            if (strcmp(cmd, "setCoordinates") == 0)
            {
              if (data != NULL)
              {
                if (strlen(data) < 16)
                {
                  char splitbuf[16];
                  strcpy(splitbuf, data);
                  int cx = 0;
                  int cy = 0;
                  int buflen = strlen(splitbuf);
                  for (int splitp = 0; splitp < buflen; splitp++)
                  {
                    if (splitbuf[splitp] == ',')
                    {
                      splitbuf[splitp] = '\0';
                      cx = str2int(splitbuf);
                      cy = str2int(&splitbuf[splitp + 1]);
                      break;
                    }
                  }
                  if (cx <= 0 || cy <= 0)
                  {
                    error("Bad coordinates.", lineNumber);
                  } else {
                    coordX = cx;
                    coordY = cy;
                  }
                } else {
                  error("setCoordinates parameter format bad.", lineNumber);
                }
              } else {
                error("setCoordinates parameter missing.", lineNumber);
              }
              lineok = true;
            }
						*/

						/*
            if (strcmp(cmd, "addBuilding") == 0)
            {
              Building *building = new Building(data, coordX, coordY);
              game->buildings->addBuilding(building);
              lineok = true;
            }
						*/

            if (strcmp(cmd, "setMapConfig") == 0)
            {
              if (game->currentMap != NULL)
              {
                delete [] game->currentMap;
                game->currentMap = NULL;
              }
              if (data != NULL)
              {
                game->currentMap = new char[strlen(data) + 1];
                strcpy(game->currentMap, data);
              } else {
                error("setMapConfig parameter expected.", lineNumber);
              }
              lineok = true;
            }

            if (strcmp(cmd, "setMissionScript") == 0)
            {
              game->setMissionScript(data);
              lineok = true;
            }

            if (strcmp(cmd, "setMissionId") == 0)
            {
              game->setMissionId(data);
              lineok = true;
            }

            if (strcmp(cmd, "setSectionScript") == 0)
            {
              game->setSectionScript(data);
              lineok = true;
            }

            if (strcmp(cmd, "setBuildingsScript") == 0)
            {
              game->setBuildingsScript(data);
              lineok = true;
            }

            if (strcmp(cmd, "setFailureMission") == 0)
            {
              game->setFailureMission(data);
              lineok = true;
            }

            if (strcmp(cmd, "setSuccessMission") == 0)
            {
              game->setSuccessMission(data);
              lineok = true;
            }

/*
            if (strcmp(cmd, "allowPartType") == 0)
            {
              if (!PARTTYPE_ID_STRING_VALID(data))
              {
                if (data == NULL)
                  error("Expected part type id.", lineNumber);
                else
                  error("Illegal part type id.", lineNumber);
              } else {
                PartType *pt = getPartTypeById(PARTTYPE_ID_STRING_TO_INT(data));
                if (pt == NULL) 
                { 
                  error("Reference to unloaded part type.", lineNumber);
                } else {
                  if (game->partTypesAvailable->isPartTypeAvailable(player, pt))
                  {
                    error("Part type is already available to player.", lineNumber, false);
                  } else {
                    game->partTypesAvailable->addPartType(player, pt);
                  }
                }
              }
              lineok = true;
            }
*/
/*
            if (strcmp(cmd, "addCharacter") == 0)
            {
              UnitType *ut = getUnitTypeByName(DEFAULT_UNIT_TYPE_NAME);
              if (ut == NULL)
              {
                error("Default unit type does not exist.", lineNumber);
              } else {
                unit = ut->getNewUnitInstance(player);
                if (unit == NULL)
                {
                  error("Internal error while creating unit.", lineNumber);
                } else {
                  game->units->addUnit(unit);

                  Character *ch = new Character(data);
                  unit->setCharacter(ch);
                }
              }
              lineok = true;
            }
*/
/*
            if (strcmp(cmd, "addUnit") == 0)
            {
              if (data == NULL)
              {
                error("Expected unit type name.", lineNumber);
              } else {
                UnitType *ut = getUnitTypeByName(data);
                if (ut == NULL)
                {
                  error("Reference to unknown unit type.", lineNumber);
                } else {
                  unit = ut->getNewUnitInstance(player);
                  if (unit == NULL)
                  {
                    error("Internal error while creating unit.", lineNumber);
                  } else {
                    game->units->addUnit(unit);
                    if (script != NULL)
                    {
                      unit->setScript(script);
                    }
                  }
                }
              }
              lineok = true;
            }
*/
/*
            if (strcmp(cmd, "addStoragePart") == 0)
            {
              if (!PARTTYPE_ID_STRING_VALID(data))
              {
                if (data == NULL)
                  error("Expected part type id.", lineNumber);
                else
                  error("Illegal part type id.", lineNumber);
              } else {
                PartType *pt = getPartTypeById(PARTTYPE_ID_STRING_TO_INT(data));
                if (pt == NULL) 
                { 
                  error("Reference to unloaded part type.", lineNumber);
                } else {
                  part = pt->getNewPartInstance();
                  part->setOwner(player);
                  game->parts->addPart(part);
                  partInUnit = false;
                }
              }
              lineok = true;
            }
*/
/*
            if (strcmp(cmd, "addUnitRootPart") == 0)
            {
              if (!PARTTYPE_ID_STRING_VALID(data))
              {
                if (data == NULL)
                  error("Expected part type id.", lineNumber);
                else
                  error("Illegal part type id.", lineNumber);
              } else {
                PartType *pt = getPartTypeById(PARTTYPE_ID_STRING_TO_INT(data));
                if (pt == NULL) 
                { 
                  error("Reference to unloaded part type.", lineNumber);
                } else {
                  if (unit == NULL)
                  {
                    error("Attempted to add root part to undefined unit.", lineNumber);
                  } else {
                    if (unit->getRootPart() != NULL)
                      game->detachParts(unit, unit->getRootPart());
                    part = pt->getNewPartInstance();
                    part->setOwner(player);
                    unit->setRootPart(part);
                    partInUnit = true;
                  }
                }
              }
              lineok = true;
            }
*/
/*
            if (strcmp(cmd, "setUnitCoordinates") == 0)
            {
              if (data != NULL)
              {
                if (strlen(data) < 16)
                {
                  char splitbuf[16];
                  strcpy(splitbuf, data);
                  int cx = 0;
                  int cy = 0;
                  int buflen = strlen(splitbuf);
                  for (int splitp = 0; splitp < buflen; splitp++)
                  {
                    if (splitbuf[splitp] == ',')
                    {
                      splitbuf[splitp] = '\0';
                      cx = str2int(splitbuf);
                      cy = str2int(&splitbuf[splitp + 1]);
                      break;
                    }
                  }
                  if (cx <= 0 || cy <= 0)
                  {
                    error("Bad coordinates.", lineNumber);
                  } else {
                    if (unit == NULL)
                    {
                      error("Attempted to set coordinates for undefined unit.", lineNumber);
                    } else {
                      VC3 spawn = VC3((float)cx, 0, (float)cy);
                      unit->setSpawnCoordinates(spawn);
                    }
                  }
                } else {
                  error("setUnitCoordinates parameter format bad.", lineNumber);
                }
              } else {
                error("setUnitCoordinates parameter missing.", lineNumber);
              }
              lineok = true;
            }
*/
/*
            if (strcmp(cmd, "addSubPart") == 0)
            {
              if (!PARTTYPE_ID_STRING_VALID(data))
              {
                if (data == NULL)
                  error("Expected part type id.", lineNumber);
                else
                  error("Illegal part type id.", lineNumber);
              } else {
                PartType *pt = getPartTypeById(PARTTYPE_ID_STRING_TO_INT(data));
                if (pt == NULL) 
                { 
                  error("Reference to unloaded part type.", lineNumber);
                } else {
                  if (!partInUnit || part == NULL)
                  {
                    error("Attempted to add sub-part to undefined unit part.", lineNumber);
                  } else {
                    Part *parentPart = part;
                    int slotamount = parentPart->getType()->getSlotAmount();
                    int slot;
                    for (slot = 0; slot < slotamount; slot++)
                    {
                      if (parentPart->getSubPart(slot) == NULL 
                        && pt->isInherited(parentPart->getType()->getSlotType(slot)))
                      break;
                    }
                    if (slot < slotamount)
                    {
                      part = pt->getNewPartInstance();
                      part->setOwner(player);
                      parentPart->setSubPart(slot, part);
                      // done by setSubPart
                      //part->setParent(parentPart);
                    } else {
                      error("Added sub-part does not fit unit part.", lineNumber);
                    }
                  }
                }
              }
              lineok = true;
            }
*/
/*
            if (strcmp(cmd, "toParentPart") == 0)
            {
              if (part == NULL)
              {
                error("Attempted to select parent for undefined part.", lineNumber);
              } else {
                if (!partInUnit)
                {
                  error("Attempted to select parent for storage part.", lineNumber);
                } else {
                  part = part->getParent();
                }
              }
              lineok = true; 
            }
*/
/*
            if (strcmp(cmd, "setScript") == 0)
            {
              if (script != NULL)
              {
                delete [] script;
              }
              script = NULL;
              if (data != NULL
                && strcmp(data, "") != 0)
              {
                script = new char[strlen(data) + 1];
                strcpy(script, data);
              }
              lineok = true;
            }
*/

          }

          // TODO: lots of commands...

          if (!lineok && okSection)
          {
            error("Unknown command.", lineNumber, false);
          }
        }

        // skip leading spaces for next entry
        while (buf[i + 1] == ' ') 
        { 
          i++; 
        }
        lastpos = i + 1;
      }
    }

    delete [] buf;

    filesystem::fb_fclose(f);

		if (game->getMissionId() == NULL)
		{
			Logger::getInstance()->error("MissionParser::parseMission - Mission id missing from mission file.");
		}

    return;
  }

}

