#include "precompiled.h"

// Copyright 2002-2004 Frozenbyte Ltd.

#ifndef __GNUC__
#pragma warning(disable:4103)
#pragma warning(disable:4786)
#endif

#include "LinkedList.h"

#include <assert.h>
#include "../util/Debug_MemoryManager.h"


#ifndef LINKEDLIST_USE_NODE_POOL
// if NOT defined...
void uninitLinkedListNodePool()
{
	// do nothing.
}
#endif

#ifdef LINKEDLIST_USE_NODE_POOL
// simple pooling of nodes (currently of static maximum size)
// reduces memory allocations
// may be useful when node adding/removing happening a lot.

#define LINKEDLIST_NODE_POOL_DEFAULT_SIZE 65536
//#define LINKEDLIST_NODE_POOL_DEFAULT_SIZE 32768
//#define LINKEDLIST_NODE_POOL_DEFAULT_SIZE 16

ListNode *linkedListNodePool = NULL;
int nextLinkedListPoolNode = 0;
int linkedListNodePoolAlloced = 0;
int linkedListNodePoolUsed = 0;

void initLinkedListNodePool()
{
	linkedListNodePoolAlloced = LINKEDLIST_NODE_POOL_DEFAULT_SIZE;
	linkedListNodePoolUsed = 0;
  linkedListNodePool = new ListNode[linkedListNodePoolAlloced];
  for (int i = 0; i < linkedListNodePoolAlloced; i++)
  {
    linkedListNodePool[i].item = linkedListNodePool; // free slot
    linkedListNodePool[i].poolIndex = i;
  }
}

void uninitLinkedListNodePool()
{
	if (linkedListNodePool == NULL) return;

	int leaked = 0;
  for (int i = 0; i < linkedListNodePoolAlloced; i++)
  {
    if (linkedListNodePool[i].item != linkedListNodePool)
			leaked++;
  }
	// this check is necessary if this uninit is not the very last
	// thing called! no linked lists may be used after this (if leaked).
	//if (leaked != 0)
	//{
		//assert(!"Linkedlist - Node pool not empty at uninit (something leaking nodes).");
	//} else {
		delete [] linkedListNodePool;
		linkedListNodePool = NULL;
		linkedListNodePoolAlloced = 0;
	//}
}

ListNode *allocateLinkedListNode(void *ptr)
{
  assert(linkedListNodePool != NULL);

	linkedListNodePoolUsed++;
	assert(linkedListNodePoolUsed <= linkedListNodePoolAlloced);

	// recreate node pool if 3/4 full.
	//if (linkedListNodePoolUsed > linkedListNodePoolAlloced * 3 / 4)
	// recreate node pool if 1/2 full.
  if (linkedListNodePoolUsed > (linkedListNodePoolAlloced >> 1))
	{
		// TODO: FIX EXTERNAL DEPENDENCIES, NEED TO BE UPDATED TO
		// THE NEW NODE POOLS...

		assert(!"LinkedList - Node pool exhausted, recreating a bigger one.");

		ListNode *oldNodePool = linkedListNodePool;

		linkedListNodePoolAlloced *= 2;
		linkedListNodePool = new ListNode[linkedListNodePoolAlloced];
		for (int i = 0; i < linkedListNodePoolAlloced; i++)
		{
			if (i < linkedListNodePoolAlloced / 2)
			{
				// copy from old...
				if (oldNodePool[i].item == oldNodePool)
				{
					// old free slot
					linkedListNodePool[i].item = linkedListNodePool; // free slot
				} else {
					linkedListNodePool[i].item = oldNodePool[i].item;
					if (oldNodePool[i].next == NULL)
						linkedListNodePool[i].next = NULL;
					else
						linkedListNodePool[i].next = &linkedListNodePool[oldNodePool[i].next->poolIndex];

					if (oldNodePool[i].prev == NULL)
						linkedListNodePool[i].prev = NULL;
					else
						linkedListNodePool[i].prev = &linkedListNodePool[oldNodePool[i].prev->poolIndex];
					assert(oldNodePool[i].poolIndex == i);
				}

				linkedListNodePool[i].poolIndex = oldNodePool[i].poolIndex;
			} else {
				// create new empty nodes...
				linkedListNodePool[i].item = linkedListNodePool; // free slot
				linkedListNodePool[i].poolIndex = i;
			}
		}

		// TODO: can't delete the old pool if someone still has
		// references there... (and the references most likely do exist)
		// WARNING: currently leaks memory
		//delete [] oldNodePool;
	}

  if (linkedListNodePool[nextLinkedListPoolNode].item != linkedListNodePool)
  {
		static int linkedListRand[8] = { 13, 19, 28, 17, 19, 11, 31, 29 };
		static int linkedListRandIndex = 0;

		linkedListRandIndex = ((linkedListRandIndex + 1) & 7);

		nextLinkedListPoolNode = (nextLinkedListPoolNode + linkedListRand[linkedListRandIndex]) % linkedListNodePoolAlloced;

		//int prevLinkedListPoolNode = nextLinkedListPoolNode;
		while (linkedListNodePool[nextLinkedListPoolNode].item != linkedListNodePool)
		{
			nextLinkedListPoolNode = (nextLinkedListPoolNode + 1) % linkedListNodePoolAlloced;
	//    if (nextLinkedListPoolNode == prevLinkedListPoolNode)
	//    {
	//			assert(!"LinkedList - Node pool exhausted.");
	//      return NULL; // pool exhausted (all nodes in use)
	//			// recreate a bigger pool...			
	//    }
		}
	}

  linkedListNodePool[nextLinkedListPoolNode].item = ptr;
  linkedListNodePool[nextLinkedListPoolNode].next = null;
  linkedListNodePool[nextLinkedListPoolNode].prev = null;

	int added = nextLinkedListPoolNode;
	nextLinkedListPoolNode = (nextLinkedListPoolNode + 1) % linkedListNodePoolAlloced;

  return &linkedListNodePool[added];
}

void freeLinkedListNode(const ListNode *node)
{
	linkedListNodePoolUsed--;
	assert(linkedListNodePoolUsed >= 0);

  linkedListNodePool[node->poolIndex].item = linkedListNodePool; // free slot 
	/*
	if (node->poolIndex == nextLinkedListPoolNode)
	{
		if (nextLinkedListPoolNode > 0)
		{
			nextLinkedListPoolNode--;
			if (linkedListNodePool[nextLinkedListPoolNode].item == linkedListNodePool)
			{
				if (nextLinkedListPoolNode > 0)
				{
					nextLinkedListPoolNode--;
				}
			}
		}
	}
	*/
}

ListNode::ListNode()
{
  // nop
}

#endif


ListNode::ListNode(void *ptr)
{
  item = ptr;
  next = null;
  prev = null;
}


LinkedList::LinkedList(void)
{
  first = null;
  last = null;
  walk_node = null;
  remove_count = 0;

#ifdef LINKEDLIST_USE_NODE_POOL
  if (linkedListNodePool == NULL)
    initLinkedListNodePool();
#endif
}

LinkedList::~LinkedList(void)
{
  while (!isEmpty())
  {
    popFirst();
  }
}

void LinkedList::append(void *ptr)
{
#ifdef LINKEDLIST_USE_NODE_POOL
  ListNode *node = allocateLinkedListNode(ptr);
#else
  ListNode *node = new ListNode(ptr);
#endif

  if (isEmpty())
  {
    first = node;
    last = node;
  } else {
    node->next = null;
    node->prev = last;
    last->next = node;
    last = node;
  }
}

void LinkedList::prepend(void *ptr)
{
#ifdef LINKEDLIST_USE_NODE_POOL
  ListNode *node = allocateLinkedListNode(ptr);
#else
  ListNode *node = new ListNode(ptr);
#endif

  if (isEmpty())
  {
    first = node;
    last = node;
  } else {
    node->next = first;
    node->prev = null;
    first->prev = node;
    first = node;
  }
}

void LinkedList::remove(void *ptr)
{
  ListNode *tmp = first;
    
  while (tmp != null)
  {
    if (tmp->item == ptr)
    {
      if (first == tmp) first = tmp->next;
      if (last == tmp) last = tmp->prev;
      if (tmp->prev != null) (tmp->prev)->next = tmp->next;
      if (tmp->next != null) (tmp->next)->prev = tmp->prev;
#ifdef LINKEDLIST_USE_NODE_POOL
      freeLinkedListNode(tmp);
#else
      delete tmp;
#endif
      remove_count++;
      break;
    }
    tmp = tmp->next;
  }
}
    
void *LinkedList::popFirst() throws (EmptyLinkedListException *)
{
  ListNode *tmp = first;
  void *ptr;
 
  if (tmp == null)
  {
    throw(new EmptyLinkedListException());
  } 
  
  //if (last->prev == first) last->prev = NULL;
  if (first->next != NULL) (first->next)->prev = NULL;
  first = tmp->next;
  if (first == NULL) last = NULL;
  ptr = tmp->item;
#ifdef LINKEDLIST_USE_NODE_POOL
  freeLinkedListNode(tmp);
#else
  delete tmp;
#endif
  remove_count++;

  return ptr;
}
    
void *LinkedList::popLast() throws (EmptyLinkedListException *)
{
  ListNode *tmp = last;
  void *ptr;
 
  if (tmp == null)
  {
    throw(new EmptyLinkedListException());
  }
    
  //if (first->next == last) first->next = NULL;
  if (last->prev != NULL) (last->prev)->next = NULL;
  last = tmp->prev;
  if (last == NULL) first = NULL;
  ptr = tmp->item;
#ifdef LINKEDLIST_USE_NODE_POOL
  freeLinkedListNode(tmp);
#else
  delete tmp;
#endif
  remove_count++;

  return ptr;
}

void *LinkedList::peekFirst() throws (EmptyLinkedListException *)
{
  ListNode *tmp = first;
  void *ptr;
 
  if (tmp == null)
  {
    throw(new EmptyLinkedListException());
  }
    
  ptr = tmp->item;
  return ptr;
}
      
void *LinkedList::peekLast() throws (EmptyLinkedListException *)
{
  ListNode *tmp = last;
  void *ptr;
 
  if (tmp == null)
  {
    throw(new EmptyLinkedListException());
  }
    
  ptr = tmp->item;  
  return ptr;
}
      
bool LinkedList::isEmpty()
{
  if (first == null)
    return true;
  else
    return false;
}

const ListNode *LinkedList::getFirstNode() const 
{
  return first;
}

const ListNode *LinkedList::getLastNode() const 
{
  return last;
}

void LinkedList::removeNode(const ListNode *node)
{
#ifdef _DEBUG
  ListNode *tmp = first;
    
  while (tmp != null)
  {
    if (tmp == node)
    {
      break;
    }
    tmp = tmp->next;
  }
  if (tmp == null)
  {
    abort();
  }
#endif

  if (node == first) first = first->next;
  if (node == last) last = last->prev;
  if (node->prev != null) (node->prev)->next = node->next;
  if (node->next != null) (node->next)->prev = node->prev;
#ifdef LINKEDLIST_USE_NODE_POOL
  freeLinkedListNode(node);
#else
	// WARNING: const -> unconst cast!
  delete (ListNode *)node;
#endif
  remove_count++;
}

void *LinkedList::iterateNext() throws (EmptyIteratorException *)
{  
  void *ret;
    
  if (walk_node == null)
  { 
    throw(new EmptyIteratorException());
  }
  
  ret = walk_node->item;
  walk_node = walk_node->next;
  
  return ret;
}     

bool LinkedList::iterateAvailable()
{
  if (walk_node == null)
    return false;
  else
    return true;
}

void LinkedList::resetIterate()
{
  walk_node = first;
}


LinkedListIterator::LinkedListIterator(const LinkedList *linkedList)
{
  this->linkedList = linkedList;
  walk_node = linkedList->getFirstNode();
  this->remove_count = linkedList->remove_count;
}

LinkedListIterator::~LinkedListIterator()
{
  walk_node = NULL;
}

/*
// inlined
void *LinkedListIterator::iterateNext() throws (EmptyIteratorException *)
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
*/    

/*
// inlined
bool LinkedListIterator::iterateAvailable()
{
  if (walk_node == null)
    return false;
  else
    return true;
}
*/

SafeLinkedListIterator::SafeLinkedListIterator(const LinkedList *linkedList)
{
  this->linkedList = new LinkedList();
  const ListNode *tmp = linkedList->getFirstNode();
  while (tmp != NULL)
  {
    this->linkedList->append(tmp->item);
    tmp = tmp->next;
  }
  walk_node = this->linkedList->getFirstNode();
}

SafeLinkedListIterator::~SafeLinkedListIterator()
{
  delete linkedList;
  walk_node = NULL;
}

void *SafeLinkedListIterator::iterateNext() throws (EmptyIteratorException *)
{  
  void *ret;
    
  if (walk_node == null)
  { 
    throw(new EmptyIteratorException());
  }
  
  ret = walk_node->item;
  walk_node = walk_node->next;
  
  return ret;
}     

bool SafeLinkedListIterator::iterateAvailable()
{
  if (walk_node == null)
    return false;
  else
    return true;
}

