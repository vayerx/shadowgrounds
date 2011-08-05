
#include "HTMLLinkParser.h"

namespace util
{

	std::vector<HTMLLinkParserData> HTMLLinkParser::parseLinks(std::string htmlBuffer)
	{
		int lastAPos = 0;
		int lastHrefPos = -1;
		int lastNamePos = -1;
		int lastClassPos = -1;
		int lastHrefEnd = -1;
		int lastNameEnd = -1;
		int lastClassEnd = -1;
		bool inA = false;
		bool inATag = false;

		std::vector<HTMLLinkParserData> ret;

		for (int i = 0; i < (int)htmlBuffer.length(); i++)
		{
			if (htmlBuffer[i] == '<')
			{
				if (htmlBuffer.substr(i, 3) == "<a ")
				{
					lastAPos = i;
					inA = true;
					inATag = true;
				}
				if (htmlBuffer.substr(i, 4) == "</a>")
				{
					inA = false;
					inATag = false;

					if (lastHrefPos != -1 && lastHrefEnd != -1 
						&& lastNamePos != -1 && lastNameEnd != -1 
						&& lastClassPos != -1 && lastClassEnd != -1)
					{
						HTMLLinkParserData lData;

						lData.linkURL = htmlBuffer.substr(lastHrefPos, lastHrefEnd-lastHrefPos);
						lData.linkName = htmlBuffer.substr(lastNamePos, lastNameEnd-lastNamePos);
						lData.linkClass = htmlBuffer.substr(lastClassPos, lastClassEnd-lastClassPos);

						ret.push_back(lData);
					}

					lastHrefPos = -1;
					lastNamePos = -1;
					lastClassPos = -1;
				}
			} else {
				if (inA)
				{
					if (htmlBuffer.substr(i, 5) == "href=")
					{
						lastHrefPos = i+5;
						lastHrefEnd = -1;

						if (htmlBuffer[lastHrefPos] == '"')
						{
							lastHrefPos++;
						}
						for (int j = lastHrefPos; j < (int)htmlBuffer.length(); j++)
						{
							if (htmlBuffer[j] == ' '
								|| htmlBuffer[j] == '"')
							{
								lastHrefEnd = j;
								break;
							}
						}
					}
					if (htmlBuffer.substr(i, 6) == "class=")
					{
						lastClassPos = i+6;
						lastClassEnd = -1;

						if (htmlBuffer[lastClassPos] == '"')
						{
							lastClassPos++;
						}
						for (int j = lastClassPos; j < (int)htmlBuffer.length(); j++)
						{
							if (htmlBuffer[j] == ' '
								|| htmlBuffer[j] == '"')
							{
								lastClassEnd = j;
								break;
							}
						}
					}
					if (inATag && htmlBuffer[i] == '>')
					{
						inATag = false;
						lastNamePos = i+1;
						lastNameEnd = -1;

						for (int j = lastClassPos; j < (int)htmlBuffer.length(); j++)
						{
							if (htmlBuffer[j] == '<')
							{
								lastNameEnd = j;
								break;
							}
						}

					}
				}
			}
		}
		return ret;
	}


}