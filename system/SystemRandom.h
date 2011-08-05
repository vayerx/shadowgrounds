
#ifndef SYSTEMRANDOM_H
#define SYSTEMRANDOM_H

// these are defined by msvc RAND_MAX...

#define SYSTEMRANDOM_MIN_VALUE 0
#define SYSTEMRANDOM_MAX_VALUE 0x7fff


class SystemRandom
{
public:
  SystemRandom();
  ~SystemRandom();

	static SystemRandom *getInstance();

	static void cleanInstance();

  int nextInt();

  //unsigned int nextUInt();
  //signed short nextShort();
  //unsigned short nextUShort();

private:
	static SystemRandom *instance;

};


#endif

