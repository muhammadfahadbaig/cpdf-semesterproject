//
// SPDF SUPPLEMENTARY
//

#include "spdf-structure.h"
#include <sstream>

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

