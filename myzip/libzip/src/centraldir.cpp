#include "private.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

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

unsigned int
read_le_int(const unsigned char* buf)
{
    return buf[0] | (buf[1] << 8) | (buf[2] << 16) | (buf[3] << 24);
}

unsigned int
read_le_short(const unsigned char* buf)
{
    return buf[0] | (buf[1] << 8);
}

static int
read_central_dir_values(Zipfile* file, const unsigned char* buf, int len)
{
    if (len < EOCD_LEN)
    {
        // looks like ZIP file got truncated
        fprintf(stderr, " Zip EOCD: expected >= %d bytes, found %d\n",
                EOCD_LEN, len);
        return -1;
    }

    file->disknum = read_le_short(&buf[0x04]);
    file->diskWithCentralDir = read_le_short(&buf[0x06]);
    file->entryCount = read_le_short(&buf[0x08]);
    file->totalEntryCount = read_le_short(&buf[0x0a]);
    file->centralDirSize = read_le_int(&buf[0x0c]);
    file->centralDirOffest = read_le_int(&buf[0x10]);
    file->commentLen = read_le_short(&buf[0x14]);

    if (file->commentLen > 0) {
        if (EOCD_LEN + file->commentLen > len) {
            fprintf(stderr, "EOCD(%d) + comment(%d) exceeds len (%d)\n",
                    EOCD_LEN, file->commentLen, len);
            return -1;
        }
        file->comment = buf + EOCD_LEN;
    }

    return 0;
}

static int
read_central_directory_entry(Zipfile* file, Zipentry* entry,ZipentryCenteral *centeral,
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
    const unsigned char*  extraField;
    const unsigned char*  fileComment;
    unsigned int dataOffset;
    unsigned short lfhExtraFieldSize;


    p = *buf;

    if (*len < ENTRY_LEN) {
        fprintf(stderr, "cde entry not large enough\n");
        return -1;
    }

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
        centeral->fileName = entry->fileName = p;
    }
    else
    {
        centeral->fileName = entry->fileName = NULL;
    }
    p += entry->fileNameLength;

//     if(strstr((char*)centeral->fileName,".dex"))
//     {
//         printf("dex");

//     }
    // extra field
    if (extraFieldLength != 0)
    {
        centeral->extraField = extraField = p;
    }
    else
    {
        centeral->extraField = extraField = NULL;
    }
    p += extraFieldLength;

    // comment, if any
    if (fileCommentLength != 0)
    {
        centeral->fileComment =  fileComment = p;
    }
    else
    {
        centeral->fileComment = fileComment = NULL;
    }
    p += fileCommentLength;

    *buf = p;

    // the size of the extraField in the central dir is how much data there is,
    // but the one in the local file header also contains some padding.
    p = file->buf + localHeaderRelOffset;
    extraFieldLength = read_le_short(&p[0x1c]);

    dataOffset = localHeaderRelOffset + LFH_SIZE
        + entry->fileNameLength + extraFieldLength;

    entry->data = file->buf + dataOffset;
    //DY add
    entry->iDataSectionLen = entry->fileNameLength + extraFieldLength + LFH_SIZE ;
    if(entry->compressionMethod == 0 )//(STORED)
    {
        entry->iDataSectionLen += entry->uncompressedSize;
    }
    else
    {
        entry->iDataSectionLen += entry->compressedSize;
    }
    if(p[0x06]&0x08)
    {
        entry->iDataSectionLen += sizeof(DataDescriptor);
        const unsigned char* data1 = entry->data +entry->iDataSectionLen - (entry->fileNameLength + extraFieldLength + LFH_SIZE) ;
        while(true)
        {
            if(data1[0] == 0x50 && data1[1] == 0x4b &&
                    ( (data1[2] == 0x01 && data1[3] == 0x02) || (data1[2] == 0x03 && data1[3] == 0x04) ))
            {
                break;
            }
            else
            {
                data1 ++;
                entry->iDataSectionLen ++;
            }
        }
    }
     // Add end DY
#if 0
    printf("file->buf=%p entry->data=%p dataOffset=%x localHeaderRelOffset=%d "
           "entry->fileNameLength=%d extraFieldLength=%d\n",
           file->buf, entry->data, dataOffset, localHeaderRelOffset,
           entry->fileNameLength, extraFieldLength);
#endif
    return 0;
}

/*
 * Find the central directory and read the contents.
 *
 * The fun thing about ZIP archives is that they may or may not be
 * readable from start to end.  In some cases, notably for archives
 * that were written to stdout, the only length information is in the
 * central directory at the end of the file.
 *
 * Of course, the central directory can be followed by a variable-length
 * comment field, so we have to scan through it backwards.  The comment
 * is at most 64K, plus we have 18 bytes for the end-of-central-dir stuff
 * itself, plus apparently sometimes people throw random junk on the end
 * just for the fun of it.
 *
 * This is all a little wobbly.  If the wrong value ends up in the EOCD
 * area, we're hosed.  This appears to be the way that everbody handles
 * it though, so we're in pretty good company if this fails.
 */
int
read_central_dir(Zipfile *file)
{
    do
    {
        int err;

        const unsigned char* buf = file->buf;
        size_t bufsize = file->bufsize;
        const unsigned char* eocd = NULL;
        const unsigned char* p = NULL;
        const unsigned char* start = NULL ;
        size_t len;
        int i;

        // too small to be a ZIP archive?
        if (bufsize < EOCD_LEN)
        {
            fprintf(stderr, "Length is %d -- too small\n", bufsize);
            //goto bail;
             break;
        }

        // find the end-of-central-dir magic
        if (bufsize > MAX_EOCD_SEARCH)
        {
            start = buf + bufsize - MAX_EOCD_SEARCH;
        }
        else
        {
            start = buf;
        }
        p = buf + bufsize - 4;
        while (p >= start)
        {
            if (*p == 0x50 && read_le_int(p) == CD_SIGNATURE)
            {
                eocd = p;
                break;
            }
            p--;
        }
        if (p < start)
        {
            fprintf(stderr, "EOCD not found, not Zip\n");
            //goto bail;
             break;
        }

        // extract eocd values
        err = read_central_dir_values(file, eocd, (buf+bufsize)-eocd);
        if (err != 0)
        {
            //goto bail;
             break;
        }

        if (file->disknum != 0
              || file->diskWithCentralDir != 0
              || file->entryCount != file->totalEntryCount)
        {
            fprintf(stderr, "Archive spanning not supported\n");
            //goto bail;
             break;
        }

        // Loop through and read the central dir entries.
        p = buf + file->centralDirOffest;
        len = (buf+bufsize)-p;

        Zipentry* pTempEntry = NULL;
        ZipentryCenteral * pTempCenteral = NULL;
        for (i=0; i < file->totalEntryCount; i++)
        {
            Zipentry* entry = ( Zipentry*)malloc(sizeof(Zipentry));
            memset(entry, 0, sizeof(Zipentry));

            //DUYANG ADD --->USE FOR DELETE FILE IN MEMORY
            ZipentryCenteral * centeral = ( ZipentryCenteral*)malloc(sizeof(ZipentryCenteral));
            memset(centeral, 0, sizeof(ZipentryCenteral));
            //DUYANG ADD --->end


            err = read_central_directory_entry(file, entry,centeral, &p, &len);
            if (err != 0)
            {
                fprintf(stderr, "read_central_directory_entry failed\n");
                free(entry);
                //goto bail;
                break;
            }

            // add it to our list by order
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

    //        add it to our list by reversed order
    //        entry->next = file->entries;
    //        file->entries = entry;

        }
        if (err != 0)
        {
            break;
        }

        //--------------add by DUYANG order start------------------
        //order entry
        for(int i = 0 ; i < file->totalEntryCount-1 ; i++)
        {
            //Zipentry** entry_order =  &file->entries;
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
                    //entry_order = &(*entry_order)->next;
                    entry_order_temp_pre = entry_order_temp;
                    entry_order_temp =  entry_order_temp->next;
                    entry_order_temp_next = entry_order_temp->next;
                }
            }
        }

        //order centeral
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
        //--------------add by DUYANG order end------------------
        return 0;
    }while(0);
//bail:
    return -1;
}
