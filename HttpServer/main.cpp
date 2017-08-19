#include "net/net.hpp"
#include "session/httpsession.hpp"
#include "session/httprequest.hpp"
#include "session/httpresponse.hpp"
#include "route/route.hpp"
#include <stdio.h>

class HttpNet :public Net<HttpSession>
{
public:
	void recvAClientReq(HttpSession* session)
	{
		printf("get a client req\n");
		HttpRequest* req = session->getHttpRequest();
		char* url = const_cast<char*>(req->getValue("url"));
		printf("dispatch url:%s\n",url);
		Route::dispatch<HttpSession>(url,session);
	}
};

int main()
{
	Route::Instance()->setUp(const_cast<char*>("path.ini"),
							 const_cast<char*>("route.ini"));
	HttpNet http_net;
	http_net.start(4567);
	http_net.run();
}