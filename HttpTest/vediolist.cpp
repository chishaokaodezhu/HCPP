#include <string>
#include "../HttpServer/include.hpp"
#include "commonfile/encodeurl.hpp"
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <string.h>
using namespace std;
extern "C"
bool vediolist(HttpSession* session)
{
	HttpRequest* req = session->getHttpRequest();
	HttpResponse* resp = session->getHttpResponse();
	/*---------------------------------------------------------------------*/
	string hinfo_name(getModulePath());
	hinfo_name.append("/vediolist.hinfo");
	HcppFile* hcpp_file = getFileManager()->getFileObj<HcppFile>(const_cast<char*>(hinfo_name.c_str()));
	resp->putBodyData(hcpp_file->getSegNPtr(0),hcpp_file->getSegNSize(0));
			struct stat s;
		lstat("/home/gliu/ServerRoot/vedio",&s);
		struct dirent* filename;
		DIR* dir = opendir("/home/gliu/ServerRoot/vedio");
		char buf[256];
		while((filename = readdir(dir)) != nullptr)
		{
			if(strcmp(filename->d_name,"." ) == 0 ||
			   strcmp(filename->d_name,"..") == 0)
			   continue;
			string path = "http://192.168.1.8/vedio/";
			path.append(filename->d_name);
			sprintf(buf,"<div class=\"vitem\" onclick=\"window.open('%s')\">"
						"%s"
						"</div>\n\0",
						path.c_str(),filename->d_name);
			resp->putBodyData(buf,strlen(buf));
		}
			resp->putBodyData(hcpp_file->getSegNPtr(1),hcpp_file->getSegNSize(1));
	/*---------------------------------------------------------------------*/
	session->startResponse();
	return true;
}
