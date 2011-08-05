#ifndef container_Stack_h
#define container_Stack_h

//
// Stack class
// Copyright(C) Jukka Kokkonen, 2000-2001
//

// TODO: proper destructor!

#ifndef __GNUC__
#pragma warning( disable : 4290 )
#endif

// #include "csp/compat.h"
#include <stdlib.h>
#include "StackEmptyException.h"

#define null NULL
#define throws(x) throw(x)

/* ---------------------------------- */
// StackObject class

class StackObject
{
  public:
    StackObject *next;
    void *data;

    StackObject(StackObject *next, void *data)
    {
      this->next = next;
      this->data = data;
    }
};

/* ---------------------------------- */
// Stack class (prototype) definition

class Stack
{
  private:
    StackObject *first_obj;

  public:
    Stack();
    bool isEmpty();
    void push(void *data);
    void *pop() throws (StackEmptyException *);
    void *peek() throws (StackEmptyException *);
};


#endif
