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
    void decode_qname(const char*& buffer, char *binaryName, unsigned int* binaryNameLength) throw();
};
}
#endif	/* _DNS_REQUEST_H */

