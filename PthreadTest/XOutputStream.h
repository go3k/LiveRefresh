//
//  XOutputStream.h
//  Ancient
//
//  Created by admin on 12-11-26.
//
//

#ifndef __Ancient__XOutputStream__
#define __Ancient__XOutputStream__

//#include "ABHeaders.h"
#include "XStream.h"
#include <sstream>

using namespace std;

class XOutputStream
: public XStream
{
public:
	XOutputStream(int defaultLen);
    virtual ~XOutputStream();
    
    //from XStream
    void writeByte(const char b);
    void writeShort(const short s);
    void writeInt(const int i);
    void writeLong(const long l);
    void writeUTF(const char* utf);
    /**
     *	@brief	write/insert a raw data into stream
     *
     *	@param 	data 	raw data
     *	@param 	pos 	insert pos
     *	@param 	len 	raw data length
     */
    void writeData(const char* data, int pos, int len);
    
    void writeProtobuf(const MessageLite& msg);

    virtual int getDataLength();
    virtual const char* getStreamData();
//    virtual bool compressData();
    
    //interface
    void description();
    
private:
    void checkStreamBody(int add);
    
private:
//    OUT_DATA data;
    int m_pos;
    char* m_body;
    
    int m_defaultLen;
    int m_bufferLen;
};

#endif /* defined(__Ancient__XOutputStream__) */
