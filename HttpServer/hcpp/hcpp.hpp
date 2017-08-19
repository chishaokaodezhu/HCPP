#pragma once
#include <vector>
#include <fstream>
#include <stdlib.h>
#include <string>
#include <string.h>
#include <stdio.h>
using namespace std;

#define LINE_END "\r\n"

class HCPP{
private:
	char* 	mFilebuf;
	bool	mInHtml;
	char*	mSegStart;
	char*	mSegEnd;
	char*	mDeadPtr;
	size_t	mParsedSize;
	size_t	mFileSize;
	size_t	mHtmlSegCount;
	size_t	mIncludeFileSize;
	vector<int>	mSegSizeAry;

public:
	HCPP(char* file_name)
	:mFilebuf(nullptr)
	,mInHtml(true)
	,mSegStart(nullptr)
	,mSegEnd(nullptr)
	,mDeadPtr(nullptr)
	,mParsedSize(0)
	,mFileSize(0)
	,mHtmlSegCount(0)
	,mIncludeFileSize(0)
	{
		ifstream inf(file_name);
		if(!inf.is_open())
		{
			perror("open hcpp file error\n");
			exit(1);
		}
		inf.seekg(0,std::ios::end);
		mFileSize = inf.tellg();
		inf.seekg(0,std::ios::beg);

		mFilebuf = new char[mFileSize+sizeof("<?cpp")];
		inf.read(mFilebuf,mFileSize);
		mDeadPtr = mFilebuf + mFileSize;
		mSegStart = mSegEnd = mFilebuf;

		inf.close();
	}

	~HCPP()
	{
		delete[] mFilebuf;
	}

	void parse()
	{
		parse_include_file_size();
		set_start_status();
		start_parse();
	}

	void toHtmlInfoFile(char* file_name)
	{
		ofstream ofs(file_name);
		ofs<<mHtmlSegCount<<endl;
		for(int i=0;i<mSegSizeAry.size();i++)
		{
			int size = mSegSizeAry[i];
			if(size > 0)
				ofs<<size<<endl;
		}

		char* wptr = mFilebuf+mIncludeFileSize;
		for(int i=0;i<mSegSizeAry.size();i++)
		{
			int size = mSegSizeAry[i];
			if(size > 0)
				ofs.write(wptr,size);
			else
				size = 0 - size;

			wptr += size;
		}

		ofs.close();
	}

	void toCppFile(char* templet_file_name,char* cpp_file_name,char* hinfo_file_name)
	{
		ifstream ifs(templet_file_name);
		if(!ifs.is_open())
		{
			perror("open templete file error\n");
			exit(1);
		}
		int i_line = 0, c_line = 0, fn_line = 0;
		string line;
		getline(ifs,line);
		i_line = atoi(line.c_str());
		getline(ifs,line);
		fn_line = atoi(line.c_str());
		getline(ifs,line);
		c_line = atoi(line.c_str());

		ofstream ofs(cpp_file_name);
		//--------------------------------------------------------------------------------------------
		for(int i=0;i<i_line;i++)
		{
			if(!getline(ifs,line))
				break;
			ofs<<line<<LINE_END;
		}
		//write include file
		ofs.write(mFilebuf,mIncludeFileSize);
		//--------------------------------------------------------------------------------------------
		for(int i=0;i<fn_line;i++)
		{
			if(!getline(ifs,line))
				break;
			ofs<<line<<LINE_END;
		}
		string func_name(cpp_file_name);
		char* f_name = new char[func_name.size()];
		func_name.copy(f_name,func_name.size(),0);
		for(int i=0;i<func_name.size();i++){
			if(f_name[i] == '.'){
				f_name[i] = '\0';
				break;
			}
		}
		ofs<<"bool "<<f_name<<"(HttpSession* session)"<<LINE_END;
		delete[] f_name;
		//--------------------------------------------------------------------------------------------
		for(int i=0;i<c_line;i++)
		{
			if(!getline(ifs,line))
				break;
			ofs<<line<<LINE_END;
		}
		//write_code
		char 	str_buf[128];
		char* 	wptr = mFilebuf+mIncludeFileSize;
		int 	html_index = 0;
		ofs<<"\t/*---------------------------------------------------------------------*/"<<LINE_END;
		ofs<<"\tstring hinfo_name(getModulePath());"<<LINE_END;
		sprintf(str_buf,
				"\thinfo_name.append(\"/%s\");%s\0",
				hinfo_file_name,LINE_END);
		ofs<<str_buf;
		sprintf(str_buf,
				"\tHcppFile* hcpp_file = getFileManager()->getFileObj<HcppFile>(const_cast<char*>(hinfo_name.c_str()));%s\0",
				LINE_END);
		ofs<<str_buf;
		for(int i=0;i<mSegSizeAry.size();i++)
		{
			int size = mSegSizeAry[i];
			if(size > 0)
			{
				sprintf(str_buf,
						"\tresp->putBodyData(hcpp_file->getSegNPtr(%d),hcpp_file->getSegNSize(%d));%s\0",
						html_index,html_index,LINE_END);
				html_index++;
				ofs.write(str_buf,strlen(str_buf));
			}
			else
			{
				size = 0 - size;
				ofs<<"\t";
				char* p_code = wptr+sizeof("<?cpp");
				int code_size = size - sizeof("<?cpp?>");
				ofs.write(p_code,code_size);
				if(!check_new_line(p_code,code_size))
					ofs<<LINE_END;
			}
			wptr += size;
		}
		ofs<<"\t/*---------------------------------------------------------------------*/"<<LINE_END;
		//write the rest code
		while(getline(ifs,line))
		{
			ofs<<line<<LINE_END;
		}

		ifs.close();
		ofs.close();
	}
private:
	void set_start_status()
	{
		mInHtml = true;
		if(*mSegStart == '<')
			if(*(mSegStart+1) == '?')
				mInHtml = false;
	}

	void parse_include_file_size()
	{
		while(mSegEnd < mDeadPtr)
		{
			if(*mSegEnd == '#')
				parse_include_line();
			else
				break;
		}
		mIncludeFileSize = mSegEnd - mSegStart;
		mSegStart = mSegEnd;
	}

	void parse_include_line()
	{
		mSegEnd++;
		while(mSegEnd < mDeadPtr)
		{
			if(*mSegEnd == '\r')
			{
				if(*(mSegEnd+1) == '\n')
				{
					mSegEnd+=2;
					break;
				}
			}else if(*mSegEnd == '\n')
			{
				mSegEnd++;
				break;
			}else
			{
				mSegEnd++;
			}
		}
	}

	void start_parse()
	{
		while(mSegEnd < mDeadPtr)
		{
			if(mInHtml)
			{
				parse_html();
				mHtmlSegCount++;
			}
			else
			{
				parse_cpp();
			}
			//printf("parse size is:%d\n",mParsedSize);
		}
	}

	void parse_html()
	{
		//printf("------>startParseHTML\n");
		while(mSegEnd < mDeadPtr)
		{
			if(*mSegEnd == '\'')
				read_ch();
			if(*mSegEnd == '"')
				read_str();
			if(*mSegEnd == '<')
			{
				char nextch = *(mSegEnd + 1);
				if(nextch == '?')
				{
					mInHtml = false;
					mSegSizeAry.push_back(mSegEnd - mSegStart);
					mParsedSize += mSegEnd - mSegStart;

					print_parsed_data();

					mSegStart = mSegEnd;
					break;
				}
			}

			mSegEnd++;
		}

		if(mSegEnd == mDeadPtr)
		{
			mInHtml = false;
			mSegSizeAry.push_back(mSegEnd - mSegStart);
			mParsedSize += mSegEnd - mSegStart;

			print_parsed_data();

			mSegStart = mSegEnd;
		}
	}
	

	void parse_cpp()
	{
		//printf("------>startParseCPP\n");
		while(mSegEnd < mDeadPtr)
		{
			if(*mSegEnd == '\'')
				read_ch();
			if(*mSegEnd == '"')
				read_str();
			if(*mSegEnd == '/')
				read_comment();
			if(*mSegEnd == '?')
			{
				char nextch = *(mSegEnd + 1);
				if(nextch == '>')
				{
					mSegEnd += 2;
					mInHtml = true;
					mSegSizeAry.push_back(mSegStart - mSegEnd);
					mParsedSize += mSegEnd - mSegStart;

					print_parsed_data();

					mSegStart = mSegEnd;
					break;
				}
			}

			mSegEnd++;
		}

		if(mSegEnd == mDeadPtr)
		{
			mInHtml = true;
			mSegSizeAry.push_back(mSegEnd - mSegStart);
			mParsedSize += mSegEnd - mSegStart;

			print_parsed_data();

			mSegStart = mSegEnd;
		}
	}

	void print_parsed_data()
	{
		return;
		char temp = *mSegEnd;
		*mSegEnd = 0;
		printf("parse html is:\n%s\n",mSegStart);
		*mSegEnd = temp;
	}

	bool check_new_line(char* data,int size)
	{
		char* deadptr = data + size;
		while(data < deadptr)
		{
			if(*data == '\n')
				return true;
			data++;			
		}
		return false;
	}

	void read_ch()
	{
		mSegEnd++;
		while(mSegEnd < mDeadPtr)
		{
			if(*mSegEnd == '\\')
				mSegEnd+=2;
			if(*mSegEnd == '\'')
			{
				mSegEnd++;
				break;
			}
			mSegEnd++;
		}
	}

	void read_str()
	{
		mSegEnd++;
		while(mSegEnd < mDeadPtr)
		{
			if(*mSegEnd == '\\')
				mSegEnd+=2;
			if(*mSegEnd == '"')
			{
				mSegEnd++;
				break;
			}

			mSegEnd++;
		}
	}

	void read_comment()
	{
		mSegEnd++;
		if(*mSegEnd == '*')
			read_mul_line_comment();
		else
			read_line_comment();
	}

	void read_mul_line_comment()
	{
		mSegEnd++;
		while(mSegEnd < mDeadPtr)
		{
			if(*mSegEnd == '*')
			{
				char nextch = *(mSegEnd+1);
				if(nextch == '/')
				{
					mSegEnd+=2;
					break;
				}
			}

			mSegEnd++;
		}
	}

	void read_line_comment()
	{
		mSegEnd++;
		while(mSegEnd < mDeadPtr)
		{
			if(*mSegEnd == '\r')
			{
				mSegEnd++;
				if(*mSegEnd == '\n')
					mSegEnd++;
				break;
			}
			else if(*mSegEnd == '\n')
			{
				mSegEnd++;
				break;
			}else{
				mSegEnd++;
			}
		}
	}
};