//
// SPDF STRUCTURE
//

#ifndef SPDF_STRUCTURE_H
#define SPDF_STRUCTURE_H

#include <string>
#include <fstream>
#include <vector>
#include <iostream>
#include <map>
#include <string.h> // Include C-Style String Library header
#include "spdf-commonDefinitions.h"

struct pageTree
{
	int pageCatalogObject;
	int pageKidsObject;
	int totalPages;
	std::vector<int> pageObjects;
};

struct spdfObjectNorm
{
	std::vector<std::string> objectTags;
	std::map <int,std::string> conArray;
};

class spdf
{
private:
    std::fstream pdfFile;
    char pdfVersion[9];
    const char * pdfPath;
    dictionary xref;
	pageTree spdfPages;
    int eofLocaction;
	int xrefTableLocation;
public:
    spdf(const char * pathToFile);
    virtual ~spdf();
    void init();
    bool performEOFCheck();
    void loadXref(int);
    std::string getline(std::fstream&);
	void loadObjectStructure();
	void resolveObject(std::map<int, std::string>::iterator);
	std::string trim(std::string);
	bool loadPageTree(std::map<int, std::string>::iterator);
	bool isIndirectObject(const std::string &, size_t); // Indirect Objects like # # obj
	bool isIndirectObject(const std::string &, size_t, size_t&);
};

#endif
