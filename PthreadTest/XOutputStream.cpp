//
//  XOutputStream.cpp
//  Ancient
//
//  Created by admin on 12-11-26.
//
//

#include "XOutputStream.h"
//#include "Utils.h"

XOutputStream::XOutputStream(int defaultLen)
: m_body(NULL)
, m_pos(0)
, m_defaultLen(defaultLen)
, m_bufferLen(0)
{
    
}

XOutputStream::~XOutputStream()
{
    if (m_body) delete [] m_body;
}

void XOutputStream::writeByte(const char b)
{
    int plus = 1;
    checkStreamBody(plus);
    m_body[m_pos] = b & 0xFF;
    m_pos += plus;
}

void XOutputStream::writeShort(const short s)
{
    int plus = 2;
    checkStreamBody(plus);
    m_body[m_pos + 0] = s & 0xFF;
    m_body[m_pos + 1] = (s >> 8) & 0xFF;
    m_pos += plus;
}

void XOutputStream::writeInt(const int i)
{
    int plus = 4;
    checkStreamBody(plus);
    m_body[m_pos + 3] = (i >> 24) & 0xFF;
    m_body[m_pos + 2] = (i >> 16) & 0xFF;
    m_body[m_pos + 1] = (i >> 8) & 0xFF;
    m_body[m_pos + 0] = i & 0xFF;
    m_pos += plus;
}

void XOutputStream::writeLong(const long l)
{
    int plus = 8;
    checkStreamBody(plus);
	m_body[m_pos + 7] = (l >> 56) & 0xFFFF;
    m_body[m_pos + 6] = (l >> 48) & 0xFFFF;
    m_body[m_pos + 5] = (l >> 40) & 0xFFFF;
	m_body[m_pos + 4] = (l >> 32) & 0xFFFF;
    m_body[m_pos + 3] = (l >> 24) & 0xFFFF;
    m_body[m_pos + 2] = (l >> 16) & 0xFFFF;
    m_body[m_pos + 1] = (l >> 8) & 0xFFFF;
    m_body[m_pos + 0] = l & 0xFFFF;
    m_pos += plus;
}

void XOutputStream::writeUTF(const char *utf)
{
    if (!utf) return;
    
    int length = strlen(utf);
    if (length <= 0) return;
    
    int plus = length;
    checkStreamBody(plus);
    
    memcpy(m_body + m_pos, utf, length);
    m_pos += length;
}

void XOutputStream::writeData(const char* dt, int pos, int len)
{
    int plus = len;
    checkStreamBody(plus);
    
    //move data
    char* srcPos = m_body + pos;
    char* destPos = srcPos + len;
    size_t moveLen = m_pos - pos;
    memmove(destPos, srcPos, moveLen);
    
    //copy dt
    memcpy(m_body + pos, dt, len);
    
    m_pos += len;
}

void XOutputStream::writeData(unsigned char* data, long len)
{
    writeData((const char*)data, getDataLength(), (int)len);
}

void XOutputStream::writeProtobuf(const MessageLite& msg)
{
    int size = msg.ByteSize();
    char* buffer = new char[size];
    msg.SerializeToArray(buffer, size);
    writeData(buffer, getDataLength(), size);
    delete [] buffer;
}

int XOutputStream::getDataLength()
{
    return m_pos;
}

const char* XOutputStream::getStreamData()
{
    return m_body;
}

//bool XOutputStream::compressData()
//{
//    unsigned long compLength = 2 * m_pos;
//    unsigned char* compData = new unsigned char[compLength];
//    int err = gzcompress((unsigned char*)m_body, m_pos, compData, &compLength);
//    if (err != 0) return false;
//    
//    memcpy(m_body, compData, compLength);
//    m_pos = compLength;
//    
//    return true;
//}

void XOutputStream::description()
{
    printf("stream = %s\n", m_body);
    
    int size = m_pos;
    for (int i = 0; i < size; i++) {
        printf("%02X", m_body[i] & 0xFF);
    }
    printf("\n");
}

void XOutputStream::checkStreamBody(int add)
{
    if (m_bufferLen == 0)
    {
        //first mem alloc
        //alloc 1 * basic len by default
        m_body = new char[m_defaultLen];
        m_bufferLen = m_defaultLen;
    }
    
    if (m_pos + add > m_bufferLen)
    {
        //add allock 1 basic len
        int out = m_pos + add - m_bufferLen;
        int bufferPlus = (out / m_defaultLen + 1) * m_bufferLen;
        
        char* body2 = new char[m_bufferLen + bufferPlus];
        memcpy(body2, m_body, m_pos);
        delete [] m_body;
        m_body = body2;
        m_bufferLen += bufferPlus;
        
//        CCLog("Write is out of buffer %d, add buffer len = %d", out, bufferPlus);
    }
}

