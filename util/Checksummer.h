
#ifndef CHECKSUMMER_H
#define CHECKSUMMER_H

namespace util
{
  class Checksummer
	{
		public:
			static unsigned int countChecksumForFile(const char *filename);

			static bool doesChecksumAndSizeMatchFile(unsigned int checksum, int filesize, const char *filename);

		private:
			static bool countChecksumForFileImpl(unsigned int *checksum, int *filesize, const char *filename);
			
	};

}

#endif

