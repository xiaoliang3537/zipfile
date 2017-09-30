#ifndef PATHUTILS_H_
#define PATHUTILS_H_
#include <string>
#include <wchar.h>

namespace FileUtils{
	
	/*
	��·���е�б��תΪ��б�ܣ�"/"-->"\"
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
	��·���еķ�б��תΪб�ܣ�"\"-->"/"
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
	ɾ��·������沿��Ŀ¼  
	ɾ�����·����󲻰���"\"
	nΪҪɾ������Ŀ¼��
	*/
	inline std::string DelLastDirByBsl(const std::string &strPath, int n = 1)
	{
        char cBL = '\\';

		int nPos;
		if (strPath.at(strPath.length() - 1) == cBL)
			nPos = strPath.length() - 1;	// �����"\"
		else
			nPos = strPath.length();		// �����"\"

        for (int i = 0; i < n; i++)
        {
            nPos = strPath.rfind(cBL, nPos - 1);
            if (std::string::npos == nPos)
                return strPath.substr(0,0);
        }
        return strPath.substr(0, nPos);
	}


	/*
	ɾ��·������沿��Ŀ¼  Ŀ¼����"/" ,ɾ�����·�������"/"
	nΪҪɾ������Ŀ¼��
	*/
	inline std::string DelLastDirBySl(const std::string &strPath, int n = 1)
	{
        char cBL = '/';

		int nPos;
		if (strPath.at(strPath.length() - 1) == cBL)
			nPos = strPath.length() - 1;	// �����"/"
		else
			nPos = strPath.length();		// �����"/"

		for (int i = 0; i < n; i++)
		{
			nPos = strPath.rfind(cBL, nPos - 1);
			if (std::string::npos == nPos)
				return strPath.substr(0,0);
		}
		return strPath.substr(0, nPos);
	}

		/*
	��·���ϼ�˫����
	strFullPath: �����ļ�ȫ·����
	*/
	inline std::string MarkFilePath(const std::string& strFullPath)
	{	

		return "\""+strFullPath+"\"";
	}

	/*
	��ȡ�ļ���ȫ·���е�·�����֣�����������б��"\"
	strFullPath: �����ļ�ȫ·����
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
	��ȡ�ļ���ȫ·���е�·�����֣�����治������б��"\"
	strFullPath: �����ļ�ȫ·����
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
	��ȡ�ļ�ȫ·�����е��ļ���(�ļ���������׺)
	strFullPath: �����ļ�ȫ·����,·����������'\'����ʽ����
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
	ȥ���ļ�����չ��
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
	��ȡ�ļ�ȫ·�����е��ļ���(�ļ���������׺)
	strFullPath: �����ļ�ȫ·����,·����������'\'����ʽ����
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
	��ȡ�ļ����е���չ����������'.' 
	���Ҫ��'.'��ʹ��CRT:_splitpath��_wsplitpath
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
	�ı��ļ���չ��
	strFileName: �����ļ���
	strFileExt:  �����µ���չ��
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
	���ļ�·����β����һ����б��
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
	ȥ���ļ�·��β���ķ�б��
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
	ɾ��·������沿��Ŀ¼����ɾĿ¼�ɰ���"\"�ɲ�����"\" ,ɾ����·��������"\"
	nΪҪɾ������Ŀ¼��
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
			nPos = strPath.length() - 1;	// �����"\"
		else
			nPos = strPath.length();		// �����"\"

		for (int i = 0; i < n; i++)
		{
			nPos = strPath.rfind(cBL, nPos - 1);
			if (std::string::npos == nPos)
				return strPath.substr(0,0);
		}
		return strPath.substr(0, nPos + 1);
	}

	/*
	ɾ��·������沿��Ŀ¼����ɾĿ¼�ɰ���"/"�ɲ�����"/" ,ɾ����·��������"/"
	nΪҪɾ������Ŀ¼��
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
			nPos = strPath.length() - 1;	// �����"/"
		else
			nPos = strPath.length();		// �����"/"

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
