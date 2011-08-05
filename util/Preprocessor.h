
#ifndef UTIL_PREPROCESSOR_H
#define UTIL_PREPROCESSOR_H

namespace util
{
	class Preprocessor
	{
	public:
		static bool preprocess(const char *preprocessorCheck, const char *preprocessorCommand,
			const char *inputFilename, const char *outputFilename);
	};
}

#endif

