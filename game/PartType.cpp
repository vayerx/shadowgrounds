
#include "precompiled.h"

#include <stdlib.h>  // for abort
#include "../container/LinkedList.h"
#include "PartType.h"
#include "Part.h"
#include "../system/Logger.h"
#include "../convert/str2int.h"

#include "../util/Debug_MemoryManager.h"

namespace game
{

  PartType::PartType()
  {
    partTypeId = 0;
    partTypeIdString = NULL;
    atSub = PARTTYPE_SUB_NONE;
    atSlot = 0;
    image = NULL;
    mirrorImage = NULL;
    imageFilename = NULL;
    mirrorImageFilename = NULL;
    visualObjectModel = NULL;
    mirrorVisualObjectModel = NULL;
    modelFilename = NULL;
    mirrorModelFilename = NULL;
    name = NULL;
    shortname = NULL;
    //helpername = NULL;
    desc = NULL;
    slotAmount = 0;
    parentType = NULL;
    slotTypes = NULL;
    slotPositions = NULL;
    slotLevelMask = NULL;
    maxDamage = 1;
    maxHeat = 1;
    destroyHeat = 100;
    price = 0;
    weight = 0;
    level = PARTTYPE_LEVEL_MASK_ALL;
		for (int dmg = 0; dmg < DAMAGE_TYPES_AMOUNT; dmg++)
		{
			resistance[dmg] = 0;
			damagePass[dmg] = 0;
			damageAbsorb[dmg] = 0;
		}
    armorRating=0;
		reconEffect=0;
		runningEffect=0;
		stealthEffect=0;
    bonesFilename = NULL;
		originals = NULL;
  }

  void PartType::prepareNewForInherit(PartType *ret)
  {
    //PartType *ret = new PartType();

    if (imageFilename != NULL)
    {
			if (ret->imageFilename != NULL)
			{
				delete [] ret->imageFilename;
				ret->imageFilename = NULL;
			}
      ret->imageFilename = new char[strlen(imageFilename) + 1];
      strcpy(ret->imageFilename, imageFilename);
    }
    if (mirrorImageFilename != NULL)
    {
			if (ret->mirrorImageFilename != NULL)
			{
				delete [] ret->mirrorImageFilename;
				ret->mirrorImageFilename = NULL;
			}
      ret->mirrorImageFilename = new char[strlen(mirrorImageFilename) + 1];
      strcpy(ret->mirrorImageFilename, mirrorImageFilename);
    }

    if (modelFilename != NULL)
    {
			if (ret->modelFilename != NULL)
			{
				delete [] ret->modelFilename;
				ret->modelFilename = NULL;
			}
      ret->modelFilename = new char[strlen(modelFilename) + 1];
      strcpy(ret->modelFilename, modelFilename);
    }
    if (mirrorModelFilename != NULL)
    {
			if (ret->mirrorModelFilename != NULL)
			{
				delete [] ret->mirrorModelFilename;
				ret->mirrorModelFilename = NULL;
			}
      ret->mirrorModelFilename = new char[strlen(mirrorModelFilename) + 1];
      strcpy(ret->mirrorModelFilename, mirrorModelFilename);
    }

    ret->price = price;
    ret->level = level;

    if (bonesFilename != NULL)
    {
			if (ret->bonesFilename != NULL)
			{
				delete [] ret->bonesFilename;
				ret->bonesFilename = NULL;
			}
      ret->bonesFilename = new char[strlen(bonesFilename) + 1];
      strcpy(ret->bonesFilename, bonesFilename);
    }

    ret->reconEffect = reconEffect;
    ret->runningEffect = runningEffect;
    ret->stealthEffect = stealthEffect;

    ret->weight = weight;

    //ret->parentType = this;

    // WARNING: inheriting any slots makes it impossible to
    // add/modify/remove them later on!
    if (slotAmount > 0 && slotTypes != NULL)
    {
			if (ret->slotTypes != NULL)
			{
				delete [] ret->slotTypes;
				ret->slotTypes = NULL;
				delete [] ret->slotPositions;
				ret->slotPositions = NULL;
				delete [] ret->slotLevelMask;
				ret->slotLevelMask = NULL;
			}
      ret->slotAmount = slotAmount;
      ret->slotTypes = new PartType *[slotAmount];
      ret->slotPositions = new int[slotAmount];
      ret->slotLevelMask = new int[slotAmount];
      for (int i = 0; i < slotAmount; i++) 
      {
        ret->slotTypes[i] = slotTypes[i];
        ret->slotPositions[i] = slotPositions[i];
        ret->slotLevelMask[i] = slotLevelMask[i];
      }
    }

    ret->maxDamage = maxDamage;
    ret->maxHeat = maxHeat;
    ret->destroyHeat = destroyHeat;

    ret->armorRating = armorRating;

    // WARNING: resistance, damagePass and damageAbsorb ignored!

    //return ret;
  }

	void PartType::restoreOriginals()
	{
		if (originals != NULL)
		{
			// FIXME: this may not be correct way to do this!
			originals->prepareNewForInherit(this);
		} else {
			//assert(!"PartType::restoreOriginals - Attempt to restore when no originals saved.");
		}
	}

	void PartType::saveOriginals()
	{
		// WARNING: ANY PART TYPE SUBCLASS THAT OVERRIDES prepareNewForInherit,
		// MUST ALSO OVERRIDE THIS saveOriginals
		if (originals == NULL)
		{
			originals = new PartType();
		} else {
			assert(!"PartType::saveOriginals - Attempt to save originals multiple times.");
		}

		// FIXME: this may not be correct way to do this!
		this->prepareNewForInherit(originals);
	}

  void PartType::setPartTypeId(int partTypeId, bool modifyList)
  {
    if (this->partTypeId != 0 && modifyList)
    {
      partTypeIds.remove(this);
    }
    this->partTypeId = partTypeId;
    if (partTypeId != 0 && modifyList)
    {
      partTypeIds.append(this);
    }
  }

  int PartType::getPartTypeId() const
  {
    return partTypeId;
  }

  void PartType::setPartTypeIdString(const char *partTypeIdString)
  {
		if (this->partTypeIdString != NULL)
		{
			delete [] this->partTypeIdString;
			this->partTypeIdString = NULL;
		}
		if (partTypeIdString != NULL)
		{
			this->partTypeIdString = new char[strlen(partTypeIdString) + 1];
			strcpy(this->partTypeIdString, partTypeIdString);
		}
  }

  const char *PartType::getPartTypeIdString() const
  {
    return partTypeIdString;
  }

  bool PartType::setData(char *key, char *value)
  {
    return setRootData(key, value);
  }

  bool PartType::setSub(const char *key)
  {
    return setRootSub(key);
  }

  // TODO: leaks memory...
  // (but that is not important, these are global variables, 
  // inited only once)
  bool PartType::setRootData(const char *key, char *value)
  {
    // the über sophisticated data parsing ;)

    if (atSub == PARTTYPE_SUB_NONE)
    {
      if (strcmp(key, "id") == 0)
      {
        if (!PARTTYPE_ID_STRING_VALID(value)) 
				{
					Logger::getInstance()->error("PartType::setRootData - Part type id string is invalid.");
					return false;
				}

        PartType *prev = getPartTypeById(PARTTYPE_ID_STRING_TO_INT(value));
        if (prev != NULL)
        {
					Logger::getInstance()->error("PartType::setRootData - Part type id already used.");
          return false;
        }

        setPartTypeId(PARTTYPE_ID_STRING_TO_INT(value));
        setPartTypeIdString(value);
        return true;
      }
      if (strcmp(key, "type") == 0)
      {
        if (!PARTTYPE_ID_STRING_VALID(value)) 
				{
					Logger::getInstance()->error("PartType::setRootData - Part parent type string is invalid.");
					return false;
				}

        int val = PARTTYPE_ID_STRING_TO_INT(value);
        PartType *parent = getPartTypeById(val);
        if (parent == NULL) 
				{
					Logger::getInstance()->error("PartType::setRootData - Part parent type does not exist or has not yet been loaded.");
					return false;
				}
        parentType = parent;
        parentType->prepareNewForInherit(this);
        return true;
      }
      if (strcmp(key, "image") == 0)
      {
        if (imageFilename != NULL) delete [] imageFilename;
        imageFilename = new char[strlen(value) + 1];
        strcpy(imageFilename, value);
        return true;
      }
      if (strcmp(key, "model") == 0)
      {
        if (modelFilename != NULL) delete [] modelFilename;
        modelFilename = new char[strlen(value) + 1];
        strcpy(modelFilename, value);
        return true;
      }
      if (strcmp(key, "mirrorimage") == 0)
      {
        if (mirrorImageFilename != NULL) delete [] mirrorImageFilename;
        mirrorImageFilename = new char[strlen(value) + 1];
        strcpy(mirrorImageFilename, value);
        return true;
      }
      if (strcmp(key, "mirrormodel") == 0)
      {
        if (mirrorModelFilename != NULL) delete [] mirrorModelFilename;
        mirrorModelFilename = new char[strlen(value) + 1];
        strcpy(mirrorModelFilename, value);
        return true;
      }
      if (strcmp(key, "name") == 0)
      {
        if (name != NULL) delete [] name;
        name = new char[strlen(value) + 1];
        strcpy(name, value);
        return true;
      }
      if (strcmp(key, "shortname") == 0)
      {
        if (shortname != NULL) delete [] shortname;
        shortname = new char[strlen(value) + 1];
        strcpy(shortname, value);
        return true;
      }
      /*
      if (strcmp(key, "helpername") == 0)
      {
        if (helpername != NULL) delete [] helpername;
        helpername = new char[strlen(value) + 1];
        strcpy(helpername, value);
        return true;
      }
      */
      if (strcmp(key, "desc") == 0)
      {
        if (desc != NULL) delete [] desc;
        desc = new char[strlen(value) + 1];
        strcpy(desc, value);
        int slen = strlen(desc);
        for (int i = 0; i < slen; i++)
        {
          if (desc[i] == '\\') desc[i] = '\n';
        }
        return true;
      }
      if (strcmp(key, "desc+") == 0)
      {
        if (desc == NULL) return false;
        char *olddesc = desc;
        int olddesclen = strlen(olddesc);
        desc = new char[olddesclen + strlen(value) + 1];
        strcpy(desc, olddesc);
        strcpy(&desc[olddesclen], value);
        delete [] olddesc;
        int slen = strlen(desc);
        for (int i = 0; i < slen; i++)
        {
          if (desc[i] == '\\') desc[i] = '\n';
        }
        return true;
      }
      if (strcmp(key, "maxdamage") == 0)
      {
        maxDamage = str2int(value);
        if (maxDamage == 0) return false;
        return true;
      }
      if (strcmp(key, "maxheat") == 0)
      {
        maxHeat = str2int(value);
        if (maxHeat == 0) return false;
        return true;
      }
      if (strcmp(key, "destroyheat") == 0)
      {
        destroyHeat = str2int(value);
        if (destroyHeat == 0) return false;
        return true;
      }
      if (strcmp(key, "armorrating") == 0)
      {
        armorRating = str2int(value);
        return true;
      }
      if (strcmp(key, "reconeffect") == 0)
      {
        reconEffect = str2int(value);
        return true;
      }
      if (strcmp(key, "runningeffect") == 0)
      {
        runningEffect = str2int(value);
        return true;
      }
      if (strcmp(key, "stealtheffect") == 0)
      {
        stealthEffect = str2int(value);
        return true;
      }
      if (strcmp(key, "bonesfilename") == 0)
      {
        if (bonesFilename != NULL) delete [] bonesFilename;
        bonesFilename = new char[strlen(value) + 1];
        strcpy(bonesFilename, value);
        return true;
      }
      if (strcmp(key, "price") == 0)
      {
        price = str2int(value);
        if (price == 0) return false;
        return true;
      }
      if (strcmp(key, "weight") == 0)
      {
        weight = str2int(value);
        return true;
      }
      if (strcmp(key, "level") == 0)
      {
        int val = str2int(value);
        if (val <= 0 || val > PARTTYPE_MAX_LEVEL) return false;
        level = 1 << (val - 1);  // set proper level bit
        return true;
      }
      if (strcmp(key, "slots") == 0)
      {
        slotAmount = str2int(value);
        if (slotAmount != 0)
        {
          // TODO: delete old slots

					if (slotAmount > MAX_PART_CHILDREN)
					{
						Logger::getInstance()->error("PartType::setRootData - Slot amount over maximum limit.");
						assert(!"PartType::setRootData - Slot amount over maximum limit.");
					}

          slotTypes = new PartType *[slotAmount];
          slotPositions = new int[slotAmount];
          slotLevelMask = new int[slotAmount];
          for (int i = 0; i < slotAmount; i++) 
          {
            slotTypes[i] = NULL;
            slotPositions[i] = SLOT_POSITION_INVALID;
            slotLevelMask[i] = 0;
          }
        } else {
          slotTypes = NULL;
          slotPositions = NULL;
          slotLevelMask = NULL;
        }
        return true;
      }
    }

    // damage values for each type...
    int isdam = DAMAGE_TYPE_INVALID;
    if (strcmp(key, "projectile") == 0)
    {
      isdam = DAMAGE_TYPE_PROJECTILE;
    }
    if (strcmp(key, "heat") == 0)
    {
      isdam = DAMAGE_TYPE_HEAT;
    }
    if (strcmp(key, "electric") == 0)
    {
      isdam = DAMAGE_TYPE_ELECTRIC;
    }
    if (strcmp(key, "stun") == 0)
    {
      isdam = DAMAGE_TYPE_STUN;
    }
    int damval = str2int(value);
    if (isdam != DAMAGE_TYPE_INVALID)
    {
      if (atSub == PARTTYPE_SUB_RESISTANCE)
      {
        resistance[isdam] = damval;
        return true;
      }
      if (atSub == PARTTYPE_SUB_PASS)
      {
        damagePass[isdam] = damval;
        return true;
      }
      if (atSub == PARTTYPE_SUB_ABSORB)
      {
        damageAbsorb[isdam] = damval;
        return true;
      }
    }
    
    // slots
    if (atSub == PARTTYPE_SUB_SLOT)
    {
      if (strcmp(key, "type") == 0)
      {
        if (!PARTTYPE_ID_STRING_VALID(value)) return false;

        int val = PARTTYPE_ID_STRING_TO_INT(value);

        PartType *pt = getPartTypeById(val);
        if (pt == NULL) return false;
        slotTypes[atSlot] = pt;
        return true;
      }
      if (strcmp(key, "position") == 0)
      {
        for (int i = 0; i < SLOT_POSITIONS; i++)
        {
          if (strcmp(value, slotPositionStrings[i]) == 0)
          {
            slotPositions[atSlot] = i;
            return true;
          }
        }
        return false;
      }
      if (strcmp(key, "level+") == 0)
      {
        int val = str2int(value);
        if (val <= 0 || val > PARTTYPE_MAX_LEVEL) return false;
        slotLevelMask[atSlot] += 1 << (val - 1);
        return true;
      }
    }
 
    return false;
  }

  bool PartType::setRootSub(const char *key)
  {
    if (key == NULL)
    {
      if (atSub == PARTTYPE_SUB_SLOT)
      {
        atSlot++;
      } 
      atSub = PARTTYPE_SUB_NONE;
      return true;
    }
    if (strcmp(key, "resistance") == 0)
    {
      atSub = PARTTYPE_SUB_RESISTANCE;
      return true;
    }
    if (strcmp(key, "damagepass") == 0)
    {
      atSub = PARTTYPE_SUB_PASS;
      return true;
    }
    if (strcmp(key, "damageabsorb") == 0)
    {
      atSub = PARTTYPE_SUB_ABSORB;
      return true;
    }
    if (strcmp(key, "slot") == 0)
    {
      if (atSlot >= slotAmount)
      {
        return false;
      }
      atSub = PARTTYPE_SUB_SLOT;
      return true;
    }
    return false;
  }

  Part *PartType::getNewPartInstance()
  {
    if (partTypeId != 0)
    {
      // TODO: might actually want to create instances of some other 
      // classes than this based on the id
      Part *ret = new Part();
      ret->setType(this);
      return ret;
    } else {
      // not allowed to create instances of the abstract parttype 
      // must define some id for it, then it's allowed
      abort();
    }
    return NULL;
  } 

  bool PartType::isInherited(PartType *partType)
  {
    PartType *tmp = this;
    int failsafe = 0;
    while ((tmp = tmp->getParentType()) != NULL)
    {
      if (tmp == partType) return true;
      failsafe++;
      if (failsafe > 999)
      {
        // we seem to have some kind of weird cyclic inheritance!
        abort();
      }
    }
    return false;
  }

  ui::Visual2D *PartType::getVisual2D()
  {
    if (image == NULL && imageFilename != NULL)
    {
      image = new ui::Visual2D(imageFilename);
    }
    return image;
  }

  ui::Visual2D *PartType::getMirrorVisual2D()
  {
    if (mirrorImage == NULL && mirrorImageFilename != NULL)
    {
      mirrorImage = new ui::Visual2D(mirrorImageFilename);
    }
    return mirrorImage;
  }

  ui::VisualObjectModel *PartType::getVisualObjectModel()
  {
    if (visualObjectModel == NULL)
    {
      if (modelFilename != NULL)
      {
        visualObjectModel = new ui::VisualObjectModel(modelFilename);
      }
    }
    return visualObjectModel;
  }

  ui::VisualObjectModel *PartType::getMirrorVisualObjectModel()
  {
    if (mirrorVisualObjectModel == NULL)
    {
      if (mirrorModelFilename != NULL)
      {
        mirrorVisualObjectModel = new ui::VisualObjectModel(mirrorModelFilename);
      }
    }
    return mirrorVisualObjectModel;
  }

  char *PartType::getName()
  {
    return name;
  }

  char *PartType::getShortName()
  {
    return shortname;
  }

  /*
  char *PartType::getHelperName()
  {
    return helpername;
  }
  */

  char *PartType::getDescription()
  {
    return desc;
  }

  //virtual VisualObject *PartType::getVisualObject();

  int PartType::getSlotAmount()
  {
    return slotAmount;
  }

  PartType *PartType::getParentType()
  {
    return parentType;
  }

  PartType *PartType::getSlotType(int slotNumber)
  {
    if (slotNumber < 0 || slotNumber > slotAmount) abort();
    if (slotTypes == NULL) abort();
    return slotTypes[slotNumber];
  }

  int PartType::getSlotPosition(int slotNumber)
  {
    if (slotNumber < 0 || slotNumber > slotAmount) abort();
    if (slotPositions == NULL) abort();
    return slotPositions[slotNumber];
  }

  int PartType::getSlotLevelMask(int slotNumber)
  {
    if (slotNumber < 0 || slotNumber > slotAmount) abort();
    if (slotLevelMask == NULL) abort();
    return slotLevelMask[slotNumber];
  }

  int PartType::getPrice() 
  { 
    return price; 
  }

  int PartType::getLevel() 
  { 
    return level; 
  }

  int PartType::getMaxDamage() 
  { 
    return maxDamage; 
  }

  int PartType::getMaxHeat() 
  { 
    return maxHeat; 
  }

  int PartType::getDestroyHeatPercentage() 
  { 
    return destroyHeat; 
  }

  int PartType::getArmorRating() 
  { 
    return armorRating; 
  }

  int PartType::getWeight() 
  { 
    return weight; 
  }

  int PartType::getReconEffect() 
  { 
    return reconEffect; 
  }

  int PartType::getRunningEffect() 
  { 
    return runningEffect; 
  }

  int PartType::getStealthEffect() 
  { 
    return stealthEffect; 
  }

  char *PartType::getBonesFilename()
  {
    return bonesFilename;
  }

  int PartType::getResistance(int damageType)
  {
    return resistance[damageType];
  }

  int PartType::getDamagePass(int damageType)
  {
    return damagePass[damageType];
  }

  int PartType::getDamageAbsorb(int damageType)
  {
    return damageAbsorb[damageType];
  }

	void PartType::deleteVisual()
	{
	  if (visualObjectModel != NULL) { delete visualObjectModel; visualObjectModel = NULL; }
    if (mirrorVisualObjectModel != NULL) { delete mirrorVisualObjectModel; mirrorVisualObjectModel = NULL; }	
	}


  PartType partType = PartType();

  // TODO: more optimal data structure for searches
  LinkedList partTypeIds = LinkedList();


  PartType *getPartTypeById(int id)
  {
    LinkedListIterator iter = LinkedListIterator(&partTypeIds);
    while (iter.iterateAvailable())
    {
      PartType *pt = (PartType *)iter.iterateNext();
      if (pt->getPartTypeId() == id) return pt;
    }
    return NULL;
  }

  const char *slotPositionStrings[SLOT_POSITIONS] =
  {
    "(RESERVED)",
    "Head",
    "LeftArm",
    "RightArm",
    "LeftWaist",
    "RightWaist",
    "LeftShoulder",
    "RightShoulder",
    "LeftLeg",
    "RightLeg",
    "LeftHead",
    "RightHead",
    "ExternalItem",
    "InternalItem",
    "WeaponLeft",
    "WeaponRight",
    "Weapon",
    "Back",
    "Center"
  };

#ifdef PARTTYPE_ID_STRING_EXTENDED
  int partTypeIdStringToIntConv(const char *idstr)
  {
    int ret = *((int *)idstr);

    if (idstr[4] == '\0' || idstr[3] == '\0')
			return ret;

#ifndef NDEBUG
		if (idstr[0] == '\0'
			|| idstr[1] == '\0'
			|| idstr[2] == '\0')
		{
			assert(!"partTypeIdIntToStringConv - invalid part type id string (length < 3).");
			return 0;
		}
#endif

		if (idstr[5] == '\0'
			|| idstr[6] == '\0')
		{
			ret ^= ((*((int *)&idstr[2])) >> 16);
			return ret;
		}

		if (idstr[7] == '\0'
			|| idstr[8] == '\0')
		{
			ret ^= (*((int *)&idstr[4]));
			return ret;
		}

		if (idstr[9] == '\0'
			|| idstr[10] == '\0')
		{
			ret ^= (*((int *)&idstr[4]));
			ret ^= ((*((int *)&idstr[6])) >> 16);
			return ret;
		}

		assert(idstr[11] == '\0' || idstr[12] == '\0');

		ret ^= (*((int *)&idstr[4]));
		ret ^= (*((int *)&idstr[8]));

		return ret;
	}

  char partTypeConvBuf[12+1];

  char *partTypeIdIntToStringConv(int id)
  {
		assert(!"partTypeIdIntToStringConv - deprecated, do not use!");

    *((int *)partTypeConvBuf) = id ^ (1<<7) ^ (1<<15) ^ (1<<23) ^ (1<<31);
    *((int *)&partTypeConvBuf[4]) = (1<<7) ^ (1<<15) ^ (1<<23) ^ (1<<31);
    partTypeConvBuf[8] = '\0';
    return partTypeConvBuf;
  }
#else
  char partTypeConvBuf[8+1];

  char *partTypeIdIntToStringConv(int id)
  {
    *((int *)partTypeConvBuf) = id ^ (1<<7) ^ (1<<15) ^ (1<<23) ^ (1<<31);
    *((int *)&partTypeConvBuf[4]) = (1<<7) ^ (1<<15) ^ (1<<23) ^ (1<<31);
    partTypeConvBuf[8] = '\0';
    return partTypeConvBuf;
  }
#endif
}

