
#ifndef SIMPLEPARSER_H
#define SIMPLEPARSER_H

namespace util
{
  /**
   * A simple parser for files containing key=value pairs.
   * Ignores empty lines and lines starting with comments (two slashes)
   * @version 1.0, 22.10.2002
   * @author Jukka Kokkonen <jukka@frozenbyte.com>
   */
  class SimpleParser
  {
  public:
    SimpleParser();
    ~SimpleParser();

    /**
     * Enables or disables type checking for values.
     * By default, type checking is disabled.
     * If type checking is enabled, an error occurs when requested
     * value type does not match the type determined by parser.
     */
    /*
    void setTypeCheck(bool checkTypes);
    */

    /**
     * Loads a file into the parser.
     * @param  filename  char*, the file to load.
     * @return  bool, true on success, if file load failed, returns false.
     */
    bool loadFile(const char *filename);

    /**
     * Loads a memory buffer into the parser.
     */
    void loadMemoryBuffer(const char *buffer, int length);
    
    /**
     * Moves to next line. 
     * After loading a file, the position is set before the first line.
     * Should call this once before getting any values.
     * @return  bool, true if line is available, false if no more lines.
     */
    bool next(bool list_comments = false);

    /**
     * Returns the key at current position. NULL is returned if the
     * line has no key - or in other words, if no equal sign on that line.
     * If there is no equal sign at the line, you must use getLine to 
     * get the whole line if you want to know what the line contains.
     * @return  char*, key at current position or NULL if line has no key.
     */
    char *getKey();

    /**
     * Returns the string value at current position. NULL is returned if the
     * line has no value - or in other words, if no equal sign on that line.
     * @return  char*, string value at current position or NULL if line has no value.
     */
    char *getValue();

    /**
     * Return the int value at curret position.
     * If the line has no equal sign, value being returned is undefined.
     * (Hint: its really zero, but to do it properly, use getKey to 
     * check that the equal sign exists)
     */
    int getIntValue();

    /**
     * Return the float value at curret position.
     * If the line has no equal sign, value being returned is undefined.
     */
    float getFloatValue();

    /**
     * Returns the whole current line.
     */
    char *getLine();

    /**
     * Return current line number (to assist debugging in case of error).
     * Notice: value will be off by one when the file has *nix line breaks
     * (lf only) instead of having DOS line breaks (cr+lf).
     */
    int getLineNumber(); 

    /**
     * Used by the class internally, but can also be used by other classes
     * whenever they encounter a parsing error after getting some data
     * from parser. This way you will get the file and line numbers logged.
     */
    void error(const char *err);

  private:
    char *buf;
    char *cutBuf;
    int nextPos;
    int currentPos;
    int keyPos;
    int valuePos;
    char *currentFile;
    int linenum;
    int bufLen;

  };

}

#endif

