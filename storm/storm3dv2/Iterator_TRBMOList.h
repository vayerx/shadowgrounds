// Copyright 2002-2004 Frozenbyte Ltd.

#pragma once


//------------------------------------------------------------------
// Protos
//------------------------------------------------------------------
class TRBlock_MatHandle;
class TRBlock_MatHandle_Obj;
class Storm3D_Terrain;


//------------------------------------------------------------------
// Includes
//------------------------------------------------------------------
#include <windows.h>

// Storm3D includes 
#include "storm3d_common.h"
#include "..\..\util\Debug_MemoryManager.h"


//------------------------------------------------------------------
// Iterator template stuff for TRBMOList
//------------------------------------------------------------------
template <class A> class ST3D_EXP_DLLAPI IteratorIM_TRBMOList : public Iterator<A>
{

	PtrList<TRBlock_MatHandle> *ptrset;
	PtrListIterator<TRBlock_MatHandle_Obj> it;
	PtrListIterator<TRBlock_MatHandle> itm;
	int ecc;

public:

	void Next()
	{
		// Test if IsEnd has already taken next
		if (ecc>0) ecc--;
		else
		{
			it++;
			if ((*it)==NULL)
			{
				itm++;
				if ((*itm)!=NULL)
				{			
					it=(*itm)->objects.Begin();
				}
			}
		}
	}

	bool IsEnd()
	{
		return (*it)==NULL;

		if ((*it)==NULL)
		{
			itm++;
			if ((*itm)==NULL)
			{
				return true;
			}
			it=(*itm)->objects.Begin();
			ecc++;	// Add counter (so Next knows, that the next is already taken)
		}
		return false;
	}

	A GetCurrent()
	{
		return (A)(*it);
	}

	IteratorIM_TRBMOList(PtrList<TRBlock_MatHandle> *_ptrset) : it(NULL), itm(NULL), ecc(0)
	{
		ptrset=_ptrset;
		itm=ptrset->Begin();
		it=(*itm)->objects.Begin();
	}

	~IteratorIM_TRBMOList()
	{
	};

	friend class Storm3D_Terrain;
};



template <class A> class ST3D_EXP_DLLAPI ICreateIM_TRBMOList : public ICreate<A>
{
	
	PtrList<TRBlock_MatHandle> *ptrset;

public:

	Iterator<A> *Begin()
	{
		// psd
		if(ptrset->IsEmpty() == false)
			return new IteratorIM_TRBMOList<A>(ptrset);
		else
			return 0;
	}

	ICreateIM_TRBMOList(PtrList<TRBlock_MatHandle> *_ptrset)
	{
		ptrset=_ptrset;
	}
};


