
#ifndef PARTTYPEPARSER_H
#define PARTTYPEPARSER_H

namespace game
{
	class PartTypeParserImpl;

  class PartTypeParser
  {
  public:
		PartTypeParser();
		~PartTypeParser();

    void loadPartTypes(const char *filename);

		static void clearLoadedList(const char *fileExtension);

  private:
    void error(const char *err, int linenum);

		PartTypeParserImpl *impl;
  };

}

#endif
