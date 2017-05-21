#ifndef CPDF_STRUCTURE_H
#define CPDF_STRUCTURE_H

#include <string>
#include <fstream>
#include <vector>
#include <iostream>
#include<string.h> // Include C-Style String Library header
#include "cpdf-commonDefinitions.h"

struct cpdfXref
{
	int startPos;
	int endPos;
	int xrefLength;
	int pos;
	std::string xrefData;
};

class cpdf
{
private:
	std::fstream pdfFile;
	std::fstream rawText;
	char pdfVersion[9];
	const char * pdfPath;
	std::vector<cpdfXref> pdfXref;
	int eofLocaction;
public:
	cpdf(const char * pathToFile);
	virtual ~cpdf();
	// bool validatePDF(); Deleted Function
	void init();
	bool performEOFCheck();
	dictionary loadXref(size_t);
	std::string getline(std::fstream&);
};

#endif
