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

//fs socket
static ODSocket             s_fsSocket;

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
    const char* filect = "HelloWorld";
    proto.set_content_size(strlen(filect));
    
    stream->writeShort(proto.ByteSize());
    stream->writeProtobuf(proto);
    
    //write file stream
    stream->writeUTF(filect);
    
    stream->description();
    
    int ret = s_fsSocket.Send(stream->getStreamData(), stream->getDataLength());
    
    delete stream;
    
    return ret;
}
