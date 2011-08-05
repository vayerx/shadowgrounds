
#ifndef PARTTYPE_H
#define PARTTYPE_H

//
// This is the armor part type class. (the class of a part instance)
//

#include "../container/LinkedList.h"
#include "../ui/Visual2D.h"
#include "../ui/VisualObjectModel.h"

#include "damagetypes.h"

#include "../util/Debug_MemoryManager.h"


#define SLOT_POSITION_INVALID 0

#define SLOT_POSITION_HEAD 1
#define SLOT_POSITION_LEFT_ARM 2
#define SLOT_POSITION_RIGHT_ARM 3
#define SLOT_POSITION_LEFT_WAIST 4
#define SLOT_POSITION_RIGHT_WAIST 5
#define SLOT_POSITION_LEFT_SHOULDER 6
#define SLOT_POSITION_RIGHT_SHOULDER 7
#define SLOT_POSITION_LEFT_LEG 8
#define SLOT_POSITION_RIGHT_LEG 9
#define SLOT_POSITION_LEFT_HEAD 10
#define SLOT_POSITION_RIGHT_HEAD 11
#define SLOT_POSITION_EXTERNAL_ITEM 12
#define SLOT_POSITION_INTERNAL_ITEM 13
#define SLOT_POSITION_WEAPON_LEFT 14
#define SLOT_POSITION_WEAPON_RIGHT 15
#define SLOT_POSITION_WEAPON 16
#define SLOT_POSITION_BACK 17
#define SLOT_POSITION_CENTER 18

#define SLOT_POSITIONS 19

#ifdef PROJECT_SHADOWGROUNDS
//#undef PARTTYPE_ID_STRING_EXTENDED
#else
#define PARTTYPE_ID_STRING_EXTENDED
#endif

// id 3-8 char string -> id int, for intel byte order
// (this is actually a kind of a very primitive hash code calculation)
// notice that ABCDEFGH id int == EFGHABCD id int!
#ifdef PARTTYPE_ID_STRING_EXTENDED
#define PARTTYPE_ID_STRING_TO_INT(x) \
  ( \
    ((x)[4] == '\0' || (x)[3] == '\0') ? \
    ( \
      *((int *)(x)) \
    ) : ( \
			game::partTypeIdStringToIntConv(x) \
    ) \
  )
#else
#define PARTTYPE_ID_STRING_TO_INT(x) \
  ( \
    ((x)[4] == '\0' || (x)[3] == '\0') ? \
    ( \
      *((int *)(x)) \
    ) : ( \
      ((x)[5] == '\0' || (x)[6] == '\0') ? \
      ( \
        (*((int *)(x)) ^ (*((int *)&(x)[2]) >> 16)) \
      ) : ( \
        (*((int *)(x)) ^ *((int *)&(x)[4])) \
      ) \
    ) \
  )
#endif

// id int -> 3-8 (new: 3-12) char string 
// warning: the result string is not the same as the one the id int
// was generated from. the operation is not reversable.
// (id's are nolonger unique, cannnot go back from 32 bit to 64 bit)
#define PARTTYPE_ID_INT_TO_STRING(x) partTypeIdIntToStringConv(x)

// check for validity of id string
#ifdef PROJECT_SHADOWGROUNDS
#define PARTTYPE_ID_STRING_VALID(x) \
  (x != NULL && strlen(x) >= 3 && strlen(x) <= 8)
#else
#define PARTTYPE_ID_STRING_VALID(x) \
  (x != NULL && strlen(x) >= 3 && strlen(x) <= 12)
#endif


// tech levels
#define PARTTYPE_MAX_LEVEL 8
#define PARTTYPE_LEVEL_MASK_ALL 255

// subs (for data parsing)
#define PARTTYPE_SUB_NONE 0
#define PARTTYPE_SUB_RESISTANCE 1
#define PARTTYPE_SUB_PASS 2
#define PARTTYPE_SUB_ABSORB 3
#define PARTTYPE_SUB_SLOT 4


namespace game
{

  class Part;
  class PartTypeParser;

  
  // list of data file based part types
  // these are not objects with actual C++ classes, but objects with PartType
  // class. they however have an id number to identify
  // don't use directly, externed only for cleanup
  extern LinkedList partTypeIds;


  class PartType
  {
  public:
    PartType();
    
    // (destructor after private variables, because it uses them)

    // should return a new instance of this part 
    // (the corresponding class extending part)
    virtual Part *getNewPartInstance();

    int getPartTypeId() const;

    void setPartTypeId(int id, bool modifyList = true);

    const char *getPartTypeIdString() const;

    void setPartTypeIdString(const char *partTypeIdString);

    // parser calls this function to tell that we're in a sub conf
    // called with NULL key when exiting a sub conf
    // should return true if valid sub key
    virtual bool setSub(const char *key);

    // parser calls this function to configure the part type based on file
    // should return true if key and value pair was identified and valid
    virtual bool setData(char *key, char *value);

    // should return the 2d image of this part, NULL if it has none
    virtual ui::Visual2D *getVisual2D();
    // right hand side...
    virtual ui::Visual2D *getMirrorVisual2D();

    // return 3d model for the part, NULL if it has none
    virtual ui::VisualObjectModel *getVisualObjectModel();
    // right hand side...
    virtual ui::VisualObjectModel *getMirrorVisualObjectModel();

    // returns the name for this part
    virtual char *getName();

    // returns the short name for this part
    virtual char *getShortName();

    // returns the helper (attach to) name for this part
    //virtual char *getHelperName();

    // returns the description for this part
    virtual char *getDescription();

    // should return the amount of extension slots this part has
    virtual int getSlotAmount();

    // returns the parent part type (the class this one is derived from)
    virtual PartType *getParentType();

    // returns the valid type of a slot (sub part)
    virtual PartType *getSlotType(int slotNumber);

    // returns a bit mask representing valid tech levels
    virtual int getSlotLevelMask(int slotNumber);

    // returns the positioning of the slot
    virtual int getSlotPosition(int slotNumber);

    // returns true if this part type is inherited from given type
    bool isInherited(PartType *partType);

    // guess what this does ;)
    int getPrice();

    // get tech level (value with level bit set!)
    int getLevel();

    int getMaxDamage();

    int getMaxHeat();

    int getDestroyHeatPercentage();

    int getWeight();

    int getReconEffect();
    int getRunningEffect();
    int getStealthEffect();

    char *getBonesFilename();

    int getResistance(int damageType);
    int getDamagePass(int damageType);
    int getDamageAbsorb(int damageType);

    int getArmorRating();

		void deleteVisual();

		virtual void saveOriginals();
		void restoreOriginals();

  protected:
    ui::Visual2D *image;
    ui::Visual2D *mirrorImage;
    char *imageFilename;
    char *mirrorImageFilename;

    ui::VisualObjectModel *visualObjectModel;
    ui::VisualObjectModel *mirrorVisualObjectModel;
    char *modelFilename;
    char *mirrorModelFilename;
    
    int atSub; // used at parsing state...
    int atSlot; // ...

    char *name;
    char *shortname;
    //char *helpername;
    char *desc;
    int price;
    int level;

		char *bonesFilename;
		int reconEffect;
		int runningEffect;
		int stealthEffect;

    int weight;

    int partTypeId;
		char *partTypeIdString;

    PartType *parentType;
    PartType **slotTypes;
    int *slotPositions;
    int *slotLevelMask; // acceptable tech levels (bits)

    // for colored status window with all parts?
    // int statSlotX;
    // int statSlotY;
    // ui::Visual2D *statImage; // need multiple images? different colors?

    int slotAmount;

    int maxDamage; // max damage before totally destroyed
    int maxHeat;   // added to unit's max heat
    int destroyHeat; // max unit's heat _percentage_ before part destroyed

    // resistance - how much of the inflicted damage is "reflected" off 
    // by the part. (absolute values, not percentages?)
    int resistance[DAMAGE_TYPES_AMOUNT];

    // pass - how much of the inflicted damage is passed to sub parts
    // percentage, after resistance, divided equally between sub parts
    int damagePass[DAMAGE_TYPES_AMOUNT];

    // absorb - how much of the inflicted damage is converted to final damage
    // percentage
    int damageAbsorb[DAMAGE_TYPES_AMOUNT];

    // the armor rating that will be added to unit's total armor rating
    int armorRating;

		PartType *originals;

    // root level methods, pass setSub and setData to these in extending 
    // classes if the data is not recognized by the extending class.
    bool setRootSub(const char *key);
    bool setRootData(const char *key, char *value);

  public:
    // to get a new cloned parttype for a new inherited parttype
    // correction: actually does not create a new one, instead
    // takes some possibly inherited object and sets it accordingly
    virtual void prepareNewForInherit(PartType *partType);

    virtual ~PartType() 
    { 
			if (originals != NULL)
			{
				PartType *orig = originals;
				originals = NULL;
				delete orig;
			}
      if (partTypeId != 0)
      {
        partTypeIds.remove(this);
      }
      if (partTypeIdString != NULL) { delete [] partTypeIdString; partTypeIdString = NULL; }
      if (image != NULL) { delete image; image = NULL; }
      if (mirrorImage != NULL) { delete mirrorImage; mirrorImage = NULL; }
      if (visualObjectModel != NULL) { delete visualObjectModel; visualObjectModel = NULL; }
      if (mirrorVisualObjectModel != NULL) { delete mirrorVisualObjectModel; mirrorVisualObjectModel = NULL; }
      if (name != NULL) { delete [] name; name = NULL; }
      if (shortname != NULL) { delete [] shortname; shortname = NULL; }
      //if (helpername != NULL) { delete [] helpername; helpername = NULL; }
      if (desc != NULL) { delete [] desc; desc = NULL; }
      if (imageFilename != NULL) { delete [] imageFilename; imageFilename = NULL; }
      if (mirrorImageFilename != NULL) { delete [] mirrorImageFilename; mirrorImageFilename = NULL; }
      if (modelFilename != NULL) { delete [] modelFilename; modelFilename = NULL; }
      if (mirrorModelFilename != NULL) { delete [] mirrorModelFilename; mirrorModelFilename = NULL; }
      if (slotTypes != NULL) { delete [] slotTypes; slotTypes = NULL; }
      if (slotPositions != NULL) { delete [] slotPositions; slotPositions = NULL; }
      if (slotLevelMask != NULL) { delete [] slotLevelMask; slotLevelMask = NULL; }
      if (bonesFilename != NULL) { delete [] bonesFilename; bonesFilename = NULL; }
    };


    friend class PartTypeParser;
  };

  // the root part type (never to be used directly, only subclasses used)
  extern PartType partType;

  extern PartType *getPartTypeById(int id);

  extern const char *slotPositionStrings[SLOT_POSITIONS];

#ifdef PARTTYPE_ID_STRING_EXTENDED
  extern int partTypeIdStringToIntConv(const char *idstr);
#endif

  // for conversion from id int to id string
  extern char *partTypeIdIntToStringConv(int id);

  // don't use directly, externed only for cleanup
  //extern LinkedList partTypeIds;
}

#endif

