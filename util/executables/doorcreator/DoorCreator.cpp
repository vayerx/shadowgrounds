
#include "DoorCreator.h"

#include "../../fb_assert.h"
#include "../../TextFileModifier.h"
#include "../../../system/FileTimeStampChecker.h"
#include "../../../convert/str2int.h"


#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>


void DoorCreator::replaceStrings(util::TextFileModifier &tfm, const char *name, const char *partType, const char *
	modelFilename, bool singleSide, const char *leftLayer, const char *rightLayer,
	const char *partTypeLowerCase, int blockRadius, unsigned int timeStamp)
{
	tfm.replaceString("<DOOR_PART_TYPE>", partType, false);	
	tfm.replaceString("<DOOR_NAME>", name, false);	
	tfm.replaceString("<DOOR_MODEL_FILENAME>", modelFilename, false);	
	tfm.replaceString("<DOOR_LEFT_SIDE_LAYER>", leftLayer, false);
	tfm.replaceString("<DOOR_RIGHT_SIDE_LAYER>", rightLayer, false);
	if (singleSide)
	{
		tfm.replaceString("<DOOR_SINGLE_SIDE>", "1", false);
		tfm.replaceString("<DOOR_NAME_LEFT_POSTFIX>", "", false);
		tfm.replaceString("<DOOR_NAME_RIGHT_POSTFIX>", ", invalid", false);
	} else {
		tfm.replaceString("<DOOR_SINGLE_SIDE>", "0", false);
		tfm.replaceString("<DOOR_NAME_LEFT_POSTFIX>", ", left", false);
		tfm.replaceString("<DOOR_NAME_RIGHT_POSTFIX>", ", right", false);
	}
	tfm.replaceString("<DOOR_BLOCK_RADIUS>", int2str(blockRadius), false);	
	tfm.replaceString("<DOOR_UNIT_ID>", int2str(timeStamp), false);

	// macro include file?
	if (tfm.setStartSelectionNearMarker("<-- DOOR_MACRO_INCLUDE_MARKER -->"))
	{
		if (tfm.setEndSelectionNearMarker("<-- DOOR_MACRO_INCLUDE_MARKER -->"))
		{
			std::string tmp = std::string("#include \"Doors/door_macro_") + partTypeLowerCase + ".h\"\r\n";
			tfm.addBeforeSelection(tmp.c_str());
		}
	}

	// editor unit conf include file?
	if (tfm.setStartSelectionNearMarker("<-- DOOR_UNIT_INCLUDE_MARKER -->"))
	{
		if (tfm.setEndSelectionNearMarker("<-- DOOR_UNIT_INCLUDE_MARKER -->"))
		{
			std::string tmp = std::string("#include \"Doors/Units_door_") + partTypeLowerCase + ".fbt\"\r\n";
			tfm.addBeforeSelection(tmp.c_str());
		}
	}

	// part type file, need to cut right side off (single sided)?
	if (singleSide)
	{
		if (tfm.setStartSelectionNearMarker("<-- DOOR_SINGLE_SIDE_CUT_MARKER -->"))
		{
			tfm.setEndSelectionToEnd();
			tfm.deleteSelection();
		}
	}

}


bool DoorCreator::createDoor(const char *name, const char *partType, const char *
	modelFilename, bool singleSide, const char *leftLayer, const char *rightLayer,
	int blockRadius)
{
	fb_assert(name != NULL);
	fb_assert(partType != NULL);
	fb_assert(modelFilename != NULL);
	fb_assert(leftLayer != NULL);
	fb_assert(rightLayer != NULL);
	
	fb_assert(strlen(partType) >= 4 && strlen(partType) <= 7);

  time_t ltime;
	time(&ltime);

	unsigned int timeStamp = ltime;

	int slen = strlen(partType);
	char *partTypeLowerCase = new char[strlen(partType) + 1]; // (this leaks, so what.)
	for (int i = 0; i < slen; i++)
	{
		partTypeLowerCase[i] = tolower(partType[i]);
	}
	partTypeLowerCase[slen] = '\0';

#ifdef LEGACY_FILES
	//std::string doorCreatePropertiesFilename = std::string("Editor/door_create_properties.txt");
	std::string doorMacroTemplateFilename = std::string("Editor/Dev/door_macro_template.txt");
	std::string doorMacroIncludesFilename = std::string("Editor/Dev/door_macro_includes.h");
	std::string doorMacroFilename = std::string("Editor/Dev/Doors/door_macro_") + partTypeLowerCase + ".h";
	std::string doorEditorUnitFilename = std::string("Editor/Units/Doors/Units_door_") + partTypeLowerCase + ".fbt";
	std::string doorEditorUnitTemplateFilename = std::string("Editor/Units/Doors/Units_door_template.txt");
	std::string doorEditorUnitIncludesFilename = std::string("Editor/Units/Units_doors.fbt");
	std::string doorPartFilename = std::string("Data/Parts/Solids/Doors/") + partTypeLowerCase + ".dhp";
	std::string doorPartTemplateFilename = std::string("Data/Parts/Solids/Doors/door_template.txt");
	std::string doorUnitFilename = std::string("Data/Units/") + partTypeLowerCase + ".dhu";
	std::string doorUnitTemplateFilename = std::string("Data/Units/door_template.txt");
#else
	//std::string doorCreatePropertiesFilename = std::string("editor/door_create_properties.txt");
	std::string doorMacroTemplateFilename = std::string("editor/dev/door_macro_template.txt");
	std::string doorMacroIncludesFilename = std::string("editor/dev/door_macro_includes.h");
	std::string doorMacroFilename = std::string("editor/dev/doors/door_macro_") + partTypeLowerCase + ".h";
	std::string doorEditorUnitFilename = std::string("editor/units/doors/units_door_") + partTypeLowerCase + ".fbt";
	std::string doorEditorUnitTemplateFilename = std::string("editor/units/doors/units_door_template.txt");
	std::string doorEditorUnitIncludesFilename = std::string("editor/units/units_doors.fbt");
	std::string doorPartFilename = std::string("data/parts/solids/doors/") + partTypeLowerCase + ".dhp";
	std::string doorPartTemplateFilename = std::string("data/parts/solids/doors/door_template.txt");
	std::string doorUnitFilename = std::string("data/units/") + partTypeLowerCase + ".dhu";
	std::string doorUnitTemplateFilename = std::string("data/units/door_template.txt");
#endif

	FILE *f = fopen(doorPartFilename.c_str(), "rb");
	if (f != NULL)
	{
		fclose(f);
		return false;
	}

	util::TextFileModifier tfm;
	bool success = false;

	// create door part type conf file
	success = tfm.loadFile(doorPartTemplateFilename.c_str());
	fb_assert(success);
	replaceStrings(tfm, name, partType, modelFilename, singleSide, leftLayer, rightLayer,
		partTypeLowerCase, blockRadius, timeStamp);
	tfm.saveFileAs(doorPartFilename.c_str());
	tfm.closeFile();

	// create door unit type conf file
	success = tfm.loadFile(doorUnitTemplateFilename.c_str());
	fb_assert(success);
	replaceStrings(tfm, name, partType, modelFilename, singleSide, leftLayer, rightLayer,
		partTypeLowerCase, blockRadius, timeStamp);
	tfm.saveFileAs(doorUnitFilename.c_str());
	tfm.closeFile();

	// create door macro file
	success = tfm.loadFile(doorMacroTemplateFilename.c_str());
	fb_assert(success);
	replaceStrings(tfm, name, partType, modelFilename, singleSide, leftLayer, rightLayer,
		partTypeLowerCase, blockRadius, timeStamp);
	tfm.saveFileAs(doorMacroFilename.c_str());
	tfm.closeFile();

	// create door editor unit conf file
	success = tfm.loadFile(doorEditorUnitTemplateFilename.c_str());
	fb_assert(success);
	replaceStrings(tfm, name, partType, modelFilename, singleSide, leftLayer, rightLayer,
		partTypeLowerCase, blockRadius, timeStamp);
	tfm.saveFileAs(doorEditorUnitFilename.c_str());
	tfm.closeFile();

	// add include line to editor dev macros
	success = tfm.loadFile(doorMacroIncludesFilename.c_str());
	fb_assert(success);
	replaceStrings(tfm, name, partType, modelFilename, singleSide, leftLayer, rightLayer,
		partTypeLowerCase, blockRadius, timeStamp);
	tfm.saveFileAs(doorMacroIncludesFilename.c_str());
	tfm.closeFile();

	// add include line to editor unit confs
	success = tfm.loadFile(doorEditorUnitIncludesFilename.c_str());
	fb_assert(success);
	replaceStrings(tfm, name, partType, modelFilename, singleSide, leftLayer, rightLayer,
		partTypeLowerCase, blockRadius, timeStamp);
	tfm.saveFileAs(doorEditorUnitIncludesFilename.c_str());
	tfm.closeFile();

	return true;
}

