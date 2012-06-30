#ifndef INCLUDED_PROFILER_H
#define INCLUDED_PROFILER_H

//#if !defined(FINAL_RELEASE)
//    #define FB_PROFILER
//#endif

// Temp!
//#define FB_PROFILER

namespace frozenbyte {
    namespace profiler {
#if defined(FB_PROFILER)
        // Do not use this imp directly, always use the macros!
        void pushIdentifier(const char *id) __attribute__( ( visibility("default") ) );
        void pushIdentifier(const char *className, const char *id) __attribute__( ( visibility("default") ) );
        void pushIdentifier(const char *className, const char *id,
                            const char *part) __attribute__( ( visibility("default") ) );
        void popIdentifier() __attribute__( ( visibility("default") ) );
        void popIdentifier(const char *className, const char *id) __attribute__( ( visibility("default") ) );
        void printStats(const char *info) __attribute__( ( visibility("default") ) );
        void resetStats() __attribute__( ( visibility("default") ) );

        struct ProfilerImplementationClass {
            const char *className;
            const char *idName;
            ProfilerImplementationClass(const char *id)
                :   className(0),
                idName(id)
            {
                pushIdentifier(id);
            }

            ProfilerImplementationClass(const char *className_, const char *id)
                :   className(className_),
                idName(id)
            {
                pushIdentifier(className, id);
            }

            ProfilerImplementationClass(const char *className_, const char *id, const char *part)
                :   className(className_),
                idName(id)
            {
                pushIdentifier(className, id, part);
            }

            ~ProfilerImplementationClass()
            {
                release();
            }

            void release()
            {
                if (idName) {
                    popIdentifier(className, idName);
                    idName = 0;
                    className = 0;
                }
            }
        };

#  define FB_PROFILE_FUNC()                           frozenbyte::profiler::ProfilerImplementationClass \
    profilerImplementationClass(__FUNCTION__)
#  define FB_PROFILE_CLASS_FUNC(className)            frozenbyte::profiler::ProfilerImplementationClass \
    profilerImplementationClass(className, __FUNCTION__)
#  define FB_PROFILE_FUNC_END()                       profilerImplementationClass.release();
#  define FB_PROFILE_PRINT_STATS(info)                frozenbyte::profiler::printStats(info)
#  define FB_PROFILE_RESET_STATS()                    frozenbyte::profiler::resetStats()
#  define FB_PROFILE_FUNC_PART(part)                  frozenbyte::profiler::ProfilerImplementationClass part( \
        0, \
        __FUNCTION__, \
        # part)
#  define FB_PROFILE_CLASS_FUNC_PART(className, part) frozenbyte::profiler::ProfilerImplementationClass part( \
        className, \
        __FUNCTION__, \
        # part)

        // It is REQUIRED to always call end() when using custom id profiling.
        // Otherwise the state of internal stack are undefined.
        // Also, there can be no other active id's which are already active and are not destroyed before this one
#  define FB_PROFILE_CUSTOM_ID(id)   frozenbyte::profiler::pushIdentifier(id)
#  define FB_PROFILE_CUSTOM_ID_END() frozenbyte::profiler::popIdentifier()
#else
#  define FB_PROFILE_FUNC()
#  define FB_PROFILE_CLASS_FUNC(className)
#  define FB_PROFILE_FUNC_END()
#  define FB_PROFILE_PRINT_STATS(info)
#  define FB_PROFILE_RESET_STATS()
#  define FB_PROFILE_CUSTOM_ID(id)
#  define FB_PROFILE_CUSTOM_ID_END()
#endif

    }
}

#endif
