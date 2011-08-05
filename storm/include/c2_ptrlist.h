// Copyright 2002-2004 Frozenbyte Ltd.

#pragma once
#include <cassert>


//------------------------------------------------------------------
// Protos
//------------------------------------------------------------------
template <class A> class PtrListData;
template <class A> class PtrListIterator;
template <class A> class PtrList;


//------------------------------------------------------------------
// PtrListData
//------------------------------------------------------------------
template <class A> class PtrListData
{

public:

	A *data;
	PtrListData<A> *next;

	// Constructor
	PtrListData(A *_data) : data(_data),next(NULL) {}
};



//------------------------------------------------------------------
// PtrListIterator
//------------------------------------------------------------------
template <class A> class PtrListIterator
{
	PtrListData<A> *pointer;
	PtrListData<A> *previous;

public:

	// Operators
	A* operator*()		// Get current
	{
		if (pointer) return pointer->data;
		else return NULL;
	}
	
	void operator++()		// Next
	{
		previous=pointer;
		if (pointer) pointer=pointer->next;
	}

	void operator++(int)	// Postfix version (to cancel those annoying warnings!)
	{
		operator++();
	}

	// Constructor
	PtrListIterator(PtrListData<A> *first) : pointer(first), previous(NULL) {}

	friend class PtrList<A>;
};



//------------------------------------------------------------------
// PtrList
//------------------------------------------------------------------
template <class A> class PtrList
{
	PtrListData<A> *first;

public:

	// Add & Remove
	PtrListIterator<A> Add(A *obj)
	{
		// Test
		if (!obj) return PtrListIterator<A>(first);	// Return first object iterator if obj==NULL

		PtrListData<A> *temp=first;
		first=new PtrListData<A>(obj);
		first->next=temp;
		return PtrListIterator<A>(first);	// Returns added object's iterator
	}

	void Remove(A *obj)
	{
		// Test
		if (!obj) return;

		// Search from the list
		PtrListData<A> *cur=first;
		PtrListData<A> *last=NULL;
		while (cur)
		{
			// Get next
			PtrListData<A> *next=cur->next;

			// Test if the one and delete
			if (cur->data==obj)
			{
				// Fix the list!
				if (last) last->next=next; else first=next;
				delete cur;

				return;
			}

			// Next...
			last=cur;
			cur=next;
		}

		assert(!"Object not found!");
	}
	
	void Remove(PtrListIterator<A> &iter)
	{
		// Search from the list
		PtrListData<A> *cur=iter.pointer;
		PtrListData<A> *last=iter.previous;
		if (cur)
		{
			// Move iterator forward (otherwise it would be illegal)
			iter++;

			// Get next
			PtrListData<A> *next=cur->next;

			// Fix the list!
			if (last) last->next=next; else first=next;
			delete cur;
		}
		else
		{
			assert(!"Nothing to remove");
		}
	}
	
	void Clear()
	{
		// Delete whole list
		while (first)
		{
			// Get next
			PtrListData<A> *next=first->next;

			// Delete
			delete first;

			// Next...
			first=next;
		}
	}

	// Deletes all stored objects in list and clears list
	void DeleteObjects()
	{
		// Delete whole list
		while (first)
		{
			// Get next
			PtrListData<A> *next=first->next;

			// Delete stored object
			delete first->data;

			// Delete
			delete first;

			// Next...
			first=next;
		}
	}

	// Test if empty
	bool IsEmpty()
	{
		if (first) return false;
		return true;
	}

	// Iterate
	PtrListIterator<A> Begin()
	{
		return PtrListIterator<A>(first);
	}

	// Constructor
	PtrList() : first(NULL) {};

	// Destructor
	~PtrList()
	{
		Clear();
	};
};


