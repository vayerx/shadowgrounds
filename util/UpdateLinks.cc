
#include "HTMLLinkParser.h"
#include "UpdateLinks.h"
#include <string>
#include <stdio.h>
#include "../util/hiddencommand.h"

using namespace util;

void UpdateLinks::getLinkedUpdatesFromFile(std::string linksFilename, std::string linkClass, 
  std::string fileUpdater, std::string stripOffExtension)
{
	std::vector<std::string> linklist = getLinksFromFile(linksFilename, linkClass, stripOffExtension);

	for (int i = 0; i < (int)linklist.size(); i++)
	{
		std::string cmd = fileUpdater;
		cmd += " ";
		cmd += linklist[i];

		//system(cmd.c_str());
		hiddencommand((char*)cmd.c_str(), false);
	}
}


std::vector<std::string> UpdateLinks::getLinksFromFile(std::string linksFilename, std::string linkClass, 
	std::string stripOffExtension)
{
	std::string htmlBuffer = "";

	std::vector<std::string> ret;

	// TODO: read html buffer from linksFilename file

	FILE *f = fopen(linksFilename.c_str(), "rb");
	if (f != NULL)
	{
		fseek(f, 0, SEEK_END);
		int size = ftell(f);
		fseek(f, 0, SEEK_SET);

		if (size > 0)
		{
			char *buf = NULL;
			buf = new char[size + 1];

			int got = fread(buf, size, 1, f);
			if (got == 1)
			{
				htmlBuffer = buf;
			}

			delete [] buf;
		}

		fclose(f);
	}

	std::vector<HTMLLinkParserData> links;
	links = HTMLLinkParser::parseLinks(htmlBuffer);

	for (int i = 0; i < (int)links.size(); i++)
	{
		if (links[i].linkClass == linkClass)		
		{
			// strip extension off...?
			std::string stripped = links[i].linkURL;
			if (links[i].linkURL.length() >= stripOffExtension.length())
			{
				if (links[i].linkURL.substr(links[i].linkURL.length() - stripOffExtension.length(), stripOffExtension.length()) 
					== stripOffExtension)
				{
					stripped = links[i].linkURL.substr(0, links[i].linkURL.length() - stripOffExtension.length());
				}
			}

			ret.push_back(stripped);
		}
	}

	return ret;
}

