
#ifndef DOORCREATOR_H
#define DOORCREATOR_H

namespace util
{
	class TextFileModifier;
}

class DoorCreator
{
	public:
		static bool createDoor(const char *name, const char *partType, 
			const char *modelFilename, bool singleSide, const char *leftLayer, const char *rightLayer,
			int blockRadius);

	private:
		static void replaceStrings(util::TextFileModifier &tfm, const char *name, const char *partType, 
			const char *modelFilename, bool singleSide, const char *leftLayer, const char *rightLayer,
			const char *partTypeLowerCase, int blockRadius, unsigned int timeStamp);
};

#endif

