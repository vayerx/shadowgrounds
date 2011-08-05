
#ifndef CHECKEDINTVALUE_H
#define CHECKEDINTVALUE_H

class CheckedIntValue;

bool operator== (const int &v1, const CheckedIntValue &v2);
bool operator== (const CheckedIntValue &v1, const int &v2);
bool operator== (const CheckedIntValue &v1, const CheckedIntValue &v2);
bool operator!= (const int &v1, const CheckedIntValue &v2);
bool operator!= (const CheckedIntValue &v1, const int &v2);
bool operator!= (const CheckedIntValue &v1, const CheckedIntValue &v2);
bool operator>= (const int &v1, const CheckedIntValue &v2);
bool operator>= (const CheckedIntValue &v1, const int &v2);
bool operator>= (const CheckedIntValue &v1, const CheckedIntValue &v2);
bool operator<= (const int &v1, const CheckedIntValue &v2);
bool operator<= (const CheckedIntValue &v1, const int &v2);
bool operator<= (const CheckedIntValue &v1, const CheckedIntValue &v2);
int operator+ (const int &v1, const CheckedIntValue &v2);
int operator+ (const CheckedIntValue &v1, const int &v2);
int operator+ (const CheckedIntValue &v1, const CheckedIntValue &v2);
int operator- (const int &v1, const CheckedIntValue &v2);
int operator- (const CheckedIntValue &v1, const int &v2);
int operator- (const CheckedIntValue &v1, const CheckedIntValue &v2);
int operator* (const int &v1, const CheckedIntValue &v2);
int operator* (const CheckedIntValue &v1, const int &v2);
int operator* (const CheckedIntValue &v1, const CheckedIntValue &v2);
int operator/ (const int &v1, const CheckedIntValue &v2);
int operator/ (const CheckedIntValue &v1, const int &v2);
int operator/ (const CheckedIntValue &v1, const CheckedIntValue &v2);

//int& operator+ (const CheckedIntValue &v);


/**
 * A simple class that wraps an int value and adds some runtime debug checks for it.
 *
 * @version 1.0 - 31.7.2007
 * @author Jukka Kokkonen <jpkokkon@cc.hut.fi>
 */
class CheckedIntValue
{
  public:
    CheckedIntValue();

    ~CheckedIntValue();

    CheckedIntValue(const CheckedIntValue &v);

    //CheckedIntValue(const int &v);

    CheckedIntValue& operator= (const CheckedIntValue &v);

    //CheckedIntValue& operator= (const int &v);

    CheckedIntValue& operator= (int value);
    CheckedIntValue& operator= (float value);
    CheckedIntValue& operator= (bool value); // (bool not allowed)

//???
/*
    CheckedIntValue& operator+ (int value);
    //CheckedIntValue& operator+ (float value);
    //CheckedIntValue& operator+ (bool value); // (bool not allowed)
    CheckedIntValue& operator+ (const CheckedIntValue& value);

    CheckedIntValue& operator- (int value);
    //CheckedIntValue& operator- (float value);	
    //CheckedIntValue& operator- (bool value); // (bool not allowed)
    CheckedIntValue& operator- (const CheckedIntValue& value);

    CheckedIntValue& operator* (int value);
    //CheckedIntValue& operator* (float value);	
    //CheckedIntValue& operator* (bool value); // (bool not allowed)
    CheckedIntValue& operator* (const CheckedIntValue& value);

    CheckedIntValue& operator/ (int value);
    //CheckedIntValue& operator/ (float value);	
    //CheckedIntValue& operator/ (bool value); // (bool not allowed)
    CheckedIntValue& operator/ (const CheckedIntValue& value);
*/

		// prefix
    CheckedIntValue& operator++();
    CheckedIntValue& operator--();

		// postfix
    CheckedIntValue operator++(int);
    CheckedIntValue operator--(int);

    CheckedIntValue& operator+= (int value);
    //CheckedIntValue& operator+= (float value);
    //CheckedIntValue& operator+= (bool value); // (bool not allowed)
    CheckedIntValue& operator+= (const CheckedIntValue& value);

    CheckedIntValue& operator-= (int value);
    //CheckedIntValue& operator-= (float value);	
    //CheckedIntValue& operator-= (bool value); // (bool not allowed)
    CheckedIntValue& operator-= (const CheckedIntValue& value);

    CheckedIntValue& operator*= (int value);
    //CheckedIntValue& operator*= (float value);	
    //CheckedIntValue& operator*= (bool value); // (bool not allowed)
    CheckedIntValue& operator*= (const CheckedIntValue& value);

    CheckedIntValue& operator/= (int value);
    //CheckedIntValue& operator/= (float value);	
    //CheckedIntValue& operator/= (bool value); // (bool not allowed)
    CheckedIntValue& operator/= (const CheckedIntValue& value);

    CheckedIntValue& operator%= (int value);
    //CheckedIntValue& operator/= (float value);	
    //CheckedIntValue& operator/= (bool value); // (bool not allowed)
    CheckedIntValue& operator%= (const CheckedIntValue& value);


    CheckedIntValue(int value);
    explicit CheckedIntValue(float value);
    explicit CheckedIntValue(bool value); // (bool not allowed)
    explicit CheckedIntValue(void *);


    operator int() const;
    //operator float() const;		
    //operator bool() const; // (bool not allowed)
    operator void*() const;


		// for error checking...
		bool wasInitialized();
		void clearInitializedFlag();

  private:
		int value;
		bool initialized;

  friend bool ::operator== (const int &v1, const CheckedIntValue &v2);
  friend bool ::operator== (const CheckedIntValue &v1, const int &v2);
  friend bool ::operator== (const CheckedIntValue &v1, const CheckedIntValue &v2);
  friend bool ::operator!= (const int &v1, const CheckedIntValue &v2);
  friend bool ::operator!= (const CheckedIntValue &v1, const int &v2);
  friend bool ::operator!= (const CheckedIntValue &v1, const CheckedIntValue &v2);
	friend bool ::operator>= (const int &v1, const CheckedIntValue &v2);
	friend bool ::operator>= (const CheckedIntValue &v1, const int &v2);
	friend bool ::operator>= (const CheckedIntValue &v1, const CheckedIntValue &v2);
	friend bool ::operator<= (const int &v1, const CheckedIntValue &v2);
	friend bool ::operator<= (const CheckedIntValue &v1, const int &v2);
	friend bool ::operator<= (const CheckedIntValue &v1, const CheckedIntValue &v2);
	friend int ::operator+ (const int &v1, const CheckedIntValue &v2);
	friend int ::operator+ (const CheckedIntValue &v1, const int &v2);
	friend int ::operator+ (const CheckedIntValue &v1, const CheckedIntValue &v2);
	friend int ::operator- (const int &v1, const CheckedIntValue &v2);
	friend int ::operator- (const CheckedIntValue &v1, const int &v2);
	friend int ::operator- (const CheckedIntValue &v1, const CheckedIntValue &v2);
	friend int ::operator* (const int &v1, const CheckedIntValue &v2);
	friend int ::operator* (const CheckedIntValue &v1, const int &v2);
	friend int ::operator* (const CheckedIntValue &v1, const CheckedIntValue &v2);
	friend int ::operator/ (const int &v1, const CheckedIntValue &v2);
	friend int ::operator/ (const CheckedIntValue &v1, const int &v2);
	friend int ::operator/ (const CheckedIntValue &v1, const CheckedIntValue &v2);
	//friend int& ::operator+ (const CheckedIntValue &v);
};


#endif


