/*
 *  LauncherParser.cpp
 *  trine
 *
 *  Created by Juha Hiekkamäki on 9/24/10.
 *  Copyright 2010 Frozenbyte. All rights reserved.
 *
 */

#include "LauncherParser.h"
#include <stdio.h>
#include <dirent.h>

namespace {
    bool getLine(FILE *fp, std::string &str)
    {
        for (;; ) {
            int c = fgetc(fp);
            if (c == EOF)
                break;

            str += (char)c;
            if (c == '\n')
                return true;
        }

        return !str.empty();
    }

    bool isWhiteSpace(char c)
    {
        if (c == ' ')
            return true;
        if (c == '\t')
            return true;
        if (c == '\r')
            return true;
        if (c == '\n')
            return true;

        return false;
    }

    bool parseString(const std::string &from, int start, int end, std::string &result)
    {
        --end;
        while (start <= end) {
            bool moved = false;
            if ( isWhiteSpace(from[start]) ) {
                ++start;
                moved = true;
            }

            if ( isWhiteSpace(from[end]) ) {
                --end;
                moved = true;
            }

            if (!moved)
                break;
        }

        if (start <= end) {
            result = from.substr(start, end - start + 1);
            return true;
        }

        return false;
    }

    bool hasPropertyPair(const std::string &from, std::string &key, std::string &value)
    {
        std::string::size_type ep = from.find_first_of("=");
        if (ep == from.npos)
            return false;

        if ( !parseString(from, 0, ep, key) )
            return false;
        if ( !parseString(from, ep + 1, from.size(), value) )
            return false;

        return true;
    }
}

LauncherParser::LauncherParser(const char *filename)
{
    DIR *dir = opendir("data");
    if (dir) {
        closedir(dir);
    } else {
        printf("Data folder not found!\n");
        chdir("/Users/hiekkama/fb/binary");
    }

    FILE *fp = fopen(filename, "rb");
    if (!fp)
        return;

    for (;; ) {
        std::string line;
        if ( !getLine(fp, line) )
            break;

        file.push_back(line);

        std::string key;
        std::string value;
        if ( !hasPropertyPair(line, key, value) )
            continue;

        properties[key] = value;
    }

    fclose(fp);
}

LauncherParser::~LauncherParser()
{
}

const std::string &LauncherParser::getProperty(const char *p) const
{
    std::map<std::string, std::string>::const_iterator it = properties.find(p);
    if ( it != properties.end() )
        return it->second;

    static std::string empty;
    return empty;
}

int LauncherParser::getPropertyInt(const char *p) const
{
    const std::string &v = getProperty(p);
    return atoi( v.c_str() );
}

void LauncherParser::save(const char *filename) const
{
    FILE *fp = fopen(filename, "wb");
    if (!fp)
        return;

    for (int i = 0; i < file.size(); ++i) {
        const std::string &line = file[i];

        std::string key;
        std::string value;
        if ( hasPropertyPair(line, key, value) ) {
            value = getProperty( key.c_str() );
            fprintf( fp, "   %s = %s\n", key.c_str(), value.c_str() );
        } else {
            fprintf( fp, "%s", line.c_str() );
        }
    }

    fclose(fp);
}

void LauncherParser::setProperty(const char *p, const char *v)
{
    properties[p] = v;
}

void LauncherParser::setProperty(const char *p, int v)
{
    char s[2048] = { 0 };
    sprintf(s, "%d", v);
    setProperty(p, s);
}
