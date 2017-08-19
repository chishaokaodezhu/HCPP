/*
* gliu 2017.8.5
* write_bugs@163.com
* 
*/

#pragma once
const int TABLE_SIZE = 8;

class FuncTable{
public:
	FuncTable()
	:mCount(0)
	{
		for(int i=0;i<TABLE_SIZE;i++)
			mTable[i] = nullptr;
	}	
	
	int getFuncNum()
	{
		return mCount;
	}	

	void* getFuncById(int idx)
	{
		if(idx >= mCount)
			return nullptr;
		return mTable[idx];
	}

	void putAFunc(void* func)
	{
		if(func == nullptr)
			return;
		mTable[mCount] = func;
		mCount++;
	}

private:
	void* 	mTable[TABLE_SIZE];
	int		mCount;
};