//
//  XStream.h
//  AoB
//
//  Created by VincentChou on 10/12/13.
//
//

#ifndef AoB_XStream_h
#define AoB_XStream_h

//#include "cocoa/CCObject.h"
//#include "ABTypes.h"
#include <google/protobuf/message_lite.h>

using namespace google::protobuf;

class XStream
{
public:
    //common
    virtual const char* getStreamData() { return NULL; }
    
    //write
    virtual void writeByte(const char b) {};
    virtual void writeShort(const short s) {};
    virtual void writeInt(const int i) {};
    virtual void writeLong(const long l) {};
    virtual void writeUTF(const char* utf) {};
    virtual void writeData(const char* dt, int pos, int len) {}
    virtual void writeProtobuf(const MessageLite& msg) {}
    
    virtual int getDataLength() { return 0; }
    
    virtual bool compressData() { return false; }
    
//    void writeOver();
//    void description();
//    void close();
    
    //read
    virtual char readByte() { return 0; };
    virtual short readShort() { return 0; };
    virtual int readInt() { return 0; };
	virtual long readLong() { return 0; };
    virtual char* readUTF() { return 0; };
    virtual void fastForward(int len) {}
    
};

#endif
