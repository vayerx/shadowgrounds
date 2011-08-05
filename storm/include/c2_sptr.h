// Copyright 2002-2004 Frozenbyte Ltd.


#pragma once


//------------------------------------------------------------------
// Includes
//------------------------------------------------------------------
#include <string.h>



//------------------------------------------------------------------
// Shared
//------------------------------------------------------------------
class Shared
{
	// Reference count calculator
	int reference_count;

public:

	// Construct
	inline Shared() : reference_count(0) {};

	// Add/Subtract reference count
	inline void AddRef() {reference_count++;}
	inline bool Delete()
	{
		if ((--reference_count)==0) 
		{
			delete this;
			return true;
		}
		return false;
	}

	// Virtual destructor
	inline virtual ~Shared() {};
};



//------------------------------------------------------------------
// SPtr (template)
//------------------------------------------------------------------
template <class A> class SPtr
{
	// Tha real pointer
	A *ptr;

public:

	// Construct/destruct
	inline SPtr(SPtr<A> &other)
	{
		ptr=other.ptr;
		if (ptr) ptr->AddRef();
	}
//	inline SPtr(A *p=NULL) : ptr(p) {if (ptr) ptr->AddRef();}
    inline ~SPtr() {if (ptr) ptr->Delete();}

	// Operators
	inline operator A*() {return ptr;}
	inline A &operator*() {return *ptr;}
	inline A *operator->() {return ptr;}
	inline SPtr& operator=(SPtr<A> &other)
	{
		return operator=(other.ptr);
	}
	inline SPtr& operator=(A *other)
	{
		if (ptr) ptr->Delete();
		ptr=other;
		if (ptr) ptr->AddRef();
		return *this;
	}
	inline bool operator==(const SPtr<A> &other) const
	{
		if (other.ptr==ptr) return true;
		return false;
	}
};


