//
//	SPDF FONTS EXTRACTION
//

#include"spdf-structure.h"
#include<regex>


using namespace std;

// Feed all object identifiers which represent fonts used in a page
bool spdf::loadPageFonts(int pg)
{
	using namespace std::string_literals;

	int pgObj = spdfPages.pageObjects[pg].pageObject;
	map<int, string>::iterator i = xref.find(pgObj);

	int offset = stoi(i->second.substr(0, i->second.find(',', 0)));
	int endingOffset = stoi(i->second.substr(i->second.find_last_of(",") + 1));
	pdfFile.clear(); // Dirty Clear : in future this function will be rewritten to handle streams and avoid seek corruption
	pdfFile.seekg(offset);
	vector<char> objData(endingOffset - offset);
	pdfFile.read(&objData[0], endingOffset - offset);

	// Convert vector into string
	string objectData(objData.begin(), objData.end());
	objectData = trim(objectData);

	string temp;

	objectData = loadDictionary(objectData);

	// Extract Page MediaBox
	if (objectData.find("/MediaBox") != string::npos)
	{
		temp = objectData;
		temp = temp.substr(temp.find("/MediaBox") + 1);
		// Extract the first relevant arrray
		auto arrayStart = "\\["s;
		auto arrayEnd = "\\]"s;
		regex aObj(arrayStart + "(.*)" + "[\\S\\s]*" + arrayEnd);
		smatch aMatch;
		if (regex_search(temp, aMatch, aObj)) // IF expression found i.e. array found
		{
			temp = aMatch.str();
			// Feed the data to MediaBox Array
			for (int i = 0; i < 4; i++)
			{
				if (temp[0] == ' ') temp = temp.substr(1);
				if (temp[0] == '[') temp = temp.substr(1);
				if (temp[temp.size()] == ']') temp = temp.substr(0, temp.size() - 1);
				if (temp[0] == ' ') temp = temp.substr(1); // Recleaning
				spdfPages.pageObjects[pg].pageSize[i] = stoi(temp.substr(0, temp.find_first_of(' ')));
				temp = temp.substr(temp.find_first_of(' ') + 1);
			}
		}
	}

	// Extract Fonts Location
	/*
	We have two cases for font extraction. Either the font is stored as direct reference like /Font 5 0 R
	or in a dictionary /Font <<\F1 5 0 R\F2 6 0 R>>.
	Furthermore some PDFs have /Resources flag which refer to page resources like fonts and images so we'll cater them too!
	*/
	/*
	To get to font we first need to find the /Resources flag. If multiple resources are present they are stored as dictionary. Otherwise a
	single reference will be found with no idea if it's a font or a image or anything basically (for files which have a SINGLE resource)
	*/
	// TODO : Are RESOURCES STORED AS AN ARRAY?

	// Constructing a Regex for the case /Resources<< ..... >>/
	auto startResources = "/Resources<<"s;
	auto endResources = ">>/"s;
	regex regExResources(startResources + "(.*)" + "[\\S\\s]*" + endResources);
	smatch rsMatch;
	bool foundRes = false;
	bool fndFonts = false;

	while (regex_search(objectData, rsMatch, regExResources))
	{
		temp = rsMatch.str(); // We now have the string which will surely contain /Resources<<
		foundRes = true;
		break; // first case needed only
	}

	if (foundRes == true)
	{
		// Search for /Font in the resultant temp string
		if (temp.find("/Font") != string::npos) temp = temp.substr(temp.find("/Font") + 5);
		else return false; // Fonts didnt't successfully loaded up TODO Add exceptions

		 // Now we'll extract the Font Dictionary
		temp = temp.substr(temp.find_first_of("<<") + 2);
		temp = temp.substr(0, temp.find_first_of(">>"));

		// Perform cleaning
		regex reference(" 0 R");
		temp = regex_replace(temp, reference, "");
		regex fontRef("/F([1-9][0-9]*)");
		temp = regex_replace(temp, fontRef, "");
		// For Truetype
		regex fontRefTT("/TT([1-9][0-9]*)");
		temp = regex_replace(temp, fontRef, "");

		// Feed the data
		spdfPages.pageObjects[pg].totalFonts = 0;
		while (temp != "")
		{
			if (temp[0] == ' ') temp = temp.substr(1);
			fontDef entry;
			entry.fontObject = stoi(temp.substr(0, temp.find_first_of(' ')));
			spdfPages.pageObjects[pg].totalFonts++;
			spdfPages.pageObjects[pg].fontObjects.push_back(entry);
			if (temp.find(" ") == string::npos) break;
			temp = temp.substr(temp.find_first_of(' ') + 1);
		}
		fndFonts = true; // we successfully found the fonts
	}
	else
	{
		temp = objectData;
		// Simply find /Resources tag and load up the object
		if (temp.find("/Resources") != string::npos)
		{
			temp = temp.substr(temp.find("/Resources") + 10);
			if (temp[0] == ' ') temp = temp.substr(1);
			int objectRef = stoi(temp.substr(0, temp.find_first_of(" ")));

			// Goto object
			map<int, string>::iterator tempPos = xref.find(objectRef);
			if (tempPos != xref.end())
			{
				int offset = stoi(tempPos->second.substr(0, tempPos->second.find(',', 0)));
				int endingOffset = stoi(tempPos->second.substr(tempPos->second.find_last_of(",") + 1));
				pdfFile.clear();
				pdfFile.seekg(offset, ios::beg);

				vector<char> objData(endingOffset - offset);
				pdfFile.read(&objData[0], endingOffset - offset);

				// Convert vector into string
				string objectData(objData.begin(), objData.end());
				objectData = trim(objectData);

				objectData = loadDictionary(objectData);

				// Perform a simple search and extract the required information
				if (objectData.find("/Font<<") != string::npos)
				{
					// Now we'll extract the Font Dictionary
					temp = temp.substr(temp.find_first_of("<<") + 2);
					temp = temp.substr(0, temp.find_first_of(">>"));

					// Perform cleaning
					regex reference(" 0 R");
					temp = regex_replace(temp, reference, "");
					regex fontRef("/F([1-9][0-9]*)");
					temp = regex_replace(temp, fontRef, "");
					// For Truetype
					regex fontRefTT("/TT([1-9][0-9]*)");
					temp = regex_replace(temp, fontRef, "");

					// Feed the data
					spdfPages.pageObjects[pg].totalFonts = 0;
					while (temp != "")
					{
						if (temp[0] == ' ') temp = temp.substr(1);
						fontDef entry;
						entry.fontObject = stoi(temp.substr(0, temp.find_first_of(' ')));
						spdfPages.pageObjects[pg].totalFonts++;
						spdfPages.pageObjects[pg].fontObjects.push_back(entry);
						if (temp.find(" ") == string::npos) break;
						temp = temp.substr(temp.find_first_of(' ') + 1);
					}
					fndFonts = true; // we successfully found the fonts
					
				}
				else if (objectData.find("/Font") != string::npos)
				{
					objectData = objectData.substr(objectData.find("/Font") + 5);
					if (objectData[0] == ' ') objectData = objectData.substr(1);
					spdfPages.pageObjects[pg].totalFonts = 1;
					int tempPos = stoi(objectData.substr(0, objectData.find_first_of(" ")));
					
					map<int, string>::iterator iter = xref.find(tempPos);

					int offset = stoi(iter->second.substr(0, iter->second.find(',', 0)));
					int endingOffset = stoi(iter->second.substr(iter->second.find_last_of(",") + 1));
					pdfFile.clear();
					pdfFile.seekg(offset, ios::beg);

					vector<char> objData(endingOffset - offset);
					pdfFile.read(&objData[0], endingOffset - offset);

					// Convert vector into string
					objectData = string(objData.begin(), objData.end());
					objectData = trim(objectData);

					objectData = loadDictionary(objectData);

					objectData = objectData.substr(objectData.find("/F0 ") + 4);

					fontDef entry;
					entry.fontObject = stoi(objectData.substr(0, objectData.find(" ")));
					spdfPages.pageObjects[pg].fontObjects.push_back(entry);
					spdfPages.pageObjects[pg].totalFonts = 1;
					fndFonts = true;
				}
			}
		}
	}
	return fndFonts;
}

// Feed data to the fonts contained in a page
void spdf::loadFontInformation(int pg)
{
	// A simple loop will suffice here to do our job
	for (int i = 0; i < spdfPages.pageObjects[pg].totalFonts; i++)
	{
		map<int, string>::iterator fontPos = xref.find(spdfPages.pageObjects[pg].fontObjects[i].fontObject);
		if (fontPos != xref.end())
		{
			// Font found!
			int offset = stoi(fontPos->second.substr(0, fontPos->second.find(',', 0)));
			int endingOffset = stoi(fontPos->second.substr(fontPos->second.find_last_of(",") + 1));
			pdfFile.clear(); // Dirty Clear : in future this function will be rewritten to handle streams and avoid seek corruption
			pdfFile.seekg(offset);
			vector<char> objData(endingOffset - offset);
			pdfFile.read(&objData[0], endingOffset - offset);

			// Convert vector into string
			string objectData(objData.begin(), objData.end());
			objectData = trim(objectData);

			// Load dictionary
			objectData = loadDictionary(objectData);

			string temp(objectData);

			// Simple check to ensure it's a font object
			if (temp.find("/Type/Font") != string::npos || temp.find("/Type /Font") != string::npos
				|| temp.find("/Type\n/Font") != string::npos || temp.find("/Type\r/Font") != string::npos)
			{
				// Load Basefont
				temp = temp.substr(temp.find("/BaseFont") + 9);
				regex seq("/");
				temp = regex_replace(temp, seq, " ");
				while (true)
				{
					if (temp[0] == ' ') temp = temp.substr(1);
					else break;
				}
				temp = temp.substr(0,temp.find_first_of(" "));
				temp = regex_replace(temp, regex("#20"), " ");
				replace(temp.begin(), temp.end(), '+', ' ');
				temp = regex_replace(temp, regex("ABCDEE"), "");
				temp = regex_replace(temp, regex("BCDEEE"), "");
				if (temp[0] == ' ') temp = temp.substr(1);
				spdfPages.pageObjects[pg].fontObjects[i].baseFont = spdf::trim(temp);

				// Load Subtype
				temp = objectData;
				temp = temp.substr(temp.find("/Subtype") + 8);
				temp = regex_replace(temp, regex("/"), " ");
				while (true)
				{
					if (temp[0] == ' ') temp = temp.substr(1);
					else break;
				}
				temp = temp.substr(0,temp.find_first_of(" "));
				spdfPages.pageObjects[pg].fontObjects[i].fontType = spdf::trim(temp);
			}
		}
	}
}