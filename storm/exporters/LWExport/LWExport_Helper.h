// Copyright 2002-2004 Frozenbyte Ltd.

#ifndef INCLUDED_LWEXPORT_HELPER_H
#define INCLUDED_LWEXPORT_HELPER_H

#ifndef INCLUDED_STRING
#define INCLUDED_STRING
#include <string>
#endif

#ifndef INCLUDED_LWMESHES_H
#define INCLUDED_LWMESHES_H
#include <lwmeshes.h>
#endif
#ifndef INCLUDED_LWRENDER_H
#define INCLUDED_LWRENDER_H
#include <lwrender.h>
#endif

namespace frozenbyte {
namespace exporter {

class LWHelper
{
	LWItemID lwId;
	LWItemID parentId;

	std::string name;
	std::string parentName;

public:
	explicit LWHelper(LWItemID id);
	~LWHelper();

	bool collectData();
};

} // end of namespace export
} // end of namespace frozenbyte

#endif
