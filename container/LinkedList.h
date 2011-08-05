#ifndef container_LinkedList_h
#define container_LinkedList_h

#ifdef _MSC_VER
#pragma warning( disable : 4290 ) 
#endif

//
// (Two way) linked list class
// Copyright(C) Jukka Kokkonen, 2000-2002
//
// No out of memory exception check...
//
// v1.1.1 - 3.7.2002 - modified to catch unsafe removes when iterating
// v1.1 - 14.6.2002 - added seperate iterator classes
//

#include <stdlib.h>
// #include "csp/compat.h"
#include "EmptyLinkedListException.h"
#include "EmptyIteratorException.h"

#define throws(x) throw(x)
#define null NULL

// proto...
class LinkedList;
class LinkedListIterator;
class SafeLinkedListIterator;


// just to clean up the linkedlist node pool after use.
// if pool not in use, does nothing.
extern void uninitLinkedListNodePool();


#ifdef LINKEDLIST_USE_NODE_POOL
class ListNode
{
  public:
    ListNode();
    int poolIndex;
    ListNode(void *ptr);

    ListNode *next;
    ListNode *prev;
    void *item;

  friend class LinkedList;
  friend class LinkedListIterator;
  friend class SafeLinkedListIterator;
};

#else

class ListNode
{
  private:
    ListNode(void *ptr);

    ListNode *next;
    ListNode *prev;
    void *item;

  friend class LinkedList;
  friend class LinkedListIterator;
  friend class SafeLinkedListIterator;
};

#endif

class LinkedList
{
  private:
    ListNode *first;
    ListNode *last;
    ListNode *walk_node;

    int remove_count;
    friend class LinkedListIterator;

  public:
    LinkedList();
    ~LinkedList();
    void prepend(void *ptr);
    void append(void *ptr);
    void remove(void *ptr);
    void *popFirst() throws (EmptyLinkedListException *);
    void *popLast() throws (EmptyLinkedListException *);
    void *peekFirst() throws (EmptyLinkedListException *);
    void *peekLast() throws (EmptyLinkedListException *);
    bool isEmpty();

    // these are for real hacking... usually not recommended
    const ListNode *getFirstNode() const;
    const ListNode *getLastNode() const;
    void removeNode(const ListNode *node);

    // iteration
    // deprecated: use the seperate iterators instead
    void *iterateNext() throws (EmptyIteratorException *);
    bool iterateAvailable();
    void resetIterate();
};

// basic iterator
// items should not be deleted from the list while using iterator.
class LinkedListIterator
{
  private:
    const LinkedList *linkedList;
    const ListNode *walk_node;
    int remove_count;

  public:
    LinkedListIterator(const LinkedList *linkedList);
    ~LinkedListIterator();
    inline bool iterateAvailable()
    {
      if (walk_node == null)
        return false;
      else
        return true;
    }
    inline void *iterateNext()
    {
      void *ret;

      #ifdef _DEBUG
        // this is to catch unsafe node removals while iterating.
        // (as that may cause undefined behaviour)
        if (linkedList->remove_count != this->remove_count)
        {
          abort();
        }
      #endif
  
      if (walk_node == null)
      { 
        throw(new EmptyIteratorException());
      }
  
      ret = walk_node->item;
      walk_node = walk_node->next;
  
      return ret;
    }
};

// safe iterator, keeps an internal copy of the linked list, making it 
// possible to delete items from the list safely while using iterator.
// not very effective though.
class SafeLinkedListIterator
{
  private:
    LinkedList *linkedList;
    const ListNode *walk_node;

  public:
    SafeLinkedListIterator(const LinkedList *linkedList);
    ~SafeLinkedListIterator();
    bool iterateAvailable();
    void *iterateNext() throws (EmptyIteratorException *);
};

#endif
