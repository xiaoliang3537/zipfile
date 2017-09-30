#ifndef FILEUTILS_H_
#define FILEUTILS_H_

#define _FILE_OFFSET_BITS 64
#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <string>	
#include <vector>	
#include <wchar.h>
#ifdef _WIN32
#include <windows.h>
#include <direct.h>
#include <io.h>
#else
//#include <stdarg.h>
//#include <sys/stat.h>
#include<errno.h>
#endif

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

#include "StringUtils.h"
#include "PathUtils.h"
//#include "../libutils/include/Utils.h"

#define FILE_DEBUG 0
#define MAX_PATH_LEN 256

namespace FileUtils {



inline bool exists(const char *filename)
{
    return (access(filename,0) == 0);
}


	/*
	* 删除文件夹
	*/
//    inline bool DeleteDir(std::string const & sRootDir)
//    {
//#ifdef _WIN32
//        std::string sPath;
//        sPath.append("rd /s /q ").append(sRootDir);
//        system(sPath.c_str());
//        return true;
//#else
//        if(access(sRootDir.c_str(), 0) == -1)
//        {
//            return false;
//        }
//        char szbuf[500] = {0};

//        sprintf(szbuf,"chmod 777  \"%s\" -R", sRootDir.c_str());
//        system(szbuf);
//        sprintf(szbuf,"rm -rf  \"%s\"", sRootDir.c_str());
//        system(szbuf);
//        return true;
//#endif

//    }
#ifdef _WIN32
inline std::wstring  getShortPath(const wchar_t *pLongPath)
{
    std::wstring sPath;
    sPath = pLongPath;
    wchar_t pathName [2048] = {0};
    swprintf(pathName,L"\\\\?\\%s",pLongPath);

    const int MaxPathLength = 2048;
    wchar_t shortPath[MaxPathLength];

    if (wcslen(pathName) >= MAX_PATH)
    {
//        wchar_t prePath[] = L"\\\\?\\";
//        if (wcslen(pathName) >= MaxPathLength - wcslen(pathName))
//            return false;

//        swprintf(shortPath, L"%s%s", prePath, pathName);

//        for (int iPathIndex = 0; iPathIndex < wcslen(shortPath); iPathIndex++)
//            if (shortPath[iPathIndex] == '/')
//                shortPath[iPathIndex] = '\\';

        //int dwlen = GetShortPathNameW(shortPath, shortPath, MaxPathLength);
        int dwlen = GetShortPathNameW(pathName, shortPath, MaxPathLength);
        if(dwlen==0)
        {
            std::wcout<<L"GetShortPathNameW failed:"<<pathName;
            exit(0);
        }
        sPath = shortPath;
        //return sPath;
    }
    return sPath;


}
#endif
    inline bool DeleteDir(std::string const & sRootDir)
	{
#ifdef _WIN32
        wchar_t pszW[1024] = {0};
        // Covert to Unicode.
        MultiByteToWideChar(CP_ACP, 0, sRootDir.c_str(), sRootDir.length(),pszW, 1024);

        WIN32_FIND_DATAW wfd;
		::ZeroMemory(&wfd, sizeof(wfd));        
        std::wstring sDir(pszW);
        sDir +=L"\\*";
        HANDLE hFind = ::FindFirstFileW(sDir.c_str(), &wfd);
		if (hFind == INVALID_HANDLE_VALUE)
		{
			return true;
		}
		do
		{
            std::wstring sDirName;
            sDirName = pszW;
            sDirName +=  L"\\";
			sDirName +=  wfd.cFileName;
			if((wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
			{                        
                if(wcscmp(L"..", wfd.cFileName) && wcscmp(L".", wfd.cFileName))
				{
                    char pName[1024] = {0};
                    // Covert to Unicode.
                    WideCharToMultiByte(CP_ACP, 0, sDirName.c_str(), sDirName.length(),pName, 1024,0,false);
                    if(!DeleteDir(pName))
						return false;
				}
			}
			else
            {
                sDirName = getShortPath(sDirName.c_str());
                //std::wcout<<sDirName<<std::endl;
                if(::DeleteFileW(sDirName.c_str()))
                {
#if FILE_DEBUG
                    std::wcout<<"delete file: "<<sDirName<<std::endl;
#endif

                }
                else
                {
                    std::wcout<<L"fail to delete file: "<<sDirName<<L"\t error code:"<<::GetLastError()<<std::endl;
                }
            }
		}
        while(::FindNextFileW(hFind, &wfd) != 0);
        ::FindClose(hFind);

        std::wstring sDirNamePath = getShortPath(pszW);
        //std::wcout<<sDirNamePath<<std::endl;
        if(::RemoveDirectoryW(sDirNamePath.c_str()))
		{
#if FILE_DEBUG
			std::wcout<<"delete directory: "<<sRootDir<<std::endl;
#endif
			return true;
		}
		else
		{
            std::wstring sPath;
            sPath.append(L"rd /s /q ").append(pszW);
            _wsystem(sPath.c_str());
           // std::cout<<"fail to delete directory: " << sRootDir << "\t error code:"<<::GetLastError()<<std::endl;
            //return false;
		}
		return true;
#else

        if(access(sRootDir.c_str(), 0) == -1)
        {
            return false;
        }
        char szbuf[500] = {0};

        sprintf(szbuf,"chmod 777  \"%s\" -R", sRootDir.c_str());
        system(szbuf);
        sprintf(szbuf,"rm -rf  \"%s\"", sRootDir.c_str());
        system(szbuf);
        return true;
#endif
	}


    /*
    *  文件拷贝
    */
    inline bool copyFile( const char* r_szSrcPath,const char* r_szDesPath,bool bForce)
    {
#ifdef _WIN32
        if(!::CopyFile(r_szSrcPath,r_szDesPath,!bForce))
        {
            std::cout<<GetLastError()<<std::endl;
            return false;
        }
        return true;
#else
        if(r_szSrcPath == NULL || access(r_szSrcPath ,0 ) == -1)
        {
            return false;
        }
        char szBuf[1024] = {0};

        sprintf(szBuf, "cp -f  \"%s\"  \"%s\"" , r_szSrcPath,r_szDesPath);
       // std::cout<<szBuf<<std::endl;
        system(szBuf);


        return true;
#endif

    }

	/*
    *  文件拷贝目录
	*/
    inline bool copyFile(const char* r_szSrcPath,const char* r_szDesPath)
	{
#ifdef _WIN32
		WIN32_FIND_DATA FindFileData;
		HANDLE hFind;
		char l_szTmp[MAX_PATH_LEN+1] = {0};
		//memcpy(l_szTmp,r_szSrcPath,MAX_PATH_LEN);
		memcpy(l_szTmp,r_szSrcPath,strlen(r_szSrcPath)*sizeof(TCHAR));

		char l_szSrcPath[MAX_PATH_LEN+1] = {0};
		char l_szDesPath[MAX_PATH_LEN+1] = {0};
		//memcpy(l_szSrcPath,r_szSrcPath,MAX_PATH_LEN);
		memcpy(l_szSrcPath,r_szSrcPath,strlen(r_szSrcPath)*sizeof(TCHAR));
		//memcpy(l_szDesPath,r_szDesPath,MAX_PATH_LEN);
		memcpy(l_szDesPath,r_szDesPath,strlen(r_szDesPath)*sizeof(TCHAR));

		char l_szNewSrcPath[MAX_PATH_LEN+1] = {0};
		char l_szNewDesPath[MAX_PATH_LEN+1] = {0};

		strcat(l_szTmp,"*");

		hFind = FindFirstFile(l_szTmp, &FindFileData);
		if(hFind == NULL) return false;

		do
		{

			if(FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{
				if(strcmp(FindFileData.cFileName,"."))
				{
					if(strcmp(FindFileData.cFileName,".."))
					{
						//		wprintf ("The Directory found is %s", FindFileData.cFileName);
                        sprintf(l_szNewDesPath,"%s%s\\",l_szDesPath,FindFileData.cFileName);

                        sprintf(l_szNewSrcPath,"%s%s\\",l_szSrcPath,FindFileData.cFileName);
						CreateDirectory(l_szNewDesPath,NULL);
						copyFile(l_szNewSrcPath,l_szNewDesPath);
					}
				}
			}
			else
			{
				//wprintf ("The File found is %s", FindFileData.cFileName);
				char l_szSrcFile[MAX_PATH_LEN+1] = {0};
				char l_szDesFile[MAX_PATH_LEN+1] = {0};
                sprintf(l_szDesFile,"%s%s",l_szDesPath,FindFileData.cFileName);
                sprintf(l_szSrcFile,"%s%s",l_szSrcPath,FindFileData.cFileName);
				bool l_bRet = CopyFile(l_szSrcFile,l_szDesFile,TRUE)==FALSE?false:true;

			}


		}
		while(FindNextFile(hFind, &FindFileData));
		FindClose(hFind);
		return true;
#else
        if(r_szSrcPath == NULL || access(r_szSrcPath ,0 ) == -1)
        {
            return false;
        }
 //	   if(access(r_szDesPath , 0)!= -1)
 //	   {
 //		   DeleteDir(r_szDesPath);
 //	   }
        char szBuf[1024] = {0};

        sprintf(szBuf, "cp -rf  \"%s\"*  \"%s\"" , r_szSrcPath,r_szDesPath);
        system(szBuf);

        return true;
#endif
	}


	/*
	判断文件是否存在
	*/
//    inline bool FileExistsW( const char * szFileName )
//	{
//		DWORD dwRet	= GetFileAttributes(szFileName);
//		return (dwRet != 0xFFFFFFFF) && ((FILE_ATTRIBUTE_DIRECTORY & dwRet) == 0);
//	}
	/*
	判断目录是否存在
	*/
	static inline bool DirExistsW(const char *szDir )
	{
#ifdef _WIN32
		if(_access(szDir,0) == -1)
		{
			return false;
		}
		else
		{
			return true;
		}
#else
		if(access(szDir,0) == -1)
		{
			return false;
		}
		else
			return true;
#endif
		//DWORD dwRet = GetFileAttributes(szDir);
		//return ((dwRet != 0xFFFFFFFF) && ((FILE_ATTRIBUTE_DIRECTORY & dwRet) != 0));
	}
	static inline bool DirExists(std::string wsDir )
	{
		return DirExistsW(wsDir.c_str());
	}

//	/*
//	判断目录是否存在
//	*/
//	inline bool MoveFileW( const char * lpSrcFile ,const char * lpDstFile)
//	{
//		return ::MoveFile(lpSrcFile,lpDstFile)?true:false;
//	}

	/*
	确保目录存在
	*/
	inline void MakeSureDirExsitsW(const char * lpSrcDir )
	{
#ifdef _WIN32
		if (DirExistsW(lpSrcDir))
			return;

		// 递归处理
		MakeSureDirExsitsW(FileUtils::ExtractFileDir(lpSrcDir).c_str());
		::CreateDirectory(lpSrcDir, NULL);
		//	DWORD dwAttrs  = GetFileAttributes(lpSrcDir);
		//dwAttrs  = FILE_ALL_ACCESS ;
		//SetFileAttributes(lpSrcDir,dwAttrs);
#else
        if(lpSrcDir == NULL|| strlen(lpSrcDir) <= 0)
        {
            return ;
        }
        char szSrcDir[500] = {0};
        strcpy(szSrcDir,lpSrcDir);
        char szbuf[500] = {0};
        sprintf(szbuf,"mkdir  -p  \"%s\"",szSrcDir);
        system(szbuf);
        return ;
#endif
	}
	inline void MakeSureDirExsits(std::string wsSrcDir )
	{
		return MakeSureDirExsitsW(wsSrcDir.c_str());
	}

	/*
	递归删除一个文件夹内所有文件,但不删除文件夹
	路径最后可带 "\" 可不带"\"
	*/
//	inline void DelAllFileW(const char * lpSrcFolder)
//	{
//		if (!DirExistsW(lpSrcFolder))
//			return;

//		std::string strDir = lpSrcFolder;
//		strDir += "\\*";

//		WIN32_FIND_DATA fd;
//		HANDLE hSearch = ::FindFirstFile(strDir.c_str(), &fd);
//		if(hSearch == INVALID_HANDLE_VALUE)
//			return;

//		do
//		{
//			if ((strcmp(fd.cFileName,  ".") == 0) ||
//				(strcmp(fd.cFileName,  "..") == 0))
//				continue;

//			std::string strFileName = lpSrcFolder;
//			strFileName += "\\";
//			strFileName += fd.cFileName;

//			if ((FILE_ATTRIBUTE_DIRECTORY & fd.dwFileAttributes) != 0)
//				DelAllFileW(lpSrcFolder);
//			else
//				::DeleteFile(strFileName.c_str());

//		} while(::FindNextFile(hSearch, &fd));

//		::FindClose(hSearch);


//	}

    inline bool DelFileW(const std::string &strFileName)
    {
#ifdef _WIN32
        return ::DeleteFile(strFileName.c_str())==FALSE?false:true;
#else
        return remove(strFileName.c_str()) == 0?true:false;
#endif
	}
	/*
	递归删除一个文件夹，包括文件和文件夹
	路径最后可带 "\" 可不带"\"
	*/
//	inline bool DelFolder(const std::string &strFolder)
//	{
//		if (!DirExistsW(strFolder.c_str()))
//			return false;

//		std::string strDir = strFolder;
//		strDir += "\\*";

//		WIN32_FIND_DATA fd;
//		HANDLE hSearch = ::FindFirstFile(strDir.c_str(), &fd);
//		if(hSearch == INVALID_HANDLE_VALUE)
//			return false;

//		do
//		{
//			if ((strcmp(fd.cFileName,  ".") == 0) ||
//				(strcmp(fd.cFileName,  "..") == 0))
//			{
//				::RemoveDirectory(strFolder.c_str());
//				continue;
//			}

//			std::string strFileName = strFolder;
//			strFileName += "\\";
//			strFileName += fd.cFileName;

//			if ((FILE_ATTRIBUTE_DIRECTORY & fd.dwFileAttributes) != 0)
//				DelFolder(strFileName);
//			else
//				::DeleteFile(strFileName.c_str());

//		} while(::FindNextFile(hSearch, &fd));

//		::FindClose(hSearch);

//		if(::RemoveDirectory(strFolder.c_str()))
//			return true;
//		else
//			return false;
//	}
#if 0
	/*
	通过一个地址取模块句柄
	*/
	inline HMODULE ModuleHandleByAddr(const void* ptrAddr)
	{
		MEMORY_BASIC_INFORMATION info;
		::VirtualQuery(ptrAddr, &info, sizeof(info));
		return (HMODULE)info.AllocationBase;
	}

	/*
	当前模块句柄
	*/
	inline HMODULE ThisModuleHandle()
	{
		static HMODULE s_Instance = ModuleHandleByAddr((void*)&ThisModuleHandle);
		return s_Instance;
	}


	/*
	取得程序当前的路径 包括反斜杠"\"
	*/
	inline std::string GetAppPath()
	{
		WCHAR szFile[MAX_PATH] = {0};
		if (0 == ::GetModuleFileNameW(NULL, szFile, MAX_PATH))
			return (std::string)"";

		return FileUtils::ExtractFilePath((std::string)szFile);
	}

	/*
	取当前模块路径
	*/
	inline std::string GetModulePath()
	{
		WCHAR szPath[MAX_PATH] = {0};
		::GetModuleFileNameW(ThisModuleHandle(), szPath, MAX_PATH);
		std::string strPath = szPath;
		strPath = FileUtils::ExtractFilePath(strPath);
		return strPath;
	}
#endif
//	inline void getFolderEnties(const char * lpPath, std::vector<std::string> &vecFiles ,std::string wsFileExt)
//	{
//		char szFind[MAX_PATH] = {"\0"};
//		WIN32_FIND_DATA findFileData;
//		BOOL bRet;

//		strncpy(szFind, lpPath ,MAX_PATH);
//		strcat(szFind, "\\*.*");     //这里一定要指明通配符，不然不会读取所有文件和目录

//		HANDLE hFind = ::FindFirstFile(szFind, &findFileData);
//		if (INVALID_HANDLE_VALUE == hFind)
//		{
//			return;
//		}

//		//遍历文件夹
//		while (TRUE)
//		{
//			if (findFileData.cFileName[0] != L'.')
//			{//不是当前路径或者父目录的快捷方式
//				//_tprintf(_T("%s\\%s\n"), lpPath, findFileData.cFileName);
//				std::string wsFile;
//				if(wsFileExt=="*"){
//					wsFile.append(lpPath).append("\\").append( findFileData.cFileName);
//					vecFiles.push_back(wsFile);
//				}else if(wsFileExt==FileUtils::ExtractFileExt(findFileData.cFileName)){
//					wsFile.append(lpPath).append("\\").append( findFileData.cFileName);
//					vecFiles.push_back(wsFile);
//				}
//				if (findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
//				{//这是一个普通目录
//					//设置下一个将要扫描的文件夹路径
//					strncpy(szFind,  lpPath ,MAX_PATH);
//					strcat(szFind, "\\");
//					strcat(szFind, findFileData.cFileName);
//					///_tcscat_s(szNextDir, _T("\\*"));
//					//遍历该目录
//					getFolderEnties(szFind,vecFiles,wsFileExt);
//				}
//			}
//			//如果是当前路径或者父目录的快捷方式，或者是普通目录，则寻找下一个目录或者文件
//			bRet = ::FindNextFile(hFind, &findFileData);
//			if (!bRet)
//			{//函数调用失败
//				//cout << "FindNextFile failed, error code: "
//				//  << GetLastError() << endl;
//				break;
//			}
//		}

//		::FindClose(hFind);
//	}

#ifdef _WIN32
	inline int GetFilesEntry(const std::string &dir, std::vector<std::string> &vecFiles, const std::string wsFileFilter)
    {
        _finddata_t localFindData;
        long ptrf;
        std::string rules=dir+"\\"+wsFileFilter;
        //std::cout<<rules<<std::endl;
        //_findfirst返回的是long型; long __cdecl _findfirst(const char *, struct _finddata_t *)
        if((ptrf = _findfirst(rules.c_str(), &localFindData))==-1l)
        {
            std::cout<<"文件没有找到!\n";
            return -1;
        }
        else
        {
            //int __cdecl _findnext(long, struct _finddata_t *);如果找到下个文件的名字成功的话就返回0,否则返回-1
            std::string tempName;
            while( _findnext( ptrf, &localFindData ) == 0 )
            {
                tempName = "";
                tempName = localFindData.name;
                if (tempName == "..")
                {
                    continue;
                }
                if(localFindData.attrib == _A_SUBDIR)
                {
                    std::string subdir=dir+"\\"+tempName;
                    GetFilesEntry(subdir,vecFiles,wsFileFilter);
                } else{
                    std::string filepath=dir+"\\"+tempName;
                    vecFiles.push_back(filepath);
                }

            }

        }

        _findclose(ptrf);
        return vecFiles.size();
    }
#endif
    /*
    * 获取下级目录下的文件结点，目录以'/'分割（不便利）
    */
    inline int GetFilesEntryCurrentDir(const std::string &dirpath, std::vector<std::string> &vecFiles)
    {
#ifdef _WIN32
        _finddata_t localFindData;
        long ptrf;
        std::string rules = dirpath+"\\*.*";
        if((ptrf = _findfirst(rules.c_str(), &localFindData))==-1l)
        {
            std::cout<<"文件没有找到!\n";
            return -1;
        }
        else
        {
            std::string tempName;
            while( _findnext( ptrf, &localFindData ) == 0 )
            {
                tempName = "";
                tempName = localFindData.name;
                if (tempName == "..")
                {
                    continue;
                }
                if(localFindData.attrib == _A_SUBDIR)
                {
                    std::string subdir=dirpath+"\\"+tempName;
                    vecFiles.push_back(subdir);
                }
//                else
//                {
//                    std::string filepath=dirpath+"\\"+tempName;
//                    vecFiles.push_back(filepath);
//                }
            }
        }
        _findclose(ptrf);
        return vecFiles.size();
#else
        struct stat s;
        DIR     *dir =NULL;
        struct dirent *dt;
        char dirname[2048] = {0};

        strcpy(dirname, dirpath.c_str());

        if(stat(dirname, &s) < 0)
        {
            printf("%s\n stat error: %s\n",dirname,strerror(errno));
            return 0;
        }
        if(S_ISDIR(s.st_mode))
        {
            if((dir = opendir(dirname)) == NULL)
            {
                printf("opendir %s/%s error\n");
                return 0 ;
            }
            while((dt = readdir(dir)) != NULL)
            {
                if(strcmp(dt->d_name,".") == 0 || strcmp(dt->d_name,"..") == 0)
                {
                    continue;
                }
                char szBuf[2048] = {0};
                sprintf(szBuf,"%s/%s",dirname,dt->d_name);
                if(stat(szBuf, &s) < 0)
                {
                    break;
                }
                if(S_ISDIR(s.st_mode))
                {
                    vecFiles.push_back(szBuf);
                }
            }
            closedir(dir);
        }
//        else
//        {
//            vecFiles.push_back(dirname);
//        }
        return vecFiles.size();
#endif

    }

	/*
	* 获取目录下的文件结点，目录以'/'分割
	*/
    inline int GetAllFilesEntry(const std::string &dirpath, std::vector<std::string> &vecFiles)
	{
#ifdef _WIN32
        std::string wsRootDir=dirpath;
        int nFiles =  GetFilesEntry(wsRootDir.c_str(),vecFiles,"*.*");
		//for(size_t i=0;i<vecFiles.size();i++){
		//	vecFiles[i]=FileUtils::BslToSl(vecFiles[i].substr(wsRootDir.size()+1));
		//	std::wcout<< vecFiles[i] <<std::endl;;  
		//}
		return nFiles;
#else
        struct stat s;
        DIR     *dir =NULL;
        struct dirent *dt;
        char dirname[2048] = {0};

        strcpy(dirname, dirpath.c_str());

        if(stat(dirname, &s) < 0)
        {
            printf("%s\n stat error: %s\n",dirname,strerror(errno));
            return 0;
        }
        if(S_ISDIR(s.st_mode))
         {
            if((dir = opendir(dirname)) == NULL)
            {
               printf("opendir %s/%s error\n");
               return 0 ;
            }
            while((dt = readdir(dir)) != NULL)
            {
                if(strcmp(dt->d_name,".") == 0 || strcmp(dt->d_name,"..") == 0)
               {
                 continue;
               }
                char szBuf[2048] = {0};
                sprintf(szBuf,"%s/%s",dirname,dt->d_name);
                std::string sPath = szBuf;
                GetAllFilesEntry(sPath,vecFiles);
            }
            closedir(dir);
         }
        else
        {
            vecFiles.push_back(dirname);
        }
        return vecFiles.size();
#endif

	}
	/*
	* 获取目录下的文件结点，目录以'/'分割
	*/
	inline int GetAllFilesRelativeEntry(const std::string &dir, std::vector<std::string> &vecFiles)
	{
		std::string sRootDir = dir;
        //int nFiles = GetFilesEntry(sRootDir.c_str(),vecFiles,"*.*");
        int nFiles = GetAllFilesEntry(sRootDir,vecFiles);
		for(size_t i=0;i<vecFiles.size();i++)
		{
			vecFiles[i] = vecFiles[i].substr(sRootDir.size()+1);
			//vecFiles[i]=FileUtils::BslToSl(vecFiles[i].substr(wsRootDir.size()+1));
		}
		return nFiles;

	}
    inline  int GetModuleFileName( char * buf, int count = 256)
    {
#ifdef _WIN32
        ::GetModuleFileName(NULL,buf,count);
        TCHAR * pszFind = strrchr(buf,'\\');
          *pszFind = 0;
        return 1;
#else
        int i;
        int rslt = readlink("/proc/self/exe", buf, count - 1);//注意这里使用的是self
        if (rslt < 0 || (rslt >= count - 1))
        {
            return NULL;
        }
        buf[rslt] = '\0';
        for (i = rslt; i >= 0; i--)
        {
            if (buf[i] == '/')
            {
                buf[i] = '\0';
                break;
            }
        }
        return true ;
#endif
    }
    static inline bool SystemTempDir(std::string &wsTempDir)
    {
#ifdef WIN32
        char path[256];
        if(::GetTempPath(256,path)< 0){
            return false;
        }
        wsTempDir.append(path);
        wsTempDir=ExcludeTrailingPathDelimiter(wsTempDir);
        return true;
#else
        //linux use
         char sz_exe_path[500] = {0};
         GetModuleFileName(sz_exe_path,500);
         std::string sTempPath = sz_exe_path;
         sTempPath += "/Temp";
         if(access(sTempPath.c_str(),0) == -1)
         {
             FileUtils::MakeSureDirExsits(sTempPath.c_str());
         }
         wsTempDir  = sTempPath;
         return true;

#endif

    }
	//获取一个文件夹下的所有目录列表
	static bool GetFileDirList(std::string sFileDir ,std::vector<std::string> & vcDirList)
	{
#ifdef WIN32
		vcDirList.push_back(sFileDir);
		std::string sFileFind;
		sFileFind =  sFileDir;
		sFileDir.append("\\*.*");
		WIN32_FIND_DATA  fd;
		HANDLE hFind = FindFirstFile(sFileDir.c_str(),&fd);
		if(hFind == INVALID_HANDLE_VALUE)
		{
			return false;
		}
		while (TRUE)
		{
			if(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{
				if(strcmp(fd.cFileName,".") != 0 && strcmp(fd.cFileName,"..") != 0)
				{
					//sFileFind.append("\\").append(fd.cFileName);
					std::string  sPath = sFileFind;
					sPath.append("\\").append(fd.cFileName);
					GetFileDirList(sPath,vcDirList);
				}
			}
			if(!FindNextFile(hFind,&fd))
			{
				break;
			}
		}
		FindClose(hFind);
		return true;
		
#else

		struct stat s;
		DIR     *dir;
		struct dirent *dt;
        char dirname[2048] = {0};

		strcpy(dirname, sFileDir.c_str());

        if(stat(dirname, &s) < 0)
		{
            printf("%s\n stat error: %s\n",dirname,strerror(errno));
			return false;
		}
		if(S_ISDIR(s.st_mode))
		{
			vcDirList.push_back(dirname);

			if((dir = opendir(dirname)) == NULL)
			{
				printf("opendir %s/%s error\n");
				return false;
			}
			while((dt = readdir(dir)) != NULL)
			{
				if(strcmp(dt->d_name,".") == 0 || strcmp(dt->d_name,"..") == 0)
				{
					continue;
				}
                char szBuf[2048] = {0};
				sprintf(szBuf,"%s/%s",dirname,dt->d_name);
				GetFileDirList(szBuf,vcDirList);
			}
		}
		else
		{
			return true;
		}
#endif 
		return true;
	}
	inline static std::string& trim(std::string& text)
	{
		if(!text.empty())
		{
			text.erase(0, text.find_first_not_of(" \n\r\t"));
			text.erase(text.find_last_not_of(" \n\r\t") + 1);
		}
		return text;
	}
	inline static std::string& deleteAllStr(std::string& text,char* str,int strlen)
	{
		if(!text.empty())
		{
			while(true)
			{
				int iPos = text.find(str);
				if(iPos == text.npos)
				{
					break;
				}
				text.replace(iPos,strlen,"");
			}
		}
		return text;
	}



};


#endif
