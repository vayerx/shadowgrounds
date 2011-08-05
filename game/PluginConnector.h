
#ifndef PLUGINCONNECTOR_H
#define PLUGINCONNECTOR_H

namespace game
{
  class Game;

  class PluginConnector
  {
  public:
    PluginConnector(Game *game, char *port);

    ~PluginConnector();

    void run();

    void sendCustomData(void *data, int length);

  private:
    Game *game;
  };

}

#endif

