#pragma once
#include <fstream>
#include <string>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

using namespace std;

class HcppFile
{
private:
	char*	mBuf;
	int		mSegCount;
	int*	mSegSizeAry;
	char**	mSegOffsetAry;
	int		mTotalSize;

public:
	HcppFile(char* file_name)
	:mBuf(nullptr)
	,mSegCount(0)
	,mSegSizeAry(nullptr)
	,mSegOffsetAry(nullptr)
	,mTotalSize(0)
	{
		string line;
		ifstream ifs(file_name);
		getline(ifs,line);
		mSegCount = atoi(line.c_str());

		mSegSizeAry = new int[mSegCount];
		mSegOffsetAry = new char*[mSegCount];
		for(int i=0;i<mSegCount;i++)
		{
			getline(ifs,line);
			mSegSizeAry[i] = atoi(line.c_str());
			mTotalSize += mSegSizeAry[i];
		}

		mBuf = new char[mTotalSize];
		ifs.read(mBuf,mTotalSize);
		ifs.close();

		mSegOffsetAry[0] = mBuf;
		for(int i=1;i<mSegCount;i++)
		{
			mSegOffsetAry[i] = mSegOffsetAry[i-1] + mSegSizeAry[i-1];
		}
	}

	~HcppFile()
	{
		delete[] mBuf;
	}

	inline int getSegCount()
	{
		return mSegCount;
	}

	inline char* getSegNPtr(int idx)
	{
		if(mSegOffsetAry == nullptr)
			return nullptr;
		return mSegOffsetAry[idx];
	}

	inline int getSegNSize(int idx)
	{
		if(mSegSizeAry == nullptr)
			return 0;
		return mSegSizeAry[idx];
	}
};