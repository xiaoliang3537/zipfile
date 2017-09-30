#include "tool.h"
#include <sstream>
#include <fstream>
#include <iostream>
#include <string>
#include <StringUtils.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>

int readLines(string filePath,vector<string> &list)
{
    std::ifstream fin(filePath.c_str(), std::ios::in);
    string line;
    while(std::getline(fin,line))//bug啊 某些行字符串超过2048了
    {
        list.push_back(line);
    }
    fin.clear();
    fin.close();
    return 0;
}

//http://www.cplusplus.com/reference/vector/vector/?kw=vector
int writeLines(string filePath,vector<string> list)
{
    std::ofstream fout(filePath.c_str());
    for(int i=0;i<list.size();i++)
    {
        fout<<list.at(i)<<endl;
    }
    fout.clear();
    fout.close();
    return 0;
}

int writeToFile(string filePath,string text)
{
    std::ofstream fout(filePath.c_str());
    fout<<text;
    fout.clear();
    fout.close();
    return 0;
}
int writeAppend(string filePath,string line)
{
    std::ofstream fout(filePath.c_str(),ios::app);
    fout<<line;
    fout.clear();
    fout.close();
    return 0;
}

char * strreplace( const char *  original, const char *  pattern,  const char *  replacement)
{
  size_t const replen = strlen(replacement);
  size_t const patlen = strlen(pattern);
  size_t const orilen = strlen(original);

  size_t patcnt = 0;
  const char * oriptr;
  const char * patloc;

  // find how many times the pattern occurs in the original string
  for (oriptr = original; (patloc = strstr(oriptr, pattern)); oriptr = patloc + patlen)
  {
    patcnt++;
  }

  {
    // allocate memory for the new string
    size_t const retlen = orilen + patcnt * (replen - patlen);
    char * const returned = (char *) malloc( sizeof(char) * (retlen + 1) );

    if (returned != NULL)
    {
      // copy the original string,
      // replacing all the instances of the pattern
      char * retptr = returned;
      for (oriptr = original; (patloc = strstr(oriptr, pattern)); oriptr = patloc + patlen)
      {
        size_t const skplen = patloc - oriptr;
        // copy the section until the occurence of the pattern
        strncpy(retptr, oriptr, skplen);
        retptr += skplen;
        // copy the replacement
        strncpy(retptr, replacement, replen);
        retptr += replen;
      }
      // copy the rest of the string.
      strcpy(retptr, oriptr);
    }
    return returned;
  }
}

string fixpath(string input)
{
    //C:\Users\ADMINI~1\AppData\Local\Temp\tmp/temp/smali_classes2
#ifdef _WIN32
    return StringUtils::sReplace(input,"/",FileSeparator);
#else
    return StringUtils::sReplace(input,"\\",FileSeparator);
#endif
}
char* fixpath2(const char* input)
{
    //C:\Users\ADMINI~1\AppData\Local\Temp\tmp/temp/smali_classes2
#ifdef _WIN32
    return strreplace(input,"/",FileSeparator);
#else
    return strreplace(input,"\\",FileSeparator);
#endif
}

/*
*  单一文件拷贝
*/
 bool copySingleFile(const char* r_szSrcPath,const char* r_szDesPath)
{
#ifdef _WIN32
    bool l_bRet = CopyFile(r_szSrcPath,r_szDesPath,TRUE)==FALSE?false:true;
    return l_bRet;
#else
    if(r_szSrcPath == NULL || access(r_szSrcPath ,0 ) == -1)
    {
        return false;
    }
    char szBuf[1024] = {0};
    sprintf(szBuf, "cp  \"%s\"*  \"%s\"" , r_szSrcPath,r_szDesPath);
    system(szBuf);
    return true;
#endif
}

 bool IncludeChinese(const char *str)//返回0：无中文，返回1：有中文
 {
     char c;
     while(1)
     {
         c=*str++;
         if (c==0) break;  //如果到字符串尾则说明该字符串没有中文字符
         if (c&0x80)        //如果字符高位为1且下一字符高位也是1则有中文字符
             if (*str & 0x80) return true;
     }
     return false;
 }
 string abs_path(string path)
 {
     if(path.empty())
     {
         return path;
     }
 #ifdef _WIN32
      #define max_path 4096
      char resolved_path[max_path]={0};
     _fullpath(resolved_path,path.c_str(),max_path);
 #else
     //linux release有个坑，需要大点的空间
     #define max_path 40960
     char resolved_path[max_path]={0};
     realpath(path.c_str(),resolved_path);
 #endif
     return string(resolved_path);
 }

 int wjh_debug_string(string format, ...)
{
     va_list ap;
     va_start(ap, format);
     int ret=wjh_debug(format.c_str(),ap);
     va_end(ap);
     return ret;
}
 int wjh_debug(const char *format, ...)
 {

#if __cplusplus
     char line[512]={0};
     va_list ap;
     va_start(ap, format);
     vsprintf(line,format,ap);
     cout<<line<<endl;
     va_end(ap);
#else
     va_list ap;
     va_start(ap, format);
     vprintf(format,ap);
     printf("\n");
     va_end(ap);
#endif
     return 0;
 }






