#ifndef TOOL
#define TOOL
#include <string>
#include <iostream>
#include <vector>
#ifdef _WIN32
#include <windows.h>
#define ACCESS _access
#define MKDIR(a) _mkdir((a))
#elif _LINUX
#define ACCESS access
#define MKDIR(a) mkdir((a),0755)
#endif

#ifndef _WIN32
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#endif

using namespace std;

#ifdef _WIN32
#define FileSeparator "\\"
#define LineSeparator "\n"
#else
#define FileSeparator "/"
#define LineSeparator "\n"
#endif

#define CN_Sample  "CN_Sample.H"

#define ID_Config "ID.Config"



#define WJH_DEBUG 1

#if WJH_DEBUG
#define LOGD(...) wjh_debug(__VA_ARGS__)
#define LOGD_STRING(...) wjh_debug_string(__VA_ARGS__)
#else
#define LOGD(...) while(0){}
#define LOGD_STRING(...) while(0){}
#endif


int readLines(string filePath,vector<string> &list);

int writeLines(string filePath,vector<string> list);

int writeToFile(string filePath,string text);

int writeAppend(string filePath,string line);

char * strreplace( const char *  original, const char *  pattern,  const char *  replacement);

bool IncludeChinese(const char *str);

string abs_path(string path);


string fixpath(string input);

char* fixpath2(const char* input);

 bool copySingleFile(const char* r_szSrcPath,const char* r_szDesPath);

int wjh_debug(const char *format, ...);

int wjh_debug_string(string format, ...);

#endif // TOOL

