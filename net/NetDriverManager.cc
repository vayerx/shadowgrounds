
#include <assert.h>
#include <string.h>
#include "NetDriverManager.h"

namespace net
{

  void NetDriverManager::registerDriver(INetDriver *driver)
  {
    if (driverList == NULL)
    {
      driverList = new LinkedList();
    }
    driverList->append(driver);
  }

  INetDriver *NetDriverManager::getDriver(char *name)
  {
    assert(name != NULL);
    if (driverList == NULL)
    {
      return NULL;
    }
    LinkedListIterator iter = LinkedListIterator(driverList);
    while (iter.iterateAvailable())
    {
      INetDriver *tmp = (INetDriver *)iter.iterateNext();

      const char *tmpname = tmp->getDriverName();
      if (tmpname != NULL)
      {
        if (strcmp(name, tmpname) == 0)
        {
          return tmp;
        }
      }
    }
    return NULL;
  }

  LinkedList *NetDriverManager::driverList = NULL;

}

