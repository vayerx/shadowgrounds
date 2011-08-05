
#include "TextFileModifier.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Debug_MemoryManager.h"

int main()
{
	FILE *f = fopen("textfilemod_test.txt", "wb");
	if (f == NULL)
	{
		printf("ERROR: Failed to open test file for writing.");
		return 1;
	}

	fprintf(f, "foobar1\r\n// <foobartag> foo\r\nthis is test line\r\n\r\nfinal foo\r\n");

	fclose(f);

	util::TextFileModifier tfm;

	bool success;
	success = tfm.loadFile("textfilemod_test.txt");
	if (!success)
	{
		printf("ERROR: loadFile failed.");
	}

	tfm.setStartSelectionNearMarker("<foobartag>");
	tfm.setEndSelectionNearMarker("<foobartag>");
	char *foo = tfm.getSelectionAsNewBuffer();
	printf("This should contain the footag line: \"%s\"\r\n", foo);

	delete [] foo;

	tfm.deleteSelection();

	success = tfm.saveFileAs("textfilemod_test2.txt");
	if (!success)
	{
		printf("ERROR: SaveFileAs failed.\r\n");
	}
	success = tfm.saveFile();
	if (!success)
	{
		printf("ERROR: SaveFile failed.\r\n");
	}

	tfm.closeFile();

	tfm.loadFile("textfilemod_test2.txt");

	tfm.setStartSelectionToStart();
	tfm.setEndSelectionToEnd();
	char *foo2 = tfm.getSelectionAsNewBuffer();
	printf("This should contain file with tag deleted: \"%s\"\r\n", foo2);

	delete [] foo2;

	tfm.setStartSelectionNearMarker("this is ");
	tfm.setEndSelectionNearMarker("this is ");

	tfm.addAfterSelection("(this is an added line 2)\r\n");
	tfm.addAfterSelection("(this is an added line 1)\r\n");
	tfm.addBeforeSelection("(this line was added before the first this is line)\r\n");
	tfm.addBeforeSelection("foo bar replace test \r\n");
	tfm.addBeforeSelection("foo bar replace test 2\r\n");

	tfm.setStartSelectionToStart();
	tfm.setEndSelectionToEnd();

	tfm.replaceString("bar", "foo", false);

	success = tfm.saveFileAs("textfilemod_test3.txt");
	if (!success)
	{
		printf("ERROR: SaveFileAs failed (textfilemod_test3.txt).\r\n");
	}

	tfm.setStartSelectionToStart();
	tfm.setEndSelectionToEnd();
	char *foo3 = tfm.getSelectionAsNewBuffer();
	printf("And with some lines added: \"%s\"\r\n", foo3);

	delete [] foo3;

	tfm.closeFile();

	printf("Done.\r\n");

	return 0;

}

