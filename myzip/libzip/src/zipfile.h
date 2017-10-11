/*
 * Copyright (C) 2008 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef _ZIPFILE_ZIPFILE_H
#define _ZIPFILE_ZIPFILE_H

#include <stddef.h>
#include "private.h"
#include"zip.h"
#include <string>
#include <vector>

//#ifdef _WIN32
//#define DLLEXPORT
//#ifdef DLLEXPORT
//#define EXPORT __declspec(dllexport)
//#else
//#define EXPORT __declspec(dllimport)
//#endif
//#define EXPORT
//#endif

using namespace std;
typedef std::string typeFileInfo;

#ifdef _WIN32
#define EXPORT _declspec(dllexport)
#else
#define EXPORT __attribute__ ((visibility("default")))
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef void* zipfile_t;
typedef void* zipentry_t;

// Provide a buffer.  Returns NULL on failure.
EXPORT zipfile_t init_zipfile(const void* data, size_t size);

EXPORT zipfile_t init_zipfile_EX(const char* zipFilePath);

// Release the zipfile resources.
EXPORT void release_zipfile(zipfile_t file);

// Get a named entry object.  Returns NULL if it doesn't exist
// or if we won't be able to decompress it.  The zipentry_t is
// freed by release_zipfile()
EXPORT zipentry_t lookup_zipentry(zipfile_t file, const char* entryName);

EXPORT int  lookup_Dir(zipfile_t file, const char* entryName);

// Return the size of the entry.
EXPORT size_t get_zipentry_size(zipentry_t entry);

// return the filename of this entry, you own the memory returned
EXPORT char* get_zipentry_name(zipentry_t entry);

// The buffer must be 1.001 times the buffer size returned
// by get_zipentry_size.  Returns nonzero on failure.
EXPORT int decompress_zipentry(zipentry_t entry, void* buf, int bufsize);

// iterate through the entries in the zip file.  pass a pointer to
// a void* initialized to NULL to start.  Returns NULL when done
EXPORT zipentry_t iterate_zipfile(zipfile_t file, void** cookie);

//int compress_zipentry(void *in ,unsigned long  inlen ,void* out, unsigned long *outsize);

EXPORT int ZipAddFiles(zipFile zf, const char * lpszFileNameInZip, const char * lpszFiles, bool bUtf8 = false, bool bPassByList = false);
EXPORT int ZipAddFile(zipFile zf, const char * lpszFileNameInZip, const char * lpszFilePath, bool bUtf8 = false,bool bisDir = false, bool bPassByList = false);


EXPORT zipfile_t init_zipfile_delete(const void* data, size_t size);

//ѹ���ļ�
EXPORT int ZipCompress(const char * lpszSourceFiles, const char * lpszDestFile, bool bUtf8 = false,bool bZipDir = true/*�ļ���ѹ��ʱ���Ƿ�ѹ����Ŀ¼*/ , bool bPassByList = false);
//ѹ�����Ŀ¼
int ZipCompress_ForMultiFiles(const char * lpszSourceFiles, const char * lpszDestFile, bool bUtf8,bool bZipDir,bool bPassByList);
//��ѹ�ļ�
EXPORT int ZIPDecompression(const char *apkpath,const char * sTempDir);

EXPORT int ZIPDecompressionFile(const char *apkpath,const char * sTempDir,char * pFile);

//��ѹ�����ļ�
int ZIPDecompression_ForSinlgeFile(const char *apkpath,const char * sTempDir,const char * single_file_name);

//��ѹ����ļ����ѹ���Ŀ¼
int ZIPDecompression_ForMultiFile(const char *apkpath,const char * sTempDir,vector<string> list,bool isdir);
/**
 * @brief addDataOrFileToZip ����ļ���zip�ļ� ����ѹԭ�ļ�������
 * @param pZipFilePath       Ҫ����ļ���zip
 * @param pAddPathNameInZip  �ļ���zip�����·��������
 * @param pInFile            ���iDataLenΪ0 �����Ҫ����ӵ��ļ�·��  ���iDataLen����0 �����Ҫ����ӵ������׵�ַ
 * @param iDataLen           Ҫ��ӵ����ݳ���
 * @param bNewZip            if create new zip file
 * @return
 */
EXPORT int addDataOrFileToZip(const char * pZipFilePath ,const char * pAddPathNameInZip,const char * pInFile,int iDataLen ,bool bNewZip = false);

/**
 * @brief addFilePathToZip  ����ļ��е�zip�ļ� ����ѹԭ�ļ�������
 * @param pZipFilePath      zip�ļ�·��
 * @param pAddPathNameInZip �ļ�����zip�����·��
 * @param pInFile           �ļ���·��
 * @param bNewZip           if create new zip file
 * @return 1 ���� !1ʧ��
 */
EXPORT int addFilePathToZip(const char * pZipFilePath ,const char * pAddPathNameInZip,const char * pInFile,bool bNewZip = false);
/**
 * @brief DeleteFileFromZip ��zip�ļ�ɾ��һ���ض����ļ�����Ŀ¼
 * @param pZipFilePath      zip·��
 * @param pDeletePathNameInZip Ҫ��ɾ�����ļ�����Ŀ¼ ��zip����
 * @param bool bDir            pDeletePathNameInZip is dir or file
 * @return 1 sucess 0 faile
 */
EXPORT int DeleteFileFromZip(const char * pZipFilePath ,const char * pDeletePathNameInZip,bool bDir);
/**
 * @brief DeleteFilesFromZip ��zip�ļ�ɾ���ض����ļ�
 * @param pZipFilePath       zip·��
 * @param vcDeletePath       Ҫ��ɾ�����ļ�����Ŀ¼ ��zip����
 * @return 1 sucess 0 faile
 */
EXPORT int DeleteFilesFromZip(const char * pZipFilePath , std::vector<std::string> vcDeletePath);
/** �ж��ļ�����Ŀ¼�Ƿ����zip��
 * @brief FileOrPathInZip
 * @param pZipFilePath
 * @param pPathNameInZip
 * @param bDir
 * @return
 */
int FileOrPathInZip(const char *pZipFilePath, const char *pPathNameInZip ,bool bDir);

EXPORT bool bIsFindFile(Zipentry * entry,  const char * pfilepath ,bool bDir);
EXPORT bool bIsIncudeFindFile(Zipentry * entry,  std::vector<std::string> vcDeletePath);
/**
 * @brief filetime
 * @param f     name of file to get info on
 * @param tmzip return value: access, modific. and creation times
 * @param dt    dostime
 * @return
 */
uLong filetime(char *f, tm_zip * tmzip, uLong * dt);


/*************************************************************************************************************************************/
/**                                        ���²�����ʽ�Դ��͵�apk������ռ�ù����ڴ�                                                        **/
/*************************************************************************************************************************************/

/** @brief ͨ������APK�� ����Ƿ������Ӧ�ļ�
 *  @param strFilePath APK��·��
 *  @param strZipFilePath �������ļ�
 *  @return 0 ���� !0ʧ��
 * */
EXPORT int CheckFile(const char *strFilePath, char *strZipFilePath);


/** @brief ����ѹ����Ƿ����������ļ�����Ŀ¼
  * @param file ��Ӧ��ʼ��������ļ������ṹ
  * @param strZipFilePath �ļ���Ŀ¼��zip�����·��
  * @param isDir �Ƿ�Ŀ¼
  * @return 0 ���� !0ʧ��
  */
EXPORT int checkZipFile(Zipfile* file, const char* strZipFilePath, bool isDir);

/** @brief ��ǰ�ļ�����Ŀ¼�Ƿ����������ļ�����Ŀ¼
 *  @param strFileName ��ǰzip�ļ�
 *  @param strZipFilePath ����֤�ļ�����Ŀ¼
 *  @param isDir ����֤�ļ��Ƿ���Ŀ¼
 *  @return 0 ���� !0ʧ��
 * */
EXPORT int checkZipFileEx(const char* strFileName, const char* strZipFilePath, bool isDir);

/** @brief ��ѹ�����ļ�
 *  @param pstrFilePath ����ѹ��zip�ļ�
 *  @param pstrSavePath �ļ�����Ŀ¼
 *  @param strZipFilePath ����ѹ�ļ����zip�ļ�·��
 *  @return 0 ���� !0ʧ��
 */
EXPORT int DecompressionFile(const char *pstrFilePath, const char *pstrSavePath, const char *strZipFilePath);

/** @brief ��ѹ����ļ�
 *  @param pstrFilePath ����ѹ��zip�ļ�
 *  @param pstrSavePath �ļ�����Ŀ¼
 *  @param strZipDirPath ����ѹ�ļ����zip�ļ�·��
 *  @param fuzzyMatch �Ƿ����ģ��ƥ��
 *  @return 1 ���� !1ʧ��
 */
EXPORT int DecompressionFiles(vector<char*> vecZipPath, const char *pstrSavePath ,const char *strZipFilePath, bool fuzzyMatch = false );

/** @brief ��ѹ����ļ� ͨ��ָ�������ļ��б��Լ�����ļ��б�
 *  @param len ����ѹ��zip�ļ����� ��2 3 ������Ӧ
 *  @param pstrFilePath ����ѹ��zip�ļ� ����ļ��á�;���ָ�
 *  @param pstrSavePath �ļ�����Ŀ¼ ����ļ��á�;���ָ� ���ѹ�ļ���Ӧ(Ŀǰ��֧�֣� ���Ŀ¼Ϊ�����ļ���)
 *  @param strZipDirPath ����ѹ�ļ����zip�ļ�·��
 *  @param fuzzyMatch �Ƿ����ģ��ƥ��
 *  @return 1 ���� !1ʧ��
 */
EXPORT int DecompressionFilesEx(int len, const char* listZipPath, const char *listSavePath ,const char *strZipFilePath, bool fuzzyMatch = false );


/** @brief ��ѹ����Ŀ¼
 *  @param pstrFilePath ����ѹ��zip�ļ�
 *  @param pstrSavePath �ļ�����Ŀ¼
 *  @param strZipDirPath ����ѹ�ļ����zip�ļ�·��
 *  @return 1 ���� !1ʧ��
 */
EXPORT int DecompressionDir(const char *pstrFilePath, const char *pstrSavePath, const char *strZipDirPath);

/** @brief ��ѹ�����zip�ļ�
 *  @param strSave
 */
EXPORT int DecompressionZip(const char *strSavePath, const char *strZipFilePath);

/** @brief ����ļ���zip��
 *  @param strSrcFile ����ӵ�Դ�ļ�
 *  @param strZipPath ����ļ���zip�����Ŀ¼
 *  @param zipFilePack zip�ļ�
 *  @return 0 ���� !0ʧ��
 * */
EXPORT int AddFileToZip(const char* strSrcFile, const char* strZipPath, const char* zipFilePack);

/** @brief ����ļ��б�zip��
 *  @param count �ļ�����
 *  @param strSrcFile ����ӵ�Դ�ļ� ����ļ��á�;���ָ�
 *  @param strZipPath ����ļ���zip�����Ŀ¼ ����ļ��á�;���ָ�
 *  @param zipFilePack zip�ļ�
 *  @return 0 ���� !0ʧ��
 * */
EXPORT int AddFileToZipEx(int count, const char* strSrcFile, const char* strZipPath, const char* zipFilePack);


/** @brief ���Ŀ¼��zip�� �����Ѵ��ڵ�Ŀ¼�ṹ��Ŀ¼��ӵ���Ӧ��Ŀ¼����,Ŀ¼���ļ��б���Ŀ¼�ṹ�������
 *  @param strSrcDirPath ����ӵ�Դ�ļ�Ŀ¼
 *  @param strZipPath ����ļ���zip�����Ŀ¼
 *  @param zipFilePack zip�ļ�
 *  @return 0 ���� !0ʧ��
 * */
EXPORT int AddDirToZip(const char* strSrcDirPath, const char* strZipPath, const char* zipFilePack, bool bNewZip = false);

/** @brief ����ѹ���ļ�
 * */
EXPORT int CreateZipFile(const char* strPath, const char* strZipName);

/** @brief ɾ��zip���е��ļ� �ļ��� ����mini zip �ӿ�
 *  @param zipFilePack Դzipѹ����
 *  @param strDelName ��ɾ���ļ� Ϊzip�ļ������·��
 *  @return 0 ���� !0ʧ��
 * */
EXPORT int DeleteInZipFile(const char* zipFilePack, const char* strDelName,bool bIsDir = false);

/** @brief ɾ��zip���е��ļ� �ļ��� ���� Դ��̫ģʽ
 *  @param zipFilePack Դzipѹ����
 *  @param strDelName ��ɾ���ļ� Ϊzip�ļ������·��
 *  @return 1 ���� !1 ʧ��
 * */
EXPORT int DeleteInZipFileEx(const char* zipFilePack, const char* strDelName,bool bDir = false);

/** @brief ɾ��zip���е��ļ��б�
 *  @param zipFilePack Դzipѹ����
 *  @param strDelName ��ɾ���ļ� Ϊzip�ļ������·�� ����ļ��á�;���ָ�
 *  @return 1 ���� !1 ʧ��
 * */
EXPORT int deleteFileInApkEx(const char* strFilePath, const char* deleteFilePath);

/** @brief ��ʼ��zip�ļ�
 *  @param file �ļ��ṹ����
 *  @param strFilePath zip�ļ�·��
 *  @return 0 ���� !0ʧ��
 * */
EXPORT int initApkFile(Zipfile* file, const char* strFilePath);

/** @brief ����������
 *  @return 0 ���� !0ʧ��
 * */
EXPORT int clearZipFileInfo(Zipfile* file);

/** @brief ��ȡ�ļ�������Ϣ */
EXPORT int read_central_directory_entry_ex(Zipfile* file, Zipentry* entry,ZipentryCenteral *centeral,
    const unsigned char** buf, size_t* len);

/*************************************************************************************************************************************/
/*************************************************************************************************************************************/

#ifdef __cplusplus
} // extern "C"
#endif

#endif // _ZIPFILE_ZIPFILE_H
