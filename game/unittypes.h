
#ifndef UNITTYPES_H
#define UNITTYPES_H

// --- claw proto unit actors ---

#ifdef PROJECT_CLAW_PROTO

#define UNIT_ACTOR_ARMOR 0
#define UNIT_ACTOR_CLAW 1

#define MAX_UNIT_ACTORS 2

#endif

// --- sg unit actors ---

#ifdef PROJECT_SHADOWGROUNDS

#define UNIT_ACTOR_ARMOR 0

#define MAX_UNIT_ACTORS 1

#endif

// --- survivor unit actors ---

#ifdef PROJECT_SURVIVOR

#define UNIT_ACTOR_ARMOR 0

#define MAX_UNIT_ACTORS 1

#endif

// --- aov unit actors ---

#ifdef PROJECT_AOV

#define UNIT_ACTOR_ARMOR 0
#define UNIT_ACTOR_SIDEWAYS 1

#define MAX_UNIT_ACTORS 2

#endif

// --- end of unit actors ---

namespace game
{

  class UnitActor;
  class Unit;
	class UnitType;
  class Game;


  /**
   * Return proper actor class instance for given unit.
   * The object returned is contained within this implementation.
   * A pointer to the internal object is returned, so do not delete
   * object yourself!
   */
  UnitActor *getUnitActorForUnit(Unit *unit);

  /**
   * Creates the unit actors.
   * Call at program init before creating the unit types.
   */ 
  void createUnitActors(Game *game);

  /**
   * To clean up unit actors.
   * Call when program is terminating.
   */ 
  void deleteUnitActors();

  /**
   * Creates the unit types.
   * Call at program init after creating the unit actors.
   */ 
  void createUnitTypes();

  /**
   * To clean up unit types.
   * Call when program is terminating.
   */ 
  void deleteUnitTypes();

	/**
	 * Create a new unittype instance of given classname.
	 */
	UnitType *getNewUnitTypeForUnitTypeName(const char *baseclass);


  extern UnitActor *unitActorArray[MAX_UNIT_ACTORS];

}

#endif

