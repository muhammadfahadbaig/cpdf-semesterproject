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
}

bool spdf::loadPageTree(map<int, string>::iterator i)
{
	using namespace std::string_literals;

	int offset = stoi(i->second.substr(0, i->second.find(',', 0)));
	int endingOffset = stoi(i->second.substr(i->second.find_last_of(",") + 1));
	pdfFile.clear(); // Dirty Clear : in future this function will be rewritten to handle streams and avoid seek corruption
	pdfFile.seekg(offset);
	vector<char> objData(endingOffset - offset);
	pdfFile.read(&objData[0], endingOffset - offset);

	// Convert vector into string
	string objectData(objData.begin(), objData.end());
	objectData = trim(objectData);

	// Extract dictionary
	auto dictStart = "<<"s;
	auto dictEnd = ">>"s;
	regex dictRx(dictStart + "(.*)" + "[\\S\\s]*" + dictEnd);
	smatch rxMatch;
	while (regex_search(objectData, rxMatch, dictRx))
	{
		objectData = rxMatch.str(); // Apparently the first match is (in most of the cases) all we need
		break; // skip the other matches (they're probably useless)
	}
	/*objectData = objectData.substr(objectData.find_first_of("<<"), objectData.find_last_of(">>") - objectData.find_first_of("<<") + 1);*/

	// Find Catalog first
	bool fndCatalog = false;
	if (objectData.find("/Type/Catalog") != string::npos || objectData.find("/Type\0/Catalog") != string::npos ||
		objectData.find("/Type\r/Catalog") != string::npos || objectData.find("/Type\n/Catalog") != string::npos)
	{
		spdfPages.pageCatalogObject = i->first;
		fndCatalog = true;
	}

	// Find Pages Containing Object
	
	auto pageStart = "/Pages "s;
	auto pageEnd = "\/"s; // TODO Further improve implementation to enable more diverse trees
	regex pObj(pageStart + "(.*)");
	smatch pMatch;
	bool fndKids = false;
	while (regex_search(objectData, pMatch, pObj) && fndCatalog == true)
	{
		// Found Pages Object
		string temp = pMatch[1].str();
		temp = temp.substr(0, temp.find(",", 0));
		spdfPages.pageKidsObject = stoi(temp);
		fndKids = true;
		break;
	}

	if (fndKids == true)
	{
		// Start Populating Kids
		bool fndPages = false;
		map<int, string>::iterator kidsObj = xref.find(spdfPages.pageKidsObject);
		offset = stoi(kidsObj->second.substr(0, kidsObj->second.find(',', 0)));
		endingOffset = stoi(kidsObj->second.substr(kidsObj->second.find_last_of(",") + 1));

		pdfFile.seekg(offset);
		vector<char> objData(endingOffset - offset);
		pdfFile.read(&objData[0], endingOffset - offset);
		
		objectData = string(objData.begin(), objData.end());

		// Extract dictionary
		while (regex_search(objectData, rxMatch, dictRx))
		{
			objectData = rxMatch.str(); // Apparently the first match is (in most of the cases) all we need
			break; // skip the other matches (they're probably useless)
		}

		// Find Pages tag first
		if (objectData.find("/Type/Pages") != string::npos || objectData.find("/Type\0/Pages") != string::npos ||
			objectData.find("/Type\r/Pages") != string::npos || objectData.find("/Type\n/Pages") != string::npos)
		{
			fndPages = true; // Found Pages Tag
		}

		// Extract Page Count
		auto pageStart = "/Count "s;
		regex pObj(pageStart + "(.*)");
		smatch pMatch;
		bool fndCount = false;
		while (regex_search(objectData, pMatch, pObj) && fndCatalog == true)
		{
			// Found Page Count
			string temp = pMatch[1].str();
			temp = temp.substr(0, temp.find(",", 0));
			spdfPages.totalPages = stoi(temp);
			fndCount = true;
			break;
		}

		// Extract Kids Object References
		// We now know how many kids (pages) references to expect So we'll start extracting them
		// Such object will ALWAYS have ONE array
		if (fndCount == true) // An object containing count must have the kids object
		{
			if (objectData.find("/Kids"))
			{ // Kids located
				objectData.substr(objectData.find("/Kids") + 1);
				auto arrayStart = "\\["s;
				auto arrayEnd = "\\]"s;
				regex aObj(arrayStart + "(.*)" + "[\\S\\s]*" + arrayEnd);
				smatch aMatch;
				if (regex_search(objectData, aMatch, aObj)) // IF expression found i.e. array found
				{
					string temp = aMatch.str();
					// Since we're ignoring generation count we'll remove 0 R to get a clean array of integers
					regex refer(" 0 R");
					temp = regex_replace(temp, refer, "");
					return true;
				}
				else
				{
					// Illegal syntax or something else
					// TODO Catch Invalid PDF exception
				}
			}
		}

	}

	return false; // Page Catalog not found in selected iterator	
}