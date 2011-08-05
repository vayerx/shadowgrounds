
#include "Stack.h"

/* ---------------------------------- */
// Stack class methods

Stack::Stack()
{
  first_obj = NULL;
}

bool Stack::isEmpty()
{
  if (first_obj == NULL)
    return true;
  else
    return false;
}

void Stack::push(void *data)  // throws(StackException *)
{
  StackObject *tmp;
  tmp = new StackObject(first_obj, data);
  // just crash to the OutOfMemory exception...
  // try {
  //} catch(...) {
  //  throw(new StackException_OutOfMemory());
  //}
  first_obj = tmp;
}

void *Stack::pop() throws (StackEmptyException *)
{
  if (first_obj == NULL)
  {
    throw(new StackEmptyException());
  }
  void *tmp = first_obj->data;
  StackObject *new_first = first_obj->next;
  delete first_obj;
  first_obj = new_first;
  return tmp;
}

void *Stack::peek() throws (StackEmptyException *)
{
  if (first_obj == NULL)
  {
    throw(new StackEmptyException());
  }
  return first_obj->data;
}

