#pragma once
#include <map>
#include <string>
#include <iostream>
using namespace std;

class FileManager
{
private:
	typedef map<string,void*> FileObjMap;
	FileObjMap	mFileObjs;

public:
	FileManager()
	{

	}

	template <typename FileType>
	FileType* getFileObj(char* file_name)
	{
		cout<<"======>getFileObj:"<<file_name<<endl;
		FileObjMap::iterator it = mFileObjs.find(file_name);
		if(it != mFileObjs.end())
			return (FileType*)it->second;
		FileType* file_obj = new FileType(file_name);
		string filename(file_name);
		mFileObjs.insert(pair<string,void*>(filename, (void*)file_obj));
		return file_obj;
	}
};


extern FileManager* getFileManager();
extern "C" void setModulePath(char* path);
extern char* getModulePath();
/*
{
	static FileManager fileManager;
	return &fileManager;
}
*/