
#ifndef UBERCRYPT_H
#define UBERCRYPT_H


namespace util
{

	class UberCrypt
	{
		public:
			static void crypt(void *data, int length);

			static void decrypt(void *data, int length);
	};

}


#endif

