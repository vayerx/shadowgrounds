
#ifndef SAVEDATA_H
#define SAVEDATA_H

typedef unsigned char BYTE;

namespace game
{

  class GameObject;


  class SaveData
  {
  public:
    SaveData(int id, int size, BYTE *data, int childAmount = 0, 
      GameObject **children = NULL);
    ~SaveData();

  // public just for easy access in save routines, don't modify directly
  // private:
    int id;
    int size;
    BYTE *data;
    GameObject **children;
  };

}

#endif

