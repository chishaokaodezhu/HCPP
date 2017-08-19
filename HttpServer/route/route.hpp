/*
* gliu 2017.8.5
* write_bugs@163.com
* 
* Route从本地文件读取路由表，并根据路由表执行调用顺序
* route.ini内容示例如下
* /index xxx/xxx/lib32/module:auth xxx/xxx/lib32/module:index
* /list xxx/xxx/funclib:auth xxx/xxx/funclib:index
* 
* 上述module:auth module:index funclib:auth funclib:index中module
* 和funclib为动态连接库的名字，auth和index为动态连接库中的函数名。
* 组成url path/lib:func1 path/lib:func2的一行内容，对于请求的url会
* 依次调用func1 func2。一条url最多对应8个执行函数，每个函数的返回
* 值为ture/false,返回true则排在该函数后面的函数不会被执行，如返回
* false则继续调用该函数的下一个函数。
* 
* 为了能够处理未建立映射的url，route.ini中必须填入错误处理映射
* notfound /xxxx/xxxx/error:notfound
* 错误处理函数路径为绝对路径
* 
* 上面的路径为相对路径，在path.ini中读取workdir作为工作目录或者
* 为根目录。
* path.ini
* /root/home/xxx/
* 
*/
#pragma once

#include <map>
#include <fstream>
#include <iostream>
#include <string>

#include <dlfcn.h>
#include <unistd.h>
#include <linux/limits.h>

#include "functable.hpp"

using namespace std;
/*
#define PATH_CONF  "/xxx/CPPHttpServer/path.ini"
#define ROUTE_CONF "/xxx/CPPHttpServer/route.ini"
*/
#define MODULE_EXTENSION_NAME ".so"
class Route{
public:
	typedef FuncTable FuncTbl;
	typedef std::map<string,FuncTbl*> FuncMap;

private:
	Route()
	{

	}

public:
	static Route* Instance()
	{
		static Route route;
		return &route;
	}

	/*
	* Route类在程序启动时需要调用本函数实现路由表的加载
	*/
	void setUp(char* path_ini_path,char* route_ini_path)
	{
		load_root_dir(path_ini_path);
		load_route_map(route_ini_path);
	}

	/*
	* 根据输入的url调用对应的服务函数。如果url对应的服务函数
	* 列表中任何一个函数返回了true，则dispatch过程结束
	*/
	template <typename ArgType>
	static void dispatch(char* url,ArgType* arg)
	{
		typedef bool (*ServerFunc)(ArgType*);
		FuncTbl* func_tbl = Instance()->getFuncTbl(url);
		if(func_tbl == nullptr)
		{
			func_tbl = Instance()->getFuncTbl(const_cast<char*>("notfound"));
			if(func_tbl == nullptr)
				return;
			void* func = func_tbl->getFuncById(0);
			if(func == nullptr)
				return;
			ServerFunc sfunc = (ServerFunc)func;
			sfunc(arg);
			return;
		}

		for(int i=0;i<func_tbl->getFuncNum();i++)
		{
			void* func = func_tbl->getFuncById(i);
			if(func == nullptr)
				continue;
			ServerFunc sfunc = (ServerFunc)func;
			if(sfunc(arg))
				break;
		}
	}

	FuncTbl* getFuncTbl(char* url)
	{
		FuncMap::iterator it = mRouteMap.find(url);
		if(it != mRouteMap.end())
			return it->second;
		return nullptr;
	}
private:
	void load_root_dir(char* path_ini_path)
	{
		ifstream inf(path_ini_path);
		getline(inf,mRootDir);
		inf.close();
	}

	void load_route_map(char* route_ini_path)
	{
		ifstream inf(route_ini_path);
		string line;
		while(getline(inf,line))
		{
			//get url
			int index = line.find_first_of(' ',0);
			if(index == std::string::npos)
				continue;
			string url=line.substr(0,index);
			//cout<<"url:"<<url<<"<"<<endl;
			if(url.size() <= 0)
				continue;

			FuncTbl* ftbl = new FuncTbl();
			//get module
			int last = index + 1;
			index = line.find_first_of(' ',last);
			while(index != std::string::npos)
			{
				string module = line.substr(last,index-last);
				//cout<<"module:"<<module<<"<"<<endl;
				if(module.size() > 0)
					ftbl->putAFunc(load_module(module));
				last = index + 1;
				index = line.find_first_of(' ',last);
			}
			
			string module = line.substr(last,line.size()-last);
			//cout<<"module:"<<module<<"<"<<endl;
			if(module.size() > 0)
				ftbl->putAFunc(load_module(module));
			//new a map item
			if(ftbl->getFuncNum() > 0)
			{
				mRouteMap.insert(pair<string,FuncTbl*>(url, ftbl));
			}
		}

	}

	void* load_module(string module)
	{
		int colon_index = module.find_first_of(':',0);
		if(colon_index == std::string::npos)
			return nullptr;

		string r_path = module.substr(0,colon_index);
		colon_index++;
		string func_name = module.substr(colon_index,module.size()-colon_index);
		if(r_path.size() <= 0 || func_name.size() <=0)
			return nullptr;

		if(r_path[0] == '/')
			return get_func_addr(r_path,func_name);
		return get_func_addr(mRootDir + r_path,func_name);
	}

	typedef void (*setModulePath)(char*);
	void* get_func_addr(string lib_path,string func_name)
	{
 		lib_path += MODULE_EXTENSION_NAME;
 		void* pdl_handle = dlopen(lib_path.c_str(),RTLD_LAZY);
 		if(pdl_handle == nullptr){
 			//cout<<"the lib name is:"<<lib_path<<endl;
 			exit_e("dlopen error");
 		}

 		void* func = dlsym(pdl_handle,"setModulePath");
 		char* errofinfo = dlerror();
 		if(errofinfo != nullptr)
 			exit_e("invalid module:no setModulePath()");

 		setModulePath set_func = (setModulePath)func;
 		set_func(const_cast<char*>(lib_path.c_str())); 

 		func = dlsym(pdl_handle,func_name.c_str());
 		errofinfo = dlerror();
 		if(errofinfo != nullptr)
 			exit_e(errofinfo);

 		return func;
	}

	void exit_e(const char* e)
	{
		perror(e);
		exit(1);
	}
private:
	string 	mRootDir;
	FuncMap mRouteMap;
};