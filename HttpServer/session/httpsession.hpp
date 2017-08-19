/*
* gliu 2017.8.5
* write_bugs@163.com
* 
*/
#pragma once
#include "../net/session.hpp"
#include "httprequest.hpp"
#include "httpresponse.hpp"

class HttpSession:public Session{
public:
	HttpSession(responseHandle _handle, int client_fd, int _epfd)
	:Session(_handle,client_fd,_epfd)
	,mReqParseOver(false)
	{

	}

	/*
	* 用户调用此函数发起一次响应，派送类重写时必须在函数最后调用此函数
	*/
	void startResponse()
	{
		mHttpResponse.format_response_data();
		Session::startResponse();
	}


	/*
	* 派送类重写此函数时必须在函数返回时写如下语句
	* return (void*)this;
	*/
	void* getInstancePtr()
	{
		return (void*)this;
	}


	/*
	* 重置会话的数据状态，派生类重写此函数时必须调用此函数
	*/
	void reset()
	{
		Session::reset();
		mHttpRequest.reset();
		mHttpResponse.reset();
		mReqParseOver = false;
	}

	HttpRequest* getHttpRequest()
	{
		return &mHttpRequest;
	}

	HttpResponse* getHttpResponse()
	{
		return &mHttpResponse;
	}

	/********************used by net**********************/
	char* getReqBufWritePtr()
	{
		return mHttpRequest.get_req_buf_write_ptr();
	}

	size_t getRestReqBufSize()
	{
		return mHttpRequest.get_rest_req_buf_size();
	}

	void commitReqData(size_t data_size)
	{
		mHttpRequest.commit_req_data(data_size);
		mReqParseOver = mHttpRequest.parse_req_data();
		if(mReqParseOver)
		{
			char* http_version = const_cast<char*>(mHttpRequest.getValue("version"));
			mHttpResponse.set_http_version(http_version);
		}
	}

	bool isDataOver()
	{
		return mReqParseOver;
	}

	void expandReqBuf()
	{
		mHttpRequest.expand_req_buf();
	}

	size_t getRestRespDataSize()
	{
		mHttpResponse.getRestRespDataSize();
	}

	char* getCouldSentDataPtr()
	{
		mHttpResponse.getCouldSentDataPtr();
	}

	size_t getCouldSentDataSize()
	{
		mHttpResponse.getCouldSentDataSize();
	}

	void commitSentDataSize(size_t sent_size)
	{
		mHttpResponse.commitSentDataSize(sent_size);
	}

private:
	HttpRequest 	mHttpRequest;
	HttpResponse 	mHttpResponse; 
	bool			mReqParseOver;
};