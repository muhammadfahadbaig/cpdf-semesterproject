//
// Created by Test on 5/27/2017.
//

#ifndef SPDF_STRUCTURE_H
#define SPDF_STRUCTURE_H

#include <string>
#include <fstream>
#include <vector>
#include <iostream>
#include <string.h> // Include C-Style String Library header
#include "spdf-commonDefinitions.h"

struct pageTree
{
	int pageCatalogObject;
	int pageKidsObject;
	int pageNumber;
	std::vector<int> pageObjects;
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
public:
    spdf(const char * pathToFile);
    virtual ~spdf();
    void init();
    bool performEOFCheck();
    void loadXref(int);
    std::string getline(std::fstream&);
	void loadPageStructure();
};

#endif
