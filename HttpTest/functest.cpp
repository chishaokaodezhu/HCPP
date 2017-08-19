#include <string>
#include "../HttpServer/include.hpp"
using namespace std;
extern "C"
bool functest(HttpSession* session)
{
	HttpRequest* req = session->getHttpRequest();
	HttpResponse* resp = session->getHttpResponse();
	/*---------------------------------------------------------------------*/
	string hinfo_name(getModulePath());
	hinfo_name.append("/functest.hinfo");
	HcppFile* hcpp_file = getFileManager()->getFileObj<HcppFile>(const_cast<char*>(hinfo_name.c_str()));
	resp->putBodyData(hcpp_file->getSegNPtr(0),hcpp_file->getSegNSize(0));
	resp->putBodyData("hello world",sizeof("hello world"));
	resp->putBodyData(hcpp_file->getSegNPtr(1),hcpp_file->getSegNSize(1));
	resp->putBodyData("hello",sizeof("hello")); 
	resp->putBodyData(hcpp_file->getSegNPtr(2),hcpp_file->getSegNSize(2));
	/*---------------------------------------------------------------------*/
	session->startResponse();
	return true;
}
