#include "encodeurl.hpp"
using namespace std;
std::string UrlEncode(char* url)
{
	std::string src(url);
    char hex[] = "0123456789ABCDEF";
    string dst;
 
    for (size_t i = 0; i < src.size(); ++i)
    {
        unsigned char cc = src[i];
        if (isascii(cc))
        {
            if (cc == ' ')
            {
                dst += "%20";
            }
            else
                dst += cc;
        }
        else
        {
            unsigned char c = static_cast<unsigned char>(src[i]);
            dst += '%';
            dst += hex[c / 16];
            dst += hex[c % 16];
        }
    }
    return dst;
}