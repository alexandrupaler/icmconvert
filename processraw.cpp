#include "gatenumbers.h"
#include "cnotcounter.h"
#include "databasereader.h"

#include <stdio.h>
#include <vector>

using namespace std;

//hardcoded Toffoli decompositions
//herein T= T\dagger, v=V^\dagger, however care should be taken later
/*
int toffolidecomp1length = 17;
int toffolidecomp1height = 3;
int toffolidecomp1cnots = 8;
long toffolidecomp1[51]= {
		WIRE, WIRE, WIRE, WIRE, WIRE, CTRL, WIRE, WIRE, WIRE, WIRE, WIRE, CTRL, TGATE, TGT, TGATE, TGT, WIRE,
		TGATE, TGT, TGATE, TGT, WIRE, TGT, TGATE, TGT, TGATE, TGT, WIRE, TGT, WIRE, WIRE, WIRE, WIRE, WIRE,
		HGATE, CTRL, TGATE, CTRL, HGATE, WIRE, HGATE, CTRL, TGATE, CTRL, HGATE, WIRE, HGATE, CTRL, TGATE, CTRL, HGATE
};

int toffolidecomp2length = 13;
int toffolidecomp2height = 3;
int toffolidecomp2cnots = 6;
long toffolidecomp2[39]= {
		WIRE, WIRE, WIRE, CTRL, WIRE, WIRE, WIRE, CTRL, WIRE, CTRL, WIRE, CTRL, TGATE,
		WIRE, CTRL, WIRE, WIRE, WIRE, CTRL, WIRE, WIRE, TGATE, TGT, TGATE, TGT, PGATE,
		HGATE, TGT, TGATE, TGT, TGATE, TGT, TGATE, TGT, TGATE, HGATE, WIRE, WIRE, WIRE
};
*/


vector<vector<int> > intcirc;
databasereader dbReader;

int getMaxQubit(FILE* fp, long max)
{
	int ret = max;
	char c = '?';
	while (!feof(fp) && (c!='\r') && (c!='\n'))
	{
		long val = 0;
		c = getc(fp);
		if(c==EOF || (c=='\r') || (c=='\n'))
			continue;

		if(c==' ')
		{
			fscanf(fp, "%ld", &val);
			printf("%ld ", val);
			if (val > ret)
				ret = val;
		}
	}

	return ret;
}

void readOtherQubits(FILE* fp, vector<long>& list)
{
	char c = '?';
	while (!feof(fp) && (c!='\r') && (c!='\n'))
	{
		long val = 0;
		c = getc(fp);
		if(c==EOF || (c=='\r') || (c=='\n'))
			continue;

		fscanf(fp, "%ld", &val);
		list.push_back(val-1);//indices from zero?
	}
}

void readRawFile(const char* fname)
{
	int nrqubits = 0;
	int nrToffoli = 0;
	int nrgates = 0;

	//read the file just to know the number of qubits and gates;
	FILE* file = fopen(fname, "r");
	while(!feof(file))
	{
		char gate = '?';
		long qubit = -1;
		fscanf(file, "%c", &gate);
		printf("%c", gate);
		if ((gate=='\r')||(gate=='\n')||feof(file))
		{
			continue;//finished a gate, or empty line
		}

		nrgates++;

		//read first number after gate name
		fscanf(file, "%ld", &qubit);
		if(qubit > nrqubits)
			nrqubits = qubit;

		//if cnot or Toffoli, there are other qubits after it
		if(gate=='c' || gate=='T')
		{
			nrqubits = getMaxQubit(file, nrqubits);
			nrgates++;
		}
		if(gate == 'T')
			nrToffoli++;
	}
	//close the file
	fclose(file);

	//update nrGates to reflect the decomposition of the Toffoli
	int toffolength = dbReader.decomp["toffoli"].gates[0].size();
	nrgates = nrgates + (toffolength - 1)*nrToffoli;

	//reopen it again
	file = fopen(fname, "r");
	for(int i=0; i<nrqubits; i++)
	{
		vector<int> line(nrgates + 2, WIRE);//a circuit consisting of a single line
		line.at(0) = INPUT; //this one is circuit input
		line.at(nrgates + 1) = OUTPUT;
		intcirc.push_back(line);
	}

	vector<long> list;
	int nrgate = 0;
	while(!feof(file))
	{
		list.clear();

		char gate = '?';
		long qubit = -1;
		fscanf(file, "%c", &gate);
		if ((gate=='\r')||(gate=='\n')||feof(file))
		{
			continue;//empty line
		}

		nrgate++;

		//read first number after gate name
		fscanf(file, "%ld", &qubit);
		qubit -= 1 ;//indices from zero?

		int opnumber = WIRE;
		switch(gate)
		{
		case 'h':
			opnumber = HGATE;
			break;
		case 'p':
			opnumber = PGATE;
			break;
		case 'v':
			opnumber = RGATE;
			break;
		case 't':
			opnumber = TGATE;
			break;
		case 'c':
			opnumber = getCnotNumber();
			readOtherQubits(file, list);
			break;
		case 'T':
			//decompose Toffoli
			readOtherQubits(file, list);//will contain exactly two qubits: control2 and target
			break;
		}

		if(gate!='T')
			intcirc.at(qubit).at(nrgate) = opnumber;

		//if cnot or Toffoli, there are other qubits after it
		if(gate=='c')
		{
			for(int i=0; i<list.size(); i++)
			{
				intcirc.at(list.at(i)).at(nrgate) = opnumber + 1;
			}
			nrgate++;
		}
		else if(gate == 'T')
		{
			decomposition el = dbReader.decomp["toffoli"];
			int toffheight = el.gates.size();
			int tofflength = el.gates.at(0).size();
			int toffcnots = 0;
			for(int i=0; i<toffheight; i++)
			{
				for(int j=0; j<tofflength; j++)
				{
					if(el.gates[i][j] == CTRL)
						toffcnots++;
				}
			}

			vector<int> cnots;
			//for(int i=0; i< toffolidecomp1cnots; i++)
			for(int i=0; i< toffcnots; i++)
				cnots.push_back(getCnotNumber());

			int pos[] = {qubit, list[0], list[1]};

			int currcnot = -1;
			for(int j=0; j<tofflength; j++)
			{
				int lastcnot = currcnot;
				for(int i=0; i<toffheight; i++)
				{
					int opnumber = el.gates[i][j];

					if((opnumber == CTRL || opnumber == TGT) && lastcnot == currcnot)
						currcnot++;
					if(opnumber == CTRL)
						opnumber = cnots.at(currcnot);
					if(opnumber == TGT)
						opnumber = cnots.at(currcnot) + 1;
					intcirc.at(pos[i]).at(nrgate + j) = opnumber;
				}
			}
			nrgate += tofflength;

		
		}
	}

	fclose(file);
}

void writeInFile(const char* fname)
{
	char nfname[255];
	sprintf(nfname, "%s.in", fname);
	FILE* file = fopen(nfname, "w");

	fprintf(file, "%d\n", (int) intcirc.size());
	fprintf(file, "%d\n", (int) intcirc.at(0).size());

	for(int i=0; i<intcirc.size(); i++)
	{
		for(int j=0; j<intcirc.at(i).size(); j++)
			fprintf(file, "%5d ", intcirc.at(i).at(j));
		fprintf(file, "\n");
	}

	fclose(file);
}

int main(int argc, char** argv)
{
	printf("Hello World!\n");
	if(argc == 1)
	{
		printf(".raw file name parameter missing");
		return 0;
	}

	printf("processing %s\n", argv[1]);

	readRawFile(argv[1]);
	writeInFile(argv[1]);

	return 1;
}
