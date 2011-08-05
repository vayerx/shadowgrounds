
#ifndef PLUGINSTARTER_H
#define PLUGINSTARTER_H

namespace game
{
  class PluginManager;

  class PluginStarter
  {
  public:
    static void startPlugin(PluginManager *pman, char *pluginname);
  };

}

#endif

