
#ifndef PART_DATA_READER_MACROS_H
#define PART_DATA_READER_MACROS_H

#define PDATA_READ_FLOAT(stringname, varname) \
if (strcmp(key, stringname) == 0) \
{ \
  varname = (float)atof(value); \
  return true; \
} 

#define PDATA_READ_INT(stringname, varname) \
if (strcmp(key, stringname) == 0) \
{ \
  varname = str2int(value); \
  return true; \
} 

#define PDATA_READ_STRING(stringname, varname) \
if (strcmp(key, stringname) == 0) \
{ \
  if (varname != NULL) delete [] varname; \
  varname = new char[strlen(value) + 1]; \
  strcpy(varname, value); \
  int slen = strlen(varname); \
  for (int i = 0; i < slen; i++) \
  { \
    if (varname[i] == '\\') varname[i] = '\n'; \
  } \
  return true; \
}

#define PDATA_READ_STRINGPLUS(stringname, varname) \
if (strcmp(key, stringname) == 0) \
{ \
  if (varname == NULL) return false; \
  char *olddesc = varname; \
  int olddesclen = strlen(olddesc); \
  varname = new char[olddesclen + strlen(value) + 1]; \
  strcpy(varname, olddesc); \
  strcpy(&varname[olddesclen], value); \
  delete [] olddesc; \
  int slen = strlen(varname); \
  for (int i = 0; i < slen; i++) \
  { \
    if (varname[i] == '\\') varname[i] = '\n'; \
  } \
  return true; \
}

#define PDATA_DELETE(varname) \
if (varname != NULL) { delete varname; varname = NULL; }

#define PDATA_DELETE_ARRAY(varname) \
if (varname != NULL) { delete [] varname; varname = NULL; }

#define PDATA_READ_BOOLINT(stringname, varname) \
if (strcmp(key, stringname) == 0) \
{ \
  int val = str2int(value); \
  if (val != 0 && val != 1) return false; \
  if (val == 1) \
    varname = true; \
  else \
    varname = false; \
  return true; \
} 

#define PDATA_GETFUNC(type, varname, funcname) \
type funcname() const \
{ \
  return varname;\
}

#define PDATA_DEF(type, varname, funcname) \
protected: \
  type varname; \
public: \
  type funcname() const { return varname; } \

#endif

