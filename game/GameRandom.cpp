
#include "precompiled.h"

#include <stdlib.h>
#include <stdio.h>

#include "idlist.h"
#include "GameRandom.h"
#include "../util/fb_assert.h"

#include "../filesystem/input_stream_wrapper.h"
#include "../util/Debug_MemoryManager.h"

// value must be 2^n - 1, depends on the size of data file
#define GRAND_COUNTER_LOOP 0x7fff

// random data will be loaded once, and never freed until program terminates
static int *gamerandom_static_data = NULL;
static int *gamerandom_instance_count = 0;

using namespace frozenbyte;

namespace game
{

  GameRandom::GameRandom()
  {
    gamerandom_instance_count++;
    if (gamerandom_static_data == NULL) 
    {
      gamerandom_static_data = new int[GRAND_COUNTER_LOOP + 1];

#ifdef LEGACY_FILES
      filesystem::FB_FILE *f = filesystem::fb_fopen("Data/Misc/Grand.dat", "rb");
#else
      filesystem::FB_FILE *f = filesystem::fb_fopen("data/misc/grand.dat", "rb");
#endif
      if (f == NULL) 
      {  
        // if we cannot load the game random data, we're f*cked and
        // there really is no point to continue...
        fb_assert(!"GameRandom - Failed to read random data file.");

        // but we'll do it anyway... with this great random generator ;)
        int foorandom[7] = { 31, 4857, 24689, 23456, 35762, 23434, 135 };
        for (int i = 0; i < GRAND_COUNTER_LOOP + 1; i++)
        {
          gamerandom_static_data[i] = foorandom[i % 7] ^ (i % 17);
        }
      } else {
        // should check return value, so that we get the whole data!
        filesystem::fb_fread(gamerandom_static_data, sizeof(int), GRAND_COUNTER_LOOP + 1, f);
        filesystem::fb_fclose(f);
      }
    }
    data = gamerandom_static_data;
    counter = 0;
  }

  GameRandom::~GameRandom()
  {
    gamerandom_instance_count--;
  }

  void GameRandom::seed(int seed)
  {
    counter = seed & GRAND_COUNTER_LOOP;
  }

  int GameRandom::nextInt()
  {
    counter = (counter + 1) & GRAND_COUNTER_LOOP;;
    return data[counter] & GAMERANDOM_MAX_VALUE;
  }

  float GameRandom::nextFloat()
  {
    return nextInt() / (float)GAMERANDOM_MAX_VALUE;
  }

  SaveData *GameRandom::getSaveData() const
  {
    SaveData *data = new SaveData(GAMERANDOM_ID, sizeof(int), (BYTE *)&counter);
    return data;
  }

	const char *GameRandom::getStatusInfo() const
  {
		return "GameRandom";
	}

  void GameRandom::uninit()
  {
    if (gamerandom_instance_count > 0) 
    {
      //abort();
    }
    delete [] gamerandom_static_data;
    gamerandom_static_data = NULL;
  }

}

