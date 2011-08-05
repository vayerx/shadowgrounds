
#include "precompiled.h"

#include "Part.h"

#include "../system/Logger.h"
#include "../util/Debug_MemoryManager.h"

namespace game
{

  Part::Part()
  {
    parent = NULL;
    partType = NULL;
    children = new Part *[MAX_PART_CHILDREN];
    for (int i = 0; i < MAX_PART_CHILDREN; i++)
    {
      children[i] = NULL;
    }
    damage = 0;
    heat = 0;
    purchasePending = false;
    owner = NO_PART_OWNER;
    visualObject = NULL;
  }

  Part::~Part()
  {
    delete [] children;
  }

  SaveData *Part::getSaveData() const
  {
    // TODO
    return NULL;
  }

	const char *Part::getStatusInfo() const 
	{
		static std::string weapon_status_info_buf;
		weapon_status_info_buf = std::string("WeaponObject");
		//weapon_status_info_buf += PARTTYPE_ID_STRING_TO_INT(this->partType->getPartTypeId());
		//weapon_status_info_buf += ")";
		return weapon_status_info_buf.c_str();
	}

  PartType *Part::getType()
  {
    return partType;
  }

  void Part::setType(PartType *partType)
  {
    this->partType = partType;
  }

  // monkey code ;)

  int Part::getOwner()
  {
    return owner;
  }

  void Part::setOwner(int player)
  {
    owner = player;
  }

  Part *Part::getParent()
  {
    return parent;
  }

  /*
  // this is handled by setSubPart
  void Part::setParent(Part *part)
  {
    parent = part;
  }
  */

    // returns a child part in given slot number or NULL if does not exist
  Part *Part::getSubPart(int slotNumber)
  {
    if (slotNumber < 0 || slotNumber >= MAX_PART_CHILDREN) 
		{
			Logger::getInstance()->error("Part::getSubPart - Slot number out of range.");
			assert(!"Part::getSubPart - Slot number out of range.");
			return NULL;
		}

    return children[slotNumber];
  }

  void Part::setSubPart(int slotNumber, Part *part)
  {
    if (slotNumber < 0 || slotNumber >= MAX_PART_CHILDREN) 
		{
			Logger::getInstance()->error("Part::setSubPart - Slot number out of range.");
			assert(!"Part::setSubPart - Slot number out of range.");
			return;
		}

    if (children[slotNumber] != NULL)
      children[slotNumber]->parent = NULL;

    children[slotNumber] = part;

    if (part != NULL)
      part->parent = this;
  }

  bool Part::isPurchasePending()
  {
    return purchasePending;
  }

  void Part::setPurchasePending(bool pending)
  {
    purchasePending = pending;
  }

  int Part::getDamage()
  {
    return damage;
  }

  int Part::getRepairPrice()
  {
    if (partType->getMaxDamage() == 0) return 0;
    int ret = (damage * partType->getPrice() / partType->getMaxDamage()) / 2;
    // damaged parts always costs at least 1
    if (damage > 0 && ret == 0) ret = 1;
    return ret;
  }

  void Part::repair()
  {
    damage = 0;
  }

  void Part::addDamage(int damage)
  {
    this->damage += damage;
    if (damage > partType->getMaxDamage())
      damage = partType->getMaxDamage();
  }

  void Part::setVisualObject(ui::VisualObject *visualObject)
  {
    this->visualObject = visualObject;
  }

  ui::VisualObject *Part::getVisualObject()
  {
    return visualObject;
  }

}

