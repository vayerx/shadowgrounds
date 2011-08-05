
#include "precompiled.h"

#include "OguiException.h"
#include "../util/Debug_MemoryManager.h"

OguiException *ogui_last_exception = NULL;
char *ogui_last_exception_msg = NULL;
void *ogui_last_exception_data = NULL;


const char *OguiException::GetErrorMessage()
{
  return msg;
}

OguiException::OguiException(const char *errmsg, void *errdata)
{ 
  msg = new char[strlen(errmsg) + 1];
  strcpy(msg, errmsg);
  ogui_last_exception = this;
  ogui_last_exception_msg = msg;
  ogui_last_exception_data = errdata;
}

OguiException::~OguiException()
{
  delete [] msg;
}


