/*

  Common library v2
  (C) Sebastian Aaltonen 2001

  String routines

  Classes:
  
	String			- String class

*/


//------------------------------------------------------------------
// Includes
//------------------------------------------------------------------
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <c2_string.h>



//------------------------------------------------------------------
// String construct/destruct
//------------------------------------------------------------------
String::String()
{
	length=0;
	data=new char[1];
	data[length]=0;
}


String::String(const char *str)
{
	length=strlen(str);
	data=new char[length+1];
	strcpy(data,str);
}


String::String(const char ch)
{
	length=1;
	data=new char[length+1];
	data[0]=ch;
	data[1]=0;
}


String::String(const unsigned char ch)
{
	length=1;
	data=new char[length+1];
	data[0]=ch;
	data[1]=0;
}


String::String(const int val)
{
	// Create string
	char str[20];
	snprintf(str, 20, "%d", val);

	// Copy
	length=strlen(str);
	data=new char[length+1];
	strcpy(data,str);
}


String::String(const float val)
{
	// Create string
	char str[20];
	sprintf(str,"%f",val);

	// Copy
	length=strlen(str);
	data=new char[length+1];
	strcpy(data,str);
}


String::String(const String &orig)
{
	length=orig.GetLength();
	data=new char[length+1];
	orig.ConvertToNullTermCharString(data,length+1);
}


String::~String()
{
	SAFE_DELETE_ARRAY(data);
}


//------------------------------------------------------------------
// String functions
//------------------------------------------------------------------
int String::GetLength() const
{
	return length;
}


void String::SetLength(int _length)
{
	if (_length>length)
	{
		char *_data=new char[_length+1];
		memcpy(_data,data,length);
		_data[_length]=0;				// do a null terminated string
		SAFE_DELETE_ARRAY(data);
		data=_data;
	}
	length=_length;
}


void String::ConvertToNullTermCharString(char *dest,int array_length) const
{
	if (array_length<length+1)
	{
		memcpy(dest,data,array_length-1);
		dest[array_length-1]=0;	// do a null terminated string
	}
	else
	{
		memcpy(dest,data,length);
		dest[length]=0;	// do a null terminated string
	}
}


char *String::GetAsNullTermCharString()
{
	return data;
}


const char *String::GetAsNullTermCharString() const
{
	return data;
}

void String::AddNonNullTerminatedString(char *_data,int _length)
{
	char *old_data=data;
	int old_length=length;

	length+=_length;
	data=new char[length+1];
	memcpy(data,old_data,old_length);
	memcpy(data+old_length,_data,_length);
	data[length]=0;	// do a null terminated string

	// Delete old data
	SAFE_DELETE_ARRAY(old_data);
}


void String::CreateFromNullTerminatedString(char *_data,int _length)
{
	char *old_data=data;

	length=_length;
	data=new char[length+1];
	memcpy(data,_data,_length);
	data[length]=0;	// do a null terminated string

	// Delete old data
	SAFE_DELETE_ARRAY(old_data);
}


//------------------------------------------------------------------
// String splitting functions
//------------------------------------------------------------------
String String::GetSubString(int first,int _length) const
{
	String temp;
	if ((first>=0)&&(_length>0)&&(_length+first<=length))
	{
		temp.CreateFromNullTerminatedString(data+first,_length);
	}
	return temp;
}


String String::GetAndRemoveFirstWord(char separator)
{
	String temp;

	// Search for separator
	int i;
	for (i=0;(i<length)&&(data[i]!=separator);i++); //OK
	
	// Found?
	if (i==length)
	{
		// If not then result is the whole string
		temp=*this;

		// Clear (because all was taken)
		length=0;
		SAFE_DELETE_ARRAY(data);
		data=new char[1];
		data[length]=0;
	}
	else
	{
		temp.CreateFromNullTerminatedString(data,i);
		memmove(data,data+i+1,length-i);
		length-=i+1;
	}
	return temp;
}


String String::GetAndRemoveLastWord(char separator)
{
	String temp;

	// Search for separator
	int i;
	for (i=length-1;(i>=0)&&(data[i]!=separator);i--); //OK
	
	// Found?
	if (i<0)
	{
		// If not then result is the whole string
		temp=*this;

		// Clear (because all was taken)
		length=0;
		SAFE_DELETE_ARRAY(data);
		data=new char[1];
		data[length]=0;
	}
	else
	{
		temp=String(data+i+1);
		length=i;
	}
	return temp;
}


//------------------------------------------------------------------
// Advanced compare
//------------------------------------------------------------------
int String::CompareTo(const String &other) const
{
	int len=length;
	if (other.length<len) len=other.length;
	int value=memcmp(data,other.data,len);

	if (value!=0) return value;
	else
	{
		if (other.length>length) return -1;
		if (other.length<length) return 1;
		return 0;
	}
}


bool String::BeginsWith(const String &other) const
{
	if (length<other.length) return false;
	if (memcmp(data,other.data,other.length)!=0) return false;
	return true;
}


bool String::EndsWith(const String &other) const
{
	if (length<other.length) return false;
	if (memcmp(data+length-other.length,other.data,other.length)!=0) return false;
	return true;
}


//------------------------------------------------------------------
// Misc
//------------------------------------------------------------------

// Removes backspaces and characters before them
void String::ProcessBackSpaces()
{
	for (int i=0;i<length;i++)
	if (data[i]==8)	// Backspace is ascii 8
	{
		if (i>0)
		{
			memmove(data+i-1,data+i+1,length-i);
			length-=2;
			i-=2;
		}
		else
		{
			memmove(data,data+1,length);
			length--;
			i--;
		}
	}
}


// Removes all instances of one character
void String::DeleteCharacter(char c)
{
	for (int i=0;i<length;i++)
	if (data[i]==c)
	{
		memmove(data+i,data+i+1,length-i);
		length--;
		i--;
	}
}


// Replaces all instances of one character with another
void String::ReplaceCharacterWithAnother(char old_c,char new_c)
{
	for (int i=0;i<length;i++)
		if (data[i]==old_c) data[i]=new_c;
}


//------------------------------------------------------------------
// String operators
//------------------------------------------------------------------
char String::operator[](int i) const
{
	if (i<=length) return data[i]; else return 0;
}


void String::operator=(const char *str)
{
	// Delete old first
	SAFE_DELETE_ARRAY(data);

	// Copy
	length=strlen(str);
	data=new char[length+1];
	strcpy(data,str);
}


void String::operator=(const int val)
{
	// Delete old first
	SAFE_DELETE_ARRAY(data);

	// Create string
	char str[20];
	snprintf(str, 20, "%d", val);

	// Copy
	length=strlen(str);
	data=new char[length+1];
	strcpy(data,str);
}


void String::operator=(const float val)
{
	// Delete old first
	SAFE_DELETE_ARRAY(data);

	// Create string
	char str[20];
	sprintf(str,"%f",val);

	// Copy
	length=strlen(str);
	data=new char[length+1];
	strcpy(data,str);
}


void String::operator=(const String &other)
{
	if (&other==this) return;

	// Delete old first
	SAFE_DELETE_ARRAY(data);

	// Set
	length=other.GetLength();
	data=new char[length+1];
	other.ConvertToNullTermCharString(data,length+1);
}


bool String::operator==(const String &other) const
{
	if (length!=other.length) return false;
	if (memcmp(data,other.data,length)!=0) return false;
	return true;
}


void String::operator+=(const String &other)
{
	char *old_data=data;
	int old_length=length;

	length+=other.GetLength();
	data=new char[length+1];
	memcpy(data,old_data,old_length);
	other.ConvertToNullTermCharString(&data[old_length],other.GetLength()+1);
	data[length]=0;	// do a null terminated string

	// Delete old data
	SAFE_DELETE_ARRAY(old_data);
}


String String::operator+(const String &other)
{
	String temp(*this);
	temp+=other;
	return temp;
}


//------------------------------------------------------------------
// Operators
//------------------------------------------------------------------
String operator+(char *s1,const String &s2)
{
	String temp(s1);
	temp+=s2;
	return temp;
}


