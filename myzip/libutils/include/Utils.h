#ifndef UTILS_H_
#define UTILS_H_
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string.h>
#ifdef _WIN32
#include<windows.h>
#endif
/*****************************************************************************
* arg_to_int - Convert argument string to integer.
*
* min - Minimum allowed value, inclusive.
* max - Maximum allowed value, inclusive.
* defalt - The default value, in case of an error.
* opt - Option string of this argument.  (ex., "-h");
*/

inline int arg_to_int(const char* arg, int min, int max, int defalt, const char* opt)
{
#ifdef _WIN32
	int i = defalt;
	int rv;

	/* no argument means we use the default value */
	if(!arg) goto done;

	/* make sure we got an integer argument */
    rv = sscanf_s(arg, "%d", &i);
	if(rv != 1) {
		fprintf(stderr, "%s: integer argument required.\n", opt);
		i = defalt;
		goto done;
	}

	/* make sure the integer argument is within the desired range */
	if(i < min || max < i) {
		fprintf(stderr, "%s: argument out of integer range.\n", opt);
		i = defalt;
		goto done;
	}

done:
    return i;

#else
int i = defalt;
int rv;

/* no argument means we use the default value */
if(!arg) goto done;

/* make sure we got an integer argument */
rv = sscanf(arg, "%d", &i);
if(rv != 1) {
    fprintf(stderr, "%s: integer argument required.\n", opt);
    i = defalt;
    goto done;
}

/* make sure the integer argument is within the desired range */
if(i < min || max < i) {
    fprintf(stderr, "%s: argument out of integer range.\n", opt);
    i = defalt;
    goto done;
}

done:
return i;
#endif
}
extern FILE* g_pFile;
inline bool mysystem( const char * pszCmd,std::string &strOutput)
{
#ifdef _WIN32
    BOOL bRet = TRUE;
    HANDLE hRead,hWrite;
    SECURITY_ATTRIBUTES sa;
    sa.nLength = sizeof(SECURITY_ATTRIBUTES);
    sa.lpSecurityDescriptor = NULL; //使用系统默认的安全描述符
    sa.bInheritHandle = TRUE; //创建的进程继承句柄
    do
    {
        if (!CreatePipe(&hRead,&hWrite,&sa,0)) //创建匿名管道
        {
            bRet = FALSE;
            break;
        }
        if ( ! SetHandleInformation(hRead, HANDLE_FLAG_INHERIT, 0) )
        {
            std::cout<<"SetHandleInformation hRead failed"<<std::endl;
            bRet = FALSE;
            break;
        }
        STARTUPINFO si;
        PROCESS_INFORMATION pi;

        ZeroMemory(&si,sizeof(STARTUPINFO));
        si.cb = sizeof(STARTUPINFO);
        GetStartupInfo(&si);
        si.hStdError = hWrite;
        si.hStdOutput = hWrite;    //新创建进程的标准输出连在写管道一端
        si.wShowWindow = SW_HIDE; //隐藏窗口
        si.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;

        char buffer[1024] = {0};
        DWORD bytesRead;
        std::string scmd;
        scmd.append("cmd /C \"").append(pszCmd).append("\"");
        if (!CreateProcess(NULL,(LPSTR)scmd.c_str(),NULL,NULL,TRUE,NULL,NULL,NULL,&si,&pi)) //创建子进程
        {
            bRet = FALSE;
            break;
        }
        CloseHandle(hWrite); //关闭管道句柄
        strOutput.clear();\
        DWORD excode = 0;
        GetExitCodeProcess(pi.hProcess, &excode);
        if(excode != STILL_ACTIVE)
        {
            printf("Process seems to have stopped\n");
            bRet = FALSE;
            break;
        }
        while (true)
        {
            memset(buffer,0,1024);
            if (ReadFile(hRead,buffer,1024,&bytesRead,NULL) == NULL) //读取管道
            {
                GetExitCodeProcess(pi.hProcess, &excode);
                printf("Process code %d \n",excode);
                break;
            }
            strOutput.append(buffer,bytesRead);
        }
        CloseHandle(hRead);
    }while (0);
    return bRet;
#else
    FILE *pFile = popen(pszCmd, "r"); //建立管道
    if (!pFile)
    {
        return 0;
    }
    g_pFile = pFile;
    strOutput.clear();
    char buf [1024] = {0};
    while(true)
    {
        memset(buf,0,1024);
        if( 0 == fread(buf,1,1024,pFile))
        {
            break;
        }
        strOutput.append(buf);
    }
    pclose(pFile); //关闭管道
    g_pFile = NULL;
    return 1;
#endif
}
inline bool  WindowsExec(const char* cmdline, bool uCmdShow)
{
#ifdef _WIN32
    PROCESS_INFORMATION pi;
    STARTUPINFO si;
    si.cb = sizeof(STARTUPINFO);
    si.lpReserved = NULL;
    si.lpDesktop = NULL;
    si.lpTitle = NULL;
    si.dwFlags = 0;
    si.cbReserved2 = 0;
    si.lpReserved2 = NULL;
    si.dwFlags = STARTF_USESHOWWINDOW;
    si.wShowWindow = uCmdShow?SW_SHOWNORMAL :SW_HIDE;

    // PVOID OldValue = NULL;
    //	 Wow64DisableWow64FsRedirection(&OldValue);

    std::string cmdline_fixed;
    cmdline_fixed.append("cmd /c \"").append(cmdline).append("\"");


    BOOL bres = CreateProcess(NULL,(LPSTR)cmdline_fixed.c_str(),NULL,NULL,false,
                              NORMAL_PRIORITY_CLASS,
                              NULL,NULL,&si,&pi);

    //启用重定向
    //  Wow64RevertWow64FsRedirection(&OldValue);

    if(bres==false)
    {
        std::cout << "进程创建失败" << std::endl;
        return false;
    }
    else
    {
        CloseHandle(pi.hThread);
        DWORD dwret=WaitForSingleObject(pi.hProcess, INFINITE);
        switch(dwret)
        {
        case WAIT_OBJECT_0:
        {
            DWORD dwexitcode;
            bres = GetExitCodeProcess(pi.hProcess,&dwexitcode);
            TCHAR exitmsgbuf[1024] = {0};
            if(bres)
            {
                //wsprintf(exitmsgbuf,"exit code:%d",dwexitcode);
            }
            else
            {
                sprintf(exitmsgbuf,"exit code failed to return");
                return false;
            }
            std::cout << exitmsgbuf << std::endl;
            break;
        }
        default:
            std::cout << "exit for other reason" << std::endl;
            return false;
        }
        CloseHandle(pi.hProcess);
    }
    return true;
#else
    if(cmdline == NULL || strlen(cmdline) <= 0)
    {
        return false;
    }
    char szbuf[1024] = {0};
    sprintf(szbuf,"%s",cmdline);
    std::cout<<szbuf<<std::endl;
    int status = system(szbuf);
    if(-1 != status && WIFEXITED(status)&&0 == WEXITSTATUS(status))
    {
        return true;
    }
    return false;
#endif
}

//inline bool  WindowsExec(char* cmdline, bool uCmdShow,char * lpOutfile)
//{
//	PROCESS_INFORMATION pi;
//	STARTUPINFO si;
//	si.cb = sizeof(STARTUPINFO);
//	si.lpReserved = NULL;
//	si.lpDesktop = NULL;
//	si.lpTitle = NULL;
//	si.dwFlags = 0;
//	si.cbReserved2 = 0;
//	si.lpReserved2 = NULL;
//	si.dwFlags = STARTF_USESHOWWINDOW;
//	si.wShowWindow = uCmdShow?SW_SHOWNORMAL :SW_HIDE;

//	if(lpOutfile){
//		//在CreatePipe、CreateProcess等Create系列函数中，
//		//通常都有一个SECURITY_ATTRIBUTES类型的参数
//		SECURITY_ATTRIBUTES saAttr = {0};
//		saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
//		//若把该参数的bInheritHandle项设为TRUE，
//		//则它创建出来的句柄可被子进程继承。
//		//例如，用CreatePipe创建的管道可用于CreateProcess创建的进程
//		saAttr.bInheritHandle = TRUE;
//		saAttr.lpSecurityDescriptor = NULL;

//		//ChildIn_Write是子进程的输入句柄，ChildIn_Read是父进程用于写入子进程输入的句柄
//		HANDLE ChildIn_Read, ChildIn_Write;
//		CreatePipe(&ChildIn_Read, &ChildIn_Write, &saAttr, 0);
//		//设置子进程不能继承接收输入管道的另一端：ChildIn_Write
//		SetHandleInformation(ChildIn_Write, HANDLE_FLAG_INHERIT, 0);
//		//ChildOut_Write是子进程的输出句柄，ChildOut_Read是父进程用于读取子进程输出的句柄
//		HANDLE ChildOut_Read, ChildOut_Write;
//		CreatePipe(&ChildOut_Read, &ChildOut_Write, &saAttr, 0);
//		//设置子进程不能继承发送输出管道的另一端：ChildOut_Read
//		SetHandleInformation(ChildOut_Read, HANDLE_FLAG_INHERIT, 0);

//		//CreateProcess的第一个参数
//		STARTUPINFO StartupInfo = {0};
//		StartupInfo.cb = sizeof(STARTUPINFO);
//		//将标准输出和错误输出定向到我们建立的ChildOut_Write上
//		StartupInfo.hStdError = ChildOut_Write;
//		StartupInfo.hStdOutput = ChildOut_Write;
//		//将标准输入定向到我们建立的ChildIn_Read上
//		StartupInfo.hStdInput = ChildIn_Read;
//		//设置子进程接受StdIn以及StdOut的重定向
//		StartupInfo.dwFlags |= STARTF_USESTDHANDLES;

//		//CreateProcess的第二个参数
//		PROCESS_INFORMATION ProcessInfo = {0};
//	}
//	//  PVOID OldValue = NULL;
//	// Wow64DisableWow64FsRedirection(&OldValue);
//	std::string cmdline_fixed;
//	cmdline_fixed.append("cmd /c ").append(cmdline);
	
//	BOOL bres = CreateProcess(NULL,(LPSTR)cmdline_fixed.c_str(),NULL,NULL,false,
//		NORMAL_PRIORITY_CLASS,
//		NULL,NULL,&si,&pi);
//	  //启用重定向
//  // Wow64RevertWow64FsRedirection(&OldValue);

//	if(bres==false)
//	{
//		std::cout << "进程创建失败" << std::endl;
//		return false;
//	}
//	else
//	{
//		CloseHandle(pi.hThread);
//		DWORD dwret=WaitForSingleObject(pi.hProcess,20* 60 * 1000);
//		switch(dwret)
//		{
//		case WAIT_OBJECT_0:
//			DWORD dwexitcode;
//			bres = GetExitCodeProcess(pi.hProcess,&dwexitcode);
//			TCHAR exitmsgbuf[1024];
//			if(bres)
//			{
//				wsprintf(exitmsgbuf,"exit code:%d",dwexitcode);
//			}else{
//				wsprintf(exitmsgbuf,"exit code failed to return");
//				return false;
//			}
//			std::cout << exitmsgbuf << std::endl;
//			break;
//		default:

//			std::cout << "exit for other reason" << std::endl;
//			return false;
//		}
//		CloseHandle(pi.hProcess);
//	}
//	return true;
//}


//static int execute(std::string executable, std::vector<std::string>& args, bool block=false, bool useStdout=false, std::string stdoutfile="")
//{
//  std::string clString = "\"" + executable + "\"";

//  for (int i = 0; i < args.size(); i++)
//    clString += ' ' + args.at(i);

// std::string clwString = StringToWString(clString);
//  std::string outfile = StringToWString(stdoutfile);

// HANDLE h = CreateFile (stdoutfile.c_str(),
//    GENERIC_WRITE,
//    0,
//    NULL,
//    OPEN_ALWAYS,         
//    FILE_ATTRIBUTE_NORMAL, 
//    NULL);

// if (h == INVALID_HANDLE_VALUE) return -1;

// CloseHandle(h);

//  STARTUPINFO siStartInfo;
//  ZeroMemory( &siStartInfo, sizeof(STARTUPINFO) );
//  siStartInfo.cb = sizeof(STARTUPINFO);

//  if (useStdout) {
//    siStartInfo.hStdError = GetStdHandle(STD_OUTPUT_HANDLE); 
//   siStartInfo.hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);
//   siStartInfo.dwFlags |= STARTF_USESTDHANDLES;

//     Redirect stdout
//    freopen( stdoutfile.c_str(), "w", stdout );
// }

//   process information structure
// PROCESS_INFORMATION pi;

//  Start the process
//  CreateProcess( NULL, const_cast<LPSTR>(clString.c_str()), NULL, NULL, TRUE, 0, NULL, NULL, &siStartInfo, &pi );
//  if (block || useStdout)
//    WaitForSingleObject( pi.hProcess, INFINITE );

//  if (useStdout){
//     Restore stdout to console
//    freopen( "CON", "w", stdout );
//  }

//  return 0;
//}

//static int ConvertError()
//{
//  int err = 0;
//  int lastErr = GetLastError();
//  if (lastErr == ERROR_FILE_NOT_FOUND ||
//    lastErr == ERROR_PATH_NOT_FOUND)
//  {
//    err = ERROR_NOT_FOUND;
//  }
//  return err;
//}

#endif
