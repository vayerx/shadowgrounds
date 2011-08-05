#ifndef INCLUDED_UI_MAP_WINDOW_H
#define INCLUDED_UI_MAP_WINDOW_H

#include <DatatypeDef.h>
#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>
#include <string>

class Ogui;

namespace game {
	class Game;
} // game

namespace ui {

class Map;

class MapWindow
{
	struct Data;
	boost::scoped_ptr<Data> data;

public:
	MapWindow(game::Game &game, Ogui &ogui, boost::shared_ptr<Map> &map);
	~MapWindow();

	enum Entity
	{
		Player,
		Checkpoint
	};

	void setEntity(Entity entity, const VC2 &position, float rotation);
	void setActiveLayer(const std::string &id);
	void update(int ms);
	void show();
	void hide();
	void raise();

	enum ObjectiveType
	{
		Primary,
		Secondary
	};

	void setObjectivePoint(const std::string &id, const VC3 &position, float radius);
	void setObjectivePointLayer(const std::string &id, const std::string &layer);
	void addObjective(ObjectiveType type, const std::string &id);
	void addActiveObjective(const std::string &id);
	void completeObjective(const std::string &id);
	void removeObjective(const std::string &id);
	void setMissionId(const std::string &id);

	void addPortal(const std::string &fromLayer, const std::string &toLayer, const VC3 &fromPosition, const VC3 &toPosition);
	void removePortal(const std::string &fromLayer, const std::string &toLayer, const VC3 &fromPosition, const VC3 &toPosition);
	void updateMissionObjectiveToPosition(const std::string &id, const VC3 &position);

	bool isVisible() const;
	int getFadeInTime() const;
	int getFadeOutTime() const;
	const std::string &getActiveLayer() const;

	bool getCurrentObjectivePosition(VC3 &position) const;
	bool doesMissionObjectiveExist(const std::string &id) const;
	bool isMissionObjectiveComplete(const std::string &id) const;

	void effectUpdate(int delta);

	void clearMapFog();
};

} // ui

#endif
