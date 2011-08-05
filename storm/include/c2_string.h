// Copyright 2002-2004 Frozenbyte Ltd.


#pragma once


//------------------------------------------------------------------
// Includes
//------------------------------------------------------------------
#include "c2_common.h"


//------------------------------------------------------------------
// String
//------------------------------------------------------------------
class String
{
	char *data;
	int length;

public:

	// Conversion (to null terminated char[])
	// Fills dest until string ends or dest array is full (array_length-1)
	// (In either way) adds a null character at end of the char-string.
	void ConvertToNullTermCharString(char *dest,int array_length) const;

	// Gets string data (null terminated string)
	const char *GetAsNullTermCharString() const;
	char *GetAsNullTermCharString();
	inline char *NTS() {return GetAsNullTermCharString();}	// Shorter version

	// Length
	int GetLength() const;
	void SetLength(int _length);
	
	// Non-null-terminated char[] functions
	void AddNonNullTerminatedString(char *_data,int _length);
	void CreateFromNullTerminatedString(char *_data,int _length);

	// String splitting functions
	String GetSubString(int first,int _length) const;
	String GetAndRemoveFirstWord(char separator);
	String GetAndRemoveLastWord(char separator);

	// Advanced compare
	int CompareTo(const String &other) const;	// returns: <0 less than other, >0 greater than other, 0 identical
	bool BeginsWith(const String &other) const;
	bool EndsWith(const String &other) const;

	// Misc
	void ProcessBackSpaces();			// Removes backspaces and characters before them
	void DeleteCharacter(char c);		// Removes all instances of one character
	void ReplaceCharacterWithAnother(char old_c,char new_c);	// Replaces all instances of one character with another

	// Operators
	char operator[](int i) const;
	void operator=(const char *str);
	void operator=(const int val);
	void operator=(const float val);
	void operator=(const String &other);
	bool operator==(const String &other) const;	// compare to other
	void operator+=(const String &other);			// append other at end
	String operator+(const String &other);

	// Constructors
	String();										// empty string
	String(const char *str);
	String(const char ch);
	String(const unsigned char ch);
	String(const int val);
	String(const float val);
	String(const String &orig);				// copy constructor

	// Destructor
	~String();
};


//------------------------------------------------------------------
// Operators
//------------------------------------------------------------------
String operator+(char *s1,const String &s2);


