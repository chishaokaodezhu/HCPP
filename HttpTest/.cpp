#include <string>
#include "../HttpServer/include.hpp"
using namepsace std;
extern "C"
bool (HttpSession* session)
{
	HttpRequest* req = session->getHttpRequest();
	HttpResponse* resp = session->getHttpResponse();
	/*---------------------------------------------------------------------*/
	string hinfo_name(getModulePath());
	hinfo_name.append("/.hinfo");
	HcppFile* hcpp_file = getFileManager()->getFileObj<HcppFile>(const_cast<char*>(hinfo_name.c_str()));
	resp.putBody(hccp_file->getSegNPtr(0),hccp_file->getSegNSize(0));
	cout<<"helloword"<<endl;
	resp.putBody(hccp_file->getSegNPtr(1),hccp_file->getSegNSize(1));
	cout<<"hello"<<endl; 
	resp.putBody(hccp_file->getSegNPtr(2),hccp_file->getSegNSize(2));
	/*---------------------------------------------------------------------*/
	session->startResponse();
	return true;
}
