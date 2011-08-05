
#include "precompiled.h"

#include "UberCrypt.h"
#include "../../game/GameRandom.h"


#ifdef CRYPT_GERMANY

#undef NO_CRYPT
#define CRYPT_KEY_LEN 64
#define CRYPT_RAND_SEED 94
#define CRYPT_OFFSET_MODULO 129
#define CRYPT_OFFSET_MODULO_VALUE 13
#define CRYPT_OFFSET_ADD 9
const char *version_branch_name = "germany";


unsigned char ubercrypt_key[CRYPT_KEY_LEN+1] = 
{
	43, 23, 56, 34, 13, 23, 56, 234, 143, 73, 16, 253, 34, 235, 84, 13, 
	1, 2, 7, 97, 12, 56, 34, 128, 178, 193, 55, 234, 56, 15, 65, 66, 
	43, 23, 56, 34, 13, 23, 56, 234, 113, 73, 16, 253, 34, 235, 84, 13, 
	6, 1, 72, 197, 12, 56, 3, 128, 178, 93, 55, 234, 56, 15, 125, 162,
	0
};


#elif CRYPT_GERMANY_DEMO


#undef NO_CRYPT
#define CRYPT_KEY_LEN 69
#define CRYPT_RAND_SEED 129
#define CRYPT_OFFSET_MODULO 193
#define CRYPT_OFFSET_MODULO_VALUE 17
#define CRYPT_OFFSET_ADD 5
const char *version_branch_name = "germany_demo";

unsigned char ubercrypt_key[CRYPT_KEY_LEN+1] = 
{
	43, 23, 56, 34, 13, 21, 56, 134, 143, 73, 16, 253, 34, 235, 84, 13, 
	1, 2, 7, 27, 12, 56, 24, 128, 178, 193, 55, 234, 56, 15, 65, 66, 
	43, 23, 56, 34, 13, 23, 56, 234, 113, 73, 160, 253, 34, 235, 84, 13, 
	6, 41, 72, 197, 12, 56, 3, 128, 178, 93, 155, 134, 56, 15, 125, 162, 
	42, 11, 119, 87, 69,
	0
};

#elif CRYPT_GERMANY_BETA


#undef NO_CRYPT
#define CRYPT_KEY_LEN 65
#define CRYPT_RAND_SEED 152
#define CRYPT_OFFSET_MODULO 173
#define CRYPT_OFFSET_MODULO_VALUE 5
#define CRYPT_OFFSET_ADD 3
const char *version_branch_name = "germany_beta";

unsigned char ubercrypt_key[CRYPT_KEY_LEN+1] = 
{
	43, 23, 56, 34, 13, 21, 56, 134, 143, 73, 16, 253, 34, 235, 84, 13, 
	1, 2, 7, 27, 12, 56, 24, 128, 178, 193, 55, 234, 56, 15, 65, 66, 
	43, 23, 56, 34, 13, 23, 56, 234, 113, 73, 160, 253, 34, 235, 84, 13, 
	6, 41, 72, 197, 12, 56, 3, 128, 178, 93, 155, 134, 56, 15, 125, 162, 
	42,
	0
};

#elif CRYPT_AUSTRALIA

#undef NO_CRYPT
#define CRYPT_KEY_LEN 71
#define CRYPT_RAND_SEED 217
#define CRYPT_OFFSET_MODULO 171
#define CRYPT_OFFSET_MODULO_VALUE 51
#define CRYPT_OFFSET_ADD 6
const char *version_branch_name = "australia";


unsigned char ubercrypt_key[CRYPT_KEY_LEN+1] = 
{
	143, 23, 51, 1, 13, 221, 16, 33, 25, 13, 46, 126, 11, 85, 14, 23, 
	31, 12, 17, 7, 212, 156, 146, 218, 111, 128, 125, 140, 6, 215, 165, 166, 
	86, 77, 48, 94, 55, 127, 165, 144, 23, 173, 60, 62, 134, 3, 10, 52, 
	26, 94, 231, 139, 195, 94, 68, 242, 122, 12, 57, 4, 198, 86, 55, 62, 
	242, 54, 169, 37, 149, 236, 23,
	0
};


#elif CRYPT_FINLAND

#undef NO_CRYPT
#define CRYPT_KEY_LEN 121
#define CRYPT_RAND_SEED 85
#define CRYPT_OFFSET_MODULO 115
#define CRYPT_OFFSET_MODULO_VALUE 17
#define CRYPT_OFFSET_ADD 7
const char *version_branch_name = "finland";


unsigned char ubercrypt_key[CRYPT_KEY_LEN+1] = 
{
	43, 23, 56, 34, 13, 23, 56, 24, 143, 73, 16, 253, 34, 235, 84, 13, 
	43, 23, 56, 34, 13, 23, 56, 234, 113, 73, 16, 53, 34, 235, 84, 13, 
	1, 2, 7, 97, 12, 56, 34, 128, 178, 93, 51, 234, 56, 15, 65, 66, 
	6, 1, 72, 197, 12, 56, 3, 128, 178, 93, 55, 234, 56, 15, 25, 162, 
	1, 2, 7, 97, 12, 56, 34, 128, 178, 193, 55, 234, 56, 15, 65, 66, 
	43, 23, 56, 34, 118, 23, 56, 234, 143, 73, 16, 253, 34, 235, 84, 13, 
	6, 1, 72, 17, 12, 56, 3, 128, 178, 93, 55, 234, 56, 15, 125, 162, 
	43, 23, 156, 34, 13, 23, 56, 234, 113,  
	0
};

#elif CRYPT_FINLAND2

#undef NO_CRYPT
#define CRYPT_KEY_LEN 111
#define CRYPT_RAND_SEED 46
#define CRYPT_OFFSET_MODULO 154
#define CRYPT_OFFSET_MODULO_VALUE 73
#define CRYPT_OFFSET_ADD 11
const char *version_branch_name = "finland2";


unsigned char ubercrypt_key[CRYPT_KEY_LEN+1] = 
{
	92, 163, 7, 134, 13, 23, 56, 24, 143, 73, 16, 23, 34, 235, 84, 13, 
	13, 23, 56, 4, 38, 133, 56, 234, 113, 73, 16, 53, 34, 210, 84, 23, 
	1, 2, 7, 97, 12, 56, 34, 128, 178, 93, 51, 234, 56, 15, 65, 66, 
	6, 1, 72, 197, 12, 56, 3, 128, 7, 93, 55, 14, 56, 15, 25, 162, 
	12, 2, 7, 97, 12, 16, 34, 68, 178, 193, 55, 234, 56, 15, 65, 66, 
	43, 23, 56, 34, 118, 23, 56, 234, 143, 73, 16, 253, 34, 235, 84, 13, 
	6, 1, 72, 17, 12, 56, 3, 128, 178, 93, 55, 234, 56, 15, 5, 
	0
};

#elif CRYPT_UKR_PUBLISHER


#undef NO_CRYPT
#define CRYPT_KEY_LEN 74
#define CRYPT_RAND_SEED 446
#define CRYPT_OFFSET_MODULO 175
#define CRYPT_OFFSET_MODULO_VALUE 54
#define CRYPT_OFFSET_ADD 3
const char *version_branch_name = "ukr_publisher";


unsigned char ubercrypt_key[CRYPT_KEY_LEN+1] = 
{
	143, 23, 51, 1, 13, 221, 16, 33, 25, 13, 46, 126, 11, 85, 14, 23, 
	31, 12, 17, 7, 212, 156, 146, 218, 111, 128, 125, 140, 6, 215, 165, 166, 
	86, 77, 48, 94, 55, 127, 165, 144, 23, 173, 60, 62, 134, 3, 10, 52, 
	26, 94, 231, 139, 195, 94, 68, 242, 122, 12, 57, 4, 198, 86, 55, 62, 
	242, 54, 169, 37, 149, 236, 23, 220, 107, 83,
	0
};

#elif CRYPT_FRA_PUBLISHER

#undef NO_CRYPT
#define CRYPT_KEY_LEN 82
#define CRYPT_RAND_SEED 158
#define CRYPT_OFFSET_MODULO 133
#define CRYPT_OFFSET_MODULO_VALUE 71
#define CRYPT_OFFSET_ADD 5
const char *version_branch_name = "fra_publisher";

unsigned char ubercrypt_key[CRYPT_KEY_LEN+1] = 
{
	43, 211, 121, 13, 13, 11, 216, 3, 250, 23, 46, 226, 110, 85, 14, 23, 
	10, 101, 100, 7, 212, 156, 146, 18, 111, 118, 15, 141, 6, 215, 165, 166, 
	182, 231, 48, 94, 55, 127, 165, 144, 23, 173, 160, 162, 4, 3, 1, 1, 
	16, 94, 231, 139, 195, 94, 68, 242, 122, 12, 57, 4, 198, 86, 55, 62, 
	29, 53, 91, 30, 49, 26, 23, 208, 17, 181, 66, 12, 42, 87, 125, 140, 6, 215,
	0
};

#elif CRYPT_FRA_ENG

#undef NO_CRYPT
#define CRYPT_KEY_LEN 84
#define CRYPT_RAND_SEED 119
#define CRYPT_OFFSET_MODULO 73
#define CRYPT_OFFSET_MODULO_VALUE 53
#define CRYPT_OFFSET_ADD 1
const char *version_branch_name = "fra_eng";

unsigned char ubercrypt_key[CRYPT_KEY_LEN+1] = 
{
	143, 21, 121, 13, 13, 11, 216, 13, 250, 23, 46, 226, 110, 85, 14, 23, 
	10, 101, 100, 7, 212, 156, 146, 18, 111, 118, 15, 141, 6, 215, 165, 166, 
	182, 231, 48, 94, 55, 127, 165, 214, 23, 173, 160, 62, 4, 3, 21, 21, 
	16, 94, 231, 139, 95, 14, 68, 25, 122, 12, 57, 18, 9, 86, 55, 62, 
	29, 53, 91, 30, 49, 26, 23, 217, 17, 181, 66, 12, 42, 87, 125, 140, 6, 215, 15, 62,
	0
};

#elif CRYPT_FRA_DEMO

#undef NO_CRYPT
#define CRYPT_KEY_LEN 69
#define CRYPT_RAND_SEED 251
#define CRYPT_OFFSET_MODULO 74
#define CRYPT_OFFSET_MODULO_VALUE 32
#define CRYPT_OFFSET_ADD 2
const char *version_branch_name = "fra_demo";

unsigned char ubercrypt_key[CRYPT_KEY_LEN+1] = 
{
	221, 214, 22, 48, 13, 21, 16, 134, 113, 73, 16, 253, 34, 235, 84, 13, 
	1, 12, 127, 27, 12, 56, 24, 128, 78, 93, 55, 24, 56, 151, 65, 66, 
	43, 23, 56, 34, 13, 23, 56, 234, 113, 73, 160, 253, 34, 235, 84, 13, 
	6, 41, 72, 197, 12, 56, 131, 128, 178, 93, 155, 134, 156, 15, 125, 162, 
	42, 1, 119, 123, 55,
	0
};

#elif CRYPT_ENG_DEMO

#undef NO_CRYPT
#define CRYPT_KEY_LEN 68
#define CRYPT_RAND_SEED 127
#define CRYPT_OFFSET_MODULO 46
#define CRYPT_OFFSET_MODULO_VALUE 7
#define CRYPT_OFFSET_ADD 1
const char *version_branch_name = "eng_demo";

unsigned char ubercrypt_key[CRYPT_KEY_LEN+1] = 
{
	21, 24, 22, 18, 13, 21, 16, 134, 113, 73, 16, 253, 34, 235, 84, 13, 
	1, 12, 127, 27, 12, 56, 24, 128, 78, 93, 55, 24, 56, 151, 65, 66, 
	43, 23, 56, 34, 13, 23, 56, 234, 113, 73, 160, 253, 34, 235, 84, 13, 
	6, 41, 72, 197, 12, 56, 131, 128, 178, 93, 155, 134, 156, 15, 125, 162, 
	42, 1, 119, 123,
	0
};

#elif CRYPT_ENG2_DEMO

#undef NO_CRYPT
#define CRYPT_KEY_LEN 68
#define CRYPT_RAND_SEED 227
#define CRYPT_OFFSET_MODULO 36
#define CRYPT_OFFSET_MODULO_VALUE 5
#define CRYPT_OFFSET_ADD 2
const char *version_branch_name = "eng2_demo";

unsigned char ubercrypt_key[CRYPT_KEY_LEN+1] = 
{
	11, 4, 122, 118, 113, 221, 216, 14, 18, 171, 16, 253, 34, 235, 84, 13, 
	43, 23, 56, 34, 13, 23, 56, 234, 113, 73, 160, 253, 34, 235, 84, 13, 
	1, 12, 127, 27, 12, 56, 24, 128, 78, 93, 55, 24, 56, 151, 65, 66, 
	6, 41, 72, 117, 12, 56, 119, 128, 178, 93, 15, 134, 156, 15, 125, 162, 
	42, 111, 71, 33,
	0
};



#elif CRYPT_UNITEDKINGDOM

#undef NO_CRYPT
#define CRYPT_KEY_LEN 70
#define CRYPT_RAND_SEED 163
#define CRYPT_OFFSET_MODULO 34
#define CRYPT_OFFSET_MODULO_VALUE 22
#define CRYPT_OFFSET_ADD 4
const char *version_branch_name = "unitedkingdom";

unsigned char ubercrypt_key[CRYPT_KEY_LEN+1] = 
{
	220, 214, 22, 48, 13, 21, 16, 134, 113, 73, 16, 253, 34, 235, 84, 13, 
	1, 12, 127, 27, 12, 56, 124, 128, 78, 93, 155, 24, 56, 151, 65, 66, 
	43, 23, 56, 34, 13, 23, 56, 234, 23, 13, 163, 253, 34, 235, 84, 13, 
	6, 41, 72, 197, 12, 56, 131, 128, 178, 93, 155, 134, 156, 15, 125, 162, 
	42, 1, 119, 123, 55, 25,
	0
};


#elif CRYPT_REPLAY

#undef NO_CRYPT
#define CRYPT_KEY_LEN 69
#define CRYPT_RAND_SEED 127
#define CRYPT_OFFSET_MODULO 73
#define CRYPT_OFFSET_MODULO_VALUE 14
#define CRYPT_OFFSET_ADD 2
const char *version_branch_name = "replay";

unsigned char ubercrypt_key[CRYPT_KEY_LEN+1] = 
{
	40, 14, 12, 41, 23, 21, 26, 164, 1, 13, 162, 32, 14, 15, 84, 13, 
	1, 22, 17, 27, 12, 56, 124, 128, 78, 93, 155, 24, 56, 151, 65, 66, 
	43, 23, 56, 34, 13, 23, 156, 234, 23, 13, 13, 253, 34, 35, 84, 13, 
	63, 41, 122, 197, 12, 251, 131, 128, 178, 93, 15, 134, 15, 125, 87, 162, 
	42, 1, 94, 123, 5, 
	0
};

#elif CRYPT_STEAM

#undef NO_CRYPT
#define CRYPT_KEY_LEN 71
#define CRYPT_RAND_SEED 721
#define CRYPT_OFFSET_MODULO 62
#define CRYPT_OFFSET_MODULO_VALUE 11
#define CRYPT_OFFSET_ADD 3
const char *version_branch_name = "steam";

unsigned char ubercrypt_key[CRYPT_KEY_LEN+1] = 
{
	15, 17, 120, 41, 23, 21, 26, 164, 1, 13, 162, 32, 14, 15, 84, 13, 
	43, 23, 56, 34, 13, 23, 156, 234, 23, 13, 13, 253, 34, 35, 84, 13, 
	1, 22, 17, 27, 12, 56, 124, 128, 78, 93, 155, 24, 56, 151, 65, 66, 
	63, 41, 122, 197, 12, 52, 32, 28, 128, 93, 15, 134, 15, 125, 87, 12, 
	42, 1, 94, 123, 5, 44, 83,
	0
};

#elif CRYPT_POLAND

#undef NO_CRYPT
#define CRYPT_KEY_LEN 68
#define CRYPT_RAND_SEED 424
#define CRYPT_OFFSET_MODULO 134
#define CRYPT_OFFSET_MODULO_VALUE 63
#define CRYPT_OFFSET_ADD 4
const char *version_branch_name = "poland";

unsigned char ubercrypt_key[CRYPT_KEY_LEN+1] = 
{
	119, 11, 73, 197, 12, 251, 131, 128, 178, 93, 15, 134, 15, 125, 87, 162, 
	40, 14, 12, 41, 23, 21, 26, 164, 1, 13, 162, 32, 14, 15, 84, 13, 
	31, 22, 17, 27, 12, 56, 124, 128, 78, 93, 155, 24, 56, 151, 65, 66, 
	43, 23, 56, 34, 213, 23, 156, 234, 23, 13, 13, 253, 34, 35, 84, 13, 
	42, 1, 94, 123, 
	0
};

#elif CRYPT_HUNGARY

#undef NO_CRYPT
#define CRYPT_KEY_LEN 67
#define CRYPT_RAND_SEED 251
#define CRYPT_OFFSET_MODULO 156
#define CRYPT_OFFSET_MODULO_VALUE 85
#define CRYPT_OFFSET_ADD 2
const char *version_branch_name = "hungary";

unsigned char ubercrypt_key[CRYPT_KEY_LEN+1] = 
{
	71, 152, 207, 27, 12, 56, 124, 128, 78, 93, 155, 32, 14, 15, 84, 13, 
	119, 11, 73, 197, 12, 251, 131, 128, 178, 93, 15, 134, 15, 125, 87, 162, 
	40, 14, 12, 41, 23, 21, 26, 164, 1, 13, 162, 24, 56, 151, 65, 66, 
	43, 23, 56, 34, 213, 23, 156, 234, 23, 13, 13, 253, 34, 35, 84, 13, 
	42, 1, 94,
	0
};

#elif CRYPT_USA

#undef NO_CRYPT
#define CRYPT_KEY_LEN 71
#define CRYPT_RAND_SEED 21
#define CRYPT_OFFSET_MODULO 63
#define CRYPT_OFFSET_MODULO_VALUE 35
#define CRYPT_OFFSET_ADD 1
const char *version_branch_name = "usa";

unsigned char ubercrypt_key[CRYPT_KEY_LEN+1] = 
{
	222, 214, 22, 48, 13, 21, 16, 134, 113, 73, 16, 253, 34, 235, 84, 13, 
	1, 12, 127, 27, 112, 56, 24, 128, 78, 93, 55, 24, 56, 151, 65, 66, 
	43, 23, 56, 34, 13, 223, 56, 234, 13, 73, 160, 253, 34, 235, 84, 13, 
	6, 41, 72, 197, 12, 56, 231, 28, 178, 93, 155, 134, 156, 15, 125, 162, 
	42, 1, 119, 123, 55, 25, 34,
	0
};

#elif CRYPT_FINLAND_DEMO


#undef NO_CRYPT
#define CRYPT_KEY_LEN 67
#define CRYPT_RAND_SEED 313
#define CRYPT_OFFSET_MODULO 124
#define CRYPT_OFFSET_MODULO_VALUE 5
#define CRYPT_OFFSET_ADD 1
const char *version_branch_name = "finland_demo";

unsigned char ubercrypt_key[CRYPT_KEY_LEN+1] = 
{
	13, 134, 176, 4, 13, 21, 56, 134, 113, 73, 16, 253, 34, 235, 84, 13, 
	1, 2, 7, 27, 12, 56, 24, 128, 78, 193, 55, 234, 56, 15, 65, 66, 
	43, 23, 56, 34, 13, 23, 56, 234, 113, 73, 160, 253, 34, 235, 84, 13, 
	6, 41, 72, 197, 12, 56, 3, 128, 178, 93, 155, 134, 56, 15, 125, 162, 
	42, 11, 119,
	0
};


#elif CRYPT_XXXXX


#undef NO_CRYPT
#define CRYPT_KEY_LEN 128
#define CRYPT_RAND_SEED 59
#define CRYPT_OFFSET_MODULO 157
#define CRYPT_OFFSET_MODULO_VALUE 9
#define CRYPT_OFFSET_ADD 4
const char *version_branch_name = "xxxxx";

unsigned char ubercrypt_key[CRYPT_KEY_LEN+1] = 
{
	43, 23, 56, 34, 13, 23, 56, 234, 143, 73, 16, 253, 34, 235, 84, 13, 
	43, 23, 56, 34, 13, 23, 56, 234, 113, 73, 16, 253, 34, 235, 84, 13, 
	1, 2, 7, 97, 12, 56, 34, 128, 178, 193, 55, 234, 56, 15, 65, 66, 
	6, 1, 72, 197, 12, 56, 3, 128, 178, 93, 55, 234, 56, 15, 25, 162, 
	1, 2, 7, 97, 12, 56, 34, 128, 178, 193, 55, 234, 56, 15, 65, 66, 
	43, 23, 56, 34, 13, 23, 56, 234, 143, 73, 16, 253, 34, 235, 84, 13, 
	6, 1, 72, 197, 12, 56, 3, 128, 178, 93, 55, 234, 56, 15, 125, 162, 
	43, 23, 56, 34, 13, 23, 56, 234, 113, 73, 16, 253, 34, 235, 84, 13, 
	0
};

#else


#define NO_CRYPT 1
#define CRYPT_KEY_LEN 1
#define CRYPT_RAND_SEED 5
#define CRYPT_OFFSET_MODULO 2
#define CRYPT_OFFSET_MODULO_VALUE 1
#define CRYPT_OFFSET_ADD 0
//const char *version_branch_name = "dev";

#endif


namespace util
{

	void UberCrypt::crypt(void *data, int length)
	{
#ifdef NO_CRYPT
		return;
#else
		game::GameRandom grand;
		grand.seed(CRYPT_RAND_SEED);

		char *datac = (char *)data;

		for (int i = 0; i < length; i++)
		{
			char curkey = grand.nextInt();
			curkey ^= ubercrypt_key[i % CRYPT_KEY_LEN];
			datac[i] = ((datac[i] ^ curkey) & 255);

			if ((i % CRYPT_OFFSET_MODULO) == CRYPT_OFFSET_MODULO_VALUE)
			{
				// WARNING: relying on char overflow
				datac[i] += CRYPT_OFFSET_ADD;
			}

			if (datac[i] == 66)
			{
				datac[i] = 126;
			} 
			else if (datac[i] == 126) 
			{
				datac[i] = 66;
			}

		}
#endif
	}

	void UberCrypt::decrypt(void *data, int length)
	{
#ifdef NO_CRYPT
		return;
#else
		game::GameRandom grand;
		grand.seed(CRYPT_RAND_SEED);

		char *datac = (char *)data;

		for (int i = 0; i < length; i++)
		{
			if (datac[i] == 66)
			{
				datac[i] = 126;
			} 
			else if (datac[i] == 126) 
			{
				datac[i] = 66;
			}

			if ((i % CRYPT_OFFSET_MODULO) == CRYPT_OFFSET_MODULO_VALUE)
			{
				// WARNING: relying on char overflow
				datac[i] -= CRYPT_OFFSET_ADD;
			}

			char curkey = grand.nextInt();
			curkey ^= ubercrypt_key[i % CRYPT_KEY_LEN];
			datac[i] = ((datac[i] ^ curkey) & 255);
		}
#endif
	}

}

