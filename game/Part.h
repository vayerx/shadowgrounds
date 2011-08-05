
#ifndef PART_H
#define PART_H

//
// This is the armor part class.
//

// An instance of this class represents an instance of an armor part in 
// the game. PartType class represents the type (class) of an armor part.
// Try not to mix the concepts! Understanding this difference is crucial.

// PartType (/ extending classes) != Part (/ extending classes)
// "Part instanceof PartType" in game world


#include "GameObject.h"
#include "PartType.h"

#include "../ui/VisualObject.h"


#define NO_PART_OWNER -19561

#define MAX_PART_CHILDREN 40


namespace game
{

  class Part : public GameObject
  {
  public:
    Part();

    ~Part();

    // should return data needed to save the part
    virtual SaveData *getSaveData() const;

		virtual const char *getStatusInfo() const;
    
    // returns type of this part (the "class" of this part)
    virtual PartType *getType();

    // sets the type of this part (general advice: don't use this.)
    // only useful after creating a new instance.
    virtual void setType(PartType *partType);

    // returns the parent part (the one that this is attached to)
    Part *getParent();

    // TODO: there should be no need for this, calling setSubPart for the
    // new parent must set the objects parent 
    // void setParent(Part *part);

    // get the player number owning this part
    int getOwner();

    void setOwner(int player);

    // returns a child part in given slot number or NULL if does not exist
    Part *getSubPart(int slotNumber);

    void setSubPart(int slotNumber, Part *part);

    bool isPurchasePending();

    void setPurchasePending(bool pending);

    int getDamage();

    void addDamage(int damage);

    int getRepairPrice();

    void repair();

    // sets the visual object for this part
    // NOTICE: the part does not delete it in destructor (maybe should?)
    virtual void setVisualObject(ui::VisualObject *visualObject);

    virtual ui::VisualObject *getVisualObject();

  protected:
    Part *parent;
    Part **children;

    PartType *partType;

    ui::VisualObject *visualObject;

    int damage;
    int heat;

    int owner;

    // not yet paid for, deleted upon exit from armor construction
    bool purchasePending; 

  };

}

#endif

