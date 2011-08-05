
#ifndef OGUIEXCEPTION_H
#define OGUIEXCEPTION_H

#include <string.h>

// : public std::exception

class OguiException
{
public:
  OguiException(const char *errmsg, void *errdata = 0L);
  ~OguiException();

  // notice, returns a pointer to internal data, 
  // don't delete it nor store it anywhere (for temporary use only)
  const char *GetErrorMessage();

private:
  char *msg;

};

#endif

