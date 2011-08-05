
#ifndef TEXTFILEMODIFIER_H
#define TEXTFILEMODIFIER_H

namespace util
{
	class TextFileModifierImpl;

	class TextFileModifier
	{
		public:
			TextFileModifier();

			~TextFileModifier();

			// replaces occurances of given string with a new string
			// note: this works on single character level, whereas the other methods
			// in this class work on whole line level.
			// returns amount of string occurances replaced.
			int replaceString(const char *stringToBeReplaced, const char *newString, bool insideSelectionOnly);

			// begin selection from the start of the file
			void setStartSelectionToStart();

			// end selection to the very end of the file
			void setEndSelectionToEnd();

			// move selection to the beginning of the file
			void setBothSelectionsToStart();

			// move selection to the end of the file
			void setBothSelectionsToEnd();

			// returns true if found, else false
			// (inclusive, includes the marker line)
			bool setStartSelectionNearMarker(const char *marker);

			// returns true if found, else false
			// (inclusive, includes the marker line)
			bool setEndSelectionNearMarker(const char *marker);

			// just combines start to start and end to end.
			void selectAll();

			// begin selection from given position (inclusive)
			void setStartSelectionToPosition(int position);

			// end selection to given position (exclusive)
			void setEndSelectionToPosition(int position);

			// keeps only the inside of the positions
			void cropToSelection();

			// keeps only the outside of the positions
			void deleteSelection();

			// returns the selection inside a new buffer
			// (delete the buffer once done with it)
			char *getSelectionAsNewBuffer();

			void addAfterSelection(const char *text);

			void addBeforeSelection(const char *text);

			// returns true if success, else false
			bool loadFile(const char *filename);

			bool saveFile();
			
			bool saveFileAs(const char *filename);
			
			// will NOT save the file even if modified. just get rid of it.
			void closeFile();

			void newFile();

			// load a memory buffer as a file (note, can be saved with saveFileAs only)
			void loadMemoryFile(const char *buf, int buflen);

		private:
			TextFileModifierImpl *impl;

	};

}

#endif




