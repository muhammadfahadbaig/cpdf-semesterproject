#include "cpdf-structure.h"

using namespace std;

cpdf::cpdf(const char * pathToFile)
{
		pdfFile.open(pathToFile, ios::in | ios::binary); // Open in read and binary mode
		try {
			if (pdfFile.is_open())
			{
				// Start writing the whole file in buffer mode (backup file on which we'll operate)
				{
					rawText.open("D:\generated.raw", ios::out);
					rawText << pdfFile.rdbuf();
					rawText.close(); // Close the file stream
					rawText.open("D:\generated.raw", ios::in); // Open the stream in normal text mode
				}
				rawText.clear(); // clear the EOF flag set on the file stream
				rawText.seekg(0, ios::beg);
				rawText.read(pdfVersion, 8); // Write the initial 8 bytes buffer which contains PDF version
				pdfVersion[8] = '\n';
				init(); // Start initialization

			}
			else throw invalid_argument("Unable to open the supplied file");
		}
		catch (invalid_argument &except)
		{

		}
}

cpdf::~cpdf()
{

}

