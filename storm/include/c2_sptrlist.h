// Copyright 2002-2004 Frozenbyte Ltd.


#pragma once



//------------------------------------------------------------------
// SPtrListData
//------------------------------------------------------------------
template <class A> class SPtrListData
{

public:

	SPtr<A> data;
	SPtrListData<A> *next;

	// Constructor
	SPtrListData(SPtr<A> &_data) : data(_data),next(NULL) {}
};



//------------------------------------------------------------------
// SPtrListIterator
//------------------------------------------------------------------
template <class A> class SPtrListIterator
{
	SPtrListData<A> *pointer;

public:

	// Operators
	SPtr<A> operator*()		// Get current
	{
		if (pointer) return pointer->data;
		else return NULL;
	}
	
	SPtrListIterator<A> &operator++()		// Next
	{
		if (pointer) pointer=pointer->next;
		return *this;
	}

	// Constructor
	SPtrListIterator(SPtrListData<A> *first) : pointer(first) {}
};



//------------------------------------------------------------------
// SPtrList
//------------------------------------------------------------------
template <class A> class SPtrList
{
	SPtrListData<A> *first;

public:

	// Add & Remove
	void Add(SPtr<A> &obj)
	{
		SPtrListData<A> *temp=first;
		first=new SPtrListData<A>(obj);
		first->next=temp;
	}

	void Add(A *obj)
	{
		Add(SPtr<A>(obj));
	}

	void Remove(SPtr<A> &obj)
	{
		// Search from the list
		SPtrListData<A> *cur=first;
		SPtrListData<A> *last=NULL;
		while (cur)
		{
			// Get next
			SPtrListData<A> *next=cur->next;

			// Test if the one and delete
			if (cur->data==obj)
			{
				// Fix the list!
				if (last) last->next=next; else first=next;
				delete cur;
			}

			// Next...
			last=cur;
			cur=next;
		}
	}
	
	void Remove(A *obj)
	{
		Remove(SPtr<A>(obj));
	}

	void Clear()
	{
		// Delete whole list
		while (first)
		{
			// Get next
			SPtrListData<A> *next=first->next;

			// Delete
			delete first;

			// Next...
			first=next;
		}
	}

	// Iterate
	SPtrListIterator<A> Begin()
	{
		return SPtrListIterator<A>(first);
	}

	// Constructor
	SPtrList() : first(NULL) {};

	// Destructor
	~SPtrList()
	{
		Clear();
	};
};


