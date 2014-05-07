#include"Utility.h"

bool Utility::isStringNumber(const std::string inputString)
{
	char tempChar;
	for(int i=0;i<inputString.size();i++)
	{
		tempChar = inputString[i];
		if(tempChar<'0' || tempChar>'9')
		{
			return false;	
		}
	}
	return true;
}

void  Utility::toUpper(string & inputString)
{
	for(int i=0;i<inputString.size();i++)
	{
		if(inputString[i]>='a' && inputString[i]<='z')
		{
			inputString[i] = inputString[i] -32;
		}
	}
}