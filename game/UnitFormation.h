#ifndef INCLUDED_UNIT_FORMATION_H
#define INCLUDED_UNIT_FORMATION_H

#include <vector>
#include "Unit.h"

namespace game {

struct UnitFormationData;
class Unit;
class GameMap;
class GameScene;

class UnitFormation
{
	UnitFormationData *data;

	// Not implemented
	UnitFormation(const UnitFormation &rhs);
	UnitFormation &operator = (const UnitFormation &rhs);

public:
	enum FormationType
	{
		FormationFoo,
		FormationBar
	};

	// Creators

	UnitFormation();
	~UnitFormation();

	void setGameMap(GameMap *gameMap);

	void setGameScene(GameScene *gameScene);

	// Mutators

	// Call from tactical mode
	void addMovePoint(std::vector<Unit *> *units, const VC3 &scaledMapPos, Unit::MoveType moveType);
	// Call from 'normal' mode.
	void setMovePoint(std::vector<Unit *> *units, const VC3 &scaledMapPos);
	
	// Call from both
	void setTarget(std::vector<Unit *> *units, const VC3 &targetPosition, Unit::FireType fireType);
	void setTarget(std::vector<Unit *> *units, Unit *targetUnit, Unit::FireType fireType);

	void setFormation(FormationType formation);

	void clearTarget(std::vector<Unit *> *units);

	void clearMovePoint(std::vector<Unit *> *units);

	// Accessors

	FormationType getFormation() const;
};

} // end of namespace game

#endif
