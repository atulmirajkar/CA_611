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