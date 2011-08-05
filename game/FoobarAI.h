
#ifndef FOOBARAI_H
#define FOOBARAI_H

namespace game
{
  class Game;
}

class FoobarAI
{
public:
  FoobarAI(game::Game *game, int player);
  void doStuff();
private:
  game::Game *game;
  int player;
};

#endif

