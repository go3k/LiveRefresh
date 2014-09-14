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
#include "Utils.h"

extern pthread_cond_t              s_fsCondition;

//fs socket
static pthread_t                s_fsRevThread;
static ODSocket                 s_fsSocket;

extern std::vector<fileinfo>    s_addFiles;

void* revFSLoop(void * p)
{
    while (true)
    {
        if (s_fsSocket.Select() != 0) continue;
        
        char startflag[13]={0};
        s_fsSocket.Recv(startflag, sizeof(startflag) - 1);
        if (strcmp(startflag,"RuntimeSend:") != 0){
            continue;
        }
        
        union
        {
            char char_type[3];
            unsigned short uint16_type;
        }protonum;
        s_fsSocket.Recv(protonum.char_type, sizeof(protonum.char_type) - 1);
        union
        {
            char char_type[3];
            unsigned short uint16_type;
        }protolength;
        s_fsSocket.Recv(protolength.char_type, sizeof(protolength.char_type) - 1);
        
        char* protoBuf = new char[protolength.uint16_type];
        memset(protoBuf, 0, protolength.uint16_type);
        s_fsSocket.Recv(protoBuf, protolength.uint16_type);
        
        runtime::FileSendComplete fsComp;
        fsComp.ParseFromArray(protoBuf, protolength.uint16_type);
        
        const char* ksucess = "sucess";
        if (fsComp.result() != 0) ksucess = "failed";
        printf("send file %s result %s \n",
               fsComp.file_name().c_str(),
               ksucess);
        delete [] protoBuf;
        
        for (std::vector<fileinfo>::iterator i = s_addFiles.begin(); i != s_addFiles.end(); i++)
        {
            if (i->first.compare(fsComp.file_name()) == 0)
            {
                s_addFiles.erase(i);
                break;
            }
        }
        
        if (s_addFiles.size() <= 0)
        {
            pthread_cond_signal(&s_fsCondition);
        }
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

int sendFile(const char* file, long time)
{
    XOutputStream* stream = new XOutputStream(256);
    stream->writeUTF("RuntimeSend:");
    //proto count
    stream->writeShort(1);
    
    runtime::FileSendProtos proto;
    proto.set_file_name(file);
    proto.set_package_seq(1);
    proto.set_package_sum(1);
    proto.set_compress_type(runtime::FileSendProtos_CompressType_NO_COMPRESS);
    
    std::string fullfile = projectFolder;
    fullfile.append("/");
    fullfile.append(file);
    FILE *fp = fopen(fullfile.c_str(), "r");
    unsigned long fsize;
    fseek(fp,0,SEEK_END);
    fsize = ftell(fp);
    fseek(fp,0,SEEK_SET);
    unsigned char* pBuffer = new unsigned char[fsize];
    fsize = fread(pBuffer,sizeof(unsigned char), fsize,fp);
    fclose(fp);
    proto.set_content_size(fsize);
    
    unsigned long mdt = time * 1000;
    proto.set_modified_time(mdt);
    
    //proto length
    stream->writeShort(proto.ByteSize());
    stream->writeProtobuf(proto);
    
    //write file stream
    stream->writeData(pBuffer, fsize);
//    stream->description();
    
    int ret = s_fsSocket.Send(stream->getStreamData(), stream->getDataLength());
    
    delete stream;
    return ret;
}
