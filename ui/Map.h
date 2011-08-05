#ifndef INCLUDED_UI_MAP_H
#define INCLUDED_UI_MAP_H

#include <DatatypeDef.h>
#include <boost/scoped_ptr.hpp>
#include <string>

class IStorm3D_Texture;

namespace game {
	class Game;
} // game

namespace ui {

class Map
{
	struct Data;
	boost::scoped_ptr<Data> data;

public:
	explicit Map(game::Game &game);
	~Map();

	void clearMapFog();

	void setMission(const std::string &dir);
	void startLayer(const std::string &id, const VC2 &start);
	void endLayer(const std::string &id, const VC2 &end);
	void loadLayer(const std::string &id);

	void update(const VC2 &player, int ms);
	void drawTo(IStorm3D_Texture &texture);

	const std::string &getMission() const;
	void getLayerCoordinates(VC2 &p1, VC2 &p2) const;

	void getPlayerCoordinates(const VC2 &ppos, float prot, VC2 &pos, float &rot);
};

} // ui

#endif
