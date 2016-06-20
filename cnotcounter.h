#ifndef CNOTCOUNTER_H__
#define CNOTCOUNTER_H__

int startCnotNumber = 10000;
int numberCnot = 0;

int getCnotPart(int val)
{
	if(val > startCnotNumber)
	{
		int type = val%2;
		type += (type == 0 ? TGT : 0);
		return type;
	}
	return -1;
}

bool isCnot(int val)
{
	return (getCnotPart(val) != -1);
}

int getCnotNumber()
{
	//this will be the number of the control
	int ret = startCnotNumber + 2*numberCnot + 1;
	//all the targets of a CNOT will have the same number(id) 10000+2*numberCnot + 2;
	numberCnot++;

	return ret;
}

bool sameCnot(int ctrlval, int targetval)
{
	return (ctrlval + 1 == targetval);
}

#endif


