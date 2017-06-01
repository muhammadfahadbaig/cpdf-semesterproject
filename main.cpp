#include<iostream>
#include<fstream>
#include<string>
#include "spdf-structure.h"

using namespace std;

int main(void)
{
    spdf testRun("D:\\demo.pdf");
	testRun.loadObjectStructure();
	cout << "loaded";
}