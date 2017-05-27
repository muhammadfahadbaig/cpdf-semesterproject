//
// SPDF Initialize Functions
//

#include "spdf-structure.h"

using namespace std;

void spdf::init() // TODO Deal with Linearized PDFs
{
    int xrefLocation{}; // Main document cross reference table location
    pdfFile.seekg(0, ios::end); // seek to the end of the file

    // Performing EOF check
    if (!performEOFCheck())
        throw "Unable to find %%EOF tag. Probably invalid PDF file.";

    // Performing search for startxref flag that'll point to xref location
    char charTemp{ '\0' };
    do
    {
        pdfFile.seekg(-2, ios::cur);
        if (charTemp == 'f') //startxref ends in f
            break;
    } while (pdfFile.get(charTemp));

    spdf::getline(pdfFile); // Move to next line
    string xrefLocationStr{};
    xrefLocationStr = getline(pdfFile);
    xrefLocation = stoi(xrefLocationStr); 

    if (xrefLocation == 0)
    {
        throw "Unable to find Xref location. Probably an invalid PDF file.";
    }

    // Start loading xref (assuming uncompressed)
    loadXref(xrefLocation);

}