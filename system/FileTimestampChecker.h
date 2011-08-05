
#ifndef FILETIMESTAMPCHECKER_H
#define FILETIMESTAMPCHECKER_H

/**
 *
 * A utility class for checking file timestamps (modification times).
 * Provides an easy way to see if a file is newer than another file.
 *
 * @version 1.0, 27.8.2002
 * @author Jukka Kokkonen <jukka@frozenbyte.com>
 *
 */

class FileTimestampChecker
{
public:

  /**
   *
   * Check if given file with given name has been modified after another
   * file of given name.
   * @param file  char *, the name of the file to check.
   * @param thanfile  char *, the name of the file to check against.
   * @return bool, true if given "file" is newer than given "thanfile".
   *
   */

	static bool isFileNewerThanFile(const char *file, const char *thanfile);
	static bool isFileNewerOrSameThanFile(const char *file, const char *thanfile);
	static bool isFileNewerOrAlmostSameThanFile(const char *file, const char *thanfile);

	static bool isFileUpToDateComparedTo(const char *file, const char *thanfile);

//private:
	// ok, we want access to this too...

	static int getFileTimestamp(const char *file);
};

#endif
