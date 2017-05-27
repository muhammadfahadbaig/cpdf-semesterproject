//
// SPDF Primary Functions
//

#include "spdf-structure.h"

using namespace std;

spdf::spdf(const char * pathToFile)
{
		pdfFile.open(pathToFile, ios::in | ios::ate);
		try {
			if (pdfFile.is_open())
			{
				pdfFile.seekg(0, ios::beg);
				pdfFile.read(pdfVersion, 8); // Write the initial 8 bytes buffer which contains PDF version
				pdfVersion[8] = '\n';
				init(); // Start initialization

			}
			else throw invalid_argument("Unable to open the supplied file");
		}
		catch (invalid_argument &except)
		{

		}
}

spdf::~spdf()
{

}


