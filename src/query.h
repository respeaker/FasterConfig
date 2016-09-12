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

#ifndef _DNS_REQUEST_H
#define	_DNS_REQUEST_H

#include <string>
#include <sys/types.h>
#include <unistd.h>
#include "message.h"

namespace dns {

/**
 *  Class that represents the DNS Query and is able to code itself in its
 *  corresponding message format.
 */
class Query : public Message {
public:
    /**
     *  Constructor.
     */
    Query() : Message(Message::Query) {
        usCurrentProcID = (unsigned short)getpid();
    }

    /**
     *  Destructor
     */
    virtual ~Query() { }

    /**
     *  Function that codes the query message in its format.
     *  @param buffer The buffer to code the query into.
     *  @return The size of the buffer coded
     */
    int code(char* buffer) throw();

    /**
     *  Function that decodes the query message in its format.
     *  @param buffer The buffer to decode the query into.
     *  @param size The size of the buffer to decode
     */
    void decode(const char* buffer, int size) throw();

    /**
     *  Returns the query message as a string text.
     *  @return The string text with the query information.
     */
    std::string asString() const throw();
    /**
     * init 
     */
    void init();
    const std::string& getQName() const throw() { return m_qName; }
    struct DNSHeader  getHeader() const throw() { return usDNS.usHeader; }
#if 0
    const std::string& getQName() const throw() { return usDNS.usBase.usNAME; }
    const uint getQType() const throw() { return usDNS.usBase.usTYPE; }
    const uint getQClass() const throw() { return usDNS.usBase.usCLASS; }
#endif    

private:
    unsigned short usCurrentProcID;
    std::string m_qName;
#if 0
    uint m_qType;
    uint m_qClass;
#endif
    void decode_qname(const char*& buffer,  char*& binaryName, unsigned int* binaryNameLength) throw();
};
}
#endif	/* _DNS_REQUEST_H */

