

#ifndef PLUGINMANAGER_H
#define PLUGINMANAGER_H

namespace game
{
  class Game;
  class PluginConnector;

  class PluginManager
  {
  public:
    PluginManager(Game *game);

    ~PluginManager();

  private:
    LinkedList *pluginConnectors;
  };

}

#endif

