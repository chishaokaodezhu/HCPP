/*
* gliu 2017.8.5
* write_bugs@163.com
*
*/
#pragma once

class Session{
public:
	typedef void(*responseHandle)(void*, int, int);

	/*
	* responseHandle用于在Session发起响应时调用
	*/
	Session(responseHandle _handle, int client_fd, int _epfd)
		:mClientFd(client_fd)
		, mEpfd(_epfd)
		, fd(client_fd)
		, inResponse(false)
		, mResponseHandle(_handle)
	{
		reset();
	}

	/*
	* 用户调用此函数发起一次响应，派送类重写时必须在函数最后调用此函数
	*/
	void startResponse()
	{
		mResponseHandle(getInstancePtr(), mClientFd, mEpfd);
	}

	/*
	* 派送类重写此函数时必须在函数返回时写如下语句
	* return (void*)this;
	*/
	virtual void* getInstancePtr() = 0;


	/*
	* 重置会话的数据状态，派生类重写此函数时必须调用此函数
	*/
	void reset()
	{
		inResponse = false;
	}

	virtual char* 	getReqBufWritePtr() = 0;
	virtual size_t 	getRestReqBufSize() = 0;
	virtual void 	commitReqData(size_t data_size) = 0;
	virtual bool	isDataOver() = 0;
	virtual void 	expandReqBuf() = 0;

	virtual size_t 	getRestRespDataSize() = 0;
	virtual char* 	getCouldSentDataPtr() = 0;
	virtual size_t 	getCouldSentDataSize() = 0;
	virtual void 	commitSentDataSize(size_t sent_size) = 0;

public:
	int fd;
private:
	responseHandle mResponseHandle;
	int  mClientFd;
	int  mEpfd;
	bool inResponse;
};