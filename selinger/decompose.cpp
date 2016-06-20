#include <stdio.h>
#include <vector>
#include <math.h>
#include <fstream>
#include <string>

#include "runextern.h"

using namespace std;

struct polarcx{
	double radius;//cos \theta or sin \theta in constructmatrix
	double angle;//radians, parameter to e^(i*angle)
};

struct unitary
{
	string name;
	vector<polarcx> entries;
};

vector<polarcx> constructmatrix()
{
	vector<polarcx> matrix;
	// {{m00,m01},
	// {m10,m11}}
	// =
	// {{(cos \theta, e^i\phi),(sin \theta, e^i\psi)},
	// {(-sin \theta, e^-i\phi),(sin \theta, e^-i\phi)}}

	//below a Hadamard

	polarcx m00; m00.radius = 1/sqrt(2); m00.angle = 0; matrix.push_back(m00);
	polarcx m01; m01.radius = 1/sqrt(2); m01.angle = 0; matrix.push_back(m01);
	polarcx m10; m10.radius = 1/sqrt(2); m10.angle = 0; matrix.push_back(m10);
	polarcx m11; m11.radius = -1/sqrt(2); m11.angle = 0; matrix.push_back(m11);

	return matrix;
}

vector<double> calcZYZAngles(vector<polarcx> matrix)
{
	//polar coordinates in matrix
	//Theorem 4.1, Equation2 4.11 and 4.12, Nielsen and Chuang page 176
	vector<double> ret;

	double angle1 = matrix.at(0).angle - matrix.at(1).angle;
	double angle2 = 2 * acos(matrix.at(0).radius);
	double angle3 = matrix.at(0).angle + matrix.at(1).angle;

	ret.push_back(angle1);
	ret.push_back(angle2);
	ret.push_back(angle3);

	return ret;
}

vector<double> calcZXZAngles(vector<double>& xyx)
{
	// P R_y P = R_x
	vector<double> ret;

	ret.push_back(xyx.at(0) + M_PI/2);
	ret.push_back(xyx.at(1));
	ret.push_back(xyx.at(2) - M_PI/2);

	return ret;
}

vector<unitary> readUnitaries(const char* fname)
{
	vector<unitary> ret;

	ifstream ifs;
	ifs.open (fname, std::ifstream::in);

	int nrunit = 0;
	ifs >> nrunit;
	for(int i=0; i<nrunit; i++)
	{
		unitary un;
		ifs >> un.name;
		for(int i=0; i<4; i++)
		{
			polarcx nr;
			ifs >> nr.radius;
			ifs >> nr.angle;
			un.entries.push_back(nr);
		}
		ret.push_back(un);
	}

	return ret;
}

void printDbEntry(string& name, vector<vector<string> >& decomp)
{
	printf("=%s\nnicm\n0\n", name.c_str());
	for(vector<vector<string> >::iterator it = decomp.begin(); it != decomp.end(); it++)
	{
		for(vector<string>::iterator itt = (*it).begin(); itt != (*it).end(); itt++)
		{
			printf("%s ", (*itt).c_str());
		}
	}
	printf("\n");
}

int main(int argc, char** argv)
{
	vector<unitary> unis = readUnitaries("unitary.in");
	//vector<polarcx> matrix = constructmatrix();

	for(vector<unitary>::iterator it = unis.begin(); it != unis.end(); it++)
	{
		//vector<double> zyz = calcZYZAngles(matrix);
		vector<double> zyz = calcZYZAngles((*it).entries);
		vector<double> zxz = calcZXZAngles(zyz);

		vector<vector<string> > decomp;
		for(int i=0; i<3; i++)
		{
			printf("%lf \n", zxz.at(i));
			decomp.push_back(gridsynth(zxz.at(i)));
		}

		//put H around the middle R_x -> R_z
		vector<string> h;
		h.push_back("HGATE");//hgate
		decomp.insert(decomp.begin() + 1, h);
		decomp.insert(decomp.begin() + 3, h);

		printDbEntry((*it).name, decomp);
	}

	return 1;
}
