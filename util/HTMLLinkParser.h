
#ifndef HTMLLINKPARSER_H
#define HTMLLINKPARSER_H

#include <string>
#include <vector>

namespace util
{
	struct HTMLLinkParserData
	{
		public:
			std::string linkURL;
			std::string linkName;
			std::string linkClass;
	};

	class HTMLLinkParser
	{
		public:
			/**
			 * Parses links out of html text file.
			 * return a vector of the links in data struct format
			 */
			static std::vector<HTMLLinkParserData> parseLinks(std::string htmlBuffer);
	};

}

#endif
