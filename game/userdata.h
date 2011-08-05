#ifndef IGIOS__USERDATA_H
#define IGIOS__USERDATA_H

#include <string>

std::string igios_getUserDataPrefix();
std::string igios_mapUserDataPrefix(const std::string &path);
std::string igios_unmapUserDataPrefix(const std::string &path);
void igios_initializeUserData();

#endif
