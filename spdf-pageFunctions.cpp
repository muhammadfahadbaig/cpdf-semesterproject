#include "spdf-structure.h"

using namespace std;

void spdf::loadPageStructure()
{
	/*
		To achieve this we'll start iterating on every known object searching for the tags and extracting
		all relevant information we need like catalog, kids and page count
	*/

	// Start searching the CATALOG OBJECT
	for (map<int, string>::iterator i = xref.begin(); i != xref.end(); i++)
	{
		// Verify first if the object is free or in use (free objects are simply ignored)
		if (i->second.substr(i->second.find_last_of(",")+1) == "f")
			continue; // Jump to next object
		if (i->second.substr(i->second.find_last_of(",")+1) == "n")
		{
			string objectData = resolveObject(i);
			cout << objectData;
		}
		else
		{
			break; // invalid PDF file TODO Add Exceptions
		}
	}
}