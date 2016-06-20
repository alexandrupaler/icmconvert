#include "gatenumbers.h"
#include "databasereader.h"
#include "decomposition.h"

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <algorithm>
#include <vector>
#include <set>
#include <string>
#include <map>
#include <string.h>
#include <sys/time.h>

#include "cnotcounter.h"

#define ICM 0
#define NICM 1
#define ICMINJ 2

using namespace std;

char circuitName[255];

//circuit array
vector<vector<int> > circ;

//latex file
FILE* pFile;

//geometric description
vector<vector<int> > coords;
vector<vector<int> > segs;
vector<int> io;//input outputs

map<string, int> coordmap;
map<string, int> segmap;

set<int> measurements;
set<int> initialisations;

databasereader dbReader;

int getRandomNumber(int min, int max)
{
	int q = lrand48() % max;
	while(q < min)
		q = lrand48() % max;
	return q;
}

int getXShift()
{
	int shift = 4 + 2;

	return shift;
}

void makeCliffordPart(int nrq)
{
	//cnot: 1=control 2=target

	int maxgates = 5;
	int nrgates = getRandomNumber(1, maxgates);
	for(int i=0; i<nrgates; i++)
	{
		int gtype = getRandomNumber(1, 5);
		int qnr = getRandomNumber(1, nrq + 1);

		//adauga o coloana cu WIRE
		for(int gi=0; gi<nrq; gi++)
		{
			circ.at(gi).push_back(WIRE);
		}

		if(gtype == CTRL || gtype == TGT)
		{
			int ctrlid = getCnotNumber();
			int targetid = ctrlid + 1;

			int otherq = getRandomNumber(1, circ.size() + 1);
			while(qnr == otherq)
			{
				otherq = getRandomNumber(1, circ.size() + 1);
			}
			
			circ.at(qnr - 1)[circ.at(qnr - 1).size() - 1] = (gtype == 1 ? ctrlid : targetid);
			circ.at(otherq - 1)[circ.at(otherq - 1).size() - 1] = (gtype == 1 ? targetid : ctrlid);
		}
		else
		{
			//pune gtype - dar la cnot e mai aiurea - vezi sus
			circ.at(qnr - 1)[circ.at(qnr - 1).size() - 1] = gtype;
		}
	}
}

void makeTPart(int nrq)
{
	//t: 5
	for(int i=0; i<nrq; i++)
	{
		int g = getRandomNumber(0,2);
		circ.at(i).push_back(g*5);
	}
}

void printCirc(vector<vector<int> > circ)
{
	printf("------\n");
	for(int i=0; i<circ.size(); i++)
	{
		for(int j=0; j<(circ.at(i).size()); j++)
		{
			int val = circ.at(i).at(j);
			if(isCnot(val))
				val = getCnotPart(val);
			printf("%4d ", val);
		}
		printf("\n");
	}
}

void printCirc()
{
	printCirc(circ);
}

bool isFreeSpace(int line, int col, int nrCols)
{
	bool isSpace = true;
	for(int j=1; j<(nrCols+1); j++)
	{
		int y = col + j;
		if(y<0 || y>=circ.at(0).size())
		{
			isSpace = false;
			break;
		}
		if(circ.at(line).at(y) != WIRE && circ.at(line).at(y) != INPUT)
		{
			isSpace = false;
			break;
		}
	}
	return isSpace;
}

vector<int> findTarget(int i, int j)
{
	vector<int> ret;
	int controlnumber = circ.at(i).at(j);

	for(int k=0; k<circ.size(); k++)
	{
		if(sameCnot(controlnumber, circ.at(k).at(j)))
			ret.push_back(k);
	}
	return ret;
}

void appendFile(FILE* destFile, const char* name)
{
	FILE* sourceFile = fopen (name, "r");
	char mystring [100];

	while(!feof(sourceFile))
	{
		if(fgets (mystring, 100, sourceFile) != NULL)
			fprintf(destFile, "%s", mystring);
	}
	fclose(sourceFile);
}

void writePostScriptFile()
{
	char psfilename[255];
	sprintf(psfilename, "%s.ps", circuitName);

	pFile = fopen (psfilename,"w");
	fprintf(pFile, "%%!PS\n");
	int height = (circ.size() + 1) * 30;
	int width = (circ.at(0).size() + 3) * 30;
	fprintf(pFile, "%%%%BoundingBox: 0 0 %d %d\n", width, height);

	appendFile(pFile, "templateh.ps");

	fprintf(pFile, "30 %d moveto\n", height - 15);

	//not as general as Qcircuit
	int inQubit = 0;
	int outQubit = 0;

	for(int i=0; i<circ.size(); i++)
	{
		for(int j=0; j<circ.at(i).size(); j++)
		{
			int cmd = circ.at(i).at(j);
			if(isCnot(cmd))
				cmd = getCnotPart(cmd);

			//if(j==0)
			if(initialisations.find(cmd) != initialisations.end())
			{
				/*search measurement*/
				int length = 0;
				for(int k=j+1; k<circ.at(i).size(); k++)
				{
					if(measurements.find(circ.at(i).at(k)) != measurements.end())
					{
						length = k - j;
						break;
					}
				}

				switch(cmd)
				{
					case AA:
						fprintf(pFile, "(A) ket ");
						break;
					case YY:
						fprintf(pFile, "(Y) ket ");
						break;
					case ZERO:
						fprintf(pFile, "(0) ket ");
						break;
					case PLUS:
						fprintf(pFile, "(+) ket ");
						break;
				}
				fprintf(pFile, "n %d wire ", length);
			}
			else if(cmd == MZ)
			{
				fprintf(pFile, "(Z) measure ");
			}
			else if(cmd == MX)
			{
				fprintf(pFile, "(X) measure ");
			}
			else if(cmd == MXZ)
			{
				fprintf(pFile, "(X/Z) measure ");
			}
			else if(cmd == MZX)
			{
				fprintf(pFile, "(Z/X) measure ");
			}
			else if(cmd == MA)
			{
				fprintf(pFile, "(A) measure ");
			}
			else if(cmd == MYZ)
			{
				fprintf(pFile, "(Y/Z) measure ");
			}
			else if(cmd == MY)
			{
				fprintf(pFile, "(Y) measure ");
			}
			else if(cmd == CTRL)
			{
				vector<int> targets = findTarget(i,j);				
				fprintf(pFile, "control ");

				//search maxpos and maxneg
				int maxpos = 0;
				int maxneg = 0;

				for(vector<int>::iterator it=targets.begin(); it!=targets.end(); it++)
				{
					int dist = *it - i;
					if(dist < 0 && dist < maxneg)
						maxneg = dist;
					if(dist > 0 && dist > maxpos)
						maxpos = dist;
				}
				if(maxpos != 0)
					fprintf(pFile, "%d qwx ", maxpos);
				if(maxneg != 0)
					fprintf(pFile, "%d qwx ", maxneg);

			}
			else if(cmd == TGT)
				fprintf(pFile, "target ");
			else if(cmd == WIRE)
			{
				//fprintf(pFile, "\\qw");
			}
			
			fprintf(pFile, "n ");
		}
		fprintf(pFile, "back \n");
	}

	appendFile(pFile, "templatec.ps");
	fclose(pFile);
}

void makeQCircuit()
{
	fprintf(pFile, "\\begin{figure}\n\\centerline{\n");
	fprintf(pFile, "\\Qcircuit @C=.6em @R=.7em {\n");

	int inQubit = 0;
	int outQubit = 0;

	for(int i=0; i<circ.size(); i++)
	{
		bool wasmeasure = false;
		for(int j=0; j<circ.at(i).size(); j++)
		{
			int cmd = circ.at(i).at(j);
			if(isCnot(cmd))
				cmd = getCnotPart(cmd);

			if(initialisations.find(cmd) != initialisations.end())
			{
				switch(cmd)
				{
					case AA:
						fprintf(pFile, "\\lstick{\\ket{A}}");
						break;
					case YY:
						fprintf(pFile, "\\lstick{\\ket{Y}}");
						break;
					case ZERO:
						fprintf(pFile, "\\lstick{\\ket{0}}");
						break;
					case PLUS:
						fprintf(pFile, "\\lstick{\\ket{+}}");
						break;
					case INPUT:
						fprintf(pFile, "\\lstick{\\ket{in_%d}}", inQubit++);
						break;
				}
			}
			else if(cmd == MZ)
			{
				fprintf(pFile, "\\measure{Z}");
				wasmeasure = true;
			}
			else if(cmd == MX)
			{
				fprintf(pFile, "\\measure{X}");
				wasmeasure = true;
			}
			else if(cmd == MXZ)
			{
				fprintf(pFile, "\\measure{X/Z}");
				wasmeasure = true;
			}
			else if(cmd == MZX)
			{
				fprintf(pFile, "\\measure{Z/X}");
				wasmeasure = true;
			}
			else if(cmd == HGATE)
			{
				fprintf(pFile, "\\gate{H}");
			}
			else if(cmd == TGATE)
			{
				fprintf(pFile, "\\gate{T}");
			}
			else if(cmd == PGATE)
			{
				fprintf(pFile, "\\gate{P}");
			}
			else if(cmd == RGATE)
			{
				fprintf(pFile, "\\gate{R}");
			}
			else if(cmd == CTRL)
			{
				vector<int> targets = findTarget(i,j);				
				fprintf(pFile, "\\control");

				//search maxpos and maxneg
				int maxpos = 0;
				int maxneg = 0;

				for(vector<int>::iterator it=targets.begin(); it!=targets.end(); it++)
				{
					int dist = *it - i;
					if(dist < 0 && dist < maxneg)
						maxneg = dist;
					if(dist > 0 && dist > maxpos)
						maxpos = dist;
				}
				if(maxpos != 0)
					fprintf(pFile, "\\qwx[%d]", maxpos);
				if(maxneg != 0)
					fprintf(pFile, "\\qwx[%d]", maxneg);

				fprintf(pFile, "\\qw");
			}
			else if(cmd == TGT)
				fprintf(pFile, "\\targ");
			else if(cmd == WIRE)
			{
				if(!wasmeasure)
					fprintf(pFile, "\\qw");
			}
			
			//buggy..maybe
			if((cmd==0) && j==circ.at(i).size()-1)
				fprintf(pFile, "&\\rstick{\\ket{out_%d}}", outQubit++);

			fprintf(pFile, "&");
		}
		fprintf(pFile, "\\\\\n");
	}

	fprintf(pFile, "	}\n");
	fprintf(pFile, "}\\end{figure}\n");
}

void addColumns(int line, int col, int maxCols) {
	bool isSpace = isFreeSpace(line, col, maxCols);
	if (!isSpace) {
		for (int c = 0; c < circ.size(); c++) {
			int val = WIRE;
			for (int j = 0; j < maxCols; j++)
				circ.at(c).insert(circ.at(c).begin() + col + 1 + j, val);
		}
	}
}

void addAncillae(int& nrAncillae, int& line) {
	//introduce ancillas
	for (int k = 0; k < nrAncillae; k++) {
		vector<int> nline(circ.at(line).size(), WIRE);
		circ.insert(circ.begin() + line + 1, nline);
	}
}

void moveFromCurrentToLastLine(int line, int col, int nrCols, int nrAncillae)
{
	//copy from prev line to last new line
	if(nrCols > 0)
	{
		for(int cl = col + 1 + nrCols; cl < circ.at(line).size(); cl++)
		{
			circ.at(line + nrAncillae).at(cl) = circ.at(line).at(cl);
			circ.at(line).at(cl) = WIRE;
		}
	}
}

void replaceNonICM(int line, int col, decomposition& el)
{
	int maxCols = el.maxCols();
	addColumns(line, col, maxCols);

	addAncillae(el.nrancilla, line);

	circ.at(line).at(col) = WIRE;//remove the previous gate

	for(int i=0; i<el.gates.size(); i++)
	{
		for(int j=0; j<el.gates.at(i).size(); j++)
		{
			circ.at(line + i).at(col + j + 1) = el.gates.at(i).at(j);
		}
	}
}

void replaceICM(int line, int col, decomposition& el)
{
	int maxCols = el.maxCols();
	addColumns(line, col, maxCols);

	addAncillae(el.nrancilla, line);

	circ.at(line).at(col) = WIRE;

	//put the initialisations
	for(int i=0; i<el.inits.size(); i++)
	{
		int val = el.inits.at(i);
		if(val != EMPTY)
			circ.at(line + i).at(0) = val;
	}

	//make the cnots
	for(int i=0; i<el.cnots.size(); i++)
	{
		int cnotctrlid = getCnotNumber();
		int cnottrgtid =  cnotctrlid + 1;

		int ctrl = el.cnots.at(i).at(0);
		circ.at(line + ctrl).at(col + i + 1) = cnotctrlid;

		printf("CTRL %d", ctrl);

		for(int j=1; j<el.cnots.at(i).size(); j++)
		{
			int tgt = el.cnots.at(i).at(j);
			printf(" TGT %d", tgt);
			circ.at(line + tgt).at(col + i + 1) = cnottrgtid;
		}
		printf("\n");
	}

	//put the measurements
	for(int i=0; i<el.meas.size(); i++)
	{
		int val = el.meas.at(i);
		if(val != EMPTY)
			circ.at(line + i).at(col + maxCols) = val;
	}

	moveFromCurrentToLastLine(line, col, maxCols, el.nrancilla);
}

void replacePattern(int what)
{
	//not optimal at this state: use scheduler
	bool decomposed = true;
	while(decomposed)
	{
		decomposed = false;
		for(int i=0; i<circ.size(); i++)
		{
			for(int j=0; j<circ.at(i).size(); j++)
			{
				int val = circ.at(i).at(j);
				string key = dbReader.intToName[val];
				if(dbReader.decomp.find(key) != dbReader.decomp.end())
				{
					decomposition el = dbReader.decomp[key];
					if(!el.isicm() && (what == NICM))
					{
						replaceNonICM(i, j, el);
						decomposed = true;
					}
					else if(el.isicm() && !el.isdist() && (what == ICM))
					{
						replaceICM(i, j, el);
						decomposed = true;
					}
					else if(el.isdist() && (what == ICMINJ))
					{
						replaceICM(i, j, el);
						decomposed = true;
					}
				}
			}
		}
	}
}

void removeEmptyColumns()
{
	for(int j=0; j<circ.at(0).size(); j++)
	{
		bool empty = true;
		for(int i=0; i<circ.size(); i++)
			empty = empty && (circ.at(i).at(j) == WIRE || circ.at(i).at(j) == EMPTY);
		if(empty)
		{
			for(int i=0; i<circ.size(); i++)
				circ.at(i).erase(circ.at(i).begin() + j);
			j--;//go back
		}
	}
}

void removeEmptyRows()
{
	bool emptyline = true;
	int max = -1;
	for(int i=0; i<circ.size(); i++)
	{
		emptyline = true;
		for(int j=0; j<circ.at(i).size(); j++)
		{
			emptyline = emptyline && (circ.at(i).at(j) == EMPTY);
		}
		if(!emptyline)
		{
			max = i;
			break;
		}
	}

	if(max != -1)
	{
		circ.erase(circ.begin(), circ.begin() + max);
	}
}

void writeGeometry()
{

	char geomfilename[255];
	sprintf(geomfilename, "%s.geom", circuitName);

	FILE* f = fopen(geomfilename, "w");

	fprintf(f, "%d\n", (int)io.size());
	fprintf(f, "%d\n", (int)segs.size());
	fprintf(f, "%d\n", (int)coords.size());

	for(int i=0; i<io.size(); i++)
	{
		if(i!=0)
			fprintf(f, ",");
		fprintf(f, "%d", io.at(i) + 1);
	}
	fprintf(f, "\n");

	for(int i=0; i<segs.size(); i++)
	{
		fprintf(f, "%d,%d\n", segs.at(i)[0] + 1, segs.at(i)[1] + 1);
	}

	for(int i=0; i<coords.size(); i++)
	{
		fprintf(f, "%d,%d,%d,%d\n", i+1, coords.at(i)[0], coords.at(i)[1], coords.at(i)[2]);
	}

	for(int i=0; i<io.size(); i++)
	{
		fprintf(f, "%d,%d\n", io.at(i) +1, io.at(i) + 1);
	}

	fclose(f);
}


void writeFile()
{
	FILE* f = fopen(circuitName, "w");

	int nrlines = circ.size();
	int nrcols = circ.at(0).size();

	fprintf(f, "%d\n", nrlines);
	fprintf(f, "%d\n", nrcols);

	for(int i=0; i<nrlines; i++)
	{
		for(int j=0; j<nrcols; j++)
		{
			fprintf(f, "%d ", circ.at(i).at(j));
		}
		fprintf(f, "\n");
	}

	fclose(f);
}

void readFile(const char* fname)
{
	sprintf(circuitName, "%s", fname);

	FILE* f = fopen(fname, "r");

	int nrlines = -1;
	int nrcols = -1;

	fscanf(f, "%d", &nrlines);
	fscanf(f, "%d", &nrcols);

	for(int i=0; i<nrlines; i++)
	{
		vector<int> l;
		for(int j=0; j<nrcols; j++)
		{
			int v = 0;
			fscanf(f, "%d", &v);
			l.push_back(v);
		}
		circ.push_back(l);
	}

	fclose(f);
}

void addSegment(int idx1, int idx2)
{
	vector<int> s1;
	if(idx1 == idx2)
	{
		//printf("zero distance...do not add\n");
		return;
	}

	//indicele mic e primul, al doilea e indicele mare
	s1.push_back(idx1 < idx2 ? idx1 : idx2);
	s1.push_back(idx1 > idx2 ? idx1 : idx2);


	/*new linear search*/
	string key;
	char numstr[100]; // enough to hold all numbers up to 64-bits
	for(int i=0; i<2; i++)
	{
		sprintf(numstr, ",%d", s1[i]);
		key = key + numstr;
	}

	map<string, int>::iterator it = segmap.find(key);
	if(it != segmap.end())
	{
		return;
	}

	segs.push_back(s1);

	segmap[key] = segs.size() - 1;

/*
	//daca edge nu exista, adauga
	vector<vector<int> >::iterator it = find(segs.begin(), segs.end(), s1);
	if(it != segs.end())
	{
		//printf("EDGE EXISTA!\n");
	}
	else
	{
		//printf(".");
		segs.push_back(s1);
	}
*/
}

int addCoordinate(vector<int>& c)
{
	/*new linear search*/
	string key;
	for(int i=0; i<3; i++)
	{
		char numstr[21]; // enough to hold all numbers up to 64-bits
		sprintf(numstr, ",%d", c[i]);
		key = key + numstr;
	}

	map<string, int>::iterator it = coordmap.find(key);
	if(it != coordmap.end())
	{
		return it->second;
	}
	coords.push_back(c);
	coordmap[key] = coords.size() - 1;
	return coords.size() - 1;
}

void modifyCoordinateAndAddSegment(vector<int>& c1, int pos, int value, int& lastDIndex) {
	c1[pos] = value;

	vector<int> c;
	c.insert(c.begin(), c1.begin(), c1.end());

	int currindex = addCoordinate(c);

	addSegment(lastDIndex, currindex);
	lastDIndex = currindex;
}

void addDual(int ctrli, int ctrlj)
{
	int maxheight = circ.size() * 2;

	//repair here after changing the cnot construction
	vector<int> targets = findTarget(ctrli, ctrlj);
	//int disti = targets.size();//size is wrong to call

	int min = ctrli < targets[0] ? ctrli : targets[0];
	int max = ctrli > targets[targets.size() - 1] ? ctrli : targets[targets.size() - 1];

	//2 points for control: c1 the left, c2 the right
	vector<int> c1(3,-100);//-100 represents nothing
	//c1[0] = maxheight - min + 1;
	c1[0] = 2*min - 1;
	//c1[1] = ctrlj*4 - 1;
	c1[1] = ctrlj*getXShift() - 1;//compact to bridge
	c1[2] = 1;

	vector<int> c2;
	c2.assign(c1.begin(), c1.end());
	c2[1] += 4;//ce se intampla aici?


	int firstDIndex = addCoordinate(c2);

	int lastDIndex = addCoordinate(c1);

	addSegment(lastDIndex, firstDIndex);

	int lasttarget = min;
	c1[0] += 2;
	for(int i=min; i<=max; i++)
	{
		vector<int>::iterator it = find (targets.begin(), targets.end(), i);
		if (it != targets.end())
		{
			//aici e target
			int dist = i - lasttarget - 1;
			//if(dist > 0)
			{
				//prelungeste segmentul doar daca nu e prima oara
				//printf("dist=%d\n", dist);
				modifyCoordinateAndAddSegment(c1, 0, c1[0] + 2*dist, lastDIndex);
			}

			modifyCoordinateAndAddSegment(c1, 2, -1, lastDIndex);
			modifyCoordinateAndAddSegment(c1, 0, c1[0] + 2, lastDIndex);
			modifyCoordinateAndAddSegment(c1, 2, 1, lastDIndex);

			lasttarget = i;
		}
	}

	if(ctrli == max)
	{
		//controlul nu apare in lista de targeturi, dar defectul trebuie extins sa cuprinda si bara verticala
		modifyCoordinateAndAddSegment(c1, 0, c1[0] + 2*(max-lasttarget), lastDIndex);
	}

	modifyCoordinateAndAddSegment(c1, 1, c1[1] + 4, lastDIndex);

	addSegment(lastDIndex, firstDIndex);
	
}

void shiftLastIndices(int& i, int& j, vector<int>& lastindex) {
	vector<int> c1(3, -100); //-100 represents nothing
	//c1[0] = maxheight - i*2;
	c1[0] = i * 2;
	//c1[1] = j*4;
	c1[1] = j * getXShift(); //compacted to bridge
	c1[2] = 0;
	vector<int> c2;
	c2.assign(c1.begin(), c1.end());
	c2[2] += 2;
	//add the two points
	lastindex[2] = addCoordinate(c1);
	lastindex[3] = addCoordinate(c2);
}

void addJOffsetLastIndices(int pos, int offset, vector<int>& lastindex)
{
	vector<int> c1(coords[lastindex[pos]]);
	c1[1] += offset;

	//add the two points
	lastindex[pos] = addCoordinate(c1);
}

void makeGeometry()
{
	int maxheight = circ.size() * 2;

	//here the primal space is constructed

	vector<int> lastindex(4, -100);

	for(int i=0; i<circ.size(); i++)
	{
		for(int j=0; j<circ.at(i).size(); j++)
		{
			bool isinit = initialisations.find(circ.at(i).at(j)) != initialisations.end();
			bool ismeasure = measurements.find(circ.at(i).at(j)) != measurements.end();

			//if(getCnotPart(circ.at(i).at(j)) == 1 || j == 0 || j == circ.at(i).size() - 1)
			if(getCnotPart(circ.at(i).at(j)) == CTRL || isinit || ismeasure)
			{
				shiftLastIndices(i, j, lastindex);
			}

			if(getCnotPart(circ.at(i).at(j)) == CTRL)//control will be after the case j==0 where lastindex[0,1] are set
			{
				//add segments down
				addSegment(lastindex[2], lastindex[3]);

				//add segments to the left
				for(int si=0; si<2; si++)
				{
					addSegment(lastindex[0 + si], lastindex[2 + si]);
					lastindex[0 + si] = lastindex[2 + si]; //will be used at the later point at indices 0,1
				}

				addJOffsetLastIndices(0, 2, lastindex);
				addJOffsetLastIndices(1, 2, lastindex);
				addSegment(lastindex[0], lastindex[1]);
			}

			//if(j==0 || j == circ.at(i).size() - 1)
			if(isinit || ismeasure)
			{
				//the io will be with one layer up
				vector<int> ioc;
				ioc.assign(coords.at(lastindex[2]).begin(), coords.at(lastindex[2]).end());

				if(ismeasure )
				{
					ioc[2] += 1;
				}

				int ioIndex = addCoordinate(ioc);
				io.push_back(ioIndex);
				
				if(ismeasure)
				{
					//segments
					for(int si=0; si<2; si++)
					{
						addSegment(ioIndex, lastindex[2 + si]);
					}
				}

				//if(j==0)
				//adauga pinurile
				if(isinit && (circ.at(i).at(j) == AA || circ.at(i).at(j) == YY))
				{
					vector<int> pins;
					pins.push_back(ioIndex);//nr

					for(int si=0; si<2; si++)
					{
						//vector<int> pinc = coords[lastindex[2 + si]];
						vector<int> pinc(coords[ioIndex]);

						int where = 2;

						pinc[where] += 1-2*(si%2);//stanga si dreapta ioIndex
						pins.insert(pins.begin()+1, pinc.begin(), pinc.end());//coord of pin
					}
				}
			}

			//if(j == 0)//daca e init
			if(isinit)//daca e init
			{
				for(int si=0; si<2; si++)
					lastindex[0 + si] = lastindex[2 + si];//will be used at the later point at indices 0,1
			}

			//if(j == circ.at(i).size() - 1)//daca e measure
			if(ismeasure)
			{
				//add segments to the left
				for(int si=0; si<2; si++)
				{
					addSegment(lastindex[0 + si], lastindex[2 + si]);//finalise the segments
					lastindex[0 + si] = lastindex[2 + si]; //will be used at the later point at indices 0,1
				}
			}
		}
	}
	
	//here the dual space is constructed
	//for each control search the target
	for(int i=0; i<circ.size(); i++)
	{
		for(int j=0; j<circ.at(i).size(); j++)
		{
			if(isCnot(circ.at(i).at(j)) && getCnotPart(circ.at(i).at(j)) == 1)
			{
				//printf("%d --> %d\n", i, comps.at(comps.size()-1));
				addDual(i, j);
			}			
		}
	}
}

int main(int argc, char** argv)
{
	//writeQCircuitHeader();

	timeval tim;
	gettimeofday(&tim, NULL);
	srand48((unsigned int) tim.tv_usec);

	int distrounds = 0;//number of distillation rounds

	if(argc < 2)
	{
		printf("run with [random | fname] [distrounds]\n");
		return 2;
	}

	if(argc == 3)
	{
		distrounds = atoi(argv[2]);
	}

	if(strcmp("random", argv[1]) != 0)
	{
		//open file
		readFile(argv[1]);
	}
	else
	{
		sprintf(circuitName, "random.in");

		int nrq = 4;
		//random file and saved into file.in
		for(int i=0; i<nrq; i++)
		{
			vector<int> v;
			v.push_back(INPUT);
			circ.push_back(v);
		}

		printf("== Generating Circuit\n");
		int r = getRandomNumber(2,4);
		//int r = 1;
		printf("Number of Clifford Fields %d\n", r);

		for(int i=0; i<r; i++)
		{
			makeCliffordPart(nrq);
			makeTPart(nrq);
		}

		writeFile();
	}

	/**/
	int myints1[]= {MX, MZ, MXZ, MZX, MA, MYZ, MY, OUTPUT};
	measurements=set<int>(myints1, myints1 + 8);

	/**/
	int myints2[]= {AA, YY, ZERO, PLUS, INPUT};
	initialisations=set<int>(myints2, myints2 + 5);

	//convert all nonicm, it could happen that nonicm results into nonicm
	replacePattern(NICM);
	replacePattern(ICM);

	for(int disti = 0; disti < distrounds; disti++)
	{
		replacePattern(NICM);//takes MA and MY to T,P MX
		replacePattern(ICMINJ);//insert distillation
	}

	printCirc();

	makeGeometry();
	writeGeometry();

	writePostScriptFile();

	return 1;
}
