
#include "precompiled.h"

#include <stdio.h>

#include "../system/Logger.h"
#include "../convert/str2int.h"

#include "../util/UberCrypt.h"

#include "PartType.h"
#include "PartTypeParser.h"

// for the instance creation quick hack...
#include "Bullet.h"
#include "Weapon.h"
#include "DirectWeapon.h"
#include "IndirectWeapon.h"
#include "Tool.h"
#include "Head.h"
#include "Arm.h"
#include "Leg.h"
#include "Torso.h"
#include "ItemPack.h"
#include "AmmoPack.h"
#include "PowerCell.h"
#include "Reactor.h"

// also, handle units (now this is a unitparser too ;)
#include "UnitType.h"
#include "unittypes.h"

#include "../filesystem/input_stream_wrapper.h"
#include "../util/Debug_MemoryManager.h"

#include <vector>
#include <string>

using namespace frozenbyte;

namespace game
{
	class PartTypeParserImpl
	{
	public:
		const char *ptp_error;
		int ptp_errorline;
		const char *ptp_currentfile;

		static std::vector<std::string> loadedFiles;

		PartTypeParserImpl()
		{
			ptp_error = NULL;
			ptp_errorline = 0;
			ptp_currentfile = "";
		}
	};

	std::vector<std::string> PartTypeParserImpl::loadedFiles;

  // the überparser

	PartTypeParser::PartTypeParser()
	{
		this->impl = new PartTypeParserImpl();
	}

	PartTypeParser::~PartTypeParser()
	{
		delete this->impl;
	}

  void PartTypeParser::error(const char *err, int linenum)
  {
    impl->ptp_error = err; 
    impl->ptp_errorline = linenum;

    char *buf = new char[strlen(err) + 1 + 60 + strlen(impl->ptp_currentfile)];
    strcpy(buf, err);
    strcat(buf, " (file ");
    strcat(buf, impl->ptp_currentfile);
    strcat(buf, ", line ");
    strcat(buf, int2str(linenum));
    strcat(buf, ")");
    (Logger::getInstance())->error(buf);
    delete [] buf;
  }

  void PartTypeParser::clearLoadedList(const char *fileExtension)
  {
		// FIXME: clear only the filenames that match given file extension!!!
		PartTypeParserImpl::loadedFiles.clear();
	}

  void PartTypeParser::loadPartTypes(const char *filename)
  {
		for (int loadc = 0; loadc < (int)impl->loadedFiles.size(); loadc++)
		{
			if (impl->loadedFiles[loadc] == filename)
			{
				Logger::getInstance()->debug("PartTypeParser::loadPartTypes - Already loaded file, so skipping it.");
				Logger::getInstance()->debug(filename);
				return;
			}
		}
		std::string tmp = std::string(filename);
		impl->loadedFiles.push_back(tmp);
		Logger::getInstance()->debug("PartTypeParser::loadPartTypes - About to load part/unit file...");
		Logger::getInstance()->debug(filename);

    // quick hack for error filenames
    impl->ptp_currentfile = filename;

    filesystem::FB_FILE *f = filesystem::fb_fopen(filename, "rb");
    if (f == NULL)
    {
      error("Could not open file.", 0);
      return;
    }

    char *buf;

    //fseek(f, 0, SEEK_END);
    //int size = ftell(f);
    //fseek(f, 0, SEEK_SET);
    int size = filesystem::fb_fsize(f);

    buf = new char[size + 1];
    int datalen = filesystem::fb_fread(buf, sizeof(char), size, f);
    buf[datalen] = '\0';

		if (strlen(filename) > 4
			&& (strcmp(&filename[strlen(filename) - 4], ".dhu") == 0
			|| strcmp(&filename[strlen(filename) - 4], ".DHU") == 0))
		{
			util::UberCrypt::decrypt(buf, datalen);
		}

    PartType *parsePart = NULL;
    UnitType *parseUnit = NULL;
    bool parsingUnit = false;

    int state = 0; // 0 = main, 1 = part, 2 = part sub, ...
    bool stateAwait = false;  // when expecting to change state + 1

    int lineNumber = 1;
    int lastpos = 0;
    int i;
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
          if (buf[j] == ' ' || buf[j] == '\t')  
            buf[j] = '\0';
          else 
            break;
        }

        // process if not empty and not start with comments
        if (buf[lastpos] != '\0' 
          && strncmp(&buf[lastpos], "//", 2) != 0)
        {
          bool lineok = false;
          if (state == 0)
          {
            if (strncmp(&buf[lastpos], "import ", 7) == 0)
            {
							char *importfile = &buf[lastpos + 7];
							if (strlen(importfile) > 0)
							{
								int pos = 0;
								int slen = strlen(importfile);
								for (int j = 0; j < slen; j++)
								{
									if (importfile[j] != ' ')
									{
										if (importfile[j] == '"')
										{
											pos = j + 1;
											for (int k = strlen(importfile) - 1; k > j; k--)
											{
												if (importfile[k] == '"')
												{
													importfile[k] = '\0';
													break;
												}
											}
										} else {
											pos = j;									
										}
										break;
									}
								}

								PartTypeParser ptp;
								Logger::getInstance()->debug("PartTypeParser::loadPartTypes - Importing another part/unit type.");
								Logger::getInstance()->debug(&importfile[pos]);
								ptp.loadPartTypes(&importfile[pos]);

								lineok = true;							
							} else {
                error("Import parameter missing.", lineNumber);
							}
						}
            if (strncmp(&buf[lastpos], "part : ", 7) == 0)
            {
              char *baseclass = &buf[lastpos + 7];

              parsingUnit = false;

							if (parsePart != NULL)
							{
								// save originals for the previously parsed part...
								parsePart->saveOriginals();
							}

              // quick hack solution...
              parsePart = NULL;

              if (strcmp(baseclass, "Bull") == 0) parsePart = new Bullet();
              if (strcmp(baseclass, "Tors") == 0) parsePart = new Torso();
              if (strcmp(baseclass, "Arm") == 0) parsePart = new Arm();
              if (strcmp(baseclass, "Leg") == 0) parsePart = new Leg();
              if (strcmp(baseclass, "Head") == 0) parsePart = new Head();
              if (strcmp(baseclass, "Tool") == 0) parsePart = new Tool();
              if (strcmp(baseclass, "Weap") == 0) parsePart = new Weapon();
              if (strcmp(baseclass, "Indi") == 0) parsePart = new IndirectWeapon();
              if (strcmp(baseclass, "Dire") == 0) parsePart = new DirectWeapon();
              if (strcmp(baseclass, "Pack") == 0) parsePart = new ItemPack();
              if (strcmp(baseclass, "Ammo") == 0) parsePart = new AmmoPack();
              if (strcmp(baseclass, "Powe") == 0) parsePart = new PowerCell();
              if (strcmp(baseclass, "Reac") == 0) parsePart = new Reactor();

              // todo: maybe need a real class for this.
              if (strcmp(baseclass, "Soli") == 0) parsePart = new Torso();

              if (parsePart == NULL)
              {
                error("Bad part base type.", lineNumber);
                assert(0);
              }
              lineok = true;
              stateAwait = true;
            }
            if (strncmp(&buf[lastpos], "unit : ", 7) == 0)
            {
              char *baseclass = &buf[lastpos + 7];

              parsingUnit = true;

							parseUnit = getNewUnitTypeForUnitTypeName(baseclass);

              if (parseUnit == NULL)
              {
                error("Bad unit base type.", lineNumber);
							}
							lineok = true;
							stateAwait = true;
            }
            if (strcmp(&buf[lastpos], "{") == 0)
            {
              if (!stateAwait)
                error("Unexpected { at top level.", lineNumber);
              state = 1;
              stateAwait = 0;
              lineok = true;
            }
            if (strcmp(&buf[lastpos], "}") == 0)
            {
              error("Unexpected } at top level.", lineNumber);
            }
          }
          if (!lineok && state >= 1)
          {
            if (strcmp(&buf[lastpos], "{") == 0)
            {
              if (!stateAwait)
                error("Unexpected { at part or sub level.", lineNumber);
              state++;
              stateAwait = 0;
              lineok = true;
            }
            if (strcmp(&buf[lastpos], "}") == 0)
            {
              if (stateAwait)
                error("Unexpected } at part or sub level.", lineNumber);
              state--;
              lineok = true;
              if (parsingUnit)
              {
								if (parseUnit != NULL)
	                parseUnit->setSub(NULL);
              } else {
								if (parsePart != NULL)
	                parsePart->setSub(NULL);
              }
            }
            if (!lineok)
            {
              int hasEqual = -1;
              for (int j = lastpos; j < i; j++)
              {
                if (buf[j] == '=') 
                { 
                  buf[j] = '\0';
									// trim left side of equal sign
									for (int backtrim = j-1; backtrim >= lastpos; backtrim--)
									{
										if (buf[backtrim] == ' ' || buf[backtrim] == '\t')
										{
											buf[backtrim] = '\0';
										} else {
											break;
										}
									}
									// trim right side of equal sign
									while (buf[j+1] == ' ' || buf[j+1] == '\t')
									{
										j++;
									}
                  hasEqual = j;
                  if (buf[j + 1] == '"' && buf[i - 1] == '"')
                  {
                    // remove doublequotes
                    hasEqual++;
                    buf[i - 1] = '\0';
                  }
                  break; 
                }
              }
              if (hasEqual != -1)
              {
                if (stateAwait)
                {
                  error("Expected { at part or sub level.", lineNumber);
                } else {
                  if (parsingUnit) 
                  {
										if (parseUnit != NULL)
										{
											if (!parseUnit->setData(&buf[lastpos], &buf[hasEqual + 1]))
											{
												error("Bad key or value.", lineNumber);
											}
										}
                  } else {
										if (parsePart != NULL)
										{
											if (!parsePart->setData(&buf[lastpos], &buf[hasEqual + 1]))
											{
												error("Bad key or value.", lineNumber);
											}
										}
                  }
                  lineok = true;
                }
              } else {
                stateAwait = true;
                if (parsingUnit) 
                {
									if (parseUnit != NULL)
									{
										if (!parseUnit->setSub(&buf[lastpos]))
										{
											error("Bad sub level name.", lineNumber);
										}
									}
                } else {
									if (parsePart != NULL)
									{
										if (!parsePart->setSub(&buf[lastpos]))
										{
											error("Bad sub level name.", lineNumber);
										}
									}
                }
                lineok = true;
              }
            }
          }
          if (!lineok) 
          { 
            error("Syntax error.", lineNumber);
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

 		if (parsePart != NULL)
		{
			// save originals for the very last parsed part...
			parsePart->saveOriginals();
			parsePart = NULL;
		}
	}

}
