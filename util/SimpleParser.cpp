// Copyright 2002-2004 Frozenbyte Ltd.

#include "precompiled.h"

#include "SimpleParser.h"

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "../convert/str2int.h"
#include "../system/Logger.h"
#include "../filesystem/input_stream_wrapper.h"

#include "../util/Debug_MemoryManager.h"

using namespace frozenbyte;

// uncomment this to make the parser NOT to trim leading spaces.
// #define SIMPLEPARSER_NO_LTRIM

// FIXME: rtrim only works if ltrim is used too
// uncomment this to make the parser NOT to trim ending spaces after a key or a value string
// #define SIMPLEPARSER_NO_RTRIM

namespace util
{

  SimpleParser::SimpleParser()
  {
    buf = NULL;
    cutBuf = NULL;
    nextPos = 0;
    currentPos = -1;
    keyPos = -1;
    valuePos = -1;
    currentFile = NULL;
    linenum = 0;
    bufLen = -1;
  }


  SimpleParser::~SimpleParser()
  {
    if (buf != NULL)
    {
      delete [] buf;
      buf = NULL;
    }
    if (cutBuf != NULL)
    {
      delete [] cutBuf;
      cutBuf = NULL;
    }
    if (currentFile != NULL)
    {
      delete [] currentFile;
      currentFile = NULL;
    }
  }


  void SimpleParser::loadMemoryBuffer(const char *buffer, int length)
  {
		// NOTE: copied and modified from loadFile...

    buf = new char[length + 1 + 1]; // buf + newline + null
    memcpy(buf, buffer, length);

    bufLen = length;

		// FIXED: no newline at end of file -> last line would not be read
		// now adds newline to end, if it's not there already.
		if (bufLen > 0)
		{
			if (buf[bufLen - 1] != '\n')
			{
				buf[bufLen] = '\n';
				bufLen++;
			}
		}
    buf[bufLen] = '\0';

    cutBuf = new char[bufLen + 1]; // buf (has ending newline) + null
    memcpy(cutBuf, buf, bufLen);
    cutBuf[bufLen] = '\0';
	}


  bool SimpleParser::loadFile(const char *filename)
  {
    if (buf != NULL)
    {
      delete [] buf;
      buf = NULL;
    }
    if (cutBuf != NULL)
    {
      delete [] cutBuf;
      cutBuf = NULL;
    }
    if (currentFile != NULL)
    {
      delete [] currentFile;
      currentFile = NULL;
    }

    nextPos = 0;
    currentPos = -1;
    keyPos = -1;
    valuePos = -1;
    linenum = 0;
    bufLen = -1;

    currentFile = new char[strlen(filename) + 1];
    strcpy(currentFile, filename);

    filesystem::FB_FILE *f = filesystem::fb_fopen(filename, "rb");
    if (f == NULL)
    {
      error("SimpleParser::loadFile - Could not open file.");
      return false;
    }

    //fseek(f, 0, SEEK_END);
    //int flen = ftell(f);
    //fseek(f, 0, SEEK_SET);
	int flen = filesystem::fb_fsize(f);

    buf = new char[flen + 1 + 1]; // buf + newline + null
    int got = filesystem::fb_fread(buf, sizeof(char), flen, f);
    filesystem::fb_fclose(f);

    if (got != flen)
    {
      delete [] buf;
      error("SimpleParser::loadFile - Error reading file.");
      return false;
    }

    bufLen = got;

		// FIXED: no newline at end of file -> last line would not be read
		// now adds newline to end, if it's not there already.
		if (bufLen > 0)
		{
			if (buf[bufLen - 1] != '\n')
			{
				buf[bufLen] = '\n';
				bufLen++;
			}
		}
    buf[bufLen] = '\0';

    cutBuf = new char[bufLen + 1]; // buf (has ending newline) + null
    memcpy(cutBuf, buf, bufLen);
    cutBuf[bufLen] = '\0';
    return true;
  }
  
  
  bool SimpleParser::next(bool list_comments)
  {
    currentPos = -1;
    keyPos = -1;
    valuePos = -1;

    int i;
    for (i = nextPos; i < bufLen + 1; i++)
		{
      if (buf[i] == '=')
      {
        if (keyPos == -1)
        {
          keyPos = nextPos;
          valuePos = i + 1;
          cutBuf[i] = '\0';
        }
      }
      if (buf[i] == '\n' || buf[i] == '\r' || buf[i] == '\0')
			{
        if (buf[i] == '\n')
        {
          linenum++;
					if(list_comments)
					{
						buf[i] = '\0';
						cutBuf[i] = '\0';
            currentPos = nextPos;
            nextPos = i + 1;
            return true;
					}
        }
        buf[i] = '\0';
        cutBuf[i] = '\0';

				if(list_comments)
					continue;

        if (buf[nextPos] != '\0')
				{
#ifdef SIMPLEPARSER_NO_LTRIM
          if (strncmp(&buf[nextPos], "//", 2) != 0
            && buf[nextPos] != '#')
#else
          int slen = strlen(&buf[nextPos]);
          int ltrim = 0;
          for (int j = 0; j < slen; j++)
          {
            if (buf[nextPos + j] != ' ' && buf[nextPos + j] != '\t') break;
            ltrim++;
          }
          if (strncmp(&buf[nextPos + ltrim], "//", 2) != 0
            && buf[nextPos + ltrim] != '#')
#endif
          {
            currentPos = nextPos;
            nextPos = i + 1;
            return true;
          }
        }
        keyPos = -1;
        valuePos = -1;
        nextPos = i + 1;
      }
    }

    currentPos = -1;
    keyPos = -1;
    valuePos = -1;

    return false;
  }


  char *SimpleParser::getKey()
  {
    if (keyPos != -1)
    {
#ifdef SIMPLEPARSER_NO_LTRIM
      return &cutBuf[keyPos];
#else
      int slen = strlen(&cutBuf[keyPos]);
      int ltrim = 0;
      for (int i = 0; i < slen; i++)
      {
        if (cutBuf[keyPos + i] != ' ' && cutBuf[keyPos + i] != '\t') break;
        ltrim++;
      }
#ifndef SIMPLEPARSER_NO_RTRIM
      for (int i = slen - 1; i >= ltrim; i--)
      {
        if (cutBuf[keyPos + i] != ' ' && cutBuf[keyPos + i] != '\t') break;
				cutBuf[keyPos + i] = '\0';
			}
#endif
      return &cutBuf[keyPos + ltrim];
#endif
    } else {
      return NULL;
    }
  }


  char *SimpleParser::getValue()
  {
    if (valuePos != -1)
    {
#ifdef SIMPLEPARSER_NO_LTRIM
      return &cutBuf[valuePos];
#else
      int slen = strlen(&cutBuf[valuePos]);
      int ltrim = 0;
      for (int i = 0; i < slen; i++)
      {
        if (cutBuf[valuePos + i] != ' ' && cutBuf[valuePos + i] != '\t') break;
        ltrim++;
      }
#ifndef SIMPLEPARSER_NO_RTRIM
      for (int i = slen - 1; i >= ltrim; i--)
      {
        if (cutBuf[valuePos + i] != ' ' && cutBuf[valuePos + i] != '\t') break;
				cutBuf[valuePos + i] = '\0';
			}
#endif
      return &cutBuf[valuePos + ltrim];
#endif
    } else {
      return NULL;
    }
  }


  int SimpleParser::getIntValue()
  {
    char *tmp = getValue();
    if (tmp != NULL)
    {
      //return str2int(tmp);
      return atoi(tmp);
    } else {
      return 0;
    }
  }


  float SimpleParser::getFloatValue()
  {
    char *tmp = getValue();
    if (tmp != NULL)
    {
      return (float)atof(tmp);
    } else {
      return 0;
    }  
  }


  char *SimpleParser::getLine()
  {
    if (currentPos != -1)
    {
#ifdef SIMPLEPARSER_NO_LTRIM
      return &buf[currentPos];
#else
      int slen = strlen(&buf[currentPos]);
      int ltrim = 0;
      for (int i = 0; i < slen; i++)
      {
        if (buf[currentPos + i] != ' ' && buf[currentPos + i] != '\t') break;
        ltrim++;
      }
#ifndef SIMPLEPARSER_NO_RTRIM
      for (int i = slen - 1; i >= ltrim; i--)
      {
        if (buf[currentPos + i] != ' ' && buf[currentPos + i] != '\t') break;
				buf[currentPos + i] = '\0';
			}
#endif
      return &buf[currentPos + ltrim];
#endif
    } else {
      return NULL;
    }    
  }


  int SimpleParser::getLineNumber()
  {
    return linenum;
  }


  void SimpleParser::error(const char *err)
  {
    char *errbuf = new char[strlen(err) + 1 + 60 + strlen(currentFile)];
    strcpy(errbuf, err);
    strcat(errbuf, " (file ");
    strcat(errbuf, currentFile);
    strcat(errbuf, ", line ");
    strcat(errbuf, int2str(linenum));
    strcat(errbuf, ")");
    Logger::getInstance()->error(errbuf);
    delete [] errbuf;
  }

}

