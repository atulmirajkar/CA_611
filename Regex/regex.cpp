#include<iostream>
#include<regex>
using namespace std;

void main()
{
	smatch matches;
	string str;
	while(true)
	{
		cin>>str;
		//regex e("R[1-32],[0-9]*\(R[1-32]\)");
		//regex e("(r\\d{1,2}),(\\d*)\\((r\\d{1,2})\\)",regex_constants::icase);
		//regex e("(\\w*)");
		//regex e("(r\\d{1,2}),(r\\d{1,2}),(\\w*)",regex_constants::icase);
		//regex e("(r\\d{1,2}),(r\\d{1,2}),(r\\d{1,2})",regex_constants::icase);
		//regex e("(f\\d{1,2}),(f\\d{1,2}),(f\\d{1,2})",regex_constants::icase);
		//regex e("(f[0-9]|[0][0-9]|[12][0-9]|[3][0-2]),(f[0-9]|[0][0-9]|[12][0-9]|[3][0-2]),(f[0-9]|[0][0-9]|[12][0-9]|[3][0-2])",regex_constants::icase);
		//regex e("(f[0-9]|f[0][0-9]|f[12][0-9]|f[3][0-1])",regex_constants::icase);
		regex e("(r[0-9]|r[0][0-9]|r[12][0-9]|r[3][0-1])",regex_constants::icase);

		bool match = regex_match(str,matches,e);
		cout<< (match? "Matched":"not matched")<<endl<<endl;

		cout<<matches.size()<<endl;
		for(int i=0;i<matches.size();i++)
		{
			cout<<matches[i].str()<<endl;
		}

	}

}