
#include "precompiled.h"

#include "ueoh_to_id_string.h"
#include <assert.h>
#include <string.h>


char ueoh_to_id_string_buf[8+1];

const char *ueoh_to_id_string(UniqueEditorObjectHandle ueoh)
{
	assert(sizeof(UniqueEditorObjectHandle) == 8);
	assert(ueoh & 0x8000000000000000);

	ueoh &= ~0x8000000000000000;

	for (int i = 0; i < 8; i++)
	{
		ueoh_to_id_string_buf[i] = ((char *)&ueoh)[i];
	}
	ueoh_to_id_string_buf[8] = '\0';

	return ueoh_to_id_string_buf;
}



UniqueEditorObjectHandle id_string_to_ueoh(const char *str)
{
	if (str == NULL)
	{
		assert(!"id_string_to_ueoh - null string");
		return 0;
	}
	if (strlen(str) > 8)
	{
		assert(!"id_string_to_ueoh - string too long.");
		return 0;
	}
	if (strlen(str) == 8
		&& (str[7] & 0x80) != 0)
	{
		assert(!"id_string_to_ueoh - characters should be 7 bit (especially first one in 8 char strings).");
		return 0;
	}

	assert(sizeof(UniqueEditorObjectHandle) == 8);
	char tmp[8+1];
	for (int i = 0; i < 8+1; i++)
	{
		tmp[i] = '\0';
	}
	strcpy(tmp, str);
	
	UniqueEditorObjectHandle ret = *((UniqueEditorObjectHandle *)tmp);
	ret |= 0x8000000000000000;

	return ret;
}

