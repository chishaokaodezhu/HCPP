#include "../net/net.hpp"
#include "../session/httpsession.hpp"
#include "../session/httprequest.hpp"
#include <string>
#include <iostream>
#include <unistd.h>
#include <stdio.h>
using namespace std;
class staticTest{
public:
	staticTest()
	{
		cout<<"==============================>staticTest contractor"<<endl;
	}
};

extern "C"
bool testfunc(HttpSession* session)
{
	static staticTest test;
	HttpRequest* req = session->getHttpRequest();
	HttpResponse* resp = session->getHttpResponse();
	string msg = "you url is:";
	msg.append(req->getValue("url"));
	resp->putBodyData(const_cast<char*>(msg.c_str()),msg.size());
	char buffer[100];
	getcwd(buffer,sizeof(buffer));
	printf("the dir is:%s\n",buffer);
	session->startResponse();
	return true;
}