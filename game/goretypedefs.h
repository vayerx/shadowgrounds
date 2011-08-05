
#ifndef GORETYPEDEFS_H
#define GORETYPEDEFS_H

#define GORETYPE_RESERVED 0
#define GORETYPE_SLICE 1
#define GORETYPE_PARTIAL_SLICE 2
#define GORETYPE_EXPLODE 3
#define GORETYPE_PARTIAL_EXPLODE 4
#define GORETYPE_MELT 5
#define GORETYPE_BURN 6
#define GORETYPE_GRIND 7
#define GORETYPE_ELECTRIFIED 8

#define GORETYPE_AMOUNT 9

namespace game
{
  extern const char *goreTypeName[GORETYPE_AMOUNT + 1];
  extern bool goreTypeRemovesOrigin[GORETYPE_AMOUNT + 1];
}

#endif
