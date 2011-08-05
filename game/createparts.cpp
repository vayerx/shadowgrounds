
#include "precompiled.h"

#include "PartType.h"
#include "PartTypeParser.h"
#include "Leg.h"
#include "Arm.h"
#include "Head.h"
#include "Torso.h"
#include "Tool.h"
#include "Weapon.h"
#include "DirectWeapon.h"
#include "IndirectWeapon.h"
#include "Bullet.h"
#include "ItemPack.h"
#include "AmmoPack.h"
#include "PowerCell.h"
#include "Reactor.h"

#include "../util/SimpleParser.h"
#include "../system/Logger.h"
//#include "../editor/file_wrapper.h"
#include "../filesystem/file_package_manager.h"
#include "../filesystem/ifile_list.h"
#include "createparts.h"

#include "../util/Debug_MemoryManager.h"

using namespace frozenbyte;

namespace game
{

  /**
   * Creates base part types and runs parser for part data files.
   */

  void createPartTypes()
  {
    // these get stored to the linked list containing all id'ed parts
    
    Bullet *b = new Bullet();
    b->setPartTypeId(PARTTYPE_ID_STRING_TO_INT("Bull"));
    new Reactor(PARTTYPE_ID_STRING_TO_INT("Reac"));
    new Arm(PARTTYPE_ID_STRING_TO_INT("Arm"));
    new Leg(PARTTYPE_ID_STRING_TO_INT("Leg"));
    new Head(PARTTYPE_ID_STRING_TO_INT("Head"));
    new Torso(PARTTYPE_ID_STRING_TO_INT("Tors"));
    new Tool(PARTTYPE_ID_STRING_TO_INT("Tool"));
    Weapon *w = new Weapon();
    w->setPartTypeId(PARTTYPE_ID_STRING_TO_INT("Weap"));
    new DirectWeapon(PARTTYPE_ID_STRING_TO_INT("Dire"));
    new IndirectWeapon(PARTTYPE_ID_STRING_TO_INT("Indi"));
    // maybe need a new class for this...
    new Torso(PARTTYPE_ID_STRING_TO_INT("Soli"));

    new ItemPack(PARTTYPE_ID_STRING_TO_INT("Pack"));
    new AmmoPack(PARTTYPE_ID_STRING_TO_INT("Ammo"));
    new PowerCell(PARTTYPE_ID_STRING_TO_INT("Powe"));

    PartTypeParser ptp = PartTypeParser();

    util::SimpleParser sp;
#ifdef LEGACY_FILES
		if (sp.loadFile("Data/Parts/partlist.txt"))
#else
		if (sp.loadFile("data/part/partlist.txt"))
#endif
    {
      while (sp.next())
      {
        char *filename = sp.getLine();
        if (filename != NULL && filename[0] != '\0')
        {
					// THE OLD SYSTEM, READ PART FILE LIST FROM A FILE
          //ptp.loadPartTypes(filename);
					// THE NEW SYSTEM, READ ALL FILES FROM LISTED DIRECTORIES!
					// HACK: first read all _base files!


			boost::shared_ptr<filesystem::IFileList> baseFiles = filesystem::FilePackageManager::getInstance().findFiles(filename, "*_base.dhp");
			boost::shared_ptr<filesystem::IFileList> files = filesystem::FilePackageManager::getInstance().findFiles(filename, "*.dhp");

			std::vector<std::string> allBaseFiles;
			filesystem::getAllFiles(*baseFiles, filename, allBaseFiles, true);

			for (int j = 0; j < (int)allBaseFiles.size(); j++)
			{
				const char *realfilename = allBaseFiles[j].c_str();
				ptp.loadPartTypes(realfilename);
			}
			{
				std::vector<std::string> allFiles;
				filesystem::getAllFiles(*files, filename, allFiles, true);

				for (int i = 0; i < (int)allFiles.size(); i++)
				{
					bool isBaseFile = false;
					for (int j = 0; j < (int)allBaseFiles.size(); j++)
					{
						if (allFiles[i] == allBaseFiles[j])
						{
							isBaseFile = true;
							break;
						}
					}
					if (!isBaseFile)
					{
						const char *realfilename = allFiles[i].c_str();
						ptp.loadPartTypes(realfilename);
					}
				}
			}

			/*
					frozenbyte::editor::FileWrapper fwbase(
						std::string(filename), std::string("*_base.dhp"));
					std::vector<std::string> allBaseFiles = fwbase.getAllFiles();
					for (int j = 0; j < (int)allBaseFiles.size(); j++)
					{
						const char *realfilename = allBaseFiles[j].c_str();
						ptp.loadPartTypes(realfilename);
					}
					{
						frozenbyte::editor::FileWrapper fw(
							std::string(filename), std::string("*.dhp"));
						std::vector<std::string> allFiles = fw.getAllFiles();
						for (int i = 0; i < (int)allFiles.size(); i++)
						{
							bool isBaseFile = false;
							for (int j = 0; j < (int)allBaseFiles.size(); j++)
							{
								if (allFiles[i] == allBaseFiles[j])
								{
									isBaseFile = true;
									break;
								}
							}
							if (!isBaseFile)
							{
								const char *realfilename = allFiles[i].c_str();
								ptp.loadPartTypes(realfilename);
							}
						}
					}
			*/
        }
      }
    } else {
      Logger::getInstance()->error("createPartTypes - Unable to load partlist.");
    }
  }

  /**
   * Cleans up all part types.
   */

  void deletePartTypes()
  {
		PartTypeParser::clearLoadedList(".dhp");

    SafeLinkedListIterator iter = SafeLinkedListIterator(&partTypeIds);
    while (iter.iterateAvailable())
    {
      PartType *pt = (PartType *)iter.iterateNext();
      delete pt;
    }
  }


  void reloadPartTypeVisuals()
  {
    LinkedListIterator iter = LinkedListIterator(&partTypeIds);
    while (iter.iterateAvailable())
    {
      PartType *pt = (PartType *)iter.iterateNext();
      pt->deleteVisual();
    }
  }

  void restorePartTypeOriginals()
  {
    LinkedListIterator iter = LinkedListIterator(&partTypeIds);
    while (iter.iterateAvailable())
    {
      PartType *pt = (PartType *)iter.iterateNext();
      pt->restoreOriginals();
    }
	}

}

