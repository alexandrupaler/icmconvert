#ifndef DECOMPOSITION_H__
#define DECOMPOSITION_H__

#include <string>
#include <vector>

using namespace std;

class decomposition
{
public:
	string name;

	int type;//0-icm, 1-nonicm, 2-disticm

	int nrancilla;

	//for icm
	vector<int> inits;
	vector<vector<int> > cnots;
	vector<int> meas;

	//for nonicm
	vector<vector<int> > gates;

	void toString();

	int maxCols();

	bool isicm();

	bool isdist();
};

#endif
