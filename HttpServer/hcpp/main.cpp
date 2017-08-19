#include <string>
#include <stdio.h>
#include "hcpp.hpp"
using namespace std;

#define TEMPLETE_FILE "commonfile/cpp.templete"

int main(int argc, char* argv[])
{
	if(argc <= 1){
		perror("need inpute file name\n");
		exit(1);
	}
	for(int i=1;i<argc;i++)
	{
		string inputefile(argv[i]);
		printf("start parse %s\n",inputefile.c_str());
		int str_offset = inputefile.find(".hcpp");
		if(str_offset == string::npos){
			perror("file must be end with \".hcpp\"");
			exit(1);
		}

		string file_name = inputefile.substr(0,str_offset);
		string cpp_out = file_name + ".cpp";
		string hinfo_out = file_name + ".hinfo";

		HCPP hcpp(argv[i]);
		hcpp.parse();
		hcpp.toHtmlInfoFile(const_cast<char*>(hinfo_out.c_str()));
		hcpp.toCppFile(TEMPLETE_FILE,
						const_cast<char*>(cpp_out.c_str()),
						const_cast<char*>(hinfo_out.c_str()));
	}
	
}