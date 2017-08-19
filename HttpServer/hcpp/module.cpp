#include "filemanager.hpp"
#include <string.h>

FileManager* getFileManager()
{
	static FileManager fileManager;
	return &fileManager;
}


static char* getModulePathBuf(int size)
{
	static char* buf = nullptr;
	static int buf_size = 0;
	if(size > buf_size)
	{
		if(buf != nullptr)
			delete[] buf;
		buf = new char[size];
		buf_size = size;
	}
	return buf;
}


extern "C"
void setModulePath(char* path)
{
	static bool run_once = false;
	if(run_once)
		return;
	run_once = true;

	int path_size = strlen(path);
	char* buf = getModulePath(path_size);
	memcpy(buf,path,path_size);

	for(int i = buf_size - 1;i>=0;i--)
	{
		if(buf[i] == '/')
		{
			buf[i] = '\0';
			break;
		}
	}
}

char* getModulePath()
{
	return getModulePathBuf(0);
}