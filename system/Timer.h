
#ifndef TIMER_H
#define TIMER_H

/**
 * A simple class for getting time. Provides optimized way of querying
 * the system time. Requires the update method to be called every 
 * once a while or the returned time will not be properly updated.
 * (Zero time undefined, probably the start of program or last midnight.)
 * (Timer accuracy is also undefined, probably 10 ms or something alike,
 * but not guaranteed to be that.)
 *
 * @version 1.1, 1.10.2002
 * @author Jukka Kokkonen <jukka@frozenbyte.com>
 *
 */

class Timer
{
public:
  /** 
   * Should be called at application startup.
	 * Before any getTime calls.
   */
	static void init();

  /** 
   * Should be called when the application is quitting.
   */
	static void uninit();

  /** 
   * Returns the current time in milliseconds.
   * (Value will be the time when update was last called!)
   */
  static int getTime();


  // return the current time right now
  static int getCurrentTime();


  static void setTimeFactor(float factor);
	static float getTimeFactor();

  /** 
   * Returns the current time in milliseconds, without factor multiplication
   * (Value will be the time when update was last called!)
   */
  static int getUnfactoredTime();

  /** 
   * Updates the time. Should be called regularly.
   */
  static void update();


	// (I wonder what this is???, possibly something to do with particle editor or something???)
  static void addTimeSub(int time);

private:
  static int currentTime; 
  static int currentUnfactoredTime; 
	static int factorTimeAdd;
	static int timeHaxSub;
	static int timeFactor; // note, fixed point 1/1024
};

#endif

