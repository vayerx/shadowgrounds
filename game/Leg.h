#ifndef LEG_H
#define LEG_H

#include "PartType.h"

namespace game
{
    class Leg : public PartType {
    public:
        Leg();
        Leg(int id);
        virtual ~Leg();

        virtual bool setData(const char *key, char *value);

    private:
        int carryingCapacity;
    };

    //extern Leg leg;

}

#endif
