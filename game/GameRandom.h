
#ifndef GAMERANDOM_H
#define GAMERANDOM_H

//#include <c2_sptr.h>

#include "GameObject.h"


#define GAMERANDOM_MIN_VALUE 0
#define GAMERANDOM_MAX_VALUE 0x7fffffff

namespace game
{

  class GameRandom : public GameObject   //, public Shared
  {
  public:
    GameRandom();
    ~GameRandom();

    void seed(int seed);

		// returns 0..GAMERANDOM_MAX_VALUE
    int nextInt();
		// returns 0..1
		float nextFloat();

    //unsigned int nextUInt();
    //signed short nextShort();
    //unsigned short nextUShort();

    virtual SaveData *getSaveData() const;

    virtual const char *getStatusInfo() const;

    // just to clean up so that we don't seem to leak memory.
    static void uninit();

  private:
    int counter;
    int *data;
  };

}

#endif

