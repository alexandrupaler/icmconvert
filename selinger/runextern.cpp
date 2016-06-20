#include <stdio.h>
#include <string.h>
#include <vector>
#include <math.h>
#include <string>
#include "runextern.h"

using namespace std;

vector<string> gridsynth(double angle)
{
//	printf("Hello World!\n");

	char buffer[512];
	sprintf(buffer, "wine gridsynth.exe \"(%lf)\"", angle);
	FILE* f = popen(buffer, "r");

	vector<char> output;

	char buff[512];
	while(fgets(buff, sizeof(buff), f)!=NULL)
	{
		//printf("%s", buff);
		for(int i=0; i<strlen(buff); i++)
			output.push_back(buff[i]);
	}

	pclose(f);

//	printf("\n");

	vector<string> circuitorder;
	for(int i=output.size() -1; i>=0; i--)
	{
		if(output.at(i) == 'S')
			circuitorder.push_back("PGATE");//PGATE
		else if(output.at(i) == 'H')
			circuitorder.push_back("HGATE");//HGATE
		else if(output.at(i) == 'T')
			circuitorder.push_back("TGATE");//TGATE
		else if(output.at(i) == 'X')
			{
				//skip X - will be tracked through the circuit
				/*
   			    circuitorder.push_back(4);//desfac X in HZH=HPPH - imi creste circuitul
				circuitorder.push_back(3);
				circuitorder.push_back(3);
				circuitorder.push_back(4);
				*/
			}
		else if(output.at(i) == 'W')
			continue;
		/*else
		{
			printf("'%c'", output.at(i));
			circuitorder.push_back(output.at(i));
		}*/ //scap de newline aici
	}

	return circuitorder;
}

/*
int main(int argc, char**)
{
	vector<int> circuitorder = gridsynth(M_PI/16);
	printf("1\n%d\n-100 ", circuitorder.size() + 1);
	for(int i=0; i<circuitorder.size(); i++)
	{
		printf("%d ", circuitorder.at(i));
	}

	printf("\n");
}
*/
