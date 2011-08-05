// Copyright 2002-2004 Frozenbyte Ltd.

#pragma once


//------------------------------------------------------------------
// Includes
//------------------------------------------------------------------

// Storm3D includes 
#include "Storm3D_Common.h"
#include "../../util/Debug_MemoryManager.h"



//------------------------------------------------------------------
// Iterator template stuff for STL sets
//------------------------------------------------------------------
template <class A> class /*ST3D_EXP_DLLAPI*/ IteratorIM_Set : public Iterator<A>
{

	set<A> *ptrset;
	typename set<A>::iterator it;
	
public:

	void Next()
	{
		it++;
	}

	bool IsEnd()
	{
		return it==ptrset->end();
	}

	A GetCurrent()
	{
		return (*it);
	}

	IteratorIM_Set(set<A> *_ptrset)
	{
		ptrset=_ptrset;
		it=ptrset->begin();
	}

	~IteratorIM_Set()
	{
	};
};



template <class A> class ST3D_EXP_DLLAPI ICreateIM_Set : public ICreate<A>
{
	
	set<A> *ptrset;

public:

	Iterator<A> *Begin()
	{
		return new IteratorIM_Set<A>(ptrset);
	}

	ICreateIM_Set(set<A> *_ptrset)
	{
		ptrset=_ptrset;
	}
};


//------------------------------------------------------------------
// Iterator template stuff for PtrList
//------------------------------------------------------------------
template <class A> class ST3D_EXP_DLLAPI IteratorIM_PtrList : public Iterator<A>
{

	PtrList<A> *ptrset;
	PtrListIterator<A> it;
	
public:

	void Next()
	{
		it++;
	}

	bool IsEnd()
	{
		return (*it)==NULL;
	}

	A GetCurrent()
	{
		return (*it);
	}

	IteratorIM_PtrList(PtrList<A> *_ptrset)
	{
		ptrset=_ptrset;
		it=ptrset->Begin();
	}

	~IteratorIM_PtrList()
	{
	};
};



template <class A> class ST3D_EXP_DLLAPI ICreateIM_PtrList : public ICreate<A>
{
	
	PtrList<A> *ptrset;

public:

	Iterator<A> *Begin()
	{
		return new IteratorIM_PtrList<A>(ptrset);
	}

	ICreateIM_PtrList(PtrList<A> *_ptrSet)
	{
		ptrset=_ptrSet;
	}
};


