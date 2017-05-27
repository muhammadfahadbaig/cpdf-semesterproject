//
// Created by Test on 5/27/2017.
//

#include "spdf-structure.h"

using namespace std;

// PDF version of getline that obeys PDF newline conventions \n \r or \r\n and returns the next string in PDF
std::string spdf::getline(fstream &source)
{
	string nextLine{};
	char tempChar{};
	while (source.get(tempChar))
	{
		nextLine += tempChar;
		if (tempChar == '\n')
			break;
		if (tempChar == '\r') {
			if (!source.eof() && source.peek() == '\n')
				nextLine += source.get();
			break;
		}
	}
	return nextLine;
}

// A simple string trim function specialised for our needs
string spdf::trim(string line)
{
	return line.substr(0, line.find_last_not_of("\r\t\n ") + 1);
}

/* 
	Extact the whole object's definition into a string
	The format is kind of like below:
	1 0 obj
	<<// object definition
	// other references
	// other sample>>
	endobj
*/

string spdf::resolveObject(map<int, string>::iterator i)
{
	// Extract object's offset
	int objectOffset = stoi(i->second.substr(0, i->second.find_first_of(",")));
	pdfFile.seekg(objectOffset, ios::beg);
	
	// Verify if the declaration is correct (1 X obj)
	string temp = trim(spdf::getline(pdfFile));
	if (stoi(temp.substr(0, temp.find_first_of(" "))) == i->first &&
		temp.substr(temp.find_last_of(" ")+1) == "obj")
	{
		// Now perform the operations
		temp = "";
		while (true)
		{
			string check{};
			check = trim(spdf::getline(pdfFile));
			if (check != "endobj")
			{
				temp.append(check);
				temp.append(&newline);
			}
			else
			{
				break;
			}
				
		}
	}
	else
	{
		temp = "Invalid Object!";
	}
	return temp;
}