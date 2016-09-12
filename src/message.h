/*
 * 2016 Copyright (c) Seeed Technology Inc.  All right reserved.
 * Author:Baozhu Zuo <zuobaozhu@gmail.com>
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
*/



#ifndef _DNS_MESSAGE_H
#define	_DNS_MESSAGE_H

#include <string.h>
#include <string>
#include <stdlib.h>
#include <iostream>

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

struct  DNSQuestionSection {
     char*                   usNAME;
    unsigned short          usTYPE;     //type
    unsigned short          usCLASS;    //class
    unsigned int            NameLength;
};
struct DNSAnswerSection{
     char                    *usNAME; 
    unsigned short          usTYPE;     //type
    unsigned short          usCLASS;    //class
    unsigned int            usTTL;
    unsigned short          usRDLENGTH;
     char*                   usRDATA;
    unsigned int            NameLength;
    unsigned int            RDdataLength;
};

struct DNS {
    struct DNSHeader               usHeader;
    struct DNSQuestionSection*      usQuestionSection;
    struct DNSAnswerSection*        usAnswerSection;
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

    uint getID() const throw() { return usDNS.usHeader.usTransID; }
    uint getQdCount() const throw() { return usDNS.usHeader.usQDCOUNT; }
    uint getAnCount() const throw() { return usDNS.usHeader.usANCOUNT; }
    uint getNsCount() const throw() { return usDNS.usHeader.usNSCOUNT; }
    uint getArCount() const throw() { return usDNS.usHeader.usARCOUNT; }

    void setID(uint id) throw() { usDNS.usHeader.usTransID = id; }
    void setQdCount(uint count) throw() { usDNS.usHeader.usQDCOUNT = count; }
    void setAnCount(uint count) throw() { usDNS.usHeader.usANCOUNT = count; }
    void setNsCount(uint count) throw() { usDNS.usHeader.usNSCOUNT = count; }
    void setArCount(uint count) throw() { usDNS.usHeader.usARCOUNT = count; }

    void setHeader(struct DNSHeader header) {usDNS.usHeader = header;}
protected:
    static const uint HDR_OFFSET = 12;
    uint m_opcode;
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
    struct DNSHeaderFlags usDNSFlags;
    /**
     *  Constructor.
     *  @param type The type of DNS Message
     */
    Message(Type type) : m_qr(type) { 

    }

    /**
     *  Destructor
     */
    virtual ~Message() {

    }

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

