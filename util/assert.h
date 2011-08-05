#ifndef INCLUDEDED_FB_ASSERT_H
#define INCLUDEDED_FB_ASSERT_H

#include <cassert>

namespace frozenbyte {

void assertImp(const char *predicate, const char *file, int line);

} // frozenbyte

// General
#define FB_ASSERT_IMP(predicate) \
	do \
	{ \
		if(!(predicate)) \
		{ \
			::frozenbyte::assertImp(#predicate, __FILE__, __LINE__); \
			assert(predicate); \
		} \
	} \
	while(false)

// ToDO: 
// logic to choose assert/expensive assert imps depending on build settings

#ifdef FB_TESTBUILD
	#define FB_ASSERT(predicate) \
		FB_ASSERT_IMP(predicate)
	#define FB_EXPENSIVE_ASSERT(predicate)
#elif NDEBUG
	#define FB_ASSERT(predicate)
	#define FB_EXPENSIVE_ASSERT(predicate)
#else
	#define FB_ASSERT(predicate) \
		FB_ASSERT_IMP(predicate)
	#define FB_EXPENSIVE_ASSERT(predicate) \
		FB_ASSERT_IMP(predicate)
#endif

#endif
