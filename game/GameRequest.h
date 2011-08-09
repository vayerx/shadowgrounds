
#ifndef GAMEREQUEST_H
#define GAMEREQUEST_H

namespace game
{

  class GameRequest
  {
  public:
    GameRequest();
    virtual ~GameRequest();

    //virtual void processData(uint8_t *data) = 0;

    // executes this request (when it has become an order by master)
    virtual void execute();

  // todo, friend
  //protected:
    int requestId;
    int executeTime;
    int dataSize;
    uint8_t *data;
  };

}

#endif

