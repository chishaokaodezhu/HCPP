/*
* gliu 2017.8.5
* write_bugs@163.com
* 
*/
#pragma once
#include <map>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "httpsession.hpp"
using namespace std;

class HttpRequest{
public:
	HttpRequest()
	:mBuffSize(1024)
	,mDataSize(0)
	{
		mBuff = new char[mBuffSize];
		reset();
	}

	~HttpRequest()
	{
		delete[] mBuff;
	}

	/*
	* 用户通过该函数获取字段值
	* key="content-length"
	* http请求方法名/url/版本请求主体/也可通过此函数获取
	* key="method"/"url"/"version"/"body"/
	*/
	const char* getValue(const char* key)
	{
		struct const_str str_key;
		str_key.p_key = const_cast<char*>(key);
		map<const_str, char *, const_str_cmp>::iterator iter = mRecordMap.find(str_key);
		if (iter == mRecordMap.end())
	    	return nullptr;
		return iter->second;
	}

	void reset()
	{
		mDataSize = 0;
		mParseState = 0;
		mParseOffset = 0;
		mRecordMap.clear();
	}

private:
	//data define

	struct const_str
	{
		const char *p_key;
	};

	struct const_str_cmp
	{
		bool operator()(const const_str &key1, const const_str &key2) const
		{
			int ret = strcmp(key1.p_key, key2.p_key);
	    	return ret<0;
		}
	};

	map<const_str,char*,const_str_cmp> 	mRecordMap;

	char* 								mBuff;
	size_t 								mBuffSize;
	size_t								mDataSize;

	//for parse
	int mParseState;
	int   mParseOffset;
	char* mMethod;
	char* mUrl;
	char* mVersion;
private:
	//func called by HttpSession
	friend class HttpSession;

	inline char* get_req_buf_write_ptr()
	{
		return mBuff + mDataSize;
	}

	inline size_t get_rest_req_buf_size()
	{
		return mBuffSize - mDataSize - 1;
	}

	inline void commit_req_data(size_t size)
	{
		mDataSize+=size;
		mBuff[mDataSize] = 0;
	}

	inline void expand_req_buf()
	{
		char* new_buf = new char[mBuffSize*2];
		if(new_buf == nullptr)
			return;
		memcpy(new_buf,mBuff,mBuffSize);
		delete[] mBuff;
		mBuff = new_buf;
		mBuffSize = mBuffSize * 2;
	}

	inline char* get_req_buf_ptr()
	{
		return mBuff;
	}

	inline size_t get_req_buf_size()
	{
		return mBuffSize;
	}

	inline size_t get_req_data_size()
	{
		return mDataSize;
	}
	
	bool parse_req_data()
	{
		bool ret = parse_first_line();
		if(ret)
			ret = parse_record();
		if(ret)
			ret = parse_body();

		if(mParseState != 3)
			return false;
		return true;
		
	}

private:
	//func called by self
	bool parse_first_line()
	{
		if(mParseState > 0)
			return true;
		//parse method 
		mMethod = mBuff;
		char* deadptr = mBuff + mDataSize;
		char* ret = earse_space(mBuff,deadptr);
		if(ret == deadptr)
			return false;
		//parse url
		mUrl = ret;
		ret = earse_space(mUrl,deadptr);
		if(ret == deadptr)
			return false;
		//parse version
		mVersion = ret;
		ret = earse_end(mVersion,deadptr);
		if(ret == deadptr)
			return false;
		mParseState = 1;
		mParseOffset = ret - mBuff;
		//inserto map
		const_str key;
		key.p_key = "method";
		//printf("-->parse methon is%s\n",mMethod);
		mRecordMap.insert(pair<const_str, char *>(key, mMethod));
		key.p_key = "url";
		//printf("parse Url is %s\n",mUrl);
		mRecordMap.insert(pair<const_str, char *>(key, mUrl));
		key.p_key = "version";
		//printf("parse version is %s\n",mVersion);
		mRecordMap.insert(pair<const_str, char *>(key, mVersion));
		return true;
	}

	bool parse_record()
	{
		if(mParseState < 1)
			return false;
		if(mParseState > 1)
			return true;

		char* start = mParseOffset + mBuff;
		char* end = mBuff + mDataSize;
		while(true)
		{
			char* p_key = nullptr;
			char* p_value = nullptr;
			//parse record name
			p_key = start;
			char* ret = find_and_clear_colon(start,end);
			if(ret == nullptr)
				break;
			//parse record value
			p_value = ret;
			ret = earse_end(p_value,end);
			if(ret == nullptr)
				break;
			//find a full key:value
			start = ret;
			mParseOffset = ret - mBuff;
			const_str key;
			key.p_key = p_key;
			//printf("parse record %s:%s\n",p_key,p_value);
			mRecordMap.insert(pair<const_str, char *>(key, p_value));
			//check record is over
			if(*ret == '\r' && *(ret+1)=='\n')
			{
				//printf("parse record over()!\n");
				mParseState = 2;
				mParseOffset = ret+2 - mBuff;
				return true;
			}
		}

		return false;
	}

	bool parse_body()
	{
		if(mParseState < 2)
			return false;
		if(mParseState > 2)
			return true;

		const char* content_length = getValue("Content-Length");
		if(content_length == nullptr)
		{
			mParseState = 3;
			return true;
		}

		size_t body_size = mDataSize - mParseOffset;
		if(atoi(content_length) <= body_size)
		{
			mParseState = 3;
			const_str key;
			key.p_key = "body";
			mRecordMap.insert(pair<const_str, char *>(key, mBuff + mParseOffset));
			return true;
		}
		return false;
	}

	/*
	* 对于start到end之间的字符串，略过非空格符号，
	* 将第一次遇到的空格及其连续相邻的空格变成'\0'
	* 返回空格后面的字符串指针
	*/
	inline char* earse_space(char* start,char* end)
	{
		while(start < end)
		{
			if(*start == ' ')
			{
				while(start < end && *start == ' '){
					*start = '\0';
					start++;
				}
				break;
			}

			start++;
		}

		return start;
	}

	/*
	* 对于start到end之间的字符串，略过\r\n，
	* 将第一次遇到的\r\n变成'\0'，返回\r\n后面的字符串
	*/
	inline char* earse_end(char* start,char* end)
	{
		while(start < end)
		{
			if(*start == '\r')
			{
				start++;
				if(start == end)
					return nullptr;
				if(*start != '\n')
					return nullptr;
				*start = '\0';
				*(start - 1) = '\0';
				break;
			}

			start++;
		}

		if(start == end)
			return nullptr;
		return ++start;
	}

	/*
	* 对于start到end之间的字符串，将首次遇到的连续' '或者':'
	* 替换为'\0',并返回后面的字符串
	*/
	inline char* find_and_clear_colon(char* start,char* end)
	{
		while(start < end)
		{
			if(*start == ' ' || *start == ':')
			{
				while(start < end)
				{
					if(*start == ' ' || *start == ':'){
						*start = '\0';
						start++;
					}else{
						break;
					}
				}
				break;
			}

			start++;
		}

		if(start == end)
			return nullptr;
		return start;
	}
};