#ifndef STRINGUTILS_H_
#define STRINGUTILS_H_
#include <iostream>
#include <sys/stat.h>
#include <cstring>
#include <vector>
#include <sstream>
#include <algorithm>
#include <string>
#include <wchar.h>
#include <cctype>
#include <functional>
#include <cstdlib>
#include <numeric>

using namespace std;
  
namespace StringUtils {
	 
//static bool wstricmp(std::string lhs,std::string rhs)
//{
//   return _stricmp(lhs.c_str(),rhs.c_str());
//}

//static std::string string_format(const std::string& fmt, ...)
//{
//  va_list argptr;
//  va_start(argptr, fmt);
//  int bufsize = _vsnprintf(NULL, 0, fmt.c_str(), argptr) + 2;
//  char* buf = new char[bufsize];
//  _vsnprintf(buf, bufsize, fmt.c_str(), argptr);
//  std::string s(buf);
//  delete[] buf;
//  va_end(argptr);
//  return s;
//}

/**
 * @brief split a string by delim
 *
 * @param str string to be splited
 * @param c delimiter, const char*, just like " .,/", white space, dot, comma, splash
 *
 * @return a string vector saved all the splited world
 */
inline std::vector<std::string> split(std::string& str, const char* cc) {
	char *cstr, *p;
	std::vector<std::string> res;
	cstr = new char[str.size() + 1];
	strcpy(cstr, str.c_str());
	p = strtok(cstr, cc);
	while (p != NULL) {
		res.push_back(p);
		p = strtok(NULL, cc);
	}
    delete cstr;
	return res;
}
//inline std::vector<std::string> splitw(std::string& str, const char* cc) {
//	char *cstr, *p;
//	std::vector<std::string> res;
//	cstr = new char[str.size() + 1];
//	strcpy(cstr, str.c_str());
//#ifdef WIN32
//	p = strtok(cstr,cc);
//#else
//	char *saveptr;
//	p = strtok(cstr, cc, &saveptr);
//#endif
//	while (p != NULL) {
//		res.push_back(p);
//#ifdef WIN32
//		p = strtok(NULL,cc);
//#else
//		char *saveptr;
//		p = strtok(NULL, cc, &saveptr);
//#endif
//	}
//	return res;
//}
/**
 * @brief convert a integer into string through stringstream
 *
 * @param n a integer
 *
 * @return the string form of n
 */
inline std::string int2str(int n) {
	std::stringstream ss;
	std::string s;
	ss << n;
	ss >> s;
	return s;
}

inline std::string float2str(double f) {
	std::stringstream ss;
	std::string s;
	ss << f;
	ss >> s;
	return s;
}

inline std::string char2str(char f) {
    std::stringstream ss;
    std::string s;
    ss << f;
    ss >> s;
    return s;
}

/**
 * @brief convert something to string form through stringstream
 *
 * @tparam Type Type can be int,float,double
 * @param a
 *
 * @return the string form of param a
 */
template<class Type>
std::string tostring(Type a) {
	std::stringstream ss;
	std::string s;
	ss << a;
	ss >> s;
}

/**
 * @brief convert string to int by atoi
 *
 * @param s string
 *
 * @return the integer result
 */
inline int str2int(std::string& s) {
	return atoi(s.c_str());
}

inline double str2float(std::string& s) {
	return atof(s.c_str());
}

/**
 * @brief do string convert through stringstream from FromType to ToType
 *
 * @tparam ToType target type
 * @tparam FromType source type
 * @param t to be converted param
 *
 * @return the target form of param t
 */
template<class ToType, class FromType>
ToType strconvert(FromType t) {
	std::stringstream ss;
	ToType a;
	ss << t;
	ss >> a;
	return a;
}

/**
 * @brief convert string to upper case throught transform method, also can use transform method directly
 *
 * @param s
 *
 * @return the upper case result saved still in s
 */
inline std::string strtoupper(std::string s) {
	transform(s.begin(), s.end(), s.begin(), ::toupper);
	return s;
}

/**
 * @brief convert string to upper case through toupper, which transform a char into upper case
 *
 * @param s
 *
 * @return the upper case result string
 */
inline std::string strtoupper2(std::string s) {
	std::string t = s;
	int i = -1;
	while (t[i++]) {
		t[i] = toupper(t[i]);
	}
	return t;
}

/**
 * @brief convert string to lower case throught transform method, also can use transform method directly
 *
 * @param s
 *
 * @return the lower case result saved still in s
 */
inline std::string strtolower(std::string s) {
	transform(s.begin(), s.end(), s.begin(), ::tolower);
	return s;
}

/**
 * @brief convert string to lower case through tolower, which transform a char into lower case
 *
 * @param s
 *
 * @return the lower case result string
 */
inline std::string strtolower2(std::string s) {
	std::string t = s;
	int i = -1;
	while (t[i++]) {
		t[i] = tolower(t[i]);
	}
	return t;
}
#if 1
//inline static std::string char2wstring(const char *str) {
//	int nLen = strlen(str) + 1;
//	int nwLen = MultiByteToWideChar(CP_ACP, 0, str, nLen, NULL, 0);
//	TCHAR lpszFile[256];
//	MultiByteToWideChar(CP_ACP, 0, str, nLen, lpszFile, nwLen);
//	std::string ws;
//	ws.append(lpszFile);
//	return ws;
//}
//inline char* wchar2char(const char* tchStr) {
//	int iLen = 2 * wcslen(tchStr); //CString,TCHAR汉字算一个字符，因此不用普通计算长度
//	char* chRtn = new char[iLen + 1];
//	wcstombs(chRtn, tchStr, iLen + 1); //转换成功返回为非负值
//	return chRtn;
//}
//inline std::string wchar2string(std::string str) {
//	std::string return_value;
//	//获取缓冲区的大小，并申请空间，缓冲区大小是按字节计算的
//	int len = WideCharToMultiByte(CP_ACP, 0, str.c_str(), str.size(), NULL, 0,
//			NULL, NULL);
//	char *buffer = new char[len + 1];
//	WideCharToMultiByte(CP_ACP, 0, str.c_str(), str.size(), buffer, len, NULL,
//			NULL);
//	buffer[len] = '\0';
//	//删除缓冲区并返回值
//	return_value.append(buffer);
//	delete[] buffer;
//	return return_value;
//}
#else

/* 
    string 转换为 string  
*/  
inline std::string char2wstring(const char *pc)  
{  
    std::string val = "";  
  
    if(NULL == pc)  
    {  
        return val;  
    }  
    //size_t size_of_ch = strlen(pc)*sizeof(char);  
    //size_t size_of_wc = get_wchar_size(pc);  
    size_t size_of_wc;  
    size_t destlen = mbstowcs(0,pc,0);  
    if (destlen ==(size_t)(-1))  
    {  
        return val;  
    }  
    size_of_wc = destlen+1;  
    char * pw  = new char[size_of_wc];  
    mbstowcs(pw,pc,size_of_wc);  
    val = pw;  
    delete pw;  
    return val;  
}  
/* 
    string 转换为 string 
*/  
inline std::string wchar2string(const char * pw)  
{  
    std::string val = "";  
    if(!pw)  
    {  
        return val;  
    }  
    size_t size= wcslen(pw)*sizeof(char);  
    char *pc = NULL;  
    if(!(pc = (char*)malloc(size)))  
    {  
        return val;  
    }  
    size_t destlen = wcstombs(pc,pw,size);  
    /*转换不为空时，返回值为-1。如果为空，返回值0*/  
    if (destlen ==(size_t)(0))  
    {  
        return val;  
    }  
    val = pc;  
    delete pc;  
    return val;  
}  

#endif
//////////////////////////////////////////////////////////////////////////
/*
 *   功能： 字符串替换
 */
//////////////////////////////////////////////////////////////////////////
inline std::string sReplace(const std::string& input,                  // 原始串
		const std::string& find,                // 替换源串
		const std::string& replaceWith) // 替换目的串
		{
	std::string strOut(input);
	int curPos = 0;

	int pos;
	while ((pos = strOut.find(find, curPos)) != -1) {
		strOut.replace(pos, find.size(), replaceWith);      // 一次替换
		curPos = pos + replaceWith.size();                        // 防止循环替换!!
	}

	return strOut;
}
//////////////////////////////////////////////////////////////////////////
/*
 *   功能： 字符串替换
 */
//////////////////////////////////////////////////////////////////////////
inline std::string wsReplace(const std::string& input,                  // 原始串
		const std::string& find,                // 替换源串
		const std::string& replaceWith) // 替换目的串
		{
	std::string strOut(input);
	int curPos = 0;

	int pos;
	while ((pos = strOut.find(find, curPos)) != -1) {
		strOut.replace(pos, find.size(), replaceWith);      // 一次替换
		curPos = pos + replaceWith.size();                        // 防止循环替换!!
	}

	return strOut;
}
/*
inline bool startsWith(const char* str, const char* token)
{
    if (str == NULL || token == NULL)return 0;
    return (strncmp(str, token, strlen(token)) == 0);

}

inline bool endWith(const char * str, const char * end)
{
    BOOL result = FALSE;
    if (str != NULL && end != NULL)
    {
        int l1 = strlen(str);
        int l2 = strlen(end);
        if (l1 >= l2) {
            if (strcmp(str + l1 - l2, end) == 0)
            {
                result = TRUE;
            }
        }
    }
    return result;
}*/

inline bool startWith(const std::string& str,const std::string& strStart)
{
    if(str.empty() || strStart.empty())
    {
        return false;
    }

    return str.compare(0,strStart.size(),strStart)==0?true:false;
}

inline bool endWith(const std::string& str,const std::string& strEnd)
{
    if(str.empty() || strEnd.empty())
    {
        return false;
    }
    return str.compare(str.size()-strEnd.size(),strEnd.size(),strEnd)==0?true:false;
}

inline string& ltrim(string &str) {
    string::iterator p = std::find_if(str.begin(), str.end(), not1(ptr_fun<int, int>(isspace)));
    str.erase(str.begin(), p);
    return str;
}

inline string& rtrim(string &str) {
    string::reverse_iterator p = std::find_if(str.rbegin(), str.rend(), not1(ptr_fun<int , int>(isspace)));
    str.erase(p.base(), str.end());
    return str;
}

inline string& trim(string &str) {
    ltrim(rtrim(str));
    return str;
}


};

//#define C2W(x) (StringUtils::char2wstring(x).c_str())
//#define W2C(x) (StringUtils::wchar2string(x).c_str())


//#define C2W(x)   StringUtils::char2wstring(x) 
//#define W2C(x)   StringUtils::wchar2string(x)


#endif
