//
// SPDF STRUCTURE - MAIN ENTRY POINT
//

#ifndef SPDF_STRUCTURE_H
#define SPDF_STRUCTURE_H

#include <string>
#include <fstream>
#include <vector>
#include <iostream>
#include <map>
#include <string.h>
#include "spdf-commonDefinitions.h"


// Data Structure to hold all font related information for a single instance
struct fontDef
{
	int fontObject; // Represents font object location
	std::string fontType;
	std::string baseFont;
};

// Data structure to hold all page related information for a single instance
struct pageDef
{
	int pageObject; // Represents where object is stored
	int parentObj; // Represents parent object
	int pageSize[4]; // Array of four integers that represent the page's media box
	std::vector<fontDef> fontObjects;
	int totalFonts; // To better iterate on the loop
	int pageNumber; // To better store the page number to ease information extraction
	int totalImages; // Formerly XObjects
};

// Data Structure to represent the page tree hierarchy in a PDF
struct pageTree
{
	int pageCatalogObject;
	int pageKidsObject;
	int totalPages;
	std::vector<pageDef> pageObjects;
};

// Our main SPDF - PDF Parsing Implementation main class
class spdf
{
private:

    std::fstream pdfFile; // Filestream to access the pdf file
    char pdfVersion[9]; // Array to hold PDF version information (really useful in more advanced implementations
    const char * pdfPath; // Full resolved path of the currently loaded PDF
    dictionary xref; // Xref table to contain object number along with their offset, generation number, type and ending offset
	pageTree spdfPages; // Represents a single instance of a page tree (multiple catalogs not yet supported)
    int eofLocaction; // Useful pointer to EOF location in PDF file (ignoring multiple EOF since multiple revised files not yet supported)
	int xrefTableLocation; // Useful pointer to a xref table in PDF (ignoring multiple xref for the same reason above)

public:
	
	// Constructors and Destructors
    spdf(const char * pathToFile);
    virtual ~spdf();

	// Initializer
    void init();

	// Supplementary PDF specfic and assistance functions
    bool performEOFCheck();
    std::string getline(std::fstream&);
	std::string trim(std::string);

	// Xref Functions
	void loadXref(int);

	// Page Tree and Pages loading functions
	bool loadPageTree(std::map<int, std::string>::iterator);

	// Driver function to help loading all the information easily
	void loadObjectStructure();

	// Object Manipulation Functions
	bool isIndirectObject(const std::string &, size_t); // Indirect Objects like # # obj
	bool isIndirectObject(const std::string &, size_t, size_t&);
	void resolveObject(std::map<int, std::string>::iterator);
	std::string loadDictionary(std::string&);
	
	// Simple Font finding and loading functions
	bool loadPageFonts(int);
	void loadFontInformation(int);
};

#endif
