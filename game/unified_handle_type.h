
#ifndef UNIFIED_HANDLE_TYPE_H
#define UNIFIED_HANDLE_TYPE_H

// note, this must be int because of scripting system implementation that uses ints
// (cannot be int64 or something alike, unless scripting system uses those as int numbers)

typedef int UnifiedHandle;

#endif
