/*
* gliu 2017.8.5
* write_bugs@163.com
* 
*/
#pragma once
#include <string>
#include <stdlib.h>
#include "httpsession.hpp"
using namespace std;

class HttpResponse{
public:
	HttpResponse()
	:mHeaderBuf(nullptr)
	,mHeaderBufSize(0)
	,mHeaderSize(0)
	,mBodyBufSize(1024)
	,mBodySize(0)
	,mSentBytes(0)
	,mState("200 ok\r\n")
	{
		mBodyBuf = new char[1024];
	}

	~HttpResponse()
	{
		if(mHeaderBuf != nullptr)
			delete[] mHeaderBuf;
		if(mBodyBuf != nullptr)
			delete[] mBodyBuf;
	}

	void appendHeader(char* key,char* value)
	{
		mHeader.append(key);
		mHeader.append(":");
		mHeader.append(value);
		mHeader.append("\r\n");
	}

	/*
	* 设置响应状态，例如："200 ok"
	*/
	void setHttpState(char* state)
	{
		mState = state;
		mState.append("\r\n");
	}

	void reset()
	{
		mHeader = "";
		/*
		if(mHeaderBuf != nullptr){
			delete[] mHeaderBuf;
			mHeaderBuf = nullptr;
		}
		mHeaderBufSize = 0;
		*/
		mHeaderSize = 0;
		mBodySize = 0;
		mSentBytes = 0;
	}

	void putBodyData(char* data,size_t size)
	{
		if(size > (mBodyBufSize - mBodySize -1))
			expandBuf(size);
		char* write_ptr = mBodyBuf + mBodySize;
		memcpy(write_ptr,data,size);
		mBodySize += size;
		mBodyBuf[mBodySize] = '\0';
	}

	/*
	* 此函数用于扩展数据缓存大小，用户可以使用该函数预先将
	* 缓冲区大小扩展到一个合适的大小，避免putBodyData函数
	* 频繁地进行缓冲区扩展
	*/
	void expandBuf(size_t size)
	{
		char* new_buf = new char[mBodyBufSize + size];
		if(new_buf == nullptr)
			return;
		memcpy(new_buf,mBodyBuf,mBodySize);
		delete[] mBodyBuf;
		mBodyBuf = new_buf;
		mBodyBufSize = mBodyBufSize + size;
	}

private:
	friend class HttpSession;

	inline size_t getRestRespDataSize()
	{
		size_t total_size = mHeaderSize + mBodySize;
		return total_size - mSentBytes;
	}

	char* getCouldSentDataPtr()
	{
		if(mSentBytes < mHeaderSize)
		{
			return mHeaderBuf + mSentBytes;
		}
		else
		{
			size_t body_sent_size = mSentBytes - mHeaderSize;
			return mBodyBuf + body_sent_size;
		}
	}

	inline size_t getCouldSentDataSize()
	{
		if(mSentBytes < mHeaderSize)
		{
			return mHeaderSize - mSentBytes;
		}
		else
		{
			size_t body_sent_size = mSentBytes - mHeaderSize;
			return mBodySize - body_sent_size;
		}
	}

	inline void commitSentDataSize(size_t sent_size)
	{
		mSentBytes += sent_size;
	}

	void set_http_version(char* version)
	{
		mVersion = version;
		mVersion.append(" ");
	}

	void format_response_data()
	{
		string content_length = "Content-Length:";
		content_length += std::to_string(mBodySize);
		content_length += "\r\n\r\n";
		mHeader.append(content_length);
		printf("header is:%s<\n",mHeader.c_str());

		mHeaderSize = mVersion.size() 
						 + mState.size() 
						 + mHeader.size();

		if(mHeaderBufSize < mHeaderSize)
		{
			char* new_buf = new char[mHeaderSize+1];
			new_buf[mHeaderSize] = '\0';
			delete[] mHeaderBuf;
			mHeaderBuf = new_buf;
			mHeaderBufSize = mHeaderSize;
		}

		int wpos = 0;
		memcpy(&mHeaderBuf[wpos],mVersion.c_str(),mVersion.size());
		wpos += mVersion.size();
		memcpy(&mHeaderBuf[wpos],mState.c_str(),mState.size());
		wpos += mState.size();
		memcpy(&mHeaderBuf[wpos],mHeader.c_str(),mHeader.size());
		//wpos += mState.size();
		printf("byxxxxxxxxx\n");
	}

private:
	string mVersion;
	string mState;
	string mHeader;

	char*	mHeaderBuf;
	size_t	mHeaderBufSize;
	size_t	mHeaderSize;

	char*	mBodyBuf;
	size_t	mBodyBufSize;
	size_t	mBodySize;

	size_t	mSentBytes;
};
