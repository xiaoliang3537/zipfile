#ifndef PATHUTILS_H_
#define PATHUTILS_H_
#include <string>
#include <wchar.h>

namespace FileUtils{
	
	/*
	将路径中的斜杠转为反斜杠："/"-->"\"
	*/
	inline std::string SlToBsl(const std::string& strPath)
	{
		std::string strNewPath;
		char cSlashes = 0x2F;
		char cBL = 0x5C;
		int nPos = 0;
		strNewPath = strPath;
		while (true)
		{
			nPos = strNewPath.find(cSlashes, nPos);
			if (nPos == -1) break;
			strNewPath.replace(nPos, 1, &cBL, 1);
		}	
		return strNewPath;
	}

	/*
	将路径中的反斜杠转为斜杠："\"-->"/"
	*/
	inline std::string BslToSl(const std::string& strPath)
	{
		std::string strNewPath;
		char cSlashes = 0x2F;
		char cBL = 0x5C;
		size_t nPos = 0;
		strNewPath = strPath;
		while (true)
		{
			nPos = strNewPath.find(cBL, nPos);
			if (nPos == -1) break;
			strNewPath.replace(nPos, 1, &cSlashes, 1);
		}
		return strNewPath;
	}

	
	/*
	删除路径最后面部分目录  
	删除最后，路径最后不包含"\"
	n为要删除的子目录数
	*/
	inline std::string DelLastDirByBsl(const std::string &strPath, int n = 1)
	{
        char cBL = '\\';

		int nPos;
		if (strPath.at(strPath.length() - 1) == cBL)
			nPos = strPath.length() - 1;	// 最后是"\"
		else
			nPos = strPath.length();		// 最后不是"\"

        for (int i = 0; i < n; i++)
        {
            nPos = strPath.rfind(cBL, nPos - 1);
            if (std::string::npos == nPos)
                return strPath.substr(0,0);
        }
        return strPath.substr(0, nPos);
	}


	/*
	删除路径最后面部分目录  目录包含"/" ,删除最后，路径最不包含"/"
	n为要删除的子目录数
	*/
	inline std::string DelLastDirBySl(const std::string &strPath, int n = 1)
	{
        char cBL = '/';

		int nPos;
		if (strPath.at(strPath.length() - 1) == cBL)
			nPos = strPath.length() - 1;	// 最后是"/"
		else
			nPos = strPath.length();		// 最后不是"/"

		for (int i = 0; i < n; i++)
		{
			nPos = strPath.rfind(cBL, nPos - 1);
			if (std::string::npos == nPos)
				return strPath.substr(0,0);
		}
		return strPath.substr(0, nPos);
	}

		/*
	在路径上加双引号
	strFullPath: 传入文件全路径名
	*/
	inline std::string MarkFilePath(const std::string& strFullPath)
	{	

		return "\""+strFullPath+"\"";
	}

	/*
	提取文件名全路径中的路径部分，最后面包括反斜杠"\"
	strFullPath: 传入文件全路径名
	*/
	inline std::string ExtractFilePath(const std::string& strFullPath)
	{	
		std::string strPath;
#ifdef _WIN32
        char cBL = '\\';
#else
          char cBL = '/';
#endif
		int nPos = strFullPath.rfind(cBL);
		if (nPos != -1)
			strPath = strFullPath.substr(0, nPos+1);
		return strPath;
	}

	/*
	提取文件名全路径中的路径部分，最后面不包括反斜杠"\"
	strFullPath: 传入文件全路径名
	*/
	inline std::string ExtractFileDir(const std::string& strFullPath)
	{
		std::string strDir;
#ifdef _WIN32
		char cBL =  '\\';
#else
        char cBL =  '/';
#endif
		int nPos = strFullPath.rfind(cBL);
		if (nPos != -1)
		{
			if (strFullPath[nPos] == cBL)
				strDir = strFullPath.substr(0, nPos);
			else
				strDir = strFullPath.substr(0, nPos+1);
		}
		return strDir;
	}
	/*
	提取文件全路径名中的文件名(文件名包括后缀)
	strFullPath: 传入文件全路径名,路径必须是以'\'的形式存在
	*/
	inline std::string ExtractFileName(const std::string& sFullPath)
	{
		std::string strFileName;
#ifdef _WIN32
        char cBL =  '\\';
#else
        char cBL =  '/';
#endif
		int nPos = sFullPath.rfind(cBL);
		if (nPos != -1)
        {
			strFileName = sFullPath.substr(nPos+1, sFullPath.size() - nPos);
            return strFileName;
        }
        else
        {
            return sFullPath;
        }

	}

	/*
	去除文件名扩展名
	*/
	inline std::string ExtractFileRemoveExt(const std::string& strFileName)
	{
		std::string strFileExt;
        char cPoint = '.';
		int nPos = strFileName.rfind(cPoint);
		if (nPos != -1)
			strFileExt = strFileName.substr(0, nPos);
		return strFileExt;
	}

	/*
	提取文件全路径名中的文件名(文件名包括后缀)
	strFullPath: 传入文件全路径名,路径必须是以'\'的形式存在
	*/
	inline std::string ExtractNoExtFileName(const std::string& sFullPath)
	{
        std::string strFileName=sFullPath;
#ifdef _WIN32
        char cBL =  '\\';
#else
        char cBL =  '/';
#endif
		int nPos = sFullPath.rfind(cBL);
		if (nPos != -1)
			strFileName = sFullPath.substr(nPos+1, sFullPath.size() - nPos);
		return ExtractFileRemoveExt(strFileName);
	}

	/*
	提取文件名中的扩展名，不包括'.' 
	如果要带'.'请使用CRT:_splitpath或_wsplitpath
	*/
	inline std::string ExtractFileExt(const std::string& strFileName)
	{
		std::string strFileExt;
        char cPoint = '.';
		int nPos = strFileName.rfind(cPoint);
		if (nPos != -1)
			strFileExt = strFileName.substr(nPos+1, strFileName.size() - nPos);
		return strFileExt;
	}

	/*
	改变文件扩展名
	strFileName: 传入文件名
	strFileExt:  传入新的扩展名
	*/
	inline std::string ChangeFileExt(const std::string& strFileName, const std::string& strFileExt)
	{
		std::string strNewFileName;
        char strPoint = '.';
		int nPos = strFileName.rfind(strPoint);
		if (nPos != -1)
		{
			strNewFileName = strFileName.substr(0, nPos+1);
			strNewFileName += strFileExt;
		}		
		return strNewFileName;
	}

	/*
	给文件路径的尾部加一个反斜杠
	*/
	inline std::string IncludeTrailingPathDelimiter(const std::string& strPath)
	{
		std::string strNewPath;
       // char cSlashes = '\\';
#ifdef _WIN32
        char cSlashes =  '\\';
#else
        char cSlashes =  '/';
#endif
		int nLastChar = strPath.size() - 1;
		if (strPath[nLastChar] != cSlashes)	
			strNewPath = strPath + cSlashes;
		else
			strNewPath = strPath;
		return strNewPath;
	}

	/*
	去掉文件路径尾部的反斜杠
	*/
	inline std::string ExcludeTrailingPathDelimiter(const std::string& strPath)
	{
		std::string strNewPath;
        //char cSlashes = '\\';
#ifdef _WIN32
        char cSlashes =  '\\';
#else
        char cSlashes =  '/';
#endif
		int nLastChar = strPath.size() - 1;
		strNewPath = strPath;
		if (strPath[nLastChar] == cSlashes)
			strNewPath.resize(nLastChar);
		return strNewPath;
	}

	/*
	删除路径最后面部分目录，被删目录可包含"\"可不包含"\" ,删除后路径最后包含"\"
	n为要删除的子目录数
	*/
	inline std::string DelLastPathByBsl(const std::string &strPath, int n = 1)
	{
#ifdef _WIN32
        char cBL = '\\';
#else
          char cBL = '/';
#endif

		int nPos;
		if (strPath.at(strPath.length() - 1) == cBL)
			nPos = strPath.length() - 1;	// 最后是"\"
		else
			nPos = strPath.length();		// 最后不是"\"

		for (int i = 0; i < n; i++)
		{
			nPos = strPath.rfind(cBL, nPos - 1);
			if (std::string::npos == nPos)
				return strPath.substr(0,0);
		}
		return strPath.substr(0, nPos + 1);
	}

	/*
	删除路径最后面部分目录，被删目录可包含"/"可不包含"/" ,删除后路径最后包含"/"
	n为要删除的子目录数
	*/
	inline std::string DelLastPathBySl(const std::string &strPath, int n = 1)
	{
#ifdef _WIN32
        char cBL = '\\';
#else
          char cBL = '/';
#endif

		int nPos;
		if (strPath.at(strPath.length() - 1) == cBL)
			nPos = strPath.length() - 1;	// 最后是"/"
		else
			nPos = strPath.length();		// 最后不是"/"

		for (int i = 0; i < n; i++)
		{
			nPos = strPath.rfind(cBL, nPos - 1);
			if (std::string::npos == nPos)
				return strPath.substr(0,0);
		}
		return strPath.substr(0, nPos + 1);
	}

}

#endif
