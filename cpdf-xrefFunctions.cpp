#include "cpdf-structure.h"

using namespace std;

dictionary cpdf::loadXref(size_t xrefPos) // TODO : Add compressed streams support
{
	rawText.seekg(xrefPos);
	dictionary xrefDict;
	string line;

	// Extract the xref table
	while (true)
	{
		line = getline(rawText);

		if (line.length > 0) line = line.substr(0, line.find_last_not_of("\r\t\n ") + 1); // Adopting PDF newline conventions and removing trailing spaces and break sequences
		if (line.length == 0) break;

		/*
			A normal entry in Xref table looks like one shown below
			0000000000 65535 f
			The total character count is exactly 20 (apart from some PDFs which violate 20 byte rule) (including \r\n). So a single entry must contain
			exactly two numbers with spaces between them. We will validate this since if the convention
			isn't matched the xref entry isnt valid and the file is either possibily corrupt or isn't even a valid
			PDF file or the file is not supported (again mentioning Linearized PDFs are not supported so this method
			won't work with such files)
		*/

		size_t spacePosition = line.find_first_of(" ");
		if (spacePosition != string::npos &&
			line.substr(0, spacePosition).find_first_not_of("0123456789") == string::npos &&
			line.substr(spacePosition + 1).find_first_not_of("0123456789") == string::npos)
			/*
				npos : indicates no matches
				The last two conditions check if the extracted substring contains anything which is not an integer
				since if there is a character other than integer the entry isn't valid and data should be discarded
			*/
		{
			cpdfXref xrefEntry;
			// Populate xref
			xrefEntry.startPos = atoi(line.substr(0, spacePosition).c_str()); // atoi requires c style strings
			xrefEntry.endPos = xrefEntry.startPos + atoi(line.substr(spacePosition + 1).c_str()) - 1;
			xrefEntry.pos = rawText.tellg();
		
			if (xrefEntry.startPos - xrefEntry.endPos < 0)
				break; // No support for Linearized (Network Compatible) PDFs

			pdfXref.push_back(xrefEntry);
		}



	}

}
