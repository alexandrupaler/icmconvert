#ifndef DATABASEREADER_H_
#define DATABASEREADER_H_

#include <stdio.h>
#include <stdlib.h>

#include "gatenumbers.h"
#include "decomposition.h"
#include <vector>
#include <map>
#include <string.h>

using namespace std;

class databasereader
{
public:
	map<string, int> nameToInt;
	map<int, string> intToName;
	map<string, decomposition> decomp;

	databasereader();

	void fillTranslations();

	int getNumber(char* key);

	void readOps(FILE* fp, vector<int>& list, int valoffset);

	bool readQubits(FILE* fp, vector<int>& list, int valoffset);

	decomposition readDecomposition(FILE* file, string name);

	string findDecomposition(FILE* file);

	void fillDatabase(map<string, decomposition>& decomp);
};


#endif /* DATABASEREADER_H_ */
