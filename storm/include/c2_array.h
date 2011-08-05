// Copyright 2002-2004 Frozenbyte Ltd.

#pragma once



//------------------------------------------------------------------
// Array
//------------------------------------------------------------------
template <class A> class Array
{
	A *data;
	int size;

public:

	// Get properties
	int GetSize() {return size};

	// Operators
	A operator[](int position)
	{
		// TODO: Debug overflow test here
		return a[position];
	}

	// Constructor & destructor
	~Array() {delete[] data}
	Array(int _size=0) : data(NULL),size(_size)
	{
		if (size<=0)
		{
			size=0;
		}
		else
		{
			data=new A[size];
		}
	}
};



//------------------------------------------------------------------
// DynaArray
//------------------------------------------------------------------
template <class A> class DynaArray
{
	A **data;
	int size;

	typedef A *PA;

public:

	// Get properties
	int GetSize() {return size};

	// Set properties
	int SetSize(int newsize)
	{
		A *ndata=new A[newsize];
		EI VALMIS!
	};

	// Operators
	A operator[](int position)
	{
		// TODO: Debug overflow test here
		return *a[position];
	}

	// Constructor & destructor
	~Array() 
	{
		for (int i=0;i<size;i++) delete data[i];
		delete[] data
	}
	Array(int _size=0) : data(NULL),size(_size)
	{
		if (size<=0)
		{
			size=0;
		}
		else
		{
			data=new PA[size];
			for (int i=0;i<size;i++) data[i]=new A;
		}
	}
};



