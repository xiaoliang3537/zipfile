#include "zipfile.h"
#include <stdio.h>
#include <stdlib.h>
#ifdef _WIN32
#include <windows.h>
#endif
#include<iostream>

void dump_zipfile(FILE* to, zipfile_t file);

int GetFilePresent(const char * pPath,const char * pTail)
{
    FILE *pFile = fopen(pPath,"rb");
    do
    {
        if(pFile == NULL)
        {
            break;
        }
        fseek(pFile, 0, SEEK_END);
        long size = ftell(pFile);
        fseek(pFile,0,SEEK_SET);

        pszbuf = malloc(size);
        if(pszbuf == NULL)
        {
            break;
        }
        memset(pszbuf,0,size);
        fread(pszbuf, 1, size, pFile);
        zip = init_zipfile(pszbuf, size);
        if(zip == NULL)
        {
            break;
        }
        // get all file
        Zipfile* zipfile = (Zipfile*)zip;
        Zipentry* entry = zipfile->entries;

        int iTotaltailSize = 0;
        for (int i=0; i<zipfile->entryCount; i++)
        {
            if(strstr(entry->fileName,pTail)== entry->fileName + entry->fileNameLength - strlen(pTail))
            {
                iTotaltailSize += entry->compressedSize;
            }
            entry = entry->next;
        }
        printf("%s文件所占百分比：",pTail, (iTotaltailSize /size) *100 );
    }while(0);
    if(pFile)
    {
        fclose(pFile);
    }
    if(zip)
    {
        release_zipfile(zip);
    }
    if(pszbuf)
    {
        free(pszbuf);
    }
}


int main(int argc, char** argv)
{
    char szPath[1024] = {0};
    if(argc != 2)
    {
        std::cout<<"please input path "<<std::endl;
        return -1;
    }
    strcpy(szPath,argv[1]);
    char * pFind = strrchr(szPath,'.');
    if(!pFind)
    {
        std::cout<<"can not find . in path :"<<szPath<<std::endl;
        return false;
    }
    *pFind = 0;
    if(!ZIPDecompression(argv[1],szPath))
    {
        std::cout<<" decompress failed"<<std::endl;
        return -1;
    }
#ifdef _WIN32
    ::DeleteFile(argv[1]);
#else
    remove(argv[1]);
#endif
    if(!ZipCompress(szPath,argv[1],false,false,true))
    {
         std::cout<<" compress failed"<<std::endl;
         return -1;
    }
    std::cout<<"sucess"<<std::endl;
    return 0;
//    for (int i = 0 ; i< 5 ;i ++)
//      {
//        zipfile_t zip;
//        void *pszbuf = NULL;
//        FILE *pFile = fopen("/home/user/QT/news_aijiami67/1.apk","r");
//        do
//        {
//            if(pFile == NULL)
//            {
//                break;
//            }
//            fseek(pFile, 0, SEEK_END);
//            long size = ftell(pFile);
//            fseek(pFile,0,SEEK_SET);

//            pszbuf = malloc(size);
//            if(pszbuf == NULL)
//            {
//                break;
//            }
//            memset(pszbuf,0,size);
//            fread(pszbuf, 1, size, pFile);
//            zip = init_zipfile(pszbuf, size);
//            if(zip == NULL)
//            {
//                break;
//            }
//            // get all file
//            Zipfile* zipfile = (Zipfile*)zip;
//            Zipentry* entry = zipfile->entries;

//            for (int i=0; i<zipfile->entryCount; i++)
//            {
//                size_t ulsize = get_zipentry_size(entry);
//                ulsize *= 1.001;
//                void* scratch = NULL;
//                scratch = malloc(ulsize);
//                if(scratch == NULL)
//                {
//                    break;
//                }
//                memset(scratch,0,ulsize);
//                if(decompress_zipentry(entry, scratch, ulsize) == 0)
//                {
//                    char sztemp[256] = {0};
//                    char sFielName[256] = {0};
//                    memcpy((void*)sFielName,entry->fileName,entry->fileNameLength);
//                    sprintf(sztemp,"%s/%s","/home/user/QT/news_aijiami67/temp",sFielName);
//                    FileUtils::MakeSureDirExsits(sztemp);
//                    FILE *pSave = fopen(sztemp,"wb");
//                    if(pSave !=NULL)
//                    {
//                        fwrite(scratch, ulsize, 1, pSave);
//                        fclose(pSave);
//                    }
//                    else
//                    {
//                        printf("fopen faile ZipFile\n");
//                        break;
//                    }

//                }
//                else
//                {
//                    printf("error decompressing file\n");
//                    break;
//                }
//                free(scratch);
//                scratch = NULL;
//                entry = entry->next;
//            }

//        }while(0);
//        if(pFile)
//        {
//            fclose(pFile);
//        }
//        if(zip)
//        {
//            release_zipfile(zip);
//        }
//        if(pszbuf)
//        {
//            free(pszbuf);
//        }
//    }


    FILE* f;
    size_t size, unsize;
    void* buf;
    void* scratch;
    zipfile_t zip;
    zipentry_t entry;
    int err;
    enum { HUH, LIST, UNZIP } what = HUH;

    if (strcmp(argv[2], "-") == 0 && argc == 3)
    {
        what = LIST;
    }
    else if (strcmp(argv[2], "-u") == 0 && argc == 5)
    {
        what = UNZIP;
    }
    else
    {
        fprintf(stderr, "usage: test_zipfile ZIPFILE -l\n"
                        "          lists the files in the zipfile\n"
                        "       test_zipfile ZIPFILE -u FILENAME SAVETO\n"
                        "          saves FILENAME from the zip file into SAVETO\n");
        return 1;
    }
    
    f = fopen(argv[1], "r");
    if (f == NULL)
    {
        fprintf(stderr, "couldn't open %s\n", argv[1]);
        return 1;
    }

    fseek(f, 0, SEEK_END);
    size = ftell(f);
    rewind(f);
    
    buf = malloc(size);
    fread(buf, 1, size, f);

    zip = init_zipfile(buf, size);
    if (zip == NULL)
    {
        fprintf(stderr, "inti_zipfile failed\n");
        return 1;
    }

    fclose(f);


    switch (what)
    {
        case LIST:
            dump_zipfile(stdout, zip);
            break;
        case UNZIP:
            entry = lookup_zipentry(zip, argv[3]);
            if (entry == NULL)
            {
                fprintf(stderr, "zip file '%s' does not contain file '%s'\n",
                                argv[1], argv[1]);
                return 1;
            }
            f = fopen(argv[4], "w");
            if (f == NULL)
            {
                fprintf(stderr, "can't open file for writing '%s'\n", argv[4]);
                return 1;
            }
            unsize = get_zipentry_size(entry);
            size = unsize * 1.001;
            scratch = malloc(size);
            printf("scratch=%p\n", scratch);
            err = decompress_zipentry(entry, scratch, size);
            if (err != 0)
            {
                fprintf(stderr, "error decompressing file\n");
                return 1;
            }
            fwrite(scratch, unsize, 1, f);
            free(scratch);
            fclose(f);
            break;
    }
    
    free(buf);
   // ZipCompress("/home/user/Desktop/duyangunzip.dat","/home/user/Desktop/yasuohoudat.zip");



    return 0;
}

