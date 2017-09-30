#ifndef PRIVATE_H
#define PRIVATE_H

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

typedef struct Zipentry
{
    unsigned int fileNameLength;
    const unsigned char* fileName;
    unsigned short compressionMethod;
    unsigned int uncompressedSize;
    unsigned int compressedSize;
    const unsigned char* data;
    int  iDataSectionLen;//DY ADD entry data all length  include header and extraField
    unsigned int localHeaderRelOffset;//DY ADD entry data Offset from begined file
    struct Zipentry* next;
} Zipentry;

/**
 * ZipentryCenteral add by DUYANG use for save Centeral struct
 *  delete file in memory
**/
typedef struct ZipentryCenteral
{
    unsigned int  header;
    unsigned short versionMadeBy;
    unsigned short versionToExtract;
    unsigned short GPBitFlag;
    unsigned short compressionMethod;
    unsigned short lastModFileTime;
    unsigned short lastModFileDate;
    unsigned int  CRC32;
    unsigned int  compressedSize;
    unsigned int  uncompressedSize;
    unsigned short fileNameLength;
    unsigned short extraFieldLength;
    unsigned short fileCommentLength;
    unsigned short diskNumberStart;
    unsigned short internalAttrs;
    unsigned int  externalAttrs;
    unsigned int  localHeaderRelOffset;
    const unsigned char* fileName;
    const unsigned char* extraField;
    const unsigned char* fileComment;

    struct ZipentryCenteral* next;

} ZipentryCenteral;
/**
 * DataDescriptor add by DUYANG use for save Centeral struct
 *  delete file in memory
 *  data descriptor㏒o車?車迆㊣那那??????t?1???芍那?㏒????芍11??車D?迆?角車|米?header?D赤“車?㊣那??℅???米?米迆㏒3??谷豕?a㏒㊣那㊣2??芍3???㏒?
 * ???車?迆?1?????t?∩那y?Yo車
**/
typedef struct DataDescriptor
{
     unsigned int  header;
     unsigned int  CRC32;
     unsigned int  compressedSize;
     unsigned int  uncompressedSize;

}DataDescriptor;

typedef struct Zipfile
{
    const unsigned char *buf;
    size_t bufsize;

    // Central directory
    unsigned short  disknum;            //mDiskNumber;
    unsigned short  diskWithCentralDir; //mDiskWithCentralDir;
    unsigned short  entryCount;         //mNumEntries;
    unsigned short  totalEntryCount;    //mTotalNumEntries;
    unsigned int    centralDirSize;     //mCentralDirSize;
    unsigned int    centralDirOffest;  // offset from first disk  //mCentralDirOffset;
    unsigned short  commentLen;         //mCommentLen;
    const unsigned char*  comment;            //mComment;

    Zipentry* entries;

    //ZipentryCenteral * centeral add by DUYANG use for delete file from zip-file in memory
    ZipentryCenteral * centeral;

} Zipfile;

int read_central_dir(Zipfile* file);

unsigned int read_le_int(const unsigned char* buf);
unsigned int read_le_short(const unsigned char* buf);

#endif // PRIVATE_H

