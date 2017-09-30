#include "zipfile.h"
#include"zlib.h"
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <string>

#include "zip.h"
#include "zlib.h"
#include "../zlib/unzip.h"
#include "fnmatch.h"

#ifndef WIN32
#include <errno.h>
#define MAX_PATH 260
#define FILE_ATTRIBUTE_DIRECTORY 1
#else
#endif
//#include "./include/FileUtils.h"
#include "../libutils/include/FileUtils.h"

#define DEF_MEM_LEVEL 8                // normally in zutil.h?
#define ZIP_GPBF_LANGUAGE_ENCODING_FLAG 0x800

enum {
    // finding the directory
    CD_SIGNATURE = 0x06054b50,
    EOCD_LEN     = 22,        // EndOfCentralDir len, excl. comment
    MAX_COMMENT_LEN = 65535,
    MAX_EOCD_SEARCH = MAX_COMMENT_LEN + EOCD_LEN,

    // central directory entries
    ENTRY_SIGNATURE = 0x02014b50,
    ENTRY_LEN = 46,          // CentralDirEnt len, excl. var fields

    // local file header
    LFH_SIZE = 30,
};


zipfile_t
init_zipfile(const void* data, size_t size)
{
    int err=0;

    Zipfile *file = (Zipfile *)malloc(sizeof(Zipfile));
    if (file == NULL)
        return NULL;
    memset(file, 0, sizeof(Zipfile));
    file->buf = (const unsigned char *)data;
    file->bufsize = size;

    err = read_central_dir(file);
    if(err != 0)
    {
        free(file);
        return NULL;
    }
    else
    {
        return file;

    }
    /*
    if (err != 0)
        goto fail;

    return file;
fail:
    free(file);
    return NULL;
    */
}


void release_zipfile(zipfile_t f)
{
    Zipfile* file = (Zipfile*)f;
    Zipentry* entry = file->entries;
    while (entry)
    {
        Zipentry* next = entry->next;
        free(entry);
        entry = next;
    }

    ZipentryCenteral *centeral = file->centeral;
    while(centeral)
    {
        ZipentryCenteral* next = centeral->next;
        free(centeral);
        centeral = next;
    }
    free(file);
}

zipentry_t lookup_zipentry(zipfile_t f, const char* entryName)
{
    Zipfile* file = (Zipfile*)f;
    Zipentry* entry = file->entries;
    while (entry)
    {
		int iLength = entry->fileNameLength > strlen(entryName)?entry->fileNameLength:strlen(entryName);
        if (0 == memcmp(entryName, entry->fileName, iLength))
        {
            return entry;
        }
        entry = entry->next;
    }
    return NULL;
}
int lookup_Dir(zipfile_t f, const char* dirName)
{
	Zipfile* file = (Zipfile*)f;
	Zipentry* entry = file->entries;
	while (entry)
	{
		if(strstr((char *)entry->fileName,dirName) == (char *)entry->fileName)
		{
			return 1;

		}
		entry = entry->next;
	}
	return 0;
}
size_t
get_zipentry_size(zipentry_t entry)
{
    return ((Zipentry*)entry)->uncompressedSize;
}

char*
get_zipentry_name(zipentry_t entry)
{
    Zipentry* e = (Zipentry*)entry;
    int l = e->fileNameLength;
    char* s = (char *)malloc(l+1);
    memcpy(s, e->fileName, l);
    s[l] = '\0';
    return s;
}

enum {
    STORED = 0,
    DEFLATED = 8
};

static int
uninflate(unsigned char* out, int unlen, const unsigned char* in, int clen)
{
    z_stream zstream;
    unsigned long crc;
    int err = 0;
    int zerr;

    memset(&zstream, 0, sizeof(zstream));
    zstream.zalloc = Z_NULL;
    zstream.zfree = Z_NULL;
    zstream.opaque = Z_NULL;
    zstream.next_in = (Bytef*)in;
    zstream.avail_in = clen;
    zstream.next_out = (Bytef*) out;
    zstream.avail_out = unlen;
    zstream.data_type = Z_UNKNOWN;

    // Use the undocumented "negative window bits" feature to tell zlib
    // that there's no zlib header waiting for it.
    zerr = inflateInit2(&zstream, -MAX_WBITS);
    if (zerr != Z_OK)
    {
        return -1;
    }
    // uncompress the data
    zerr = inflate(&zstream, Z_FINISH);
    if (zerr != Z_STREAM_END)
    {
        fprintf(stderr, "zerr=%d Z_STREAM_END=%d total_out=%lu\n", zerr, Z_STREAM_END, zstream.total_out);
        err = -1;
    }

     inflateEnd(&zstream);
    return err;
}

int
decompress_zipentry(zipentry_t e, void* buf, int bufsize)
{
    Zipentry* entry = (Zipentry*)e;
    switch (entry->compressionMethod)
    {
        case STORED:
            //fprintf(stdout," \nmemcpy %ld -- %ld\n",entry->compressedSize,entry->uncompressedSize);
            memcpy(buf, entry->data, entry->uncompressedSize);
            return 0;
        case DEFLATED:
            //fprintf(stdout,"\n uninflate %ld -- %ld\n",entry->compressedSize,entry->uncompressedSize);
            return uninflate((unsigned char *)buf, bufsize, (const unsigned char *)entry->data, entry->compressedSize);
        default:
            return -1;
    }
}

//compress file
//int compress_zipentry(void *in ,unsigned long  inlen ,void* out, unsigned long *outsize)
//{
//    z_stream zstream;
//    int err = 0;
//    int zerr;

//    memset(&zstream, 0, sizeof(zstream));
//    zstream.zalloc = Z_NULL;
//    zstream.zfree = Z_NULL;
//    zstream.opaque = Z_NULL;
//    // Use the undocumented "negative window bits" feature to tell zlib
//    // that there's no zlib header waiting for it.
//    //zerr = inflateInit2(&zstream, -MAX_WBITS);
//   // zerr = deflateInit(&zstream,-MAX_WBITS);
//    zerr = deflateInit(&zstream,Z_DEFAULT_COMPRESSION);
//    if (zerr != Z_OK)
//    {
//        return -1;
//    }
//    zstream.next_in = (Bytef*)in;
//    zstream.avail_in = inlen;
//    zstream.next_out = (Bytef*) out;
//    zstream.avail_out = *outsize;
//    zstream.data_type = Z_UNKNOWN;

//    // compress the data
//    zerr = deflate(&zstream, Z_FINISH);
//    if (zerr != Z_STREAM_END)
//    {
//        fprintf(stderr, "zerr=%d Z_STREAM_END=%d total_out=%lu\n", zerr, Z_STREAM_END, zstream.total_out);
//        err = -1;
//    }
//    // out size
//    *outsize = zstream.total_out;
//    deflateEnd(&zstream);
//    return err;
//}

int ZipCompress(const char * lpszSourceFiles, const char * lpszDestFile, bool bUtf8,bool bZipDir,bool bPassByList )
{
    std::string path_in_zip;
    zipFile zf = zipOpen64(lpszDestFile, 0);
    if (zf == NULL)
    {
        return 0;
    }
#ifdef _WIN32
	DWORD dwAttr = GetFileAttributes(lpszSourceFiles);
	if( INVALID_FILE_ATTRIBUTES == dwAttr )
	{
		return 0;
	}
	if( dwAttr & FILE_ATTRIBUTE_DIRECTORY )
#else
	struct stat s;
	if(stat(lpszSourceFiles, &s) < 0)
	{
        printf("stat error %d\n",errno);
		return 0;
	}
	if(S_ISDIR(s.st_mode))
#endif // 
    {
        if(bZipDir)
        {
            path_in_zip = FileUtils::ExtractFileName(lpszSourceFiles);
        }
        else
        {
            path_in_zip = "";
        }
        if (!ZipAddFiles(zf, path_in_zip.c_str(), lpszSourceFiles, bUtf8,bPassByList))
		{
			zipClose(zf,(const char *)NULL);
			return 0;
		}

    }
    else
    {
        path_in_zip = "";
		std::string file_name ,zip_path;
		path_in_zip.append(FileUtils::ExtractFileName(lpszSourceFiles));
        if (!ZipAddFile(zf, path_in_zip.c_str(), lpszSourceFiles, bUtf8,false,bPassByList))
		{
			zipClose(zf,(const char *)NULL);
			return 0;
		}
    }
    zipClose(zf,(const char *)NULL);
    return 1;

}


int ZipCompress_ForMultiFiles(const char * lpszSourceFiles, const char * lpszDestFile, bool bUtf8,bool bZipDir,bool bPassByList)
{
    std::string path_in_zip;
    zipFile zf = zipOpen64(lpszDestFile, 0);
    if (zf == NULL)
    {
        return 0;
    }
#ifdef _WIN32
    DWORD dwAttr = GetFileAttributes(lpszSourceFiles);
    if( INVALID_FILE_ATTRIBUTES == dwAttr )
    {
        return 0;
    }
    if( dwAttr & FILE_ATTRIBUTE_DIRECTORY )
#else
    struct stat s;
    if(stat(lpszSourceFiles, &s) < 0)
    {
        printf("stat error %d\n",errno);
        return 0;
    }
    if(S_ISDIR(s.st_mode))
#endif //
    {
        if(bZipDir)
        {
            path_in_zip = FileUtils::ExtractFileName(lpszSourceFiles);
        }
        else
        {
            path_in_zip = "";
        }
        if (!ZipAddFiles(zf, path_in_zip.c_str(), lpszSourceFiles, bUtf8,bPassByList))
        {
            zipClose(zf,(const char *)NULL);
            return 0;
        }

    }
    else
    {
        path_in_zip = "";
        std::string file_name ,zip_path;
        path_in_zip.append(FileUtils::ExtractFileName(lpszSourceFiles));
        if (!ZipAddFile(zf, path_in_zip.c_str(), lpszSourceFiles, bUtf8,false,bPassByList))
        {
            zipClose(zf,(const char *)NULL);
            return 0;
        }
    }
    zipClose(zf,(const char *)NULL);
    return 1;
}

const char* g_sPassFileList[] = { ".wav", ".mp2", ".mp3", ".ogg", ".aac",
                           ".mpg", ".mpeg", ".mid", ".midi", ".smf", ".jet",
                           ".rtttl", ".imy", ".xmf", ".mp4", ".m4a",
                           ".m4v", ".3gp", ".3gpp", ".3g2", ".3gpp2",
                           ".amr", ".awb", ".wma", ".wmv",0};
int ZipAddFile(zipFile zf, const char * lpszFileNameInZip, const char * lpszFilePath, bool bUtf8 ,bool bisDir ,bool bPassByList)
{
    FILE * pFile = NULL;
    if(!bisDir)
    {
        pFile = fopen(lpszFilePath,"rb");
        if(pFile == NULL)
        {
            return 0;
        }
    }

    int iRet = 1;
    do
    {
        int method = Z_DEFLATED;//Ä¬ÈÏÑ¹ËõËã·¨

        if(bPassByList)
        {
            std::string s = lpszFilePath;
            s = FileUtils::ExtractFileExt(s);
            s.insert(0,".");
            int iSize = sizeof(g_sPassFileList)/sizeof(g_sPassFileList[0]);

            for(int i = 0 ; i < iSize; i++ )
            {
                if(g_sPassFileList[i] != 0)
                {
                    if(s.compare(g_sPassFileList[i]) == 0)
                    {
                        method = 0;//²»Ñ¹Ëõ
                        printf("NOZip:%s\n",lpszFilePath);
                        break;
                    }
                }
            }
        }

        zip_fileinfo FileInfo;
        memset(&FileInfo, 0,sizeof(FileInfo));
        if (bUtf8)
        {
            if (zipOpenNewFileInZip4(zf, lpszFileNameInZip, &FileInfo, NULL, 0, NULL, 0, NULL, method, 9,
                                     0, -MAX_WBITS, DEF_MEM_LEVEL, Z_DEFAULT_STRATEGY, NULL, 0, 0, ZIP_GPBF_LANGUAGE_ENCODING_FLAG) != ZIP_OK)
            {
                iRet = 0;
                break;
            }
        }
        else
        {
            if (zipOpenNewFileInZip(zf, lpszFileNameInZip, &FileInfo, NULL, 0, NULL, 0, NULL, method, 9) != ZIP_OK)
            {
                iRet = 0;
                break;
            }
        }
        if(bisDir)
        {
            return true;
        }
        const unsigned long  BUFFER_SIZE = 4096;
        char byBuffer[BUFFER_SIZE] = {0};

        fseek(pFile,0,SEEK_END);
        unsigned long  iszie = ftell(pFile);
        fseek(pFile,0,SEEK_SET);

        while (iszie > 0)
        {
             unsigned long dwSizeToRead = iszie > (long long)BUFFER_SIZE ? BUFFER_SIZE : (unsigned long)iszie;
             unsigned long dwRead = 0;
            dwRead = fread(byBuffer,1,dwSizeToRead,pFile);
            if(dwRead <= 0)
            {
                break;
            }

            if (zipWriteInFileInZip(zf, byBuffer, dwRead) < 0)
            {
                iRet = 0;
                break;
            }

           iszie -= (long long)dwRead;
        }

    }while(0);

    fclose(pFile);

    return iRet;
}
int ZipAddFiles(zipFile zf, const char * lpszFileNameInZip, const char * lpszFiles, bool bUtf8 ,bool bPassByList )
{
	if(lpszFiles == NULL)
	{
		return 0;
	}
#ifdef _WIN32
	std::string strFilePath = lpszFiles;
	WIN32_FIND_DATA fd;
	ZeroMemory(&fd, sizeof(fd));
	char szSrcFilePath[260] = {0};
	sprintf(szSrcFilePath,"%s\\*",lpszFiles);
	HANDLE hFind = FindFirstFile(szSrcFilePath, &fd);

	if (hFind == INVALID_HANDLE_VALUE)
	{
		return FALSE;
	}

	std::string strFileNameInZip = lpszFileNameInZip;

	do 
	{
		std::string strFileName = fd.cFileName;

		if (strFileName == "." || strFileName == ".." )
		{
			continue;
		}
		std::string sPathInZipFile,sSrcFilePath;
		if(strFileNameInZip.length() > 0)
		{
			sPathInZipFile.append(strFileNameInZip).append("/").append(strFileName);
		}
		else
		{
			sPathInZipFile = strFileName;
		}
		sSrcFilePath.append(strFilePath).append("\\").append(strFileName);
		if ((fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0)
		{
            if (!ZipAddFiles(zf, sPathInZipFile.c_str(), sSrcFilePath.c_str(), bUtf8,bPassByList))
			{
				return FALSE;
			}
		}
		else
		{
            if (!ZipAddFile(zf, sPathInZipFile.c_str() , sSrcFilePath.c_str(), bUtf8,false,bPassByList))
			{
				return FALSE;
			}
		}

	} while (FindNextFile(hFind, &fd));
	FindClose(hFind);

	return TRUE;
#else
    struct stat s;
    DIR     *dir;
    struct dirent *dt;
    char dirname[500] = {0};

    strcpy(dirname, lpszFiles);

    if(stat(dirname, &s) < 0)
    {
        printf("stat error\n");
        return 0;
    }
    std::string  strFileNameInZip = lpszFileNameInZip;
    if(S_ISDIR(s.st_mode))
    {
        if((dir = opendir(dirname)) == NULL)
        {
            printf("opendir %s/%s error\n");
            return 0;
        }
        while((dt = readdir(dir)) != NULL)
        {
            struct stat s1;
            char szBuf[500] = {0};
            sprintf(szBuf,"%s/%s",dirname,dt->d_name);
            if(stat(szBuf, &s1) < 0)
            {
                printf("error in stat: %d \n",errno);
                continue;
            }
            if(S_ISDIR(s1.st_mode))
            {
                if(strcmp(dt->d_name,".") == 0 || strcmp(dt->d_name,"..") == 0)
                {
                    continue;
                }
                else
                {
                    char szBuf[500] = {0};
                    sprintf(szBuf,"%s/%s",dirname,dt->d_name);

                    std::string  zip_path;
                    zip_path.append(strFileNameInZip);
                    if(zip_path.size() > 0)
                    {
                        zip_path += "/";
                        zip_path += dt->d_name;
                    }
                    else
                    {
                        zip_path += dt->d_name;
                    }

                    if (!ZipAddFiles(zf,zip_path.c_str(),szBuf, bUtf8,bPassByList))
                    {
                        return 0;
                    }
                }
            }
            else
            {
                std::string file_name ,zip_path;
                file_name = dirname;
                file_name .append("/").append(dt->d_name);

                zip_path.append(strFileNameInZip);
                if(zip_path.size() > 0)
                {
                    zip_path += "/";
                    zip_path += dt->d_name;
                }
                else
                {
                    zip_path += dt->d_name;
                }
                if (!ZipAddFile(zf,zip_path.c_str(),file_name.c_str(), bUtf8,false,bPassByList))
                {
                    return 0;
                }
            }
        }
        closedir(dir);
    }
    else
    {
        std::string file_name ,zip_path;
        file_name = dirname;
        file_name = FileUtils::ExtractFileName(file_name);
        zip_path.append(strFileNameInZip);
        zip_path += file_name;

        if (!ZipAddFile(zf,zip_path.c_str(),dirname, bUtf8,false,bPassByList))
        {
            return 0;
        }
    }
    return 1;
#endif // 
}


void dump_zipfile(FILE* to, zipfile_t file)
{
    Zipfile* zip = (Zipfile*)file;
    Zipentry* entry = zip->entries;
    int i;
    fprintf(to, "entryCount=%d\n", zip->entryCount);
    for (i=0; i<zip->entryCount; i++) 
	{
        fprintf(to, "  file \"");
        fwrite(entry->fileName, entry->fileNameLength, 1, to);
        fprintf(to, "\"\n");
        entry = entry->next;
    }
}

zipentry_t
iterate_zipfile(zipfile_t file, void** cookie)
{
    Zipentry* entry = (Zipentry*)*cookie;
    if (entry == NULL) {
        Zipfile* zip = (Zipfile*)file;
        *cookie = zip->entries;
        return *cookie;
    } else {
        entry = entry->next;
        *cookie = entry;
        return entry;
    }
}
#ifdef _WIN32

bool IsTextUTF8(const char* str,int length)
{
    int i;
    int nBytes=0;//UFT8可用1-6个字节编码,ASCII用一个字节
    unsigned char chr;
    bool bAllAscii=true; //如果全部都是ASCII, 说明不是UTF-8
    for(i=0;i<length;i++)
    {
        chr= *(str+i);
        if( (chr&0x80) != 0 ) // 判断是否ASCII编码,如果不是,说明有可能是UTF-8,ASCII用7位编码,但用一个字节存,最高位标记为0,o0xxxxxxx
            bAllAscii= false;
        if(nBytes==0) //如果不是ASCII码,应该是多字节符,计算字节数
        {
            if(chr>=0x80)
            {
                if(chr>=0xFC&&chr<=0xFD)
                    nBytes=6;
                else if(chr>=0xF8)
                    nBytes=5;
                else if(chr>=0xF0)
                    nBytes=4;
                else if(chr>=0xE0)
                    nBytes=3;
                else if(chr>=0xC0)
                    nBytes=2;
                else
                {
                    return false;
                }
                nBytes--;
            }
        }
        else //多字节符的非首字节,应为 10xxxxxx
        {
            if( (chr&0xC0) != 0x80 )
            {
                return false;
            }
            nBytes--;
        }
    }

    if( nBytes > 0 ) //违返规则
    {
        return false;
    }

    if( bAllAscii ) //如果全部都是ASCII, 说明不是UTF-8
    {
        return false;
    }
    return true;
}

string UTF8ToGBK(const std::string& strGBK)
{
    int nLen = MultiByteToWideChar(CP_UTF8, 0, strGBK.c_str(), -1, NULL, 0);
    WCHAR * wszUTF8 = new WCHAR[nLen];
    MultiByteToWideChar(CP_UTF8, 0, strGBK.c_str(), -1, wszUTF8, nLen);

    nLen = WideCharToMultiByte(CP_ACP, 0, wszUTF8, -1, NULL, 0, NULL, NULL);
    char * szUTF8 = new char[nLen];
    WideCharToMultiByte(CP_ACP, 0, wszUTF8, -1, szUTF8, nLen, NULL, NULL);

    std::string strTemp(szUTF8);
    delete[]wszUTF8;
    delete[]szUTF8;
    return strTemp;
}
string GBKToUTF8(const std::string& strGBK)
{
    int nLen = MultiByteToWideChar(CP_ACP, 0, strGBK.c_str(), -1, NULL, 0);
    WCHAR * wszUTF8 = new WCHAR[nLen];
    MultiByteToWideChar(CP_ACP, 0, strGBK.c_str(), -1, wszUTF8, nLen);

    nLen = WideCharToMultiByte(CP_UTF8, 0, wszUTF8, -1, NULL, 0, NULL, NULL);
    char * szUTF8 = new char[nLen];
    WideCharToMultiByte(CP_UTF8, 0, wszUTF8, -1, szUTF8, nLen, NULL, NULL);

    std::string strTemp(szUTF8);
    delete[]wszUTF8;
    delete[]szUTF8;
    return strTemp;
}

char *UNICODEconvertANSI(LPWSTR wText,int targetCodePage)
{
    //wchar_t wText[20] = {L"宽字符转换实例!OK!"};
    int dwNum = WideCharToMultiByte(targetCodePage,NULL,wText,-1,NULL,0,NULL,FALSE);
    char *psText;
    psText = new char[dwNum+1];
    memset(psText,0,dwNum);
    WideCharToMultiByte (targetCodePage,NULL,wText,-1,psText,dwNum+1,NULL,FALSE);
    return psText;
}
LPWSTR ANSIConvertUNCOIDE(char* aText,int sourceCodePage)
{

    int len=MultiByteToWideChar(sourceCodePage,NULL,aText,-1,NULL,0);
    wchar_t *psText=new wchar_t[len+1];
    memset(psText,0,len);
    MultiByteToWideChar(sourceCodePage,NULL,aText,-1,psText,len+1);
    //psText[len]='\0';
    return psText;
}
#endif

int ZIPDecompression(const char *apkpath,const char * sTempDir)
{
	int iRet = 1;
	if(FileUtils::DirExists(sTempDir))
	{
		FileUtils::DeleteDir(sTempDir);
	}
	FileUtils::MakeSureDirExsits(sTempDir);
	zipfile_t zip = NULL;
	void *pszbuf = NULL;
	FILE *pFile = fopen(apkpath,"rb");
	do
	{
		if(pFile == NULL)
		{
			iRet = 0;
			break;
		}
		fseek(pFile, 0, SEEK_END);
		long size = ftell(pFile);
		fseek(pFile,0,SEEK_SET);

		pszbuf = malloc(size);
		if(pszbuf == NULL)
		{
			iRet = 0;
			break;
		}
		memset(pszbuf,0,size);
		long lRead = 0;
		while (lRead != size)
		{
			lRead += fread( (char *)pszbuf + lRead, 1, size - lRead, pFile);
		}
		
		zip = init_zipfile(pszbuf, size);
		if(zip == NULL)
		{
			iRet = 0;
			break;
		}
		// get all file
		Zipfile* zipfile = (Zipfile*)zip;
		Zipentry* entry = zipfile->entries;
		fprintf(stdout, "entryCount=%d\n", zipfile->entryCount);
		for (int i=0; i< zipfile->entryCount; i++)
		{
			size_t ulsize = entry->uncompressedSize;
			if(ulsize <= 0)
			{
				if(*(entry->fileName +entry->fileNameLength -1) == '/' )
				{
#ifdef _DEBUG
					std::cout<<"file is directory:"<<entry->fileName<<std::endl;
#endif
					entry = entry->next;
					continue;
				}
			}
			long size = ulsize *1.001;
			void* scratch = NULL;
			scratch = malloc(size);
			if(scratch == NULL)
			{
				iRet = 0;
				break;
			}
			memset(scratch,0,size);
			if(decompress_zipentry(entry, scratch, size) == 0)
			{
				std::string sztemp;
				char sFielName[500] = {0};
				memcpy((void*)sFielName,entry->fileName,entry->fileNameLength);
				//sprintf(sztemp,"%s/%s",sTempDir,sFielName);
				sztemp.append(sTempDir).append("/").append(sFielName);
#ifdef _WIN32
				sztemp = FileUtils::SlToBsl(sztemp);
#endif
				std::string szDir =  FileUtils::ExtractFileDir( sztemp).c_str();
				FileUtils::MakeSureDirExsits(szDir);
				FILE *pSave = fopen(sztemp.c_str(),"wb");
				if(pSave !=NULL)
				{
					fwrite(scratch, ulsize, 1, pSave);
					fclose(pSave);
				}
				else
				{
					printf("fopen faile ZipFile\n");
					std::cout<<sztemp<<std::endl;
					iRet = 0;
					break;
				}
			}
			else
			{
				printf("error decompressing file\n");
				iRet = 0;
				break;
			}
			free(scratch);
			scratch = NULL;
			entry = entry->next;
		}
	}while(0);
	if(pFile)
	{
		fclose(pFile);
	}
	if(pszbuf)
	{
		free(pszbuf);
	}
	if(zip)
	{
		release_zipfile(zip);
	}
	return iRet;
}
int ZIPDecompression_ForSinlgeFile(const char *apkpath,const char * sTempDir,const char * single_file_name)
{
    vector<string> list;
    list.push_back(single_file_name);
    return ZIPDecompression_ForMultiFile(apkpath,sTempDir,list,false);
}

int ZIPDecompression_ForMultiFile(const char *apkpath,const char * sTempDir,vector<string> list,bool isdir)
{
    int iRet = 1;
    /**
    if(FileUtils::DirExists(sTempDir))
    {
        FileUtils::DeleteDir(sTempDir);
    }*/
    FileUtils::MakeSureDirExsits(sTempDir);
    zipfile_t zip = NULL;
    void *pszbuf = NULL;
    FILE *pFile = fopen(apkpath,"rb");
    do
    {
        if(pFile == NULL)
        {
            printf("pFile == NULL\n");
            iRet = 0;
            break;
        }
        fseek(pFile, 0, SEEK_END);
        long size = ftell(pFile);
        fseek(pFile,0,SEEK_SET);

        pszbuf = malloc(size);
        if(pszbuf == NULL)
        {
            printf("pszbuf == NULL\n");
            iRet = 0;
            break;
        }
        memset(pszbuf,0,size);
        long lRead = 0;
        while (lRead != size)
        {
            lRead += fread( (char *)pszbuf + lRead, 1, size - lRead, pFile);
        }

        zip = init_zipfile(pszbuf, size);
        if(zip == NULL)
        {
            printf("zip == NULL\n");
            iRet = 0;
            break;
        }
        // get all file
        Zipfile* zipfile = (Zipfile*)zip;
        Zipentry* entry = zipfile->entries;
        //fprintf(stdout, "entryCount=%d\n", zipfile->entryCount);
        for (int i=0; i< zipfile->entryCount; i++)
        {
            bool inCopyList=false;
            for(int j=0;j<list.size();j++)
            {
                const char* single_file_name=list.at(j).c_str();
                if(isdir)
                {
                    if(strstr((char *)entry->fileName,single_file_name) == (char *)entry->fileName)
                    {
                        inCopyList=true;
                    }

                }else{
                   // int iLength = entry->fileNameLength > strlen(single_file_name)?entry->fileNameLength:strlen(single_file_name);
                   // if (0 == memcmp(single_file_name, entry->fileName, iLength))
                    char path[300]={0};
                    strncpy(path,(const char*)entry->fileName,entry->fileNameLength);
                    int ret = fnmatch(single_file_name, path, 0);//匹配成功返回0
                    if(ret == 0)
                    {
                        inCopyList=true;
                    }
                }

            }
            if(inCopyList==false)
            {
                entry = entry->next;
                continue;
            }

            //下面是需要拷贝出来的
            size_t ulsize = entry->uncompressedSize;
            if(ulsize <= 0)
            {
                if(*(entry->fileName +entry->fileNameLength -1) == '/' )
                {
                    //std::cout<<"file is directory:"<<entry->fileName<<std::endl;
                    entry = entry->next;
                    continue;
                }
            }
            long size = ulsize *1.001;
            void* scratch = NULL;
            scratch = malloc(size);
            if(scratch == NULL)
            {
                printf("scratch == NULL\n");
                iRet = 0;
                break;
            }
            memset(scratch,0,size);
            if(decompress_zipentry(entry, scratch, size) == 0)
            {
                std::string sztemp;
                char sFielName[500] = {0};
                memcpy((void*)sFielName,entry->fileName,entry->fileNameLength);
                //sprintf(sztemp,"%s/%s",sTempDir,sFielName);
#ifdef _WIN32
                bool is_utf8=IsTextUTF8(sFielName,strlen(sFielName));
                if(is_utf8)
                {
                    string strFileName=UTF8ToGBK(sFielName);
                    sztemp.append(sTempDir).append("/").append(strFileName);
                }else{
                    sztemp.append(sTempDir).append("/").append(sFielName);
                }
                sztemp = FileUtils::SlToBsl(sztemp);
#else
                sztemp.append(sTempDir).append("/").append(sFielName);
#endif
                std::string szDir =  FileUtils::ExtractFileDir( sztemp).c_str();
                FileUtils::MakeSureDirExsits(szDir);
                FILE *pSave = fopen(sztemp.c_str(),"wb");
                if(pSave !=NULL)
                {
                    fwrite(scratch, ulsize, 1, pSave);
                    fclose(pSave);
                }
                else
                {
                    printf("fopen faile ZipFile\n");
                    std::cout<<sztemp<<std::endl;
                    iRet = 0;
                    break;
                }
            }
            else
            {
                printf("error decompressing file\n");
                iRet = 0;
                break;
            }
            free(scratch);
            scratch = NULL;
            entry = entry->next;
        }
    }while(0);
    if(pFile)
    {
        fclose(pFile);
    }
    if(pszbuf)
    {
        free(pszbuf);
    }
    if(zip)
    {
        release_zipfile(zip);
    }
    return iRet;
}

int addDataOrFileToZip(const char *pZipFilePath, const char *pAddPathNameInZip, const char *pInFile, int iDataLen,bool bNewZip)
{
    int iRet = 1;
    if(!pZipFilePath || !pAddPathNameInZip || !pInFile)
    {
        return 1;
    }
    char *pAddData = NULL;

    FILE * pFile = NULL;
    if(iDataLen > 0)
    {
        pAddData = (char*)malloc(iDataLen);
        if(!pAddData)
        {
            return -1;
        }
        memset(pAddData , 0 , iDataLen);
        memcpy(pAddData , pInFile ,iDataLen);
    }
    else
    {
        pFile = fopen(pInFile,"rb");
        if(!pFile)
        {
            return -2;
        }
        pAddData = (char*)malloc(1024*100);
        if(!pAddData)
        {
            return -1;
        }
        memset(pAddData , 0x0 , sizeof(pAddData));
    }

    zipfile_t zip = NULL;
    do
    {
        // open source and destination file
        if(bNewZip)
        {
            zip = zipOpen64(pZipFilePath,APPEND_STATUS_CREATE);
        }
        else
        {
            zip = zipOpen64(pZipFilePath,APPEND_STATUS_ADDINZIP);
        }
        if(!zip)
        {
            std::cout<<"zipOpen64 failed"<<std::endl;
            iRet = -3;
            break;
        }
        zip_fileinfo FileInfo;
        memset(&FileInfo, 0,sizeof(FileInfo));
        filetime((char*)pAddPathNameInZip,&FileInfo.tmz_date,&FileInfo.dosDate);

        if (zipOpenNewFileInZip(zip, pAddPathNameInZip, &FileInfo, NULL, 0, NULL, 0, NULL, Z_DEFLATED, 9) != ZIP_OK)
        {
            printf("[%s] zipOpenNewFileInZip failed \n", pInFile);
            iRet = -3;
            break;
        }

        if(iDataLen > 0 )
        {
            if(zipWriteInFileInZip(zip,pAddData,iDataLen) < 0)
            {
                printf("[%s] zipWriteInFileInZip failed \n", pInFile);
                iRet = -3;
                break;
            }
        }
        else
        {
            while(!feof(pFile))
            {
                memset(pAddData , 0 , sizeof(pAddData));
                int size = fread(pAddData,1,sizeof(pAddData),pFile);
                if(zipWriteInFileInZip(zip,pAddData,size) < 0)
                {
                    printf("[%s] zipWriteInFileInZip failed \n", pInFile);
                    iRet = -3;
                    break;
                }
                if( ferror(pFile) )
                {
                    printf("[%s] file failed \n", pInFile);
                    iRet = -2;
                    break;
                }
            }
        }
        zipCloseFileInZip(zip);
    }while(0);

    if(zip)
    {
        zipClose(zip,(const char *)NULL);
    }
    if(pAddData)
    {
        free(pAddData);
    }
    if(NULL != pFile )
    {
        fclose(pFile);
    }
    return iRet;


}
int addFilePathToZip(const char *pZipFilePath, const char *pAddPathNameInZip, const char *pInFile,bool bNewZip)
{
    std::vector<std::string> vcPath;
    std::string sPath;
    sPath = pInFile;
    if(sPath.size() > 0)
    {
        FileUtils::GetAllFilesEntry(sPath,vcPath);
    }
    int iRet = 1;
    zipfile_t zip = NULL;
    do
    {
        char szPathInZIP [500] = {0};
        if(bNewZip)
        {
            zip = zipOpen64(pZipFilePath,APPEND_STATUS_CREATE);
        }
        else
        {
            zip = zipOpen64(pZipFilePath,APPEND_STATUS_ADDINZIP);
        }
        if(!zip)
        {
            std::cout<<"zipOpen64 failed"<<std::endl;
            iRet = -3;
            break;
        }
        zip_fileinfo FileInfo;
        memset(&FileInfo, 0,sizeof(FileInfo));

        char * pAddData = 0;
        pAddData = (char*)malloc(1024*100);
        if(!pAddData)
        {
            iRet = -1;
            break;
        }

        for(int i = 0 ; i < vcPath.size() ; i++)
        {
            std::string s = vcPath[i];
            if(strlen(pAddPathNameInZip)> 0)
            {
                s = s.substr(strlen(pInFile));
                sprintf(szPathInZIP,"%s%s",pAddPathNameInZip,s.c_str());
                s = szPathInZIP;
            }
            else
            {
                s = s.substr(strlen(pInFile) + 1);
            }
            s = FileUtils::BslToSl(s);

            FILE * pFile = fopen(vcPath[i].c_str(),"rb");
            if(!pFile)
            {
                iRet = -2;
                break;
            }
            memset(&FileInfo, 0,sizeof(FileInfo));
            filetime((char*)vcPath[i].c_str(),&FileInfo.tmz_date,&FileInfo.dosDate);
            if (zipOpenNewFileInZip(zip, s.c_str(), &FileInfo, NULL, 0, NULL, 0, NULL, Z_DEFLATED, 9) != ZIP_OK)
            {
                std::cout<<"zipOpenNewFileInZip failed"<<std::endl;
                iRet = -3;
                break;
            }

            while( !feof(pFile) )
            {
                memset(pAddData , 0 , 1024*100);
                int size = fread(pAddData,1,sizeof(pAddData),pFile);
                if(zipWriteInFileInZip(zip,pAddData,size) < 0)
                {
                    printf("[%s] zipWriteInFileInZip failed \n", s);
                    iRet = -3;
                    break;
                }
                if( ferror(pFile) )
                {
                    printf("[%s] zipWriteInFileInZip failed \n", s);
                    iRet = -2;
                    break;
                }
            }
            fclose(pFile);

            zipCloseFileInZip(zip);
        }

        if(pAddData)
        {
            free(pAddData);
            pAddData = NULL;
        }

    }while(0);

    if(zip)
    {
        zipClose(zip,(const char *)NULL);
    }
    return iRet;
}


int FileOrPathInZip(const char *pZipFilePath, const char *pPathNameInZip ,bool bDir)
{
    int iRet = 1;
    zipfile_t zip = NULL;
    void *pszbuf = NULL;
    FILE *pFile = fopen(pZipFilePath,"rb");
    if(pFile == NULL)
    {
        iRet = 0;
    }
    fseek(pFile, 0, SEEK_END);
    long size = ftell(pFile);
    fseek(pFile,0,SEEK_SET);
    pszbuf = malloc(size);
    memset(pszbuf,0,size);
    long lRead = 0;
    while (lRead != size)
    {
        lRead += fread( (char *)pszbuf + lRead, 1, size - lRead, pFile);
    }
    fclose(pFile);
    zip = init_zipfile(pszbuf, size);
    if(zip == NULL)
    {
        iRet = 0;
        free(pszbuf);
        return iRet;
    }
    if(bDir)
    {
        if(!lookup_Dir(zip,pPathNameInZip))
        {
            iRet = 0;
        }
    }else{
        if(!lookup_zipentry(zip,pPathNameInZip))
        {
            iRet = 0;
        }
    }
    if(pszbuf)
    {
        free(pszbuf);
    }
    //No memory leaks detected.
    if(zip)
    {
        release_zipfile(zip);
    }

    return iRet;
}




int DeleteFileFromZip(const char *pZipFilePath, const char *pDeletePathNameInZip ,bool bDir)
{
    int iRet = 1;
    zipfile_t zip = NULL;
    void *pszbuf = NULL;
    void *pTempBuf = NULL;
    FILE *pFile = fopen(pZipFilePath,"rb");
    do
    {
        if(pFile == NULL)
        {
            iRet = 0;
            break;
        }
        fseek(pFile, 0, SEEK_END);
        long size = ftell(pFile);
        fseek(pFile,0,SEEK_SET);

        pszbuf = malloc(size);
        pTempBuf = malloc(size);
        if(pszbuf == NULL || pTempBuf == NULL)
        {
            iRet = 0;
            break;
        }
        memset(pszbuf,0,size);
        memset(pTempBuf,0,size);

        long lRead = 0;
        while (lRead != size)
        {
            lRead += fread( (char *)pszbuf + lRead, 1, size - lRead, pFile);
        }
        zip = init_zipfile(pszbuf, size);
        if(zip == NULL)
        {
            iRet = 0;
            break;
        }
        if(bDir)
        {
            if(!lookup_Dir(zip,pDeletePathNameInZip))
            {
                std::cout<<"can not find DIR: "<< pDeletePathNameInZip <<std::endl;
                iRet = 0;
                break;
            }
        }
        else
        {
            if(!lookup_zipentry(zip,pDeletePathNameInZip))
            {
                std::cout<<"can not find FILE:"<< pDeletePathNameInZip <<std::endl;
                iRet = 0;
                break;
            }
        }
        unsigned int   iDeleteDataSize   = 0;
        int            iTotalDeleteCount = 0;
        unsigned long  lAlreadyWriteSize = 0;

        Zipfile* zipfile = (Zipfile*)zip;
        Zipentry* entry = zipfile->entries;
        ZipentryCenteral * centeral = zipfile->centeral;
        Zipentry* entryNext = NULL;
        //fclose(pFile);
        //pFile = fopen(pZipFilePath,"wb");

        // step 1 write data entry section into file;
        for (int i=0; i< zipfile->entryCount; i++)
        {
            int iTempDataLen = 0;
            if(i == 0)
            {
                lAlreadyWriteSize += centeral->localHeaderRelOffset;
                //add by temp buffer
                memcpy((void*)pTempBuf,(void *)(zipfile->buf),lAlreadyWriteSize);
            }
            if(i != zipfile->entryCount -1)
            {
               entryNext = entry->next;
               iTempDataLen = entryNext->localHeaderRelOffset - entry->localHeaderRelOffset;
            }
            else
            {
                iTempDataLen = entry->iDataSectionLen;
            }

            if(bIsFindFile(entry,pDeletePathNameInZip,bDir))
            {
//                iDeleteDataSize += entry->iDataSectionLen;
                iDeleteDataSize += iTempDataLen;
                entry = entry->next;
                iTotalDeleteCount++;
                continue;
            }
            memcpy((void*) ( (char*)pTempBuf + lAlreadyWriteSize),
                   (void *)(zipfile->buf + entry->localHeaderRelOffset),iTempDataLen);
//            if(iDeleteDataSize > 0)
//            {
//                memcpy( (void *)(zipfile->buf + lAlreadyWriteSize),
//                        zipfile->buf + lAlreadyWriteSize + iDeleteDataSize,iTempDataLen);
//            }
            lAlreadyWriteSize += iTempDataLen;

            entry = entry->next;
        }
        //memcpy((void*)zipfile->buf,(void*)pTempBuf,lAlreadyWriteSize);


        // step 2 write dir central section into file;
        //if user add some no ZIP used data to file  we need copy them
        char szHeader[4]  = {0x50,0x4b,0x01,0x02};
        while(1)
        {
            if(memcmp(zipfile->buf + lAlreadyWriteSize + iDeleteDataSize,szHeader,4) == 0)
            {
                break;
            }
            else
            {
                //memcpy( (void *)(zipfile->buf + lAlreadyWriteSize),zipfile->buf + lAlreadyWriteSize + iDeleteDataSize,1);
                memcpy( (char*)pTempBuf + lAlreadyWriteSize  ,zipfile->buf + lAlreadyWriteSize + iDeleteDataSize,1);
                lAlreadyWriteSize ++;
            }
        }

        unsigned long localHeaderRelOffset = 0;
        zipfile->centralDirSize = 0;
        entry = zipfile->entries;
        for (int i = 0; i< zipfile->entryCount; i++)
        {
            if(i == 0)
            {
                localHeaderRelOffset = centeral->localHeaderRelOffset;
            }
            char szbuf[46] = {0};
            if(!bIsFindFile(entry,pDeletePathNameInZip,bDir))
            {
                int j = 0;
                memcpy(szbuf,&centeral->header,sizeof(centeral->header));
                j += sizeof(centeral->header);

                memcpy(szbuf + j,&centeral->versionMadeBy,sizeof(centeral->versionMadeBy));
                j += sizeof(centeral->versionMadeBy);


                memcpy(szbuf + j,&centeral->versionToExtract,sizeof(centeral->versionToExtract));
                j += sizeof(centeral->versionToExtract);

                memcpy(szbuf + j,&centeral->GPBitFlag,sizeof(centeral->GPBitFlag));
                j += sizeof(centeral->GPBitFlag);

                memcpy(szbuf + j,&centeral->compressionMethod,sizeof(centeral->compressionMethod));
                j += sizeof(centeral->compressionMethod);

                memcpy(szbuf + j ,&centeral->lastModFileTime,sizeof(centeral->lastModFileTime));
                j += sizeof(centeral->versionToExtract);

                memcpy(szbuf + j,&centeral->lastModFileDate,sizeof(centeral->lastModFileDate));
                j += sizeof(centeral->lastModFileDate);

                memcpy(szbuf + j,&centeral->CRC32,sizeof(centeral->CRC32));
                j += sizeof(centeral->CRC32);

                memcpy(szbuf + j,&centeral->compressedSize,sizeof(centeral->compressedSize));
                j += sizeof(centeral->compressedSize);

                memcpy(szbuf + j,&centeral->uncompressedSize,sizeof(centeral->uncompressedSize));
                j += sizeof(centeral->uncompressedSize);

                memcpy(szbuf + j,&centeral->fileNameLength,sizeof(centeral->fileNameLength));
                j += sizeof(centeral->fileNameLength);

                memcpy(szbuf + j,&centeral->extraFieldLength,sizeof(centeral->extraFieldLength));
                j += sizeof(centeral->extraFieldLength);

                memcpy(szbuf + j,&centeral->fileCommentLength,sizeof(centeral->fileCommentLength));
                j += sizeof(centeral->fileCommentLength);

                memcpy(szbuf + j,&centeral->diskNumberStart,sizeof(centeral->diskNumberStart));
                j += sizeof(centeral->diskNumberStart);

                memcpy(szbuf + j,&centeral->internalAttrs,sizeof(centeral->internalAttrs));
                j += sizeof(centeral->internalAttrs);

                memcpy(szbuf + j,&centeral->externalAttrs,sizeof(centeral->externalAttrs));
                j += sizeof(centeral->externalAttrs);

                unsigned long  RelOffset = centeral->localHeaderRelOffset - localHeaderRelOffset;

                memcpy(szbuf + j,&RelOffset,sizeof(centeral->localHeaderRelOffset));
                j += sizeof(centeral->localHeaderRelOffset);

                //memcpy( (void*)(zipfile->buf + lAlreadyWriteSize) ,szbuf , j);
                memcpy( (void*)((char*)pTempBuf + lAlreadyWriteSize) ,szbuf,j);
                lAlreadyWriteSize += j;

                if(centeral->fileNameLength > 0)
                {
                    //fwrite(centeral->fileName,1,centeral->fileNameLength,pFile);
                    //memcpy( (void*)(zipfile->buf + lAlreadyWriteSize) ,centeral->fileName , centeral->fileNameLength);
                    memcpy( (void*)((char*)pTempBuf + lAlreadyWriteSize) ,centeral->fileName , centeral->fileNameLength);
                    lAlreadyWriteSize += centeral->fileNameLength;
                }
                if(centeral->extraFieldLength > 0)
                {
                   // fwrite(centeral->extraField,1,centeral->extraFieldLength,pFile);
                    //memcpy( (void*)(zipfile->buf + lAlreadyWriteSize) ,centeral->extraField , centeral->extraFieldLength);
                    memcpy( (void*)((char*)pTempBuf + lAlreadyWriteSize) ,centeral->extraField , centeral->extraFieldLength);
                    lAlreadyWriteSize += centeral->extraFieldLength;
                }
                if(centeral->fileCommentLength > 0)
                {
                    //fwrite(centeral->fileComment,1,centeral->fileCommentLength,pFile);
                    //memcpy( (void*)(zipfile->buf + lAlreadyWriteSize) ,centeral->fileComment , centeral->fileCommentLength);
                    memcpy( (void*)((char*)pTempBuf + lAlreadyWriteSize) ,centeral->fileComment , centeral->fileCommentLength);
                    lAlreadyWriteSize += centeral->fileCommentLength;
                }
                zipfile->centralDirSize += 46 + centeral->fileNameLength
                        + centeral->extraFieldLength + centeral->fileCommentLength;
            }
            else
            {
                if(centeral->next)
                {
                    localHeaderRelOffset += centeral->next->localHeaderRelOffset - centeral->localHeaderRelOffset;
                }
            }
            entry = entry->next;
            centeral = centeral->next;
        }

        memcpy((void*)zipfile->buf,(void*)pTempBuf,lAlreadyWriteSize);

        if(iRet == 0)
        {
            break;
        }
        // step 3 write end of central section into file;
        char szBufEnd[22] = {0};
        long kSignature = 0x06054b50;
        memcpy(szBufEnd ,&kSignature, 4 );
        unsigned short entryCount = zipfile->entryCount - iTotalDeleteCount;
        unsigned short totalEntryCount = zipfile->totalEntryCount - iTotalDeleteCount;
        unsigned int centralDirOffest = zipfile->centralDirOffest - iDeleteDataSize;
        memcpy(szBufEnd + 4 ,&zipfile->disknum , 2);
        memcpy(szBufEnd + 6 ,&zipfile->diskWithCentralDir , 2);
        memcpy(szBufEnd + 8 ,&entryCount, 2);
        memcpy(szBufEnd + 10 ,&totalEntryCount, 2);
        memcpy(szBufEnd + 12 ,&zipfile->centralDirSize , 4);
        memcpy(szBufEnd + 16 ,&centralDirOffest , 4);
        memcpy(szBufEnd + 20 ,&zipfile->commentLen , 2);

//        if (fwrite(szBufEnd, 1, 22, pFile) != 22)
//        {
//            iRet = 0;
//            break;
//        }

        memcpy( (void*)(zipfile->buf + lAlreadyWriteSize) ,szBufEnd , 22);
        lAlreadyWriteSize += 22;

        if (zipfile->commentLen > 0)
        {
//            if (fwrite(zipfile->comment, zipfile->commentLen, 1, pFile) != zipfile->commentLen)
//            {
//                iRet = 0;
//                break;
//            }
            memcpy( (void*)(zipfile->buf + lAlreadyWriteSize) ,zipfile->comment , zipfile->commentLen);
            lAlreadyWriteSize += zipfile->commentLen;
        }
        if(iRet)
        {
            // last write right data into file
            fclose(pFile);
            pFile = fopen(pZipFilePath,"wb");
            if(lAlreadyWriteSize != fwrite( zipfile->buf,1,lAlreadyWriteSize,pFile))
            {
                iRet =0;
                break;

            }
        }

    }while(0);
    if(pFile)
    {
        fclose(pFile);
    }
    if(pszbuf)
    {
        free(pszbuf);
    }
    if(pTempBuf)
    {
        free(pTempBuf);
    }
    if(zip)
    {
        release_zipfile(zip);
    }
    return iRet;
}
int DeleteFilesFromZip(const char *pZipFilePath, std::vector<std::string> vcDeletePath)
{
    int iRet = 1;
    zipfile_t zip = NULL;
    void *pszbuf = NULL;
    void *pTempBuf =  NULL;
    FILE *pFile = fopen(pZipFilePath,"rb");
    do
    {
        if(pFile == NULL)
        {
            iRet = 0;
            break;
        }
        fseek(pFile, 0, SEEK_END);
        long size = ftell(pFile);
        fseek(pFile,0,SEEK_SET);

        pszbuf = malloc(size);
        pTempBuf = malloc(size);
        if(pszbuf == NULL || pTempBuf == NULL)
        {
            iRet = 0;
            break;
        }
        memset(pszbuf,0,size);
        memset(pTempBuf,0,size);
        long lRead = 0;
        while (lRead != size)
        {
            lRead += fread( (char *)pszbuf + lRead, 1, size - lRead, pFile);
        }
        zip = init_zipfile(pszbuf, size);
        if(zip == NULL)
        {
            iRet = 0;
            break;
        }
        unsigned int   iDeleteDataSize   = 0;
        int            iTotalDeleteCount = 0;
        unsigned long  lAlreadyWriteSize = 0;
        //unsigned int   iCenteralOffsite = 0;

        Zipfile* zipfile = (Zipfile*)zip;
        Zipentry* entry = zipfile->entries;
        Zipentry* entryNext = NULL;
        ZipentryCenteral * centeral = zipfile->centeral;

        // step 1 write data entry section into file;
        for (int i=0; i< zipfile->entryCount; i++)
        {
            int iTempDataLen = 0;
            if(i == 0)
            {
                lAlreadyWriteSize += centeral->localHeaderRelOffset;
                //add by temp buffer
                memcpy((void*)pTempBuf,(void *)(zipfile->buf),lAlreadyWriteSize);
            }
            if(i != zipfile->entryCount -1)
            {
               entryNext = entry->next;
               iTempDataLen = entryNext->localHeaderRelOffset - entry->localHeaderRelOffset;
            }
            else
            {
                iTempDataLen = entry->iDataSectionLen;
            }
            if(bIsIncudeFindFile(entry,vcDeletePath))
            {
               // iDeleteDataSize += entry->iDataSectionLen;
                iDeleteDataSize += iTempDataLen;
                entry = entry->next;
                iTotalDeleteCount++;
                continue;
            }

            memcpy((void*) ( (char*)pTempBuf + lAlreadyWriteSize),
                   (void *)(zipfile->buf + entry->localHeaderRelOffset),iTempDataLen);

//            if(iDeleteDataSize > 0)
//            {
//                memcpy( (void *)(zipfile->buf + lAlreadyWriteSize),
//                        zipfile->buf + lAlreadyWriteSize + iDeleteDataSize,iTempDataLen);
//            }
            lAlreadyWriteSize += iTempDataLen;
            entry = entry->next;
        }
        //memcpy((void*)zipfile->buf,(void*)pTempBuf,lAlreadyWriteSize);

        // step 2 write dir central section into file;

        //if user add some no ZIP used data to file  we need copy them
        char szHeader[4]  = {0x50,0x4b,0x01,0x02};
        while(1)
        {
            if(memcmp(zipfile->buf + lAlreadyWriteSize + iDeleteDataSize,szHeader,4) == 0)
            {
                break;
            }
            else
            {
                //memcpy( (void *)(zipfile->buf + lAlreadyWriteSize),zipfile->buf + lAlreadyWriteSize + iDeleteDataSize,1);
                memcpy( (void *)((char*)pTempBuf + lAlreadyWriteSize),zipfile->buf + lAlreadyWriteSize + iDeleteDataSize,1);
                lAlreadyWriteSize ++;
            }
        }
        unsigned long localHeaderRelOffset = 0;
        zipfile->centralDirSize = 0;
        entry = zipfile->entries;
        for (int i = 0; i< zipfile->entryCount; i++)
        {
            if(i == 0)
            {
                localHeaderRelOffset = centeral->localHeaderRelOffset;
            }
            char szbuf[46] = {0};
            if(!bIsIncudeFindFile(entry,vcDeletePath))
            {
                int j = 0;
                memcpy(szbuf,&centeral->header,sizeof(centeral->header));
                j += sizeof(centeral->header);

                memcpy(szbuf + j,&centeral->versionMadeBy,sizeof(centeral->versionMadeBy));
                j += sizeof(centeral->versionMadeBy);


                memcpy(szbuf + j,&centeral->versionToExtract,sizeof(centeral->versionToExtract));
                j += sizeof(centeral->versionToExtract);

                memcpy(szbuf + j,&centeral->GPBitFlag,sizeof(centeral->GPBitFlag));
                j += sizeof(centeral->GPBitFlag);

                memcpy(szbuf + j,&centeral->compressionMethod,sizeof(centeral->compressionMethod));
                j += sizeof(centeral->compressionMethod);

                memcpy(szbuf + j ,&centeral->lastModFileTime,sizeof(centeral->lastModFileTime));
                j += sizeof(centeral->versionToExtract);

                memcpy(szbuf + j,&centeral->lastModFileDate,sizeof(centeral->lastModFileDate));
                j += sizeof(centeral->lastModFileDate);

                memcpy(szbuf + j,&centeral->CRC32,sizeof(centeral->CRC32));
                j += sizeof(centeral->CRC32);

                memcpy(szbuf + j,&centeral->compressedSize,sizeof(centeral->compressedSize));
                j += sizeof(centeral->compressedSize);

                memcpy(szbuf + j,&centeral->uncompressedSize,sizeof(centeral->uncompressedSize));
                j += sizeof(centeral->uncompressedSize);

                memcpy(szbuf + j,&centeral->fileNameLength,sizeof(centeral->fileNameLength));
                j += sizeof(centeral->fileNameLength);

                memcpy(szbuf + j,&centeral->extraFieldLength,sizeof(centeral->extraFieldLength));
                j += sizeof(centeral->extraFieldLength);

                memcpy(szbuf + j,&centeral->fileCommentLength,sizeof(centeral->fileCommentLength));
                j += sizeof(centeral->fileCommentLength);

                memcpy(szbuf + j,&centeral->diskNumberStart,sizeof(centeral->diskNumberStart));
                j += sizeof(centeral->diskNumberStart);

                memcpy(szbuf + j,&centeral->internalAttrs,sizeof(centeral->internalAttrs));
                j += sizeof(centeral->internalAttrs);

                memcpy(szbuf + j,&centeral->externalAttrs,sizeof(centeral->externalAttrs));
                j += sizeof(centeral->externalAttrs);

                unsigned long  RelOffset = centeral->localHeaderRelOffset - localHeaderRelOffset;

                memcpy(szbuf + j,&RelOffset,sizeof(centeral->localHeaderRelOffset));
                j += sizeof(centeral->localHeaderRelOffset);

                //memcpy( (void*)(zipfile->buf + lAlreadyWriteSize) ,szbuf , j);
                memcpy( (void*)((char*)pTempBuf + lAlreadyWriteSize) ,szbuf , j);
                lAlreadyWriteSize += j;

                if(centeral->fileNameLength > 0)
                {
                    //memcpy( (void*)(zipfile->buf + lAlreadyWriteSize) ,centeral->fileName , centeral->fileNameLength);
                    memcpy((void*)((char*)pTempBuf + lAlreadyWriteSize) ,centeral->fileName , centeral->fileNameLength);
                    lAlreadyWriteSize += centeral->fileNameLength;
                }
                if(centeral->extraFieldLength > 0)
                {
                    //memcpy( (void*)(zipfile->buf + lAlreadyWriteSize) ,centeral->extraField , centeral->extraFieldLength);
                    memcpy( (void*)((char*) pTempBuf + lAlreadyWriteSize) ,centeral->extraField , centeral->extraFieldLength);
                    lAlreadyWriteSize += centeral->extraFieldLength;
                }
                if(centeral->fileCommentLength > 0)
                {
                    //memcpy( (void*)(zipfile->buf + lAlreadyWriteSize) ,centeral->fileComment , centeral->fileCommentLength);
                    memcpy( (void*)((char*)pTempBuf+ lAlreadyWriteSize) ,centeral->fileComment , centeral->fileCommentLength);
                    lAlreadyWriteSize += centeral->fileCommentLength;
                }
                zipfile->centralDirSize += 46 + centeral->fileNameLength
                        + centeral->extraFieldLength + centeral->fileCommentLength;
            }
            else
            {
                if(centeral->next)
                {
                    localHeaderRelOffset += centeral->next->localHeaderRelOffset - centeral->localHeaderRelOffset;
                }
            }
            entry = entry->next;
            centeral = centeral->next;
        }
        memcpy((void*)zipfile->buf,(void*)pTempBuf,lAlreadyWriteSize);
        if(iRet == 0)
        {
            break;
        }
        // step 3 write end of central section into file;
        char szBufEnd[22] = {0};
        long kSignature = 0x06054b50;
        memcpy(szBufEnd ,&kSignature, 4 );
        unsigned short entryCount = zipfile->entryCount - iTotalDeleteCount;
        unsigned short totalEntryCount = zipfile->totalEntryCount - iTotalDeleteCount;
        unsigned int centralDirOffest = zipfile->centralDirOffest - iDeleteDataSize;
        memcpy(szBufEnd + 4 ,&zipfile->disknum , 2);
        memcpy(szBufEnd + 6 ,&zipfile->diskWithCentralDir , 2);
        memcpy(szBufEnd + 8 ,&entryCount, 2);
        memcpy(szBufEnd + 10 ,&totalEntryCount, 2);
        memcpy(szBufEnd + 12 ,&zipfile->centralDirSize , 4);
        memcpy(szBufEnd + 16 ,&centralDirOffest , 4);
        memcpy(szBufEnd + 20 ,&zipfile->commentLen , 2);

        memcpy( (void*)(zipfile->buf + lAlreadyWriteSize) ,szBufEnd , 22);
        lAlreadyWriteSize += 22;

        if (zipfile->commentLen > 0)
        {

            memcpy( (void*)(zipfile->buf + lAlreadyWriteSize) ,zipfile->comment , zipfile->commentLen);
            lAlreadyWriteSize += zipfile->commentLen;
        }

        if(iRet)
        {
            // last write right data into file
            fclose(pFile);
            pFile = fopen(pZipFilePath,"w+b");
            if(lAlreadyWriteSize != fwrite( zipfile->buf,1,lAlreadyWriteSize,pFile))
            {
                iRet =0;
                break;
            }
        }

    }while(0);
    if(pFile)
    {
        fclose(pFile);
    }
    if(pszbuf)
    {
        free(pszbuf);
    }
    if(pTempBuf)
    {
        free(pTempBuf);
    }
    if(zip)
    {
        release_zipfile(zip);
    }
    return iRet;
}

bool bIsFindFile(Zipentry *entry, const char *pfilepath, bool bDir)
{
    int iLen = strlen(pfilepath);

    if(bDir)
    {
        char szPath[500] = {0};
        if(pfilepath[iLen -1] != '/')
        {
            sprintf(szPath,"%s/",pfilepath);
            iLen ++;
        }
        else
        {
            strcpy(szPath,pfilepath);
        }
        if(entry->fileNameLength >= iLen && memcmp((void *)entry->fileName,szPath,iLen) == 0)
        {
            return 1;
        }
    }
    else
    {
        if(entry->fileNameLength == iLen && memcmp((void *)entry->fileName,pfilepath,iLen) == 0)
        {
            return 1;
        }
    }
    return 0;
}
bool bIsIncudeFindFile(Zipentry *entry, std::vector<std::string> vcDeletePath)
{
    for(int i = 0 ; i< vcDeletePath.size() ; i ++)
    {
        if(entry->fileNameLength == vcDeletePath[i].size()
                && memcmp((void *)entry->fileName,FileUtils::BslToSl(vcDeletePath[i]).c_str(),vcDeletePath[i].size()) == 0)
        {
            return 1;
        }
    }
    return 0;
}
#ifdef _WIN32
uLong filetime(char *f, tm_zip * tmzip, uLong * dt)
{
  int ret = 0;
  {
      FILETIME ftLocal;
      HANDLE hFind;
      WIN32_FIND_DATAA ff32;

      hFind = FindFirstFileA(f,&ff32);
      if (hFind != INVALID_HANDLE_VALUE)
      {
        FileTimeToLocalFileTime(&(ff32.ftLastWriteTime),&ftLocal);
        FileTimeToDosDateTime(&ftLocal,((LPWORD)dt)+1,((LPWORD)dt)+0);
        FindClose(hFind);
        ret = 1;
      }
  }
  return ret;
}
#else
#ifdef unix || __APPLE__
#define MAXFILENAME 256
uLong filetime(char * f, tm_zip * tmzip, uLong * dt)
{
  int ret=0;
  struct stat s;        /* results of stat() */
  struct tm* filedate;
  time_t tm_t=0;

  if (strcmp(f,"-")!=0)
  {
    char name[MAXFILENAME+1];
    int len = strlen(f);
    if (len > MAXFILENAME)
      len = MAXFILENAME;

    strncpy(name, f,MAXFILENAME-1);
    /* strncpy doesnt append the trailing NULL, of the string is too long. */
    name[ MAXFILENAME ] = '\0';

    if (name[len - 1] == '/')
      name[len - 1] = '\0';
    /* not all systems allow stat'ing a file with / appended */
    if (stat(name,&s)==0)
    {
      tm_t = s.st_mtime;
      ret = 1;
    }
  }
  filedate = localtime(&tm_t);

  tmzip->tm_sec  = filedate->tm_sec;
  tmzip->tm_min  = filedate->tm_min;
  tmzip->tm_hour = filedate->tm_hour;
  tmzip->tm_mday = filedate->tm_mday;
  tmzip->tm_mon  = filedate->tm_mon ;
  tmzip->tm_year = filedate->tm_year;

  return ret;
}
#else
uLong filetime(char * f, tm_zip * tmzip, uLong * dt)
{
    return 0;
}
#endif
#endif




int ZIPDecompressionFile(const char *apkpath, const char *sTempDir, char *pFilePath)
{
    int iRet = 0;
    FileUtils::MakeSureDirExsits(sTempDir);
    zipfile_t zip = NULL;
    void *pszbuf = NULL;
    FILE *pFile = fopen(apkpath,"rb");
    do
    {
        if(pFile == NULL)
        {
            iRet = 0;
            break;
        }
        fseek(pFile, 0, SEEK_END);
        long size = ftell(pFile);
        fseek(pFile,0,SEEK_SET);

        pszbuf = malloc(size);
        if(pszbuf == NULL)
        {
            iRet = 0;
            break;
        }
        memset(pszbuf,0,size);
        long lRead = 0;
        while (lRead != size)
        {
            lRead += fread( (char *)pszbuf + lRead, 1, size - lRead, pFile);
        }

        zip = init_zipfile(pszbuf, size);
        if(zip == NULL)
        {
            iRet = 0;
            break;
        }
        // get all file
        Zipfile* zipfile = (Zipfile*)zip;
        Zipentry* entry = zipfile->entries;
        fprintf(stdout, "entryCount=%d\n", zipfile->entryCount);
        for (int i=0; i< zipfile->entryCount; i++)
        {
            size_t ulsize = entry->uncompressedSize;
            if(ulsize <= 0)
            {
                if(*(entry->fileName +entry->fileNameLength -1) == '/' )
                {
                    std::cout<<"file is directory:"<<entry->fileName<<std::endl;
                    entry = entry->next;
                    continue;
                }
            }
            long size = ulsize *1.001;
            void* scratch = NULL;
            bool bFind  = false;
            if(strstr((const char *)entry->fileName,pFilePath) && strlen(pFilePath) == entry->fileNameLength)
            {
                scratch = malloc(size);
                if(scratch == NULL)
                {
                    iRet = 0;
                    break;
                }
                memset(scratch,0,size);
                if(decompress_zipentry(entry, scratch, size) == 0)
                {
                    std::string sztemp;
                    char sFielName[500] = {0};
                    memcpy((void*)sFielName,entry->fileName,entry->fileNameLength);
                    //sprintf(sztemp,"%s/%s",sTempDir,sFielName);
                    sztemp.append(sTempDir).append("/").append(sFielName);
    #ifdef _WIN32
                    sztemp = FileUtils::SlToBsl(sztemp);
    #endif
                    std::string szDir =  FileUtils::ExtractFileDir( sztemp).c_str();
                    FileUtils::MakeSureDirExsits(szDir);
                    FILE *pSave = fopen(sztemp.c_str(),"wb");
                    if(pSave !=NULL)
                    {
                        fwrite(scratch, ulsize, 1, pSave);
                        fclose(pSave);
                        bFind = true;
                    }
                    else
                    {
                        printf("fopen faile ZipFile\n");
                        std::cout<<sztemp<<std::endl;
                        iRet = 0;
                        break;
                    }
                }
                else
                {
                    printf("error decompressing file\n");
                    iRet = 0;
                    break;
                }


            }
            if(scratch)
            {
                free(scratch);
                scratch = NULL;
            }
            entry = entry->next;
            if(bFind)
            {
                iRet = 1;
                break;
            }
        }
    }while(0);
    if(pFile)
    {
        fclose(pFile);
    }
    if(pszbuf)
    {
        free(pszbuf);
    }
    if(zip)
    {
        release_zipfile(zip);
    }
    return iRet;
}



int LoadFileList(const char* pzipFilePack, vector<typeFileInfo> *pvecFileList)
{
    int iRet = 0;
    void* ts;
    unzFile unzfile = NULL;
    bool bFind = false;
    int nReturnValue = 0;
    char szZipFName[MAX_PATH];

    std::string teeee = "sdasdasdasd";
    teeee.c_str();

    unz_file_info* pFileInfo = new unz_file_info;
    unz_global_info* pGlobalInfo = new unz_global_info;
    //string stee = std::locale::global(std::locale(pzipFilePack.c_str())).name().c_str();
    unzfile = unzOpen(pzipFilePack);
    if(unzfile == NULL)
    {
        //LOG_WRITE_EX(_T("打开压缩文件失败") );
        return -2;
    }
    nReturnValue = unzGetGlobalInfo(unzfile, pGlobalInfo);
    if(nReturnValue != UNZ_OK)
    {
        //LOG_WRITE_EX(_T("获取压缩文件信息失败"));
        unzClose(unzfile);
        return -3;
    }
    do
    {
        for(int i=0; i<pGlobalInfo->number_entry; i++)
        {
            //解析得到zip中的文件信息
            iRet = unzGetCurrentFileInfo(unzfile, pFileInfo, szZipFName, MAX_PATH,
                NULL, 0, NULL, 0);
            if(iRet != UNZ_OK)
            {
                //LOG_WRITE_EX(_T("解压文件节点失败!") );
                iRet = 2;
                break;
            }
            switch(pFileInfo->external_fa)
            {
            case FILE_ATTRIBUTE_DIRECTORY:
                break;
            default:
                pvecFileList->push_back(szZipFName);
                break;
            }
            unzGoToNextFile(unzfile);
        }
    } while (0);
    if(NULL != unzfile)
    {
        unzClose(unzfile);
        unzfile = NULL;
    }
    delete pGlobalInfo ;
    delete pFileInfo;

    return iRet;
}

int CheckFile(const char *strFilePath, char *strZipFilePath)
{
    int iRet = 0;
    if (NULL == strZipFilePath || NULL == strZipFilePath)
    {
        return 5;
    }

    unzFile unzfile = NULL;
    bool bFind = false;
    int nReturnValue = 0;
    char szZipFName[MAX_PATH];
    unz_file_info* pFileInfo = new unz_file_info;
    unz_global_info* pGlobalInfo = new unz_global_info;
    //locale loc = locale::global(locale(""));
    unzfile = unzOpen(strZipFilePath);
    //locale::global(loc);
    if(unzfile == NULL)
    {
        //LOG_WRITE_EX(_T("打开压缩文件失败") );
        return -2;
    }
    nReturnValue = unzGetGlobalInfo(unzfile, pGlobalInfo);
    if(nReturnValue != UNZ_OK)
    {
        //LOG_WRITE_EX(_T("获取压缩文件信息失败"));
        unzClose(unzfile);
        return -3;
    }
    do
    {
        for(int i=0; i<pGlobalInfo->number_entry; i++)
        {
            //解析得到zip中的文件信息
            iRet = unzGetCurrentFileInfo(unzfile, pFileInfo, szZipFName, MAX_PATH,
                NULL, 0, NULL, 0);
            if(iRet != UNZ_OK)
            {
                //LOG_WRITE_EX(_T("解压文件节点失败!") );
                iRet = 2;
                break;
            }
            switch(pFileInfo->external_fa)
            {
            case FILE_ATTRIBUTE_DIRECTORY:
                break;
            default:
                {
                    if (0 == strcmp(szZipFName,strFilePath))
                    {
                        iRet = 0;
                        bFind = true;
                        break;
                    }
                }
            }
            if (0 < iRet)
            {
                break;
            }
            if (bFind)
            {
                break;
            }
            unzGoToNextFile(unzfile);
        }
    } while (0);
    if(NULL != unzfile)
    {
        unzClose(unzfile);
        unzfile = NULL;
    }
    delete pGlobalInfo ;
    delete pFileInfo;
    return bFind?0:1;
}


int checkZipFile(Zipfile* file, const char* strZipFilePath, bool isDir)
{
    int iRet = 1;
    Zipentry* entry = file->entries;
    int iLen = strlen(strZipFilePath);
    do
    {
        if(NULL == entry)
        {
            break;
        }
        if(isDir)
        {
            char szPath[512] = {0};
            if(strZipFilePath[iLen -1] != '/')
            {
                sprintf(szPath,"%s/",strZipFilePath);
                iLen++;
            }
            else
            {
                strcpy(szPath,strZipFilePath);
            }
            if(0 == strncmp((char*)entry->fileName, szPath, iLen) )
            {
                return 0;
            }
        }
        else
        {
            if(0 == strcmp((char*)entry->fileName, strZipFilePath) )
            {
                return 0;
            }
        }
        entry = entry->next;
    }while(1);

    return iRet ;
}

int checkZipFileEx(const char* strFileName, const char* strZipFilePath, bool isDir)
{
    int iRet = 1;
    if(isDir)
    {
        int iLen = strlen(strZipFilePath);
        char szPath[512] = {0};
        if(strZipFilePath[iLen -1] != '/')
        {
            sprintf(szPath,"%s/",strZipFilePath);
            iLen++;
        }
        else
        {
            strcpy(szPath,strZipFilePath);
        }
        if(0 == strncmp(strFileName, szPath, iLen) )
        {
            return 0;
        }
    }
    else
    {
        char path[300]={0};
        strncpy(path,(const char*)strFileName,strlen(strFileName));
        int ret = fnmatch(strZipFilePath, path, 0);//匹配成功返回0
        if(ret == 0)
        {
            return 0;
        }
//        if(0 == strcmp(strFileName, strZipFilePath) )
//        {
//            return 0;
//        }
    }
    return iRet;
}

// 解压文件
int DecompressionFile(const char *pstrFilePath, const char *pstrSavePath, const char *strZipFilePath)
{
    if (NULL == pstrFilePath || NULL == pstrSavePath || NULL == strZipFilePath)
    {
        return 1;
    }
    if (0 == strlen(pstrFilePath) || 0 == strlen(pstrSavePath) || 0 == strlen(strZipFilePath) )
    {
        return 1;
    }

    std::string strFileDir = FileUtils::ExtractFileDir(pstrSavePath);
    std::string strFileName= FileUtils::ExtractFileName(pstrSavePath);

    unzFile unzfile =NULL;
    int iRet = -1;
    int nReturnValue = 0;
    char szZipFName[MAX_PATH];
    bool bDone = false;
    unz_global_info* pGlobalInfo = new unz_global_info;
    unz_file_info* pFileInfo = new unz_file_info;
    locale loc = locale::global(locale(""));
    unzfile = unzOpen(strZipFilePath);
    locale::global(loc);
    if(unzfile == NULL)
    {
        //LOG_WRITE_EX(_T("打开压缩文件失败") );
        return -2;
    }
    nReturnValue = unzGetGlobalInfo(unzfile, pGlobalInfo);
    if(nReturnValue != UNZ_OK)
    {
        //LOG_WRITE_EX(_T("获取压缩文件信息失败"));
        unzClose(unzfile);
        return -3;
    }
    do
    {
        int iLen = 0;
        int iFileSaveLen = 0;
        for(int i=0; i<pGlobalInfo->number_entry; i++)
        {
            iLen = 0;
            iFileSaveLen = 0;
            //解析得到zip中的文件信息
            iRet = unzGetCurrentFileInfo(unzfile, pFileInfo, szZipFName, MAX_PATH,
                NULL, 0, NULL, 0);
            if(iRet != UNZ_OK)
            {
                //LOG_WRITE_EX(_T("解压文件节点失败!") );
                iRet = 1;
                break;
            }
            //判断是文件夹还是文件
            if(pFileInfo->external_fa != FILE_ATTRIBUTE_DIRECTORY )
            {
                if (0 == strcmp(szZipFName,pstrFilePath))
                {
                    // 添加目录
                    FileUtils::MakeSureDirExsitsW(strFileDir.c_str());

                    // 创建文件
                    FILE* file = fopen(pstrSavePath, "wb");
                    if (NULL == file)
                    {
                        iRet = 3;
                        break;
                    }
                    //打开文件
                    iRet = unzOpenCurrentFile(unzfile);
                    if(nReturnValue != UNZ_OK)
                    {
                        iRet = 4;
                        break;
                    }
                    //读取文件
                    const int BUFFER_SIZE = 4096;
                    char szReadBuffer[BUFFER_SIZE];
                    while(true)
                    {
                        memset(szReadBuffer, 0x0, BUFFER_SIZE);
                        int nReadFileSize = unzReadCurrentFile(unzfile, szReadBuffer, BUFFER_SIZE);
                        if(nReadFileSize < 0)                //读取文件失败
                        {
                            unzCloseCurrentFile(unzfile);
                            iRet = 5;
                            break;
                        }
                        else if(nReadFileSize == 0)            //读取文件完毕
                        {
                            unzCloseCurrentFile(unzfile);
                            iRet = 0;
                            bDone = true;
                            break;
                        }
                        else                                //写入读取的内容
                        {
                            fseek(file,iFileSaveLen,SEEK_SET);
                            iLen =fwrite(szReadBuffer,sizeof(char),nReadFileSize,file);
                            iFileSaveLen += iLen;
                            if(iLen == 0)
                            {
                                unzCloseCurrentFile(unzfile);
                                iRet = 7;
                                break;
                            }
                        }
                    }
                    fclose(file);
                }
            }
            if (0 < iRet)
            {
                break;
            }
            if (bDone)
            {
                break;
            }
            unzGoToNextFile(unzfile);
        }
    } while (0);
    if(NULL != unzfile)
    {
        unzClose(unzfile);
        unzfile = NULL;
    }
    delete pGlobalInfo ;
    delete pFileInfo;
    return iRet;
}

/** 检查文件列表中是否包含输入的文件 */
int checkFileInList(vector<char*> vecZipPath, const char* strFile, bool fuzzymathc )
{
    int iRet = 1;
    if(fuzzymathc)
    {
        for(int i = 0; i < vecZipPath.size(); i++ )
        {
            if(0 == fnmatch(vecZipPath.at(i), strFile, 0) )
                return 0;
        }
    }
    else
    {
        for(int i = 0; i < vecZipPath.size(); i++ )
        {
            if(0 == strcmp(vecZipPath.at(i), strFile) )
                return 0;
        }
    }
    return iRet ;
}

/** @brief 解压多个文件
 *  @param pstrFilePath 待解压的zip文件
 *  @param pstrSavePath 文件保存目录
 *  @param strZipDirPath 待解压文件相对zip文件路径
 */
int DecompressionFiles(vector<char*> vecZipPath, const char *pstrSavePath ,const char *strZipFilePath, bool fuzzyMatch)
{
    if(vecZipPath.size() == 0 )
        return 0;
#ifdef _DEBUG
    for(int i = 0; i < vecZipPath.size(); i++)
    {
        cout << "file list:" << vecZipPath.at(i) << endl;
    }
#endif
    if ( NULL == pstrSavePath || NULL == strZipFilePath)
    {
        return 2;
    }

    if (0 == strlen(pstrSavePath) || 0 == strlen(strZipFilePath) )
    {
        return 2;
    }

    std::string strFileDir;// = FileUtils::ExtractFileDir(pstrSavePath);

    unzFile unzfile =NULL;
    int iRet = 1;

    int nReturnValue = 0;
    char szZipFName[MAX_PATH];
    bool bDone = false;
    unz_global_info* pGlobalInfo = new unz_global_info;
    unz_file_info* pFileInfo = new unz_file_info;
    locale loc = locale::global(locale(""));
    unzfile = unzOpen(strZipFilePath);
    locale::global(loc);
    char szTempSavePath[512] = {0};
    if(unzfile == NULL)
    {
        return -2;
    }
    nReturnValue = unzGetGlobalInfo(unzfile, pGlobalInfo);
    if(nReturnValue != UNZ_OK)
    {
        unzClose(unzfile);
        return -3;
    }
    do
    {
        int iLen = 0;
        int iFileSaveLen = 0;
        for(int i=0; i<pGlobalInfo->number_entry; i++)
        {
            iLen = 0;
            iFileSaveLen = 0;
            //解析得到zip中的文件信息
            nReturnValue = unzGetCurrentFileInfo(unzfile, pFileInfo, szZipFName, MAX_PATH,NULL, 0, NULL, 0);
            if(nReturnValue != UNZ_OK)
            {
                iRet = 2;
                break;
            }
            //判断是文件夹还是文件
            if(pFileInfo->external_fa != FILE_ATTRIBUTE_DIRECTORY )
            {
                if( 0 == checkFileInList(vecZipPath, szZipFName,fuzzyMatch) )
                {
#ifdef _DEBUG
                    cout << "decompress file = " << szZipFName << endl;
#endif
                    // singel file save path
                    memset(szTempSavePath, 0x0, sizeof(szTempSavePath));
                    string strDiskFile;
#ifdef WIN32
                    sprintf(szTempSavePath, "%s\\%s", pstrSavePath, szZipFName );
                    strDiskFile = FileUtils::SlToBsl(szTempSavePath);
#else
                    sprintf(szTempSavePath, "%s/%s", pstrSavePath, szZipFName );
                    strDiskFile = FileUtils::BslToSl(szTempSavePath);
#endif
                    strFileDir = FileUtils::ExtractFileDir(strDiskFile);
                    // 添加目录
                    FileUtils::MakeSureDirExsitsW(strFileDir.c_str());

                    // 创建文件
                    FILE* file = fopen(strDiskFile.c_str(), "wb");
#ifdef _DEBUG
                    cout << "open file = " << strDiskFile << endl;
                    cout << "save path = " << pstrSavePath << endl;
#endif
                    if (NULL == file)
                    {
                        iRet = 3;
                        break;
                    }
                    //打开文件
                    nReturnValue = unzOpenCurrentFile(unzfile);
                    if(nReturnValue != UNZ_OK)
                    {
                        iRet = 4;
                        break;
                    }
                    //读取文件
                    const int BUFFER_SIZE = 4096;
                    char szReadBuffer[BUFFER_SIZE];
                    while(true)
                    {
                        memset(szReadBuffer, 0x0, BUFFER_SIZE);
                        int nReadFileSize = unzReadCurrentFile(unzfile, szReadBuffer, BUFFER_SIZE);
                        if(nReadFileSize < 0)                           //读取文件失败
                        {
                            unzCloseCurrentFile(unzfile);
                            iRet = 5;
                            break;
                        }
                        else if(nReadFileSize == 0)                     //读取文件完毕
                        {
                            unzCloseCurrentFile(unzfile);
                            bDone = true;
                            break;
                        }
                        else                                            //写入读取的内容
                        {
                            fseek(file,iFileSaveLen,SEEK_SET);
                            iLen =fwrite(szReadBuffer,sizeof(char),nReadFileSize,file);
                            iFileSaveLen += iLen;
                            if(iLen == 0)
                            {
                                unzCloseCurrentFile(unzfile);
                                iRet = 7;
                                break;
                            }
                        }
                    }
                    fclose(file);
                }
            }
            if (1 < iRet)
            {
                break;
            }
            unzGoToNextFile(unzfile);
        }
    } while (0);
    if(NULL != unzfile)
    {
        unzClose(unzfile);
        unzfile = NULL;
    }
    delete pGlobalInfo ;
    delete pFileInfo;
    return iRet;
}


int DecompressionFilesEx(int len, const char* listZipPath, const char *listSavePath ,const char *strZipFilePath, bool fuzzyMatch )
{
    if ( NULL == listZipPath || NULL == listSavePath || NULL == strZipFilePath )
    {
        return 2;
    }

    if (0 == strlen(listZipPath) || 0 == strlen(listSavePath) || 0 == strlen(strZipFilePath) )
    {
        return 2;
    }
    int iRet = 0;
    vector<char*> vecZipPath;
    char* p = (char*)listZipPath;
    char* p1 = p;
    int count = 0;
    char ch = *(p+count);
    int slen = strlen(listZipPath);
    while(count < slen )
    {
        count++;
        ch = *(p+count);
        if( ch == ';' )
        {
            int size = p+ count - p1+1;
            char *s = new char[size];
            memset(s, 0x0, size);
            memcpy(s, p1, p+ count - p1 );
            vecZipPath.push_back(s);
            p1 = p + count + 1;
        }
    }
    {
        int size = p+ count - p1+1;
        char *s = new char[size];
        memset(s, 0x0, size);
        memcpy(s, p1, p+ count - p1 );
        vecZipPath.push_back(s);
    }

    iRet = DecompressionFiles(vecZipPath, listSavePath, strZipFilePath, fuzzyMatch);
#ifdef _DEBUG
    cout << "decompress len = " << vecZipPath.size() << endl;
#endif
    return iRet;
}




/** @brief 解压单个目录
 *  @param pstrFilePath 待解压的zip文件
 *  @param pstrSavePath 文件保存目录
 *  @param strZipDirPath 待解压文件相对zip文件路径
 */
int DecompressionDir(const char *pstrFilePath, const char *pstrSavePath, const char *strZipDirPath)
{
    if ( NULL == pstrSavePath || NULL == pstrFilePath || NULL == strZipDirPath)
    {
        return 1;
    }

    if (0 == strlen(pstrSavePath) || 0 == strlen(pstrFilePath) || 0 == strlen(strZipDirPath) )
    {
        return 1;
    }

    std::string strFileDir;
    int iRet = 1;
    unzFile unzfile =NULL;
    int nReturnValue = 0;
    char szZipFName[MAX_PATH];
    bool bDone = false;
    unz_global_info* pGlobalInfo = new unz_global_info;
    unz_file_info* pFileInfo = new unz_file_info;
    locale loc = locale::global(locale(""));
    unzfile = unzOpen(pstrFilePath);
    locale::global(loc);
    char szTempSavePath[512] = {0};
    if(unzfile == NULL)
    {
        return -2;
    }
    nReturnValue = unzGetGlobalInfo(unzfile, pGlobalInfo);
    if(nReturnValue != UNZ_OK)
    {
        unzClose(unzfile);
        return -3;
    }
    do
    {
        int iLen = 0;
        int iFileSaveLen = 0;
        for(int i=0; i<pGlobalInfo->number_entry; i++)
        {
            iLen = 0;
            iFileSaveLen = 0;
            //解析得到zip中的文件信息
            nReturnValue = unzGetCurrentFileInfo(unzfile, pFileInfo, szZipFName, MAX_PATH,NULL, 0, NULL, 0);
            if(nReturnValue != UNZ_OK)
            {
                iRet = 2;
                break;
            }
            //判断是文件夹还是文件
            if(pFileInfo->external_fa != FILE_ATTRIBUTE_DIRECTORY )
            {
                if( 0 == strncmp(strZipDirPath, szZipFName, strlen(strZipDirPath)) )
                {
                    // singel file save path
                    memset(szTempSavePath, 0x0, sizeof(szTempSavePath));
#ifdef WIN32
                    sprintf(szTempSavePath, "%s\\%s", pstrSavePath, szZipFName );
#else
                    sprintf(szTempSavePath, "%s/%s", pstrSavePath, szZipFName );
#endif
                    string strDiskFile = FileUtils::SlToBsl(szTempSavePath);
                    strFileDir = FileUtils::ExtractFileDir(strDiskFile);
                    // 添加目录
                    FileUtils::MakeSureDirExsitsW(strFileDir.c_str());

                    // 创建文件
                    FILE* file = fopen(strDiskFile.c_str(), "wb");
                    if (NULL == file)
                    {
                        iRet = 3;
                        break;
                    }
                    //打开文件
                    nReturnValue = unzOpenCurrentFile(unzfile);
                    if(nReturnValue != UNZ_OK)
                    {
                        iRet = 4;
                        break;
                    }
                    //读取文件
                    const int BUFFER_SIZE = 4096;
                    char szReadBuffer[BUFFER_SIZE];
                    while(true)
                    {
                        memset(szReadBuffer, 0x0, BUFFER_SIZE);
                        int nReadFileSize = unzReadCurrentFile(unzfile, szReadBuffer, BUFFER_SIZE);
                        if(nReadFileSize < 0)                           //读取文件失败
                        {
                            unzCloseCurrentFile(unzfile);
                            iRet = 5;
                            break;
                        }
                        else if(nReadFileSize == 0)                     //读取文件完毕
                        {
                            unzCloseCurrentFile(unzfile);
                            bDone = true;
                            break;
                        }
                        else                                            //写入读取的内容
                        {
                            fseek(file,iFileSaveLen,SEEK_SET);
                            iLen =fwrite(szReadBuffer,sizeof(char),nReadFileSize,file);
                            iFileSaveLen += iLen;
                            if(iLen == 0)
                            {
                                unzCloseCurrentFile(unzfile);
                                iRet = 7;
                                break;
                            }
                        }
                    }
                    fclose(file);
                }
            }
            if (1 < iRet)
            {
                break;
            }
            unzGoToNextFile(unzfile);
        }
    } while (0);
    if(NULL != unzfile)
    {
        unzClose(unzfile);
        unzfile = NULL;
    }
    delete pGlobalInfo ;
    delete pFileInfo;
    return iRet;
}

// 解压zip文件到指定目录
int DecompressionZip(const char *pstrSavePath, const char *strZipFilePath)
{
    if (NULL == pstrSavePath || NULL== strZipFilePath )
    {
        return 1;
    }
    if (0 == strlen(pstrSavePath) || 0 == strlen(strZipFilePath))
    {
        return 1;
    }
    // 添加目录
    FileUtils::MakeSureDirExsitsW(pstrSavePath);
    std::string strFileDir = FileUtils::ExtractFileDir(pstrSavePath);

    unzFile unzfile =NULL;
    unz_global_info* pGlobalInfo = new unz_global_info;
    unz_file_info* pFileInfo = new unz_file_info;

    int iRet = 1;
    int nReturnValue = 0;
    char szZipFName[MAX_PATH];
    std::string strDiskFile;
    std::string strDiskPath;

    unzfile = unzOpen(strZipFilePath);
    if(unzfile == NULL)
    {
        //LOG_WRITE_EX(_T("打开压缩文件失败") );
        iRet = -2;
        return iRet;
    }
    nReturnValue = unzGetGlobalInfo(unzfile, pGlobalInfo);
    if(nReturnValue != UNZ_OK)
    {
        //LOG_WRITE_EX(_T("获取压缩文件信息失败"));
        unzClose(unzfile);
        iRet = -3;
        return iRet;
    }
    do
    {
        int iLen = 0;
        int iFileSaveLen = 0;
        for(int i=0; i<pGlobalInfo->number_entry; i++)
        {
            iLen = 0;
            iFileSaveLen = 0;

            //解析得到zip中的文件信息
            nReturnValue = unzGetCurrentFileInfo(unzfile, pFileInfo, szZipFName, MAX_PATH,
                NULL, 0, NULL, 0);
            if(nReturnValue != UNZ_OK)
            {
                //LOG_WRITE_EX(_T("解压文件节点失败!") );
                iRet = -1;
                break;
            }

            //判断是文件夹还是文件
            switch(pFileInfo->external_fa)
            {
            case FILE_ATTRIBUTE_DIRECTORY:
                strDiskPath = std::string(pstrSavePath) + ("\\") + string(szZipFName);
                strDiskFile = FileUtils::SlToBsl(strDiskFile);
                FileUtils::MakeSureDirExsitsW(strDiskPath.c_str());
                break;
            default:
                {
                    // 创建文件
                    strDiskFile = std::string(pstrSavePath) + ("\\") + string(szZipFName);
                    strDiskFile = FileUtils::SlToBsl(strDiskFile);
                    string strpath = FileUtils::ExtractFilePath(strDiskFile);
                    FileUtils::MakeSureDirExsitsW(strpath.c_str());
                    FILE* file = fopen(strDiskFile.c_str(), "wb");
                    if (NULL == file)
                    {
                        iRet = 3;
                        break;
                    }
                    //打开文件
                    nReturnValue = unzOpenCurrentFile(unzfile);
                    if(nReturnValue != UNZ_OK)
                    {
                        iRet = 4;
                        break;
                    }
                    //读取文件
                    const int BUFFER_SIZE = 4096;
                    char szReadBuffer[BUFFER_SIZE];
                    while(true)
                    {
                        memset(szReadBuffer, 0, BUFFER_SIZE);
                        int nReadFileSize = unzReadCurrentFile(unzfile, szReadBuffer, BUFFER_SIZE);
                        if(nReadFileSize < 0)                //读取文件失败
                        {
                            unzCloseCurrentFile(unzfile);
                            iRet = 5;
                            break;
                        }
                        else if(nReadFileSize == 0)            //读取文件完毕
                        {
                            unzCloseCurrentFile(unzfile);
                            iRet = 0;
                            break;
                        }
                        else                                //写入读取的内容
                        {
                            fseek(file,iFileSaveLen,SEEK_SET);
                            iLen =fwrite(szReadBuffer,sizeof(char),nReadFileSize,file);
                            iFileSaveLen += iLen;
                            if(iLen == 0)
                            {
                                unzCloseCurrentFile(unzfile);
                                iRet = 7;
                                break;
                            }
                        }
                    }
                    fclose(file);
                }
                break;
            }
            if (0 < iRet)
            {
                break;
            }
            unzGoToNextFile(unzfile);
        }
    } while (0);

    if(NULL != unzfile)
    {
        unzClose(unzfile);
        unzfile = NULL;
    }
    if (NULL != pGlobalInfo )
    {
        delete pGlobalInfo ;
        pGlobalInfo = NULL;
    }
    delete pFileInfo;

}


// 添加文件到zip中
// strFile 待添加源文件
// strPath 添加文件到zip的相对目录
// zipFileName zip文件
int AddFileToZip(const char* strSrcFile, const char* strZipPath, const char* zipFilePack)
{
    int iRet = 0;
    zipFile zf = zipOpen(zipFilePack, APPEND_STATUS_ADDINZIP); //添加zip文件
    if (zf == NULL)
    {
        // 无法打开zip文件
        iRet = -1;
        return iRet;
    }
    FILE* srcfp = NULL;

    do
    {
        //初始化写入zip的文件信息
        zip_fileinfo zi;
        zi.tmz_date.tm_sec = zi.tmz_date.tm_min = zi.tmz_date.tm_hour = zi.tmz_date.tm_mday = zi.tmz_date.tm_mon = zi.tmz_date.tm_year = 0;
        zi.dosDate = 0;
        zi.internal_fa = 0;
        zi.external_fa = 0;

        //在zip文件中创建新文件
        zipOpenNewFileInZip(zf, strZipPath, &zi, NULL, 0, NULL, 0, NULL, Z_DEFLATED, Z_DEFAULT_COMPRESSION);

        if (0 == access(strSrcFile,0))
        {
            //打开源文件
            srcfp = fopen(strSrcFile, "rb");
            if (srcfp == NULL)
            {
                // 文件无法打开
                zipCloseFileInZip(zf); //关闭zip文件
                iRet = 1;
                break;
            }

            //读入源文件并写入zip文件
            char buf[100*1024]; //buffer
            int numBytes = 0;
            while( !feof(srcfp) )
            {
                numBytes = fread(buf, 1, sizeof(buf), srcfp);
                zipWriteInFileInZip(zf, buf, numBytes);
                if( ferror(srcfp) )
                {
                    iRet = 2;
                    break;
                }
            }
            //关闭源文件
            fclose(srcfp);
        }
    } while (0);
    //关闭zip文件
    zipCloseFileInZip(zf);

    zipClose(zf, NULL); //关闭zip文件
    return iRet;
}

int AddFileToZipEx(int count, const char* strSrcFile, const char* strZipPath, const char* zipFilePack)
{
    int iRet = 0;
    if( count == 0)
        return iRet;

    return iRet;
}

/** @brief 添加目录到zip中 根据已存在的目录结构将目录添加到对应的目录下面,目录下文件列表按照目录结构进行添加
 *  *@param strSrcDirPath 待添加的源文件目录
 *  *@param strZipPath 添加文件到zip的相对目录
 *  *@param zipFilePack zip文件
 * */
int AddDirToZip(const char* strSrcDirPath, const char* strZipPath, const char* zipFilePack)
{
    return addFilePathToZip(zipFilePack, strZipPath, strSrcDirPath );
}

int CreateZipFile(const char* strPath, const char* strZipName)
{
    int iRet = 0;
    return iRet;
}


int DeleteInZipFile(const char* zipFilePack, const char* strDelName,bool bIsDir )
{
    unzFile unzfile =NULL;
    unz_global_info* pGlobalInfo = new unz_global_info;
    unz_file_info* pFileInfo = new unz_file_info;

    int iRet = 0;
    int nReturnValue = 0;
    char szZipFName[MAX_PATH];
    char szExtraField[MAX_PATH];
    char szComment[MAX_PATH];
    char szLocalExtraField[MAX_PATH];
    unzfile = unzOpen(zipFilePack);
    if(unzfile == NULL)
    {
        //LOG_WRITE_EX(_T("打开压缩文件失败") );
        iRet = -2;
        return iRet;
    }
    nReturnValue = unzGetGlobalInfo(unzfile, pGlobalInfo);
    if(nReturnValue != UNZ_OK)
    {
        //LOG_WRITE_EX(_T("获取压缩文件信息失败"));
        unzClose(unzfile);
        iRet = -3;
        return iRet;
    }

    char szZipFilePackBak[512] = {0};
    sprintf(szZipFilePackBak, "%s.bak", zipFilePack);
    zipFile zf = zipOpen(szZipFilePackBak, APPEND_STATUS_CREATE); //创建zip文件
    do
    {
        int iLen = 0;
        int iFileSaveLen = 0;
        for(int i=0; i<pGlobalInfo->number_entry; i++)
        {
            iLen = 0;
            iFileSaveLen = 0;

            //解析得到zip中的文件信息
            //iRet = unzGetCurrentFileInfo(unzfile, pFileInfo, szZipFName, MAX_PATH,
            //    NULL, 0, NULL, 0);
            memset(szZipFName, 0x0,MAX_PATH );
            memset(szExtraField, 0x0,MAX_PATH );
            memset(szComment, 0x0,MAX_PATH );
            memset(szLocalExtraField, 0x0, sizeof(szLocalExtraField));

            iRet = unzGetCurrentFileInfo(unzfile, pFileInfo, szZipFName, MAX_PATH,
                szExtraField, MAX_PATH, szComment, MAX_PATH);

            if(iRet != UNZ_OK)
            {
                //LOG_WRITE_EX(_T("解压文件节点失败!") );
                iRet = -1;
                break;
            }
            zip_fileinfo zi;
            zi.dosDate = pFileInfo->dosDate;
            zi.internal_fa = pFileInfo->internal_fa;
            zi.external_fa = pFileInfo->external_fa;
            //zi.tmz_date = pFileInfo->tmu_date;
            memcpy(&(zi.tmz_date),&(pFileInfo->tmu_date), sizeof(tm_unz));
            if (bIsDir)
            {
                int len = strlen(szZipFName);
                std::string dir = strDelName;
                dir = dir.substr(0,len);
                if (0 == strcmp(dir.c_str(),szZipFName))
                {
                    unzGoToNextFile(unzfile);
                    continue;
                }
            }
            else
            {
                if (0 == strcmp(szZipFName, strDelName))
                {
                    unzGoToNextFile(unzfile);
                    continue;
                }
            }
            //iRet = zipOpenNewFileInZip(zf, szZipFName, &zi, NULL, 0, NULL, 0, NULL, pFileInfo->compression_method, Z_DEFAULT_COMPRESSION);

            //iRet = zipOpenNewFileInZip(zf, szZipFName, &zi, NULL, 0, szExtraField, strlen(szExtraField), szComment,
            //    pFileInfo->compression_method, Z_DEFAULT_COMPRESSION);

            //判断是文件夹还是文件
            switch(pFileInfo->external_fa)
            {
            case FILE_ATTRIBUTE_DIRECTORY:
                break;
            default:
                {
                    //打开文件
                    //iRet = unzOpenCurrentFile(unzfile);
                    iRet = unzOpenCurrentFile2(unzfile,(int*)&(pFileInfo->compression_method),NULL,1);
                    if(nReturnValue != UNZ_OK)
                    {
                        iRet = 4;
                        break;
                    }
                    int len = 0;
                    len = unzGetLocalExtrafield(unzfile,szLocalExtraField,len);
                    if (0 <= len )
                    {
                        // 创建对应的压缩文件
                        iRet = zipOpenNewFileInZip2(zf, szZipFName, &zi, szLocalExtraField, len, szExtraField, strlen(szExtraField), szComment,
                            pFileInfo->compression_method, Z_DEFAULT_COMPRESSION,1);
                    }
                    else
                    {
                        // 创建对应的压缩文件
                        iRet = zipOpenNewFileInZip2(zf, szZipFName, &zi, NULL, 0, szExtraField, strlen(szExtraField), szComment,
                            pFileInfo->compression_method, Z_DEFAULT_COMPRESSION,1);
                    }

                    if (0 != iRet)
                    {
                        break;
                    }

                    //读取文件
                    const int BUFFER_SIZE = 4096;
                    char szReadBuffer[BUFFER_SIZE];
                    while(true)
                    {
                        memset(szReadBuffer, 0, BUFFER_SIZE);
                        int nReadFileSize = unzReadCurrentFile(unzfile, szReadBuffer, BUFFER_SIZE);
                        if(nReadFileSize < 0)                //读取文件失败
                        {
                            unzCloseCurrentFile(unzfile);
                            iRet = 5;
                            break;
                        }
                        else if(nReadFileSize == 0)            //读取文件完毕
                        {
                            unzCloseCurrentFile(unzfile);
                            zipCloseFileInZip(zf);
                            iRet = 0;
                            break;
                        }
                        else                                //写入读取的内容
                        {
                            zipWriteInFileInZip(zf, szReadBuffer, nReadFileSize);
                        }
                    }
                }
                break;
            }
            if (0 < iRet)
            {
                break;
            }
            unzGoToNextFile(unzfile);
        }
    } while (0);

    if(NULL != unzfile)
    {
        unzClose(unzfile);
        unzfile = NULL;
    }
    if (NULL != pGlobalInfo )
    {
        delete pGlobalInfo ;
        pGlobalInfo = NULL;
    }
    if (NULL != zf)
    {
        zipClose(zf, NULL);
    }
    delete pFileInfo;
    remove(zipFilePack);
    rename(szZipFilePackBak,zipFilePack);
    return iRet;
}


int DeleteInZipFileEx(const char* zipFilePack, const char* strDelName,bool bDir)
{
    int iRet = 1;
    int iResult = 0;
    FILE* fw = NULL;
    FILE *f = NULL;
    unsigned int totalOffSet = 0;
    unsigned int iAleadySize = 0;
    unsigned int iCurrAleadySize = 0;
    unsigned int iCurrLen = 0;
    char *szBuff = new char[MAX_COMMENT_LEN+1];
    char* p = szBuff;
    bool b = true;
    ZipentryCenteral* centeralNext = NULL;
    ZipentryCenteral* centeralCurr = NULL;
    unsigned int iTotalDeleteCount = 0;
    unsigned int iDeleteDataSize = 0;
    unsigned long lAlreadyWriteSize = 0;
    char szFileBak[512];
    memset(szFileBak, 0x0, 512);

    Zipfile* zipFile = new Zipfile;
    memset(zipFile, 0x0, sizeof(Zipfile));
    iResult = initApkFile(zipFile, zipFilePack);
    if (0 != iResult)
    {
        return iResult;
    }

    // find the file
    sprintf(szFileBak, "%s.bak", zipFilePack);

    fw = fopen(szFileBak, "wb");
    f = fopen(zipFilePack, "rb");

    if (NULL == f || NULL == fw)
    {
        iRet = -2;
        return iRet;
    }

    totalOffSet = zipFile->centralDirOffest;
    int i = 0;
    for (; i < zipFile->entryCount; )
    {
        if ( i  == 0 )
        {
            centeralCurr = zipFile->centeral;
            centeralNext = centeralCurr->next;
            iCurrLen = centeralNext->localHeaderRelOffset - centeralCurr->localHeaderRelOffset;
        }
        if (NULL == centeralCurr)
        {
            return 11;
        }
        if ( b )
        {
            memset(szBuff, 0x0, MAX_COMMENT_LEN+1);
            fseek(f, iAleadySize, SEEK_SET);
            int iReadLen = fread(szBuff, 1, MAX_COMMENT_LEN, f);
            if (iReadLen == 0 )
            {
                iRet = -2;
                break;
            }
            b = false;
            p = szBuff;
        }
        if(0 != checkZipFileEx((char*)centeralCurr->fileName, strDelName, bDir))
        {
            if ( iCurrLen - iCurrAleadySize > MAX_COMMENT_LEN - (p - szBuff) )
            {
                fwrite(p, 1, MAX_COMMENT_LEN - (p - szBuff) , fw);
                b = true;
                iAleadySize += MAX_COMMENT_LEN - (p - szBuff);
                iCurrAleadySize += MAX_COMMENT_LEN - (p - szBuff);
                continue;
            }
            else if( iCurrLen - iCurrAleadySize <= MAX_COMMENT_LEN - (p - szBuff) )
            {
                fwrite(p, 1, iCurrLen - iCurrAleadySize , fw);
                iAleadySize += iCurrLen - iCurrAleadySize;
                p += (iCurrLen - iCurrAleadySize);
                iCurrAleadySize = 0;
                // 切换
                i++;
                if (i == zipFile->entryCount)
                {
                    break;
                }
                if ( i < zipFile->entryCount - 1 )
                {
                    centeralCurr = centeralNext;
                    centeralNext = centeralCurr->next;
                    iCurrLen = centeralNext->localHeaderRelOffset - centeralCurr->localHeaderRelOffset;
                }
                else
                {
                    centeralCurr = centeralNext;
                    centeralNext = NULL;

//                    centeralCurr->fileNameLength;
//                    centeralCurr->fileCommentLength;
//                    centeralCurr->compressedSize;
//                    unsigned int offsetLen = 0;
//                    offsetLen = 30 + centeralCurr->fileNameLength + centeralCurr->fileCommentLength + centeralCurr->compressedSize;
//                    if(centeralCurr->GPBitFlag == 1 )
//                        offsetLen += 16;
                    iCurrLen = totalOffSet - centeralCurr->localHeaderRelOffset;
                }
            }
        }
        else
        {
            if (iCurrLen - iCurrAleadySize > MAX_COMMENT_LEN - (p - szBuff))
            {
                b = true;
                iDeleteDataSize += MAX_COMMENT_LEN - (p - szBuff);
                iAleadySize += MAX_COMMENT_LEN - (p - szBuff);
                iCurrAleadySize += MAX_COMMENT_LEN - (p - szBuff);
                continue;
            }
            else if( iCurrLen - iCurrAleadySize <= MAX_COMMENT_LEN - (p - szBuff) )
            {
                if (i == zipFile->entryCount - 1)
                {
                    break;
                }
                iAleadySize += iCurrLen - iCurrAleadySize;
                p += (iCurrLen - iCurrAleadySize);
                iDeleteDataSize += iCurrLen - iCurrAleadySize;
                iCurrAleadySize = 0;
                iTotalDeleteCount++;
                // 切换
                i++;
                if (i == zipFile->entryCount)
                {
                    break;
                }
                if ( i < zipFile->entryCount - 1 )
                {
                    centeralCurr = centeralNext;
                    centeralNext = centeralCurr->next;
                    iCurrLen = centeralNext->localHeaderRelOffset - centeralCurr->localHeaderRelOffset;
                }
                else
                {
                    centeralCurr = centeralNext;
                    centeralNext = NULL;

//                    centeralCurr->fileNameLength;
//                    centeralCurr->fileCommentLength;
//                    centeralCurr->compressedSize;
//                    unsigned int offsetLen = 0;
//                    offsetLen = 30 + centeralCurr->fileNameLength + centeralCurr->fileCommentLength + centeralCurr->compressedSize;
//                    if(centeralCurr->GPBitFlag == 1 )
//                        offsetLen += 16;
                    iCurrLen = totalOffSet - centeralCurr->localHeaderRelOffset;
                }
            }
        }
    }



    unsigned long localHeaderRelOffset = 0;
    zipFile->centralDirSize = 0;
    memset(szBuff, 0x0, MAX_COMMENT_LEN+1);
    ZipentryCenteral * centeral = zipFile->centeral;
    Zipentry* entry = zipFile->entries;

    iAleadySize = 0;
    for (int i = 0; i< zipFile->entryCount; i++)
    {
        if (MAX_COMMENT_LEN - iAleadySize < 4092 )
        {
            fwrite(szBuff, 1, iAleadySize, fw);
            memset(szBuff, 0x0, MAX_COMMENT_LEN+1);
            iAleadySize = 0;
        }
        if(i == 0)
        {
            localHeaderRelOffset = centeral->localHeaderRelOffset;
        }
        char szbuf[46] = {0};
        if(0 != checkZipFileEx((char*)centeral->fileName, strDelName, bDir))
        {
            int j = 0;
            memcpy(szbuf,&centeral->header,sizeof(centeral->header));
            j += sizeof(centeral->header);

            memcpy(szbuf + j,&centeral->versionMadeBy,sizeof(centeral->versionMadeBy));
            j += sizeof(centeral->versionMadeBy);


            memcpy(szbuf + j,&centeral->versionToExtract,sizeof(centeral->versionToExtract));
            j += sizeof(centeral->versionToExtract);

            memcpy(szbuf + j,&centeral->GPBitFlag,sizeof(centeral->GPBitFlag));
            j += sizeof(centeral->GPBitFlag);

            memcpy(szbuf + j,&centeral->compressionMethod,sizeof(centeral->compressionMethod));
            j += sizeof(centeral->compressionMethod);

            memcpy(szbuf + j ,&centeral->lastModFileTime,sizeof(centeral->lastModFileTime));
            j += sizeof(centeral->versionToExtract);

            memcpy(szbuf + j,&centeral->lastModFileDate,sizeof(centeral->lastModFileDate));
            j += sizeof(centeral->lastModFileDate);

            memcpy(szbuf + j,&centeral->CRC32,sizeof(centeral->CRC32));
            j += sizeof(centeral->CRC32);

            memcpy(szbuf + j,&centeral->compressedSize,sizeof(centeral->compressedSize));
            j += sizeof(centeral->compressedSize);

            memcpy(szbuf + j,&centeral->uncompressedSize,sizeof(centeral->uncompressedSize));
            j += sizeof(centeral->uncompressedSize);

            memcpy(szbuf + j,&centeral->fileNameLength,sizeof(centeral->fileNameLength));
            j += sizeof(centeral->fileNameLength);

            memcpy(szbuf + j,&centeral->extraFieldLength,sizeof(centeral->extraFieldLength));
            j += sizeof(centeral->extraFieldLength);

            memcpy(szbuf + j,&centeral->fileCommentLength,sizeof(centeral->fileCommentLength));
            j += sizeof(centeral->fileCommentLength);

            memcpy(szbuf + j,&centeral->diskNumberStart,sizeof(centeral->diskNumberStart));
            j += sizeof(centeral->diskNumberStart);

            memcpy(szbuf + j,&centeral->internalAttrs,sizeof(centeral->internalAttrs));
            j += sizeof(centeral->internalAttrs);

            memcpy(szbuf + j,&centeral->externalAttrs,sizeof(centeral->externalAttrs));
            j += sizeof(centeral->externalAttrs);

            unsigned long  RelOffset = centeral->localHeaderRelOffset - localHeaderRelOffset;

            memcpy(szbuf + j,&RelOffset,sizeof(centeral->localHeaderRelOffset));
            j += sizeof(centeral->localHeaderRelOffset);

            memcpy( (void*)((char*)szBuff + iAleadySize) ,szbuf,j);
            iAleadySize += j;

            if(centeral->fileNameLength  == 46 )
            {
                memcpy( (void*)((char*)szBuff + iAleadySize) ,centeral->fileName , centeral->fileNameLength);
                iAleadySize += centeral->fileNameLength;
            }
            else
            {
                memcpy( (void*)((char*)szBuff + iAleadySize) ,centeral->fileName , centeral->fileNameLength);
                iAleadySize += centeral->fileNameLength;
            }

            if(centeral->extraFieldLength > 0)
            {
                memcpy( (void*)((char*)szBuff + iAleadySize) ,centeral->extraField , centeral->extraFieldLength);
                iAleadySize += centeral->extraFieldLength;
            }
            if(centeral->fileCommentLength > 0)
            {
                memcpy( (void*)((char*)szBuff + iAleadySize) ,centeral->fileComment , centeral->fileCommentLength);
                iAleadySize += centeral->fileCommentLength;
            }
            zipFile->centralDirSize += 46 + centeral->fileNameLength
                + centeral->extraFieldLength + centeral->fileCommentLength;
        }
        else
        {
            if(centeral->next)
            {
                localHeaderRelOffset += centeral->next->localHeaderRelOffset - centeral->localHeaderRelOffset;
            }
        }
        entry = entry->next;
        centeral = centeral->next;
    }
    fwrite(szBuff, 1, iAleadySize, fw);
    memset(szBuff, 0x0, MAX_COMMENT_LEN+1);

    char szBufEnd[22] = {0};
    long kSignature = 0x06054b50;
    memcpy(szBufEnd ,&kSignature, 4 );
    unsigned short entryCount = zipFile->entryCount - iTotalDeleteCount;
    unsigned short totalEntryCount = zipFile->totalEntryCount - iTotalDeleteCount;
    unsigned int centralDirOffest = zipFile->centralDirOffest - iDeleteDataSize;
    memcpy(szBufEnd + 4 ,&zipFile->disknum , 2);
    memcpy(szBufEnd + 6 ,&zipFile->diskWithCentralDir , 2);
    memcpy(szBufEnd + 8 ,&entryCount, 2);
    memcpy(szBufEnd + 10 ,&totalEntryCount, 2);
    memcpy(szBufEnd + 12 ,&zipFile->centralDirSize , 4);
    memcpy(szBufEnd + 16 ,&centralDirOffest , 4);
    memcpy(szBufEnd + 20 ,&zipFile->commentLen , 2);

    memcpy(szBuff, szBufEnd, 22);
    lAlreadyWriteSize += 22;

    if (zipFile->commentLen > 0)
    {
        memcpy(szBuff+22, zipFile->comment , zipFile->commentLen);
        lAlreadyWriteSize += zipFile->commentLen;
    }

    // last write right data into file
    fwrite(szBuff, 1, lAlreadyWriteSize, fw);
    fclose(fw);
    fclose(f);
    // 文件转换
    iResult = remove(zipFilePack);
    if(0 != iResult )
    {
        printf("remove file fail\n");
    }
    else
    {
        iResult = rename(szFileBak,zipFilePack);
    }
    printf("delete file sucess\n");

    delete [] szBuff;
    clearZipFileInfo(zipFile);
    return iRet;
}

int checkZipFileList(const char* file, const char* filelist)
{
    int len = strlen(filelist);
    int offset = 0;
    char* ch ;
    char* p = (char*)filelist;
    char*p1 = p;
    char sz[512];
    do
    {
        offset++;
        ch = (char*)(filelist + offset);
        if( *ch == ';' )
        {
            memset(sz, 0x0, 512 );
            memcpy(sz, p1 , p+offset-p1);
            if( 0 == fnmatch(sz, file, 0) )
            {
                return 0;
            }
            p1 = p + offset+1;
        }
    }while( offset < len );
    if( p + offset - p1 > 0 )
    {
        memset(sz, 0x0, 512 );
        memcpy(sz, p1 , p+offset-p1);
        if( 0 == fnmatch(sz, file, 0) )
        {
            return 0;
        }
    }
    return 1;
}

int deleteFileInApkEx(const char* strFilePath, const char* deleteFilePath)
{
    if(NULL == strFilePath || NULL == deleteFilePath )
    {
        return -1;
    }
#ifdef _DEBUG
    cout << "delete file list:__ " << strFilePath << endl;
#endif
    int iRet = 1;
    int iResult = 0;
    FILE* fw = NULL;
    FILE *f = NULL;
    unsigned int totalOffSet = 0;
    unsigned int iAleadySize = 0;
    unsigned int iCurrAleadySize = 0;
    unsigned int iCurrLen = 0;
    char *szBuff = new char[MAX_COMMENT_LEN+1];
    char* p = szBuff;
    bool b = true;
    ZipentryCenteral* centeralNext = NULL;
    ZipentryCenteral* centeralCurr = NULL;
    unsigned int iTotalDeleteCount = 0;
    unsigned int iDeleteDataSize = 0;
    unsigned long lAlreadyWriteSize = 0;
    char szFileBak[512];
    memset(szFileBak, 0x0, 512);

    Zipfile* zipFile = new Zipfile;
    memset(zipFile, 0x0, sizeof(Zipfile));
    iResult = initApkFile(zipFile, strFilePath);
    if (0 != iResult)
    {
        return iResult;
    }

    // find the file
    sprintf(szFileBak, "%s.bak", strFilePath);

    fw = fopen(szFileBak, "wb");
    f = fopen(strFilePath, "rb");

    if (NULL == f || NULL == fw)
    {
        iRet = -2;
        return iRet;
    }

    totalOffSet = zipFile->centralDirOffest;
    int i = 0;
    for (; i < zipFile->entryCount; )
    {
        if ( i  == 0 )
        {
            centeralCurr = zipFile->centeral;
            centeralNext = centeralCurr->next;
            iCurrLen = centeralNext->localHeaderRelOffset - centeralCurr->localHeaderRelOffset;
        }
        if (NULL == centeralCurr)
        {
            return 11;
        }
        if ( b )
        {
            memset(szBuff, 0x0, MAX_COMMENT_LEN+1);
            fseek(f, iAleadySize, SEEK_SET);
            int iReadLen = fread(szBuff, 1, MAX_COMMENT_LEN, f);
            if (iReadLen == 0 )
            {
                iRet = -2;
                break;
            }
            b = false;
            p = szBuff;
        }
        if(0 != checkZipFileList((char*)centeralCurr->fileName, deleteFilePath ))
        {
            if ( iCurrLen - iCurrAleadySize > MAX_COMMENT_LEN - (p - szBuff) )
            {
                fwrite(p, 1, MAX_COMMENT_LEN - (p - szBuff) , fw);
                b = true;
                iAleadySize += MAX_COMMENT_LEN - (p - szBuff);
                iCurrAleadySize += MAX_COMMENT_LEN - (p - szBuff);
                continue;
            }
            else if( iCurrLen - iCurrAleadySize <= MAX_COMMENT_LEN - (p - szBuff) )
            {
                fwrite(p, 1, iCurrLen - iCurrAleadySize , fw);
                iAleadySize += iCurrLen - iCurrAleadySize;
                p += (iCurrLen - iCurrAleadySize);
                iCurrAleadySize = 0;
                // 切换
                i++;
                if (i == zipFile->entryCount)
                {
                    break;
                }
                if ( i < zipFile->entryCount - 1 )
                {
                    centeralCurr = centeralNext;
                    centeralNext = centeralCurr->next;
                    iCurrLen = centeralNext->localHeaderRelOffset - centeralCurr->localHeaderRelOffset;
                }
                else
                {
                    centeralCurr = centeralNext;
                    centeralNext = NULL;
                    iCurrLen = totalOffSet - centeralCurr->localHeaderRelOffset;
                }
            }
        }
        else
        {
            if (iCurrLen - iCurrAleadySize > MAX_COMMENT_LEN - (p - szBuff))
            {
                b = true;
                iDeleteDataSize += MAX_COMMENT_LEN - (p - szBuff);
                iAleadySize += MAX_COMMENT_LEN - (p - szBuff);
                iCurrAleadySize += MAX_COMMENT_LEN - (p - szBuff);
                continue;
            }
            else if( iCurrLen - iCurrAleadySize <= MAX_COMMENT_LEN - (p - szBuff) )
            {
                if (i == zipFile->entryCount - 1)
                {
                    break;
                }
                iAleadySize += iCurrLen - iCurrAleadySize;
                p += (iCurrLen - iCurrAleadySize);
                iDeleteDataSize += iCurrLen - iCurrAleadySize;
                iCurrAleadySize = 0;
                iTotalDeleteCount++;
                // 切换
                i++;
                if (i == zipFile->entryCount)
                {
                    break;
                }
                if ( i < zipFile->entryCount - 1 )
                {
                    centeralCurr = centeralNext;
                    centeralNext = centeralCurr->next;
                    iCurrLen = centeralNext->localHeaderRelOffset - centeralCurr->localHeaderRelOffset;
                }
                else
                {
                    centeralCurr = centeralNext;
                    centeralNext = NULL;
                    iCurrLen = totalOffSet - centeralCurr->localHeaderRelOffset;
                }
            }
        }
    }



    unsigned long localHeaderRelOffset = 0;
    zipFile->centralDirSize = 0;
    memset(szBuff, 0x0, MAX_COMMENT_LEN+1);
    ZipentryCenteral * centeral = zipFile->centeral;
    Zipentry* entry = zipFile->entries;

    iAleadySize = 0;
    for (int i = 0; i< zipFile->entryCount; i++)
    {
        if (MAX_COMMENT_LEN - iAleadySize < 4092 )
        {
            fwrite(szBuff, 1, iAleadySize, fw);
            memset(szBuff, 0x0, MAX_COMMENT_LEN+1);
            iAleadySize = 0;
        }
        if(i == 0)
        {
            localHeaderRelOffset = centeral->localHeaderRelOffset;
        }
        char szbuf[46] = {0};
        if(0 != checkZipFileList((char*)centeral->fileName, deleteFilePath ))
        {
            int j = 0;
            memcpy(szbuf,&centeral->header,sizeof(centeral->header));
            j += sizeof(centeral->header);

            memcpy(szbuf + j,&centeral->versionMadeBy,sizeof(centeral->versionMadeBy));
            j += sizeof(centeral->versionMadeBy);


            memcpy(szbuf + j,&centeral->versionToExtract,sizeof(centeral->versionToExtract));
            j += sizeof(centeral->versionToExtract);

            memcpy(szbuf + j,&centeral->GPBitFlag,sizeof(centeral->GPBitFlag));
            j += sizeof(centeral->GPBitFlag);

            memcpy(szbuf + j,&centeral->compressionMethod,sizeof(centeral->compressionMethod));
            j += sizeof(centeral->compressionMethod);

            memcpy(szbuf + j ,&centeral->lastModFileTime,sizeof(centeral->lastModFileTime));
            j += sizeof(centeral->versionToExtract);

            memcpy(szbuf + j,&centeral->lastModFileDate,sizeof(centeral->lastModFileDate));
            j += sizeof(centeral->lastModFileDate);

            memcpy(szbuf + j,&centeral->CRC32,sizeof(centeral->CRC32));
            j += sizeof(centeral->CRC32);

            memcpy(szbuf + j,&centeral->compressedSize,sizeof(centeral->compressedSize));
            j += sizeof(centeral->compressedSize);

            memcpy(szbuf + j,&centeral->uncompressedSize,sizeof(centeral->uncompressedSize));
            j += sizeof(centeral->uncompressedSize);

            memcpy(szbuf + j,&centeral->fileNameLength,sizeof(centeral->fileNameLength));
            j += sizeof(centeral->fileNameLength);

            memcpy(szbuf + j,&centeral->extraFieldLength,sizeof(centeral->extraFieldLength));
            j += sizeof(centeral->extraFieldLength);

            memcpy(szbuf + j,&centeral->fileCommentLength,sizeof(centeral->fileCommentLength));
            j += sizeof(centeral->fileCommentLength);

            memcpy(szbuf + j,&centeral->diskNumberStart,sizeof(centeral->diskNumberStart));
            j += sizeof(centeral->diskNumberStart);

            memcpy(szbuf + j,&centeral->internalAttrs,sizeof(centeral->internalAttrs));
            j += sizeof(centeral->internalAttrs);

            memcpy(szbuf + j,&centeral->externalAttrs,sizeof(centeral->externalAttrs));
            j += sizeof(centeral->externalAttrs);

            unsigned long  RelOffset = centeral->localHeaderRelOffset - localHeaderRelOffset;

            memcpy(szbuf + j,&RelOffset,sizeof(centeral->localHeaderRelOffset));
            j += sizeof(centeral->localHeaderRelOffset);

            memcpy( (void*)((char*)szBuff + iAleadySize) ,szbuf,j);
            iAleadySize += j;

            if(centeral->fileNameLength  == 46 )
            {
                memcpy( (void*)((char*)szBuff + iAleadySize) ,centeral->fileName , centeral->fileNameLength);
                iAleadySize += centeral->fileNameLength;
            }
            else
            {
                memcpy( (void*)((char*)szBuff + iAleadySize) ,centeral->fileName , centeral->fileNameLength);
                iAleadySize += centeral->fileNameLength;
            }

            if(centeral->extraFieldLength > 0)
            {
                memcpy( (void*)((char*)szBuff + iAleadySize) ,centeral->extraField , centeral->extraFieldLength);
                iAleadySize += centeral->extraFieldLength;
            }
            if(centeral->fileCommentLength > 0)
            {
                memcpy( (void*)((char*)szBuff + iAleadySize) ,centeral->fileComment , centeral->fileCommentLength);
                iAleadySize += centeral->fileCommentLength;
            }
            zipFile->centralDirSize += 46 + centeral->fileNameLength
                + centeral->extraFieldLength + centeral->fileCommentLength;
        }
        else
        {
            if(centeral->next)
            {
                localHeaderRelOffset += centeral->next->localHeaderRelOffset - centeral->localHeaderRelOffset;
            }
        }
        entry = entry->next;
        centeral = centeral->next;
    }
    fwrite(szBuff, 1, iAleadySize, fw);
    memset(szBuff, 0x0, MAX_COMMENT_LEN+1);

    char szBufEnd[22] = {0};
    long kSignature = 0x06054b50;
    memcpy(szBufEnd ,&kSignature, 4 );
    unsigned short entryCount = zipFile->entryCount - iTotalDeleteCount;
    unsigned short totalEntryCount = zipFile->totalEntryCount - iTotalDeleteCount;
    unsigned int centralDirOffest = zipFile->centralDirOffest - iDeleteDataSize;
    memcpy(szBufEnd + 4 ,&zipFile->disknum , 2);
    memcpy(szBufEnd + 6 ,&zipFile->diskWithCentralDir , 2);
    memcpy(szBufEnd + 8 ,&entryCount, 2);
    memcpy(szBufEnd + 10 ,&totalEntryCount, 2);
    memcpy(szBufEnd + 12 ,&zipFile->centralDirSize , 4);
    memcpy(szBufEnd + 16 ,&centralDirOffest , 4);
    memcpy(szBufEnd + 20 ,&zipFile->commentLen , 2);

    memcpy(szBuff, szBufEnd, 22);
    lAlreadyWriteSize += 22;

    if (zipFile->commentLen > 0)
    {
        memcpy(szBuff+22, zipFile->comment , zipFile->commentLen);
        lAlreadyWriteSize += zipFile->commentLen;
    }

    // last write right data into file
    fwrite(szBuff, 1, lAlreadyWriteSize, fw);
    fclose(fw);
    fclose(f);
    // 文件转换
    iResult = remove(strFilePath);
    if(0 != iResult )
    {
        printf("remove file fail\n");
    }
    else
    {
        iResult = rename(szFileBak,strFilePath);
    }
    printf("delete file sucess\n");

    delete [] szBuff;
    clearZipFileInfo(zipFile);
    return iRet;
}


int read_central_directory_entry_ex(Zipfile* file, Zipentry* entry,ZipentryCenteral *centeral,
                                        const unsigned char** buf, size_t* len)
{
    const unsigned char* p;

    unsigned short  versionMadeBy;
    unsigned short  versionToExtract;
    unsigned short  gpBitFlag;
    unsigned short  compressionMethod;
    unsigned short  lastModFileTime;
    unsigned short  lastModFileDate;
    unsigned long   crc32;
    unsigned short  extraFieldLength;
    unsigned short  fileCommentLength;
    unsigned short  diskNumberStart;
    unsigned short  internalAttrs;
    unsigned long   externalAttrs;
    unsigned long   localHeaderRelOffset;
    unsigned char*  extraField;
    unsigned char*  fileComment;
    unsigned int dataOffset;
    unsigned short lfhExtraFieldSize;


    p = *buf;

    if (read_le_int(&p[0x00]) != ENTRY_SIGNATURE) {
        fprintf(stderr, "Whoops: didn't find expected signature\n");
        printf("%d\n",read_le_int(&p[0x00]));
        return -1;
    }

    centeral->header = read_le_int(&p[0x00]);
    centeral->versionMadeBy = versionMadeBy = read_le_short(&p[0x04]);
    centeral->versionToExtract = versionToExtract = read_le_short(&p[0x06]);
    centeral->GPBitFlag =  gpBitFlag = read_le_short(&p[0x08]);
    centeral->compressionMethod = entry->compressionMethod = read_le_short(&p[0x0a]);
    centeral->lastModFileTime = lastModFileTime = read_le_short(&p[0x0c]);
    centeral->lastModFileDate = lastModFileDate = read_le_short(&p[0x0e]);
    centeral->CRC32 = crc32 = read_le_int(&p[0x10]);
    centeral->compressedSize =  entry->compressedSize = read_le_int(&p[0x14]);
    centeral->uncompressedSize = entry->uncompressedSize = read_le_int(&p[0x18]);
    centeral->fileNameLength =  entry->fileNameLength = read_le_short(&p[0x1c]);
    centeral->extraFieldLength = extraFieldLength = read_le_short(&p[0x1e]);
    centeral->fileCommentLength= fileCommentLength = read_le_short(&p[0x20]);
    centeral->diskNumberStart =  diskNumberStart = read_le_short(&p[0x22]);
    centeral->internalAttrs =  internalAttrs = read_le_short(&p[0x24]);
    centeral->externalAttrs =  externalAttrs = read_le_int(&p[0x26]);
    centeral->localHeaderRelOffset =  entry->localHeaderRelOffset = localHeaderRelOffset = read_le_int(&p[0x2a]);

    p += ENTRY_LEN;

    // filename
    if (entry->fileNameLength != 0)
    {
        centeral->fileName = new unsigned char[centeral->fileNameLength+1];
        entry->fileName = new unsigned char[centeral->fileNameLength+1];
        memset((void*)centeral->fileName, 0x0, centeral->fileNameLength+1);
        memset((void*)entry->fileName, 0x0, centeral->fileNameLength+1);
        memcpy((void*)centeral->fileName, p, centeral->fileNameLength);
        memcpy((void*)entry->fileName, p, centeral->fileNameLength);
    }
    else
    {
        centeral->fileName = entry->fileName = NULL;
    }
    p += entry->fileNameLength;

    if (extraFieldLength != 0)
    {
        centeral->extraField = new unsigned char[extraFieldLength+1];
        memset((void*)centeral->extraField, 0x0, extraFieldLength+1);
        memcpy((void*)centeral->extraField, p, extraFieldLength);
    }
    else
    {
        centeral->extraField = extraField = NULL;
    }
    p += extraFieldLength;

    // comment, if any
    if (fileCommentLength != 0)
    {
        //centeral->fileComment =  fileComment = p;
        centeral->fileComment = new unsigned char[fileCommentLength+1];
        memset((void*)centeral->fileComment, 0x0, fileCommentLength+1);
        memcpy((void*)centeral->fileComment, p, fileCommentLength);
    }
    else
    {
        centeral->fileComment = fileComment = NULL;
    }
    p += fileCommentLength;

    *buf = p;
    return 0;
}



int initApkFile(Zipfile* file, const char* strFilePath)
{
    if (NULL == file || NULL == strFilePath)
    {
        return 99;
    }
    int iRet = 0;
    const unsigned char* buf = NULL;
    size_t bufsize = 0;
    const unsigned char* eocd = NULL;
    const unsigned char* p = NULL;
    const unsigned char* start = NULL ;

    int err;
    FILE *f = fopen(strFilePath, "rb");
    if (NULL == f)
    {
        iRet = 1;
        return iRet;
    }
    fseek(f,0, SEEK_END);

    // too small to be a ZIP archive?
    int iFSize = ftell(f);
    if (iFSize < EOCD_LEN)
    {
        fprintf(stderr, "Length is %d -- too small\n", bufsize);
        int iRet = 2;
        return iRet;
    }

    unsigned char *szBuff = new unsigned char[MAX_EOCD_SEARCH+1];
    memset(szBuff, 0x0, MAX_EOCD_SEARCH+1);

    int iReadLen;
    start = szBuff;
    // find the end-of-central-dir magic
    if (iFSize > MAX_EOCD_SEARCH)
    {
        //fseek(f,iFSize-MAX_EOCD_SEARCH,SEEK_SET);
        fseek(f,iFSize-MAX_EOCD_SEARCH,SEEK_SET);
        unsigned int tt = ftell(f);
        iReadLen = fread(szBuff,1,MAX_EOCD_SEARCH,f);
    }
    else
    {
        fseek(f,0,SEEK_SET);
        iReadLen = fread(szBuff, iFSize, 1, f);
    }

    p = szBuff + iReadLen-4;
    while (p >= start)
    {
        if(*p == 0x50)
        {
            if ( read_le_int(p) == CD_SIGNATURE)
            {
                eocd = p;
                break;
            }
        }
        p--;
    }
    if (p < start)
    {
        fprintf(stderr, "EOCD not found, not Zip\n");
        iRet = 3;
        return iRet;
    }

    // 解析eocd
    int len = szBuff+iReadLen-eocd;

    if (len < EOCD_LEN)
    {
        // looks like ZIP file got truncated
        fprintf(stderr, " Zip EOCD: expected >= %d bytes, found %d\n",
            EOCD_LEN, len);
        iRet = 4;
        return iRet;
    }

    file->disknum = read_le_short(&eocd[0x04]);
    file->diskWithCentralDir = read_le_short(&eocd[0x06]);
    file->entryCount = read_le_short(&eocd[0x08]);
    file->totalEntryCount = read_le_short(&eocd[0x0a]);
    file->centralDirSize = read_le_int(&eocd[0x0c]);
    file->centralDirOffest = read_le_int(&eocd[0x10]);
    file->commentLen = read_le_short(&eocd[0x14]);

    if (file->commentLen > 0) {
        if (EOCD_LEN + file->commentLen > len) {
            fprintf(stderr, "EOCD(%d) + comment(%d) exceeds len (%d)\n",
                EOCD_LEN, file->commentLen, len);
            return NULL;
        }
        file->comment = new unsigned char[file->commentLen+1];
        memset((void*)file->comment, 0x0, file->commentLen+1);
        memcpy((void*)file->comment, eocd + EOCD_LEN, file->commentLen);
        //file->comment = buf + EOCD_LEN;
    }

    // 解析central_dir
    int iAleardySize = 0;
    size_t sizeLen ;//= (buf+bufsize)-p;
    Zipentry* pTempEntry = NULL;
    ZipentryCenteral * pTempCenteral = NULL;
    p = szBuff;
    iReadLen = 0;
    for (int i = 0; i < file->totalEntryCount; i++)
    {
        if(iReadLen - (p - szBuff) < 128)
        {
            iAleardySize += p - szBuff;
            fseek(f,file->centralDirOffest+iAleardySize,SEEK_SET);
            memset(szBuff, 0x0, MAX_EOCD_SEARCH+1);
            iReadLen = fread(szBuff, 1,MAX_EOCD_SEARCH, f);
            p = szBuff;
        }
        Zipentry* entry = ( Zipentry*)malloc(sizeof(Zipentry));
        memset(entry, 0, sizeof(Zipentry));
        ZipentryCenteral * centeral = ( ZipentryCenteral*)malloc(sizeof(ZipentryCenteral));
        memset(centeral, 0, sizeof(ZipentryCenteral));

        err = read_central_directory_entry_ex(file, entry,centeral, &p, &sizeLen);
        if (err != 0)
        {
            fprintf(stderr, "read_central_directory_entry failed\n");
            iRet = 6;
            return iRet;
        }
        if(i == 0)
        {
            file->entries = entry;
            pTempEntry = file->entries;

            file->centeral = centeral;
            pTempCenteral = file->centeral;
        }
        else
        {
            pTempEntry->next = entry;
            pTempEntry = pTempEntry->next;

            pTempCenteral->next = centeral;
            pTempCenteral = pTempCenteral->next;
        }
    }

    // sort
    for(int i = 0 ; i < file->totalEntryCount-1 ; i++)
    {
        Zipentry* entry_order_temp = file->entries;
        Zipentry* entry_order_temp_next = entry_order_temp->next;
        Zipentry* entry_order_temp_pre = NULL;//file->entries;
        for(int j = i ; j < file->totalEntryCount -1 ; j ++)
        {
            if( entry_order_temp->localHeaderRelOffset > entry_order_temp_next->localHeaderRelOffset)
            {
                if(j==i)
                {
                    entry_order_temp->next = entry_order_temp_next->next;
                    entry_order_temp_next->next =  entry_order_temp;

                    file->entries = entry_order_temp_next;
                    entry_order_temp_pre = file->entries;
                    entry_order_temp_next = entry_order_temp->next;
                }
                else
                {
                    entry_order_temp->next = entry_order_temp_next->next;
                    entry_order_temp_pre->next = entry_order_temp_next;
                    entry_order_temp_next->next = entry_order_temp;

                    entry_order_temp_pre = entry_order_temp_pre->next;
                    entry_order_temp_next = entry_order_temp->next;
                }
            }
            else
            {
                entry_order_temp_pre = entry_order_temp;
                entry_order_temp =  entry_order_temp->next;
                entry_order_temp_next = entry_order_temp->next;
            }
        }
    }

    //sort centeral
    for(int i = 0 ; i < file->totalEntryCount-1 ; i++)
    {
        ZipentryCenteral* entry_order_temp = file->centeral;
        ZipentryCenteral* entry_order_temp_next = entry_order_temp->next;
        ZipentryCenteral* entry_order_temp_pre = NULL;//file->centeral;
        for(int j = i ; j < file->totalEntryCount -1 ; j ++)
        {
            if( entry_order_temp->localHeaderRelOffset > entry_order_temp_next->localHeaderRelOffset)
            {
                if(j==i)
                {
                    entry_order_temp->next = entry_order_temp_next->next;
                    entry_order_temp_next->next =  entry_order_temp;

                    file->centeral = entry_order_temp_next;
                    entry_order_temp_pre = file->centeral;
                    entry_order_temp_next = entry_order_temp->next;
                }
                else
                {
                    entry_order_temp->next = entry_order_temp_next->next;
                    entry_order_temp_pre->next = entry_order_temp_next;
                    entry_order_temp_next->next = entry_order_temp;



                    entry_order_temp_pre = entry_order_temp_pre->next;
                    entry_order_temp_next = entry_order_temp->next;
                }

            }
            else
            {
                //entry_order = &(*entry_order)->next;
                entry_order_temp_pre = entry_order_temp;
                entry_order_temp =  entry_order_temp->next;
                entry_order_temp_next = entry_order_temp->next;
            }
        }
    }



    if (NULL != f)
    {
        fclose(f);
    }
    delete [] szBuff;
    return iRet;
}


int clearZipFileInfo(Zipfile* file)
{
    if (NULL == file)
    {
        return 1;
    }
    Zipentry* entries;
    ZipentryCenteral * centeral;

    Zipentry* entriesT;
    ZipentryCenteral * centeralT;    

    for (int i = 0; i < file->entryCount; i++)
    {
        if (0 == i)
        {
            entries = file->entries;
            centeral = file->centeral;
        }
        else
        {
            entriesT = entries;
            centeralT = centeral;

            entries = entries->next;
            centeral = centeral->next;

            if (NULL != entriesT)
            {
                free(entriesT);
                entriesT = NULL;
            }
            if(NULL != centeralT)
            {
                free(centeralT);
                centeralT = NULL;
            }
        }
        if (NULL != entries)
        {
            if (NULL != entries->fileName)
            {
                delete [] entries->fileName;
            }
        }
        //else
        //{
        //    return 2;
        //}
        if (NULL != centeral)
        {
            if(NULL != centeral->fileName)
                delete [] centeral->fileName;
            if (NULL != centeral->fileComment)
                delete [] centeral->fileComment;
            if (NULL != centeral->extraField)
                delete [] centeral->extraField;
        }
        //else
        //{
        //    return 3;
        //}
    }
    return 0;
}



