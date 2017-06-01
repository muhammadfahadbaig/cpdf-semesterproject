//
//	SPDF PAGE EXTRACTION (TREE IMPLEMENTATION)
// 

#include"spdf-structure.h"
#include<regex>

using namespace std;

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

	objectData = loadDictionary(objectData);

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

		objectData = loadDictionary(objectData);

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
					// Remove starting [ and ending ]
					temp = temp.substr(temp.find_first_of("[") + 1, temp.size() - 2);
					// Feed the data to array
					for (int j = 0; j < spdfPages.totalPages; j++)
					{
						pageDef entry;
						if (j == spdfPages.totalPages - 1)
						{
							entry.pageObject = stoi(temp);
						}
						else
						{
							if (temp[0] == ' ') temp = temp.substr(1);
							entry.pageObject = stoi(temp.substr(0, temp.find_first_of(" ")));
							temp = temp.substr(temp.find_first_of(" ") + 1);
						}
						entry.parentObj = i->first;
						entry.pageNumber = j + 1;
						spdfPages.pageObjects.push_back(entry);
					}
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