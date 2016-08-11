/*
  2016 Copyright (c) Seeed Technology Inc.  All right reserved.
  Author:Baozhu Zuo
  suli is designed for the purpose of reusing the high level implementation
  of each libraries for different platforms out of the hardware layer.
  suli2 is the new reversion of original suli. There're lot of improvements upon
  the previous version. Currently, it can be treated as the internal strategy for
  quick library development of seeed. But always welcome the community to
  follow the basis of suli to contribute grove libraries.
  The MIT License (MIT)
  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:
  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.
  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
  THE SOFTWARE.
*/


#ifndef _DNS_MESSAGE_H
#define	_DNS_MESSAGE_H

#include <string>

namespace dns {

typedef unsigned char uchar;
typedef unsigned int uint;
typedef unsigned long ulong;

struct DNSHeaderFlags {
    unsigned char usQR : 1;
    unsigned char usOpcode : 4;
    unsigned char usAA : 1;
    unsigned char usTC : 1;
    unsigned char usRD : 1;
    unsigned char usRA : 1;
    unsigned char usZ :3;
    unsigned char usRCODE : 4;
};

struct DNSHeader {
    unsigned short usTransID; //标识符
    unsigned short usFlags; //各种标志位
    unsigned short usQDCOUNT; //Question字段个数
    unsigned short usANCOUNT; //Answer字段个数
    unsigned short usNSCOUNT; //Authority字段个数
    unsigned short usARCOUNT; //Additional字段个数
};

struct  DNSBase {
    char*           usNAME;
    unsigned short  usTYPE;     //type
    unsigned short  usCLASS;    //class
};

struct DNS {
    struct DNSHeader    usHeader;
    struct DNSBase      usBase;
    unsigned int        usTTL;
    unsigned short      usRDLENGT;
    char*               usRDATA;
};

/**
 *  Abstract class that represents the DNS Message and is able to code itself
 *  in the corresponding Message format.
 */
class Message {
public:
    /**
     *  Type of DNS message
     */
    enum Type { Query=0, Response };

    /**
     *  Pure virtual function that will code the correspoding message type
     *  @param buffer The buffer to code the message into.
     *  @return The size of the buffer coded
     */
    virtual int code(char* buffer) throw() = 0;

    /**
     *  Pure virtual function that will decode the correspoding message type
     *  @param buffer The buffer to decode the message into.
     *  @param size The size of the buffer to decode
     */
    virtual void decode(const char* buffer, int size) throw() = 0;

    uint getID() const throw() { return m_id; }
    uint getQdCount() const throw() { return m_qdCount; }
    uint getAnCount() const throw() { return m_anCount; }
    uint getNsCount() const throw() { return m_nsCount; }
    uint getArCount() const throw() { return m_arCount; }

    void setID(uint id) throw() { m_id = id; }
    void setQdCount(uint count) throw() { m_qdCount = count; }
    void setAnCount(uint count) throw() { m_anCount = count; }
    void setNsCount(uint count) throw() { m_nsCount = count; }
    void setArCount(uint count) throw() { m_arCount = count; }

protected:
    static const uint HDR_OFFSET = 12;
#if 0
    uint m_id;
    uint m_qr;
    uint m_opcode;
    uint m_aa;
    uint m_tc;
    uint m_rd;
    uint m_ra;
    uint m_rcode;
    
    uint m_qdCount;
    uint m_anCount;
    uint m_nsCount;
    uint m_arCount;
#endif 
    uint m_qr;
    struct DNS usDNS;
    /**
     *  Constructor.
     *  @param type The type of DNS Message
     */
    Message(Type type) : m_qr(type) { }

    /**
     *  Destructor
     */
    virtual ~Message() { }

    /**
     *  Returns the DNS message header as a string text.
     *  @return The string text with the header information.
     */
    virtual std::string asString() const throw();

    /**
     *  Function that decodes the DNS message header section.
     *  @param buffer The buffer to decode the message header from.
     */
    void decode_hdr(const char* buffer) throw ();

    /**
     *  Function that codes the DNS message header section.
     *  @param buffer The buffer to code the message header into.
     */
    void code_hdr(char* buffer) throw ();

    /**
     *  Helper function that get 16 bits from the buffer and keeps it an int.
     *  It helps in compatibility issues as ntohs()
     *  @param buffer The buffer to get the 16 bits from.
     *  @return An int holding the value extracted.
     */
    int get16bits(const char*& buffer) throw();

    /**
     *  Helper function that puts 16 bits into the buffer.
     *  It helps in compatibility issues as htons()
     *  @param buffer The buffer to put the 16 bits into.
     *  @param value An unsigned int holding the value to set the buffer.
     */
    void put16bits(char*& buffer, uint value) throw ();

    /**
     *  Helper function that puts 32 bits into the buffer.
     *  It helps in compatibility issues as htonl()
     *  @param buffer The buffer to put the 32 bits into.
     *  @param value An unsigned long holding the value to set the buffer.
     */
    void put32bits(char*& buffer, ulong value) throw ();

    /**
     *  Function that logs the whole buffer of a DNS Message
     *  @param buffer The buffer to be logged.
     *  @param size The size of the buffer.
     */
    void log_buffer(const char* buffer, int size) throw();
    
private:
    static const uint QR_MASK = 0x8000;
    static const uint OPCODE_MASK = 0x7800;
    static const uint AA_MASK = 0x0400;
    static const uint TC_MASK = 0x0200;
    static const uint RD_MASK = 0x0100;
    static const uint RA_MASK = 0x8000;
    static const uint RCODE_MASK = 0x000F;
};
}
#endif	/* _DNS_MESSAGE_H */

