
#ifndef UPDATELINKS_H
#define UPDATELINKS_H

#include <string>

class UpdateLinks
{
public:
	/**
	 * Fetches the linked updates from the html file using the given updater.
	 * Only links with given class are processed.
	 */
	static void getLinkedUpdatesFromFile(std::string linksFilename, std::string linkClass, 
		std::string fileUpdater, std::string stripOffExtension);

	/**
	 * This one just returns a list containing the link urls (extension stripped).
	 */
	static std::vector<std::string> getLinksFromFile(std::string linksFilename, std::string linkClass, 
		std::string stripOffExtension);

};


#endif

