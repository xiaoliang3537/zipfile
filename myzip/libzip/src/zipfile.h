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

//压缩文件
EXPORT int ZipCompress(const char * lpszSourceFiles, const char * lpszDestFile, bool bUtf8 = false,bool bZipDir = true/*文件夹压缩时候是否压缩根目录*/ , bool bPassByList = false);
//压缩多个目录
int ZipCompress_ForMultiFiles(const char * lpszSourceFiles, const char * lpszDestFile, bool bUtf8,bool bZipDir,bool bPassByList);
//解压文件
EXPORT int ZIPDecompression(const char *apkpath,const char * sTempDir);

EXPORT int ZIPDecompressionFile(const char *apkpath,const char * sTempDir,char * pFile);

//解压单个文件
int ZIPDecompression_ForSinlgeFile(const char *apkpath,const char * sTempDir,const char * single_file_name);

//解压多个文件或解压多个目录
int ZIPDecompression_ForMultiFile(const char *apkpath,const char * sTempDir,vector<string> list,bool isdir);
/**
 * @brief addDataOrFileToZip 添加文件到zip文件 不解压原文件到磁盘
 * @param pZipFilePath       要添加文件的zip
 * @param pAddPathNameInZip  文件在zip里面的路径和名称
 * @param pInFile            如果iDataLen为0 则代表要被添加的文件路径  如果iDataLen大于0 则代表要被添加的数据首地址
 * @param iDataLen           要添加的数据长度
 * @param bNewZip            if create new zip file
 * @return
 */
EXPORT int addDataOrFileToZip(const char * pZipFilePath ,const char * pAddPathNameInZip,const char * pInFile,int iDataLen ,bool bNewZip = false);

/**
 * @brief addFilePathToZip  添加文件夹到zip文件 不解压原文件到磁盘
 * @param pZipFilePath      zip文件路径
 * @param pAddPathNameInZip 文件夹在zip里面的路径
 * @param pInFile           文件夹路径
 * @param bNewZip           if create new zip file
 * @return 1 正常 !1失败
 */
EXPORT int addFilePathToZip(const char * pZipFilePath ,const char * pAddPathNameInZip,const char * pInFile,bool bNewZip = false);
/**
 * @brief DeleteFileFromZip 从zip文件删除一个特定的文件或者目录
 * @param pZipFilePath      zip路径
 * @param pDeletePathNameInZip 要被删除的文件或者目录 在zip里面
 * @param bool bDir            pDeletePathNameInZip is dir or file
 * @return 1 sucess 0 faile
 */
EXPORT int DeleteFileFromZip(const char * pZipFilePath ,const char * pDeletePathNameInZip,bool bDir);
/**
 * @brief DeleteFilesFromZip 从zip文件删除特定的文件
 * @param pZipFilePath       zip路径
 * @param vcDeletePath       要被删除的文件或者目录 在zip里面
 * @return 1 sucess 0 faile
 */
EXPORT int DeleteFilesFromZip(const char * pZipFilePath , std::vector<std::string> vcDeletePath);
/** 判断文件或者目录是否存在zip中
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
/**                                        以下操作方式对大型的apk包不会占用过多内存                                                        **/
/*************************************************************************************************************************************/

/** @brief 通过输入APK包 检查是否存在相应文件
 *  @param strFilePath APK包路径
 *  @param strZipFilePath 待查找文件
 *  @return 0 正常 !0失败
 * */
EXPORT int CheckFile(const char *strFilePath, char *strZipFilePath);


/** @brief 检查解压后的是否包含输入的文件或者目录
  * @param file 对应初始化过后的文件描述结构
  * @param strZipFilePath 文件或目录在zip中相对路径
  * @param isDir 是否目录
  * @return 0 正常 !0失败
  */
EXPORT int checkZipFile(Zipfile* file, const char* strZipFilePath, bool isDir);

/** @brief 当前文件或者目录是否包含输入的文件或者目录
 *  @param strFileName 当前zip文件
 *  @param strZipFilePath 待验证文件或者目录
 *  @param isDir 待验证文件是否是目录
 *  @return 0 正常 !0失败
 * */
EXPORT int checkZipFileEx(const char* strFileName, const char* strZipFilePath, bool isDir);

/** @brief 解压单个文件
 *  @param pstrFilePath 待解压的zip文件
 *  @param pstrSavePath 文件保存目录
 *  @param strZipFilePath 待解压文件相对zip文件路径
 *  @return 0 正常 !0失败
 */
EXPORT int DecompressionFile(const char *pstrFilePath, const char *pstrSavePath, const char *strZipFilePath);

/** @brief 解压多个文件
 *  @param pstrFilePath 待解压的zip文件
 *  @param pstrSavePath 文件保存目录
 *  @param strZipDirPath 待解压文件相对zip文件路径
 *  @param fuzzyMatch 是否进行模糊匹配
 *  @return 1 正常 !1失败
 */
EXPORT int DecompressionFiles(vector<char*> vecZipPath, const char *pstrSavePath ,const char *strZipFilePath, bool fuzzyMatch = false );

/** @brief 解压多个文件 通过指定输入文件列表以及输出文件列表
 *  @param len 待解压的zip文件个数 与2 3 参数对应
 *  @param pstrFilePath 待解压的zip文件 多个文件用“;”分割
 *  @param pstrSavePath 文件保存目录 多个文件用“;”分割 与解压文件对应(目前不支持， 存放目录为单个文件夹)
 *  @param strZipDirPath 待解压文件相对zip文件路径
 *  @param fuzzyMatch 是否进行模糊匹配
 *  @return 1 正常 !1失败
 */
EXPORT int DecompressionFilesEx(int len, const char* listZipPath, const char *listSavePath ,const char *strZipFilePath, bool fuzzyMatch = false );


/** @brief 解压单个目录
 *  @param pstrFilePath 待解压的zip文件
 *  @param pstrSavePath 文件保存目录
 *  @param strZipDirPath 待解压文件相对zip文件路径
 *  @return 1 正常 !1失败
 */
EXPORT int DecompressionDir(const char *pstrFilePath, const char *pstrSavePath, const char *strZipDirPath);

/** @brief 解压输入的zip文件
 *  @param strSave
 */
EXPORT int DecompressionZip(const char *strSavePath, const char *strZipFilePath);

/** @brief 添加文件到zip中
 *  @param strSrcFile 待添加的源文件
 *  @param strZipPath 添加文件到zip的相对目录
 *  @param zipFilePack zip文件
 *  @return 0 正常 !0失败
 * */
EXPORT int AddFileToZip(const char* strSrcFile, const char* strZipPath, const char* zipFilePack);

/** @brief 添加文件列表到zip中
 *  @param count 文件个数
 *  @param strSrcFile 待添加的源文件 多个文件用“;”分割
 *  @param strZipPath 添加文件到zip的相对目录 多个文件用“;”分割
 *  @param zipFilePack zip文件
 *  @return 0 正常 !0失败
 * */
EXPORT int AddFileToZipEx(int count, const char* strSrcFile, const char* strZipPath, const char* zipFilePack);


/** @brief 添加目录到zip中 根据已存在的目录结构将目录添加到对应的目录下面,目录下文件列表按照目录结构进行添加
 *  @param strSrcDirPath 待添加的源文件目录
 *  @param strZipPath 添加文件到zip的相对目录
 *  @param zipFilePack zip文件
 *  @return 0 正常 !0失败
 * */
EXPORT int AddDirToZip(const char* strSrcDirPath, const char* strZipPath, const char* zipFilePack, bool bNewZip = false);

/** @brief 创建压缩文件
 * */
EXPORT int CreateZipFile(const char* strPath, const char* strZipName);

/** @brief 删除zip包中的文件 文件夹 采用mini zip 接口
 *  @param zipFilePack 源zip压缩包
 *  @param strDelName 待删除文件 为zip文件内相对路径
 *  @return 0 正常 !0失败
 * */
EXPORT int DeleteInZipFile(const char* zipFilePack, const char* strDelName,bool bIsDir = false);

/** @brief 删除zip包中的文件 文件夹 采用 源生太模式
 *  @param zipFilePack 源zip压缩包
 *  @param strDelName 待删除文件 为zip文件内相对路径
 *  @return 1 正常 !1 失败
 * */
EXPORT int DeleteInZipFileEx(const char* zipFilePack, const char* strDelName,bool bDir = false);

/** @brief 删除zip包中的文件列表
 *  @param zipFilePack 源zip压缩包
 *  @param strDelName 待删除文件 为zip文件内相对路径 多个文件用“;”分割
 *  @return 1 正常 !1 失败
 * */
EXPORT int deleteFileInApkEx(const char* strFilePath, const char* deleteFilePath);

/** @brief 初始化zip文件
 *  @param file 文件结构描述
 *  @param strFilePath zip文件路径
 *  @return 0 正常 !0失败
 * */
EXPORT int initApkFile(Zipfile* file, const char* strFilePath);

/** @brief 清理缓存数据
 *  @return 0 正常 !0失败
 * */
EXPORT int clearZipFileInfo(Zipfile* file);

/** @brief 获取文件描述信息 */
EXPORT int read_central_directory_entry_ex(Zipfile* file, Zipentry* entry,ZipentryCenteral *centeral,
    const unsigned char** buf, size_t* len);

/*************************************************************************************************************************************/
/*************************************************************************************************************************************/

#ifdef __cplusplus
} // extern "C"
#endif

#endif // _ZIPFILE_ZIPFILE_H
