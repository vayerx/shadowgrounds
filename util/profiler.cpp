#include "precompiled.h"

#include "profiler.h"
#include "../system/Logger.h"
#include <vector>
#include <cassert>
#include <stack>
#include <algorithm>

#ifdef WIN32
#  include <windows.h>
#elif defined(XBOX)
#  include <xtl.h>
#elif defined(__GLIBC__)
#  include <sys/time.h>
#endif

#if defined(FB_PROFILER)

namespace frozenbyte {
    namespace profiler {
        namespace {
#  ifdef _MSC_VER
            typedef long long          TimeType;
#  elif defined(__GLIBC__)
            typedef unsigned long long TimeType;
#  endif

            TimeType getCurrentTime()
            {
#  ifdef _MSC_VER
                LARGE_INTEGER r;
                r.QuadPart = 0;
                QueryPerformanceCounter(&r);
                return r.QuadPart;
#  elif defined(__GLIBC__)
                struct timeval tv;
                gettimeofday(&tv, 0);
                return (TimeType)tv.tv_sec * 1000000 + tv.tv_usec;
#  endif
            }

            TimeType getCurrentFrequency()
            {
#  ifdef _MSC_VER
                LARGE_INTEGER r;
                r.QuadPart = 0;
                QueryPerformanceFrequency(&r);
                return r.QuadPart / 1000;
#  elif defined(__GLIBC__)
                return 1000;
#  endif
            }

            struct Identifier {
                // __function__ is static const char*
                const char *className;
                const char *id;
                const char *part;
                // Total stats
                TimeType    time;
                TimeType    callCount;

                // Active info
                TimeType    enterTime;
                Identifier *parent;

                std::vector<Identifier *> identifiers;

                Identifier()
                    :   className(0),
                    id(0),
                    part(0),
                    time(0),
                    callCount(0),
                    enterTime(0),
                    parent(0)
                {
                }

                Identifier *getIdentifier(const char *className, const char *id, const char *part)
                {
                    // Recursion not supported atm
                    //assert(id != this->id);

                    // Already exists? (needs something more efficient?)
                    unsigned int idAmount = identifiers.size();
                    for (unsigned int i = 0; i < idAmount; ++i) {
                        Identifier *idl = identifiers[i];

                        // ToDo: Pointers should be the same without string compare
                        //if(strcmp(idl->id, id) == 0)
                        if (idl->id == id && idl->className == className && idl->part == part) {
                            assert(idl->parent == this);
                            return idl;
                        }
                    }

                    int index = identifiers.size();
                    identifiers.resize(index + 1);

                    Identifier *i = new Identifier();
                    identifiers[index] = i;
                    i->className = className;
                    i->id = id;
                    i->part = part;
                    i->parent = this;

                    return i;
                }
            };

            struct IdentifierSorter {
                bool operator () (const Identifier *a, const Identifier *b) const
                {
                    return a->time > b->time;
                }
            };

            // This leaks the stack
            Identifier dataStack;
            Identifier *current = &dataStack;

        } // unnmed

        void pushIdentifier(const char *id)
        {
            pushIdentifier(0, id);
            /*
               current = current->getIdentifier(id);
               assert(current != current->parent);
               assert(current->id == id);
               //FB_PRINTF("Adding: %s\n", id);

               ++current->callCount;
               current->enterTime = getCurrentTime();
             */
        }

        void pushIdentifier(const char *className, const char *id)
        {
            //pushIdentifier(id);
            //current->className = className;
            pushIdentifier(className, id, 0);
        }

        void pushIdentifier(const char *className, const char *id, const char *part)
        {
            current = current->getIdentifier(className, id, part);
            assert(current != current->parent);
            assert(current->id == id);
            assert(current->className == className);
            assert(current->part == part);
            //FB_PRINTF("Adding: %s\n", id);

            ++current->callCount;
            current->enterTime = getCurrentTime();
        }

        void popIdentifier()
        {
            Identifier *c = current;
            //FB_PRINTF("Removing: %s\n", current->id);

            c->time += getCurrentTime() - c->enterTime;
            current = c->parent;
            assert(c);
            assert(c != c->parent);
        }

        void popIdentifier(const char *className, const char *id)
        {
            Identifier *c = current;
            assert(c != c->parent);
            assert(className == c->className);
            assert(id == c->id);

            popIdentifier();
        }

        // --

        void printStack(Identifier *id, int spaces)
        {
            std::sort( id->identifiers.begin(), id->identifiers.end(), IdentifierSorter() );
            TimeType profiledTime = 0;

            for (unsigned int i = 0; i < id->identifiers.size(); ++i) {
                Identifier *idl = id->identifiers[i];

                std::string line;
                char buffer[255] =  { 0 };

                for (int j = 0; j < spaces * 4; ++j) {
                    line += ' ';
                }

                int timeSpaces = 60 - strlen(idl->id);

                // MSC includes class info automagically
#  ifndef _MSC_VER
                if (idl->className) {
                    line += idl->className;
                    line += ":";
                    timeSpaces -= strlen(idl->className) + 1;
                }
#  endif

                line += idl->id;

                if (idl->part) {
                    line += ":";
                    line += idl->part;
                    timeSpaces -= strlen(idl->part) + 1;
                }

                for (int j = 0; j < timeSpaces; ++j) {
                    line += ' ';
                }

                unsigned int time = (unsigned int) idl->time / getCurrentFrequency();
                sprintf(buffer, "%.5dms ", time);
                line += buffer;

                sprintf( buffer, "(%d) ", int(idl->callCount) );
                line += buffer;

                sprintf(buffer, "%.5fms", (double)time / idl->callCount);
                line += buffer;

                LOG_ERROR( line.c_str() );
                printf( "%s\n", line.c_str() );
                printStack(idl, spaces + 1);

                profiledTime += idl->time;
            }

            // Print missing time
            if ( id->time > profiledTime && !id->identifiers.empty() ) {
                std::string line;
                char buffer[255] =  { 0 };

                for (int j = 0; j < spaces * 4; ++j) {
                    line += ' ';
                }

                char *idString = "-- missing --";
                int timeSpaces = 60 - strlen(idString);
                line += idString;

                for (int j = 0; j < timeSpaces; ++j) {
                    line += ' ';
                }

                unsigned int time = (unsigned int) (id->time - profiledTime) / getCurrentFrequency();
                sprintf(buffer, "%.5dms", time);
                line += buffer;

                LOG_ERROR( line.c_str() );
                printf( "%s\n", line.c_str() );
            }
        }

        void printStats(const char *info)
        {
            LOG_ERROR("--------------------------");
            LOG_ERROR("Profiling statistics for:");
            LOG_ERROR(info);

            printStack(current, 1);

            LOG_ERROR("End profiling statistics");
            LOG_ERROR("--------------------------");
        }

        // --

        void deleteBranch(Identifier *id)
        {
            for (unsigned int i = 0; i < id->identifiers.size(); ++i) {
                deleteBranch(id->identifiers[i]);
            }

            id->identifiers.clear();
            delete id;
        }

        void resetStats()
        {
            current->time = 0;
            current->callCount = 1;
            current->enterTime = getCurrentTime();

            // Clear current childs
            for (unsigned int i = 0; i < current->identifiers.size(); ++i) {
                deleteBranch(current->identifiers[i]);
            }
            current->identifiers.clear();

            // Clear parent braches, except the current one
            if (current->parent) {
                for (unsigned int i = 0; i < current->parent->identifiers.size(); ++i) {
                    Identifier *idl = current->parent->identifiers[i];
                    if (idl != current)
                        deleteBranch(idl);
                }

                // Leave only the current one
                current->parent->identifiers.clear();
                current->parent->identifiers.push_back(current);
            }

            // ToDo: To be reall safe(tm) we should clear the timins for whole hierarchy ..
        }
    }
}

#endif
