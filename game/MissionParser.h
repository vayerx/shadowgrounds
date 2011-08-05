
#ifndef MISSIONPARSER_H
#define MISSIONPARSER_H

//
// A really quick hack mission data parser
// for now, just to get the parts available to player to be read from data files.
//

//
// v0.1 - 27.5.2002 - jpkokkon
//


#define MISSIONPARSER_SECTION_NONE 0
#define MISSIONPARSER_SECTION_BEFORE 1
#define MISSIONPARSER_SECTION_COMBAT 2
#define MISSIONPARSER_SECTION_AFTER 3

namespace game
{

  class Game;


  class MissionParser
  {
  public:
    MissionParser();
    ~MissionParser();

    // each mission as it's own data file? sounds great ;)
    void parseMission(Game *game, char *filename, int section);

  private:
    void error(const char *err, int linenum, bool isError = true);
  };

}

#endif

