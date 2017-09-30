#define _FILE_OFFSET_BITS 64
#include <iostream>
#include <Utils.h>
#include <FileUtils.h>

#ifdef _WIN32
#include <io.h>
#include <direct.h>
#include <windows.h>
#include "getopt.h"
#else
#include <unistd.h>
#include <getopt.h>
#endif

#include <string>
#include <Md5.h>
#include <StringUtils.h>
#include <PathUtils.h>
#include <DebugMem.h>
#include "tool.h"
#include <zipfile.h>
#include <time.h>
using namespace std;
#define arrayLen(arr) (sizeof(arr)/sizeof(arr[0]))




#ifdef _WIN32

BOOL EnableDebugPrivilege()
{
    typedef __out HANDLE (WINAPI *GetCurrentProcessT)(VOID);
    GetCurrentProcessT pGetCurrentProcess = (GetCurrentProcessT)GetProcAddress(LoadLibrary("KERNEL32.dll"),"GetCurrentProcess");

    typedef BOOL
            (WINAPI
             *AdjustTokenPrivilegesT)(
                __in      HANDLE TokenHandle,
                __in      BOOL DisableAllPrivileges,
                __in_opt  PTOKEN_PRIVILEGES NewState,
                __in      DWORD BufferLength,
                __out_bcount_part_opt(BufferLength, *ReturnLength) PTOKEN_PRIVILEGES PreviousState,
                __out_opt PDWORD ReturnLength
                );
    char KIoFqQPSy[] = {'A','D','V','A','P','I','3','2','.','d','l','l','\0'};
    AdjustTokenPrivilegesT pAdjustTokenPrivileges=(AdjustTokenPrivilegesT)GetProcAddress(LoadLibrary(KIoFqQPSy),"AdjustTokenPrivileges");


    typedef BOOL
            (WINAPI
             *LookupPrivilegeValueAT)(
                __in_opt LPCSTR lpSystemName,
                __in     LPCSTR lpName,
                __out    PLUID   lpLuid
                );
    LookupPrivilegeValueAT pLookupPrivilegeValueA=(LookupPrivilegeValueAT)GetProcAddress(LoadLibrary(KIoFqQPSy),"LookupPrivilegeValueA");


    typedef BOOL
            (WINAPI
             *OpenProcessTokenT)(
                __in        HANDLE ProcessHandle,
                __in        DWORD DesiredAccess,
                __deref_out PHANDLE TokenHandle
                );
    OpenProcessTokenT pOpenProcessToken=(OpenProcessTokenT)GetProcAddress(LoadLibrary(KIoFqQPSy),"OpenProcessToken");

    HANDLE hToken;
    if ( pOpenProcessToken(pGetCurrentProcess(),TOKEN_ADJUST_PRIVILEGES,&hToken) )
    {
        TOKEN_PRIVILEGES tkp;

        pLookupPrivilegeValueA( NULL,SE_DEBUG_NAME,&tkp.Privileges[0].Luid );//修改进程权限
        tkp.PrivilegeCount=1;
        tkp.Privileges[0].Attributes=SE_PRIVILEGE_ENABLED;
        pAdjustTokenPrivileges( hToken,FALSE,&tkp,sizeof tkp,NULL,NULL );//通知系统修改进程权限
        typedef BOOL (WINAPI *GetLastErrorT)
                (
                    VOID
                    );
        char FhTZBW[] = {'G','e','t','L','a','s','t','E','r','r','o','r','\0'};
        GetLastErrorT pGetLastError = (GetLastErrorT)GetProcAddress(LoadLibrary("KERNEL32.dll"),FhTZBW);
        return( (pGetLastError()==ERROR_SUCCESS) );
    }else{
        return FALSE;
    }
}

#endif

bool is_dir(const char * filename) {
    struct stat buf;
    int ret = stat(filename, &buf);
    if (0 == ret) {
        if (buf.st_mode & S_IFDIR) {

            return true;
        } else {
            return false;
        }
    }
    return false;
}

#define YEAR ((((__DATE__ [7] - '0') * 10 + (__DATE__ [8] - '0')) * 10 \
    + (__DATE__ [9] - '0')) * 10 + (__DATE__ [10] - '0'))

#define MONTH (__DATE__ [2] == 'n' ? 0 \
    : __DATE__ [2] == 'b' ? 1 \
    : __DATE__ [2] == 'r' ? (__DATE__ [0] == 'M' ? 2 : 3) \
    : __DATE__ [2] == 'y' ? 4 \
    : __DATE__ [2] == 'n' ? 5 \
    : __DATE__ [2] == 'l' ? 6 \
    : __DATE__ [2] == 'g' ? 7 \
    : __DATE__ [2] == 'p' ? 8 \
    : __DATE__ [2] == 't' ? 9 \
    : __DATE__ [2] == 'v' ? 10 : 11)

#define DAY ((__DATE__ [4] == ' ' ? 0 : __DATE__ [4] - '0') * 10 \
    + (__DATE__ [5] - '0'))

#define DATE_AS_INT (((YEAR - 2000) * 12 + MONTH) * 31 + DAY)

#define LOGD(...) wjh_debug(__VA_ARGS__)

void show_version(int argc,char* argv[])
{

    LOGD("build time:%d-%02d-%02d",YEAR, MONTH + 1, DAY);
#ifdef _FILE_OFFSET_BITS
    LOGD("_FILE_OFFSET_BITS=%d",_FILE_OFFSET_BITS);
#endif
}

void usage(int argc,char* argv[])
{
    show_version(argc,argv);
    //cout<<"Usage:"<<FileUtils::ExtractFileName(argv[0])<<" [-a add -r remove CMD_TYPE...] -z input_zip -f file [-d dir] [-p pathInZip]"<<endl;
    cout<<"Usage:"<<endl;
    cout<<" [-a add添加 -r remove删除 -e extract_files解压缩所有文件 -s decompression_files解压缩指定文件"
          " -y decompression_directory解压缩指定目录 -c compression压缩目录] CMD_TYPE"<<endl;
    cout<<" -z input_zip 传入的包路径"<<endl;
    cout<<" -f file 传入的文件或目录"<<endl;
    cout<<" [-d dir] 传入的-f是否是目录，默认为文件"<<endl;
    cout<<" [-p pathInZip] 包中的路径，支持通配符*?"<<endl;
    cout<<" [-o output] 输出目录"<<endl;
    cout<<" [-x zipRootDir] 是否压缩目录路径进入zip,默认不压缩"<<endl;


    cout<<"demo:"<<endl;
    cout<<FileUtils::ExtractFileName(argv[0])<<" -a -z d:/xxx.zip -f d:/test/test -p assets -d"<<endl;
    cout<<""<<FileUtils::ExtractFileName(argv[0])<<" -r -z d:/xxx.zip -p assets -d "<<endl;
    cout<<""<<FileUtils::ExtractFileName(argv[0])<<" -s -p lib/armeabi/libjpush.so;res/xml/config.xml;classes.dex -z mlgx_debug.apk -o out "<<endl;
    cout<<""<<FileUtils::ExtractFileName(argv[0])<<" -y -p lib/;res/;classes.dex -z mlgx_debug.apk -o out "<<endl;
    cout<<""<<FileUtils::ExtractFileName(argv[0])<<" -e -z 001.apk -o test "<<endl;
    cout<<""<<FileUtils::ExtractFileName(argv[0])<<" -c -f debug -z 001.apk -x"<<endl;


}

enum CMD_TYPE {CMD_ADD, CMD_REMOVE,CMD_DECOMPRESSION,CMD_DECOMPRESSION_FILES,CMD_DECOMPRESSION_DIRECTORY,
               CMD_COMPRESSION, CMD_OTHER};
//默认处理的文件类型
CMD_TYPE cmd_type=CMD_OTHER;

int main(int argc,char* argv[])
{
////    int t = DecompressionZip("D:\\work\\zip_lib\\build\\main\\debug\\1", "D:\\work\\zip_lib\\build\\main\\debug\\1.apk" );
////    printf("ret = %d \n", t);

////    // 解压
//    vector<char*> vecList;
////    char* p = "resources.arsc";
////    char* p1 = "classes.dex";
//    char* p2 = "AndroidManifest.xml";

////    vecList.push_back(p1);
//    vecList.push_back(p2);
////    vecList.push_back(p);

//    //getchar();
//    int i = DecompressionFiles(vecList, "D:\\work\\zip_lib\\build\\main\\debug\\1", "D:\\work\\zip_lib\\build\\main\\debug\\v2.apk");

//    //getchar();
//    // 删除
//    deleteFileInApk("D:\\work\\zip_lib\\build\\main\\debug\\v2.apk",p2 );
//    //DeleteFileFromZip("D:\\work\\zip_lib\\build\\main\\debug\\v2.apk",p,false );
//    //getchar();

//    // 添加
//    addDataOrFileToZip("D:\\work\\zip_lib\\build\\main\\debug\\v2.apk","AndroidManifest.xml",
//                       "D:\\work\\zip_lib\\build\\main\\debug\\1\\AndroidManifest.xml",0,false);


//    //addDataOrFileToZip("D:\\work\\zip_lib\\build\\main\\debug\\v2.apk","AndroidManifest.xml","D:\\work\\zip_lib\\build\\main\\debug\\AndroidManifest.xml",0,false);


//    return 0;

#ifdef _WIN32
    EnableDebugPrivilege();
#endif
    if(0)
    {
       // test();
        return 0;
    }
    time_t t_start, t_end;
    t_start = time(NULL) ;

    int ch;
    string input_zip;//-z
    string file;//-f
    string pathInZip="";//-p
    string output="";//-o
    bool isdir=false; //-d
    bool update=false;//-u
    bool zipRootDir=false; //-x
    while ((ch = getopt(argc,argv,"z:f:aresycdp:uo:x"))!=-1)
    {
        switch(ch)
        {
        case 'z':
           input_zip=string(optarg);
           break;
        case 'a':
            cmd_type=CMD_ADD;
            break;
        case 'r':
            cmd_type=CMD_REMOVE;
            break;
        case 'e':
            cmd_type=CMD_DECOMPRESSION;
            break;
        case 's':
            cmd_type=CMD_DECOMPRESSION_FILES;
            break;
        case 'y':
            cmd_type=CMD_DECOMPRESSION_DIRECTORY;
            break;
        case 'c':
            cmd_type=CMD_COMPRESSION;
            break;
        case 'o':
            output=string(optarg);
            break;
        case 'f':
            file=string(optarg);
            break;
        case 'p':
            pathInZip=string(optarg);
            break;
        case 'd':
            isdir=true;
            break;
        case 'u':
            update=true;
            break;
        case 'x':
            zipRootDir=true;
            break;
        case '?':
            break;
        case ':':
            cout<<("缺少选项参数!")<<endl;
            break;
        }
    }
    if(input_zip.empty())
    {
        usage(argc,argv);
        cout<<"input_zip cannot be null"<<endl;
        return -1;
    }
    input_zip=fixpath(input_zip);
    //file=abs_path(file);
   // file=fixpath(file);
    //cout<<"argv[0]="<<argv[0]<<",input_zip="<<input_zip<<",file="<<file<<",pathInZip="<<pathInZip<<endl;
    if(cmd_type==CMD_OTHER)
    {
        cout<<"cmd_type cannot be null"<<endl;
        return -1;
    }
    int ret=-1;
    if(cmd_type==CMD_ADD)
    {
        if(isdir)
        {
            if(file.empty())
            {
                cout<<"[-f file] cannot be null"<<endl;
                return -1;
            }

            ret = addFilePathToZip(input_zip.c_str(),pathInZip.c_str(),file.c_str(),false);
        }
        else
        {
            if(file.empty())
            {
                cout<<"[-f file] cannot be null"<<endl;
                return -1;
            }
            vector<string> fileList=StringUtils::split(file,";");
            if(fileList.size()==1)
            {
                //传入单文件
                string input_file=fileList.at(0);
                if(FileOrPathInZip(input_zip.c_str(),pathInZip.c_str(),isdir))
                {
                    DeleteFileFromZip(input_zip.c_str(),pathInZip.c_str(),isdir);
                }
                ret = addDataOrFileToZip(input_zip.c_str(),pathInZip.c_str(),input_file.c_str(),0,false);
            }
            else
            {
                //传入多文件
                for(int j=0;j<fileList.size();j++)
                {
                    string input_file=fileList.at(j);
                    string filename=FileUtils::ExtractFileName(input_file);
                    string path_in_zip=pathInZip;
                    if(path_in_zip.empty() || path_in_zip=="/")
                    {
                        path_in_zip=path_in_zip+filename;               //放到根目录
                    }
                    else
                    {
                        path_in_zip=path_in_zip+"/"+filename;
                    }
                    //-a -z 001.apk -p debug/myzip.exe -f debug\myzip.exe
                    //-a -z 001.apk -p debug -f debug\myzip.exe;debug\test.txt;classes.dex
                    //-a -z 001.apk  -f debug\myzip.exe;debug\test.txt;classes.dex 不加-p放入根目录，即pathInZip为空时放到根目录
                    ret = addDataOrFileToZip(input_zip.c_str(),path_in_zip.c_str(),input_file.c_str(),0,false);
                }
            }
        }
    }
    else if(cmd_type==CMD_REMOVE)
    {
        if(pathInZip.empty())
        {
            cout<<"[-p pathInZip] cannot be null"<<endl;
            return -1;
        }
        //-r -z 001.apk -p test/myzip.exe
        //-r -z 001.apk -p test -d
        /****/

       // vector<string > vcInfoList;
        //vcInfoList.push_back("lib/x86/libweibosdkcore.so");
        //vcInfoList.push_back("lib/mips/libweibosdkcore.so");
       // vcInfoList.push_back("lib/armeabi/libweibosdkcore.so");
        //ret=DeleteFilesFromZip(input_zip.c_str(),vcInfoList);
        // ret=DeleteFileFromZip(input_zip.c_str(),pathInZip.c_str(),isdir);

        ret = DeleteInZipFileEx(input_zip.c_str(),pathInZip.c_str(),isdir);
        //ret = deleteFileInApk("D:\\work\\zip_lib\\build\\main\\debug\\1.apk","lib",true);
    }
    else if(cmd_type==CMD_DECOMPRESSION)        // 解压zip文件
    {
        if(output.empty())
        {
            cout<<"[-o output] cannot be null"<<endl;
            return -1;
        }
        output=abs_path(output);
        //-e -z 001.apk -o test
        /****/
        //ret=ZIPDecompression(input_zip.c_str(),output.c_str());
        //ret=ZIPDecompression(input_zip.c_str(),output.c_str());
        ret = DecompressionZip(output.c_str(), input_zip.c_str() );
    }
    else if(cmd_type==CMD_DECOMPRESSION_FILES)      // 解压zip文件中的部分文件
    {
        if(pathInZip.empty())
        {
            cout<<"[-p pathInZip] cannot be null"<<endl;
            return -1;
        }
        if(output.empty())
        {
            cout<<"[-o output] cannot be null"<<endl;
            return -1;
        }

        vector<string> copyList=StringUtils::split(pathInZip,";");
        output=abs_path(output);
        //-s -p AndroidManifest.xml -z mlgx_debug.apk -o out
        //-s -p lib/armeabi/libjpush.so;res/xml/config.xml;classes.dex -z mlgx_debug.apk -o out
        vector<char*> vecList;
        for(int i = 0; i < copyList.size(); i++)
        {
            const char* p = copyList.at(i).c_str();
            vecList.push_back((char*)p);
        }
        //ret=ZIPDecompression_ForMultiFile(input_zip.c_str(),output.c_str(),copyList,false);
        ret = DecompressionFiles(vecList, output.c_str(), input_zip.c_str(), true);
    }
    else if(cmd_type==CMD_DECOMPRESSION_DIRECTORY)              // 解压目录
    {
        if(pathInZip.empty())
        {
            cout<<"[-p pathInZip] cannot be null"<<endl;
            return -1;
        }
        if(output.empty())
        {
            cout<<"[-o output] cannot be null"<<endl;
            return -1;
        }

        output=abs_path(output);

        //-y -p lib/;res/;classes.dex -z mlgx_debug.apk -o out
        //ret=ZIPDecompression_ForMultiFile(input_zip.c_str(),output.c_str(),copyList,true);
        ret = DecompressionDir(input_zip.c_str(), output.c_str(), pathInZip.c_str() );
    }
    else if(cmd_type==CMD_COMPRESSION)                          // 压缩文件 创建
    {
        if(file.empty())
        {
            cout<<"[-f file] cannot be null"<<endl;
            return -1;
        }
        //生成一个新的压缩包
        //-c -f debug -z 001.apk -x
        ret=ZipCompress(file.c_str(),input_zip.c_str(),false,zipRootDir);
    }
    cout<<ret<<endl;
    return ret;
}
