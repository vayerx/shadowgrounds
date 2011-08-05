
#ifndef TEXTFINDER_H
#define TEXTFINDER_H

namespace util
{
	class TextFinderImpl;

	class TextFinder
	{
		public:
			TextFinder(const char *textBuffer, bool skipComments = false);
			~TextFinder();

			// set comment line skipping on/off
			void setSkipComments(bool skipComments);

			// move finder to the beginning of the buffer
			void moveToStart();

			// move finder to the end of the buffer
			void moveToEnd();

			// move finder to given position in the buffer
			void moveToPosition(int position);

			// returns current finder position
			int getCurrentPosition();

			// returns position of string to find or -1 if not found
			// updates current finder position to beginning of the found string or
			// at the end of buffer in string not found
			// NOTE: remember to moveToStart or appropriate position before calling this 
			int findNext(const char *stringToFind);

			// finds the n'th occurance of the string from the beginning 
			// returns position of string to find or -1 if not found
			// updates current finder position to beginning of the found string or
			// at the end of buffer in string not found
			// NOTE: remember to moveToStart or appropriate position before calling this 
			int findOneOfMany(const char *stringToFind, int findNumber);

			// returns the number of occurances for the given string in the buffer
			// NOTE: remember to moveToStart or appropriate position before calling this 
			int countOccurances(const char *stringToFind);

		private:
			TextFinderImpl *impl;
	};
}



#endif

