
#ifndef INT64_TO_HEX_H
#define INT64_TO_HEX_H

extern char *int64_to_hex(__int64 value);
extern __int64 hex_to_int64(const char *string);
extern int hex_to_int64_errno();

#endif
