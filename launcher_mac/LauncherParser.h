/*
 *  LauncherParser.h
 *  trine
 *
 *  Created by Juha Hiekkamäki on 9/24/10.
 *  Copyright 2010 Frozenbyte. All rights reserved.
 *
 */

#ifndef INCLUDED_LAUNCHER_PARSER_H
#define INCLUDED_LAUNCHER_PARSER_H

#include <vector>
#include <map>
#include <string>

struct LauncherParser {
    LauncherParser(const char *file);
    ~LauncherParser();

    const std::string&getProperty(const char *p) const;
    int  getPropertyInt(const char *p) const;
    void save(const char *file) const;

    void setProperty(const char *p, const char *v);
    void setProperty(const char *p, int v);

    std::vector<std::string> file;
    std::map<std::string, std::string> properties;
};

#endif
