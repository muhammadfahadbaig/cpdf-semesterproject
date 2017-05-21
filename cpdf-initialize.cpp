#include "cpdf-structure.h"

using namespace std;

void cpdf::init() // TODO Deal with Linearized PDFs
{
	size_t xrefLocation{}; // Main document cross reference table location
	rawText.seekg(0, ios::end); // seek to the end of the file

								// Performing EOF check
	if (!performEOFCheck())
		throw "Unable to find %%EOF tag. Probably invalid PDF file.";

	// Performing search for startxref flag that'll point to xref location
	char charTemp{ '\0' };
	do
	{
		rawText.seekg(-2, ios::cur);
		if (charTemp == 'f') //startxref ends in f
		{
			break;
		}
	} while (rawText.get(charTemp));

	getline(rawText); // Move to next line
	string xrefLocationStr{};
	xrefLocationStr = getline(rawText);
	xrefLocation = atoi(xrefLocationStr.c_str()); // converting to c style string and getting xref location

	if (xrefLocation == 0)
	{
		throw "Unable to find Xref location. Probably an invalid PDF file.";
	}

	// Start loading xref (assuming uncompressed)

	
}