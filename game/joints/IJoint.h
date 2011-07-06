
#ifndef IJOINT_H
#define IJOINT_H

namespace game
{

class IJoint
{
public:
	// should return the unique implementing class id number for the area
	// (or possibly that of a base class, if inherited from one)
	virtual int getJointClassId() = 0;	

	virtual void run() = 0;
};

}

#endif

