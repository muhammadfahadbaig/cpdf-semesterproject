//
// Created by Test on 5/27/2017.
//

#include "spdf-structure.h"

using namespace std;

void spdf::loadXref(int xrefPos) // TODO : Add compressed streams support
{
	pdfFile.seekg(xrefPos);
	spdf::getline(pdfFile);
	string line;

	// Extract the xref table
	while (true) {
        line = getline(pdfFile);

        if (line.length() > 0) line = line.substr(0, line.find_last_not_of("\r\t\n ") +
                                                     1); // Adopting PDF newline conventions and removing trailing spaces and break sequences
        if (line.length() == 0) break;

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
        size_t lastSpacePosition = line.find_last_of(" ");
        int objectsStartingPos{};
        int objectsEndingPos{};
        if (lastSpacePosition - spacePosition == 0 &&
            line.substr(0, spacePosition).find_first_not_of("0123456789") == string::npos
            && line.substr(spacePosition + 1).find_first_not_of("0123456789") == string::npos) {
            // We've reached the objects number declaration
            objectsStartingPos = stoi(line.substr(0, spacePosition));
            objectsEndingPos = objectsStartingPos + stoi(line.substr(spacePosition + 1));
            line = spdf::getline(pdfFile);
        }
		else
		{
			break; // Work finished!
		}

        for (int i = objectsStartingPos; i <= objectsEndingPos; i++) {
			line = line.substr(0, line.find_last_not_of("\r\t\n ")+1);
			spacePosition = line.find_first_of(" ");
			lastSpacePosition = line.find_last_of(" ");

            if (lastSpacePosition - spacePosition != 0 &&
				spacePosition != string::npos &&
                line.substr(0, spacePosition).find_first_not_of("0123456789") == string::npos &&
                line.substr(spacePosition + 1, lastSpacePosition - spacePosition - 1).find_first_not_of("0123456789") == string::npos &&
				line.substr(lastSpacePosition+1).find_first_not_of("fn") == string::npos)
                /*
                    npos : indicates no matches
                    The last two conditions check if the extracted substring contains anything which is not an integer
                    since if there is a character other than integer the entry isn't valid and data should be discarded
                */
            {
                pair<int, string> xrefEntry;
                // Populate xref
                xrefEntry.first = i;
                xrefEntry.second = (line.substr(0, spacePosition)) + ",";
                xrefEntry.second.append(line.substr(spacePosition + 1, lastSpacePosition - spacePosition - 1));
                xrefEntry.second.append(",");
                xrefEntry.second.append(line.substr(lastSpacePosition + 1));

                cout << xrefEntry.first << " " << xrefEntry.second << endl;

				if (line.length() != 20) {}
                    // we'll log the error later. However the 20 byte rule is not compulsory and we'll continue to read the PDF
                xref.insert(xrefEntry);
                line = spdf::getline(pdfFile); // Move to next entry
			}
			else
			{
				break; // Invalid entry
			}
        }
    }
    return; // return nothing
}
