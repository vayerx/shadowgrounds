
#ifndef SCRIPTMANAGER_H
#define SCRIPTMANAGER_H

#include <map>
#include <vector>
#include <string>

#include "Script.h"
#include "IScriptProcessor.h"

class LinkedList;

namespace util
{

	typedef std::map<int, Script *> ScriptHashType;

	// < hashcode, < filename, timestamp > >
	typedef std::map<int, std::pair<char *, unsigned long> > ScriptFileHashType;

	class ScriptManager
	{
	public:

		ScriptManager();
		~ScriptManager();

		static ScriptManager *getInstance();

		static void cleanInstance();

		void loadScripts(const char *filename, const char *relativeToFilenamePath, bool replace = false);

		void loadMemoryScripts(const char *filename, char *buf, int buflen, bool replace);

		// returns the amount of scripts that were reloaded.
		int reloadChangedScripts();

		// returns true on success. bufOut assigned to newly allocated buffer, bufLenOut contains used buffer size
		// (note, used buffer size may still be slightly smaller than actually allocated memory block size)
		bool doInternalPreprocess(const char *filename, char **bufOut, int *bufLenOut, const char *sourceBuf, int sourceBufLen);

		void setKeywords(int amount, const char **keywords, int *datatypes);
		
		void setProcessor(IScriptProcessor *processor);

		Script *getScript(const char *scriptName);

		void setForcePreprocessForNextLoad(bool forcepp);

		// For Script class access only...
		void scriptCompileError(const char *err, bool isError);

		void loadInternalPreprocessorMacros(const char *filename);
		void clearInternalPreprocessorMacros();

		int getScriptAmount();
		std::string getStatusInfo();

	private:

		void error(const char *err, int linenum, bool isError);

		static ScriptManager *instance;

		ScriptHashType *scriptNameHash;

		ScriptFileHashType *fileHash;

		IScriptProcessor *processor;

		int keywordsAmount;
		char **keywords;
		int *keywordDatatypes;

		std::vector<std::pair<std::string, std::string> > internalMacros;

		LinkedList *allScripts;

// TEMP: argh.
// FIXME: oh please, get rid of this.
friend class Script;

	};

}

#endif
