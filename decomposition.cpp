#include <stdio.h>
#include "decomposition.h"

void decomposition::toString()
{
	printf("name: %s\n", name.c_str());
}

int decomposition::maxCols()
{
	int max = 0;

	if(!isicm())
	{
		for(int i=0; i<gates.size(); i++)
		{
			if (max < gates[i].size())
				max = gates[i].size();
		}
	}
	else
	{
		max = cnots.size();
		max++;//the measurements
	}

	return max;
}

bool decomposition::isicm()
{
	return (type == 0) || (type == 2);
}

bool decomposition::isdist()
{
	return type == 2;
}
