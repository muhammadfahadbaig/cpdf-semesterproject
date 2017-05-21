#include "cpdf-structure.h"

using namespace std;

// PDF version of getline that obeys PDF newline conventions \n \r or \r\n and returns the next string in PDF
std::string cpdf::getline(fstream &source)
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