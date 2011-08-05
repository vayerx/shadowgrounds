
#ifndef STR2INT_H
#define STR2INT_H

extern char *int2str(int value);
extern int str2int(const char *string);
extern int str2int_errno();
extern const char *time2str(int secs); // hh:mm:ss

#endif
