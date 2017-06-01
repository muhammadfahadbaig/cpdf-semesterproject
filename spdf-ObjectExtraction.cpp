//
// SPDF OBJECT EXTRACTION AND MANIPULATION
//

#include "spdf-structure.h"
#include <sstream>
#include <regex>

using namespace std;

bool spdf::isIndirectObject(const std::string &source, size_t pos)
{
	size_t end;
	return isIndirectObject(source, pos, end);
}

bool spdf::isIndirectObject(const string& source, size_t pos, size_t &end)
{
	// Consider the pattern 8 8 obj
	if (source.size() < 7)
		return false; // Fails the basic most format check

	size_t objLiteralPos = source.find("obj", pos);
	if (objLiteralPos < 4 || !(source[objLiteralPos + 3] < '!'))
		return false; // failed whitespace check

	// check for whitespace
	if (source[--objLiteralPos] == ' ')
	{
		while (source[--objLiteralPos] >= '0' && source[objLiteralPos <= '9' && objLiteralPos != 1])
		{
			if (source[objLiteralPos] == ' ' && source.find_first_not_of("123456789",pos) == objLiteralPos)
			{
				if ((objLiteralPos = source.find("endobj", objLiteralPos)) != string::npos)
					end = objLiteralPos;
				return true;
			}
		}
	}
	return false;
}

void spdf::resolveObject(map<int, string>::iterator i)
{
	// Find the ending object position We're assuming all objects are in correct order which is not always the case but still...
	map<int, string>::iterator j = i;
	j++;
	if (j == xref.end())
	{
		i->second.append(",");
		i->second.append(to_string(xrefTableLocation - 1));
		return;
	}
	else if (j->second.substr(i->second.find_last_of(",") + 1) == "f") j++;
	if ( j != xref.end())
	{
		i->second.append(",");
		i->second.append(j->second.substr(0, j->second.find(',', 0)));
	}

}

void spdf::loadObjectStructure()
{
	/*
	To achieve this we'll start finding the ending offset of every object
	*/

	for (map<int, string>::iterator i = xref.begin(); i != xref.end(); i++)
	{
		// Verify first if the object is free or in use (free objects are simply ignored)
		if (i->second.substr(i->second.find_last_of(",") + 1) == "f")
			continue; // Jump to next object
		if (i->second.substr(i->second.find_last_of(",") + 1) == "n")
		{
			resolveObject(i);
			cout << i->second << endl;
		}
		else
		{
			break; // invalid PDF file TODO Add Exceptions
		}
	}

	// Start populating page structure
	string objectDictionary{};
	int j{};
	for (map<int, string>::iterator i = xref.begin(); i != xref.end(); i++, j++)
	{
		if (i->second.find_first_of("f") != string::npos)
			continue; // Jump to next object
		if (i->second.find_first_of("n") != string::npos)
		{
			if(loadPageTree(i)) break;
		}
		if (j == 62)
		{
			cout << endl;
		}
	}

	// Now we'll iterate on every known page to find for fonts
	for (int i = 0; i < spdfPages.totalPages; i++)
	{
		loadPageFonts(i); // Start loading fonts for the specified page
		loadFontInformation(i);
	}
}

string spdf::loadDictionary(string &input)
{
	// Extract dictionary
	string output{};
	auto dictStart = "<<"s;
	auto dictEnd = ">>"s;
	regex dictRx(dictStart + "(.*)" + "[\\S\\s]*" + dictEnd);
	smatch rxMatch;
	while (regex_search(input, rxMatch, dictRx))
	{
		output = rxMatch.str(); // Apparently the first match is (in most of the cases) all we need
		break; // skip the other matches (they're probably useless)
	}
	return output;
}