//
//  FileServerHandler.cpp
//  PthreadTest
//
//  Created by VincentChou on 14-8-21.
//  Copyright (c) 2014年 VincentChou. All rights reserved.
//

#include "FileServerHandler.h"
#include "ODSocket.h"
#include "XOutputStream.h"
#include "Protos.pb.h"

#include <sys/stat.h>
#include <unistd.h>
#include <time.h>

long getFileModifyTime(const char* file)
{
    struct tm* clock;				// create a time structure
    struct stat attrib;			// create a file attribute structure
    stat(file, &attrib);		// get the attributes of afile.txt
    clock = gmtime(&(attrib.st_mtime));
    
    return clock->tm_gmtoff;
}


extern pthread_mutex_t      s_mainMutex;
extern pthread_cond_t       s_mainCondition;

//fs socket
static pthread_t            s_fsRevThread;
static ODSocket             s_fsSocket;

void* revFSLoop(void * p)
{
    while (true)
    {
        if (s_fsSocket.Select() != 0) continue;
        
        const int len = 256;
        char* buf = new char[256];
        memset(buf, '\0', len);
        int revlen = s_fsSocket.Recv(buf, len, 0);
        
        if (revlen <= 0)
        {
            printf("ERROR: console socket rev error.");
            pthread_exit(NULL);
            return 0;
        }
        
        //parse response
        do
        {
            // recv start flag
            char startflag[13]={0};
            memcpy(startflag, buf, 12);
            if (strcmp(startflag,"RuntimeSend:") != 0) break;
            
            char* pos = buf + 12;
            unsigned short protuNum;
            memcpy(&protuNum, pos, 2);
            
            pos += 2;
            unsigned short protulen;
            memcpy(&protulen, pos, 2);
            
            pos += 2;
            runtime::FileSendComplete fsComp;
            fsComp.ParseFromArray(pos, protulen);
            printf("complete filename = %s, result %d, error = %d",
                   fsComp.file_name().c_str(),
                   fsComp.result(),
                   fsComp.error_num());
            
        } while (false);
        
        delete [] buf;
        
        pthread_cond_signal(&s_mainCondition);
    }
    return 0;
}

bool fsSocketConnect(const char* ip, int port)
{
    s_fsSocket.Create(AF_INET, SOCK_STREAM);
    bool suc = s_fsSocket.Connect(ip, port);
    if (!suc)
    {
        printf("ERROR: Can't connect %s:%d\n", ip, port);
        return false;
    }
    return true;
}

void fsRevThreadBegin()
{
    pthread_create(&s_fsRevThread, NULL, revFSLoop, NULL);
}

int sendFile()
{
    XOutputStream* stream = new XOutputStream(256);
    stream->writeUTF("RuntimeSend:");
    //proto count
    stream->writeShort(1);
    //proto length
    
    runtime::FileSendProtos proto;
    proto.set_file_name("send.test");
    proto.set_package_seq(1);
    proto.set_package_sum(1);
    proto.set_compress_type(runtime::FileSendProtos_CompressType_NO_COMPRESS);
    
    const char* path = "/Users/Vincent/Documents/cocos/CocosJSGame/send.test";
    FILE *fp = fopen(path, "r");
    unsigned long fsize;
    fseek(fp,0,SEEK_END);
    fsize = ftell(fp);
    fseek(fp,0,SEEK_SET);
    unsigned char* pBuffer = new unsigned char[fsize];
    fsize = fread(pBuffer,sizeof(unsigned char), fsize,fp);
    fclose(fp);
    proto.set_content_size(fsize);
    
    stream->writeShort(proto.ByteSize());
    stream->writeProtobuf(proto);
    
    //write file stream
    stream->writeData(pBuffer, fsize);
    
    stream->description();
    
    int ret = s_fsSocket.Send(stream->getStreamData(), stream->getDataLength());
    
    delete stream;
    
    return ret;
}
