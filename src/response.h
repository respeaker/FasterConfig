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


#ifndef _DNS_RESPONSE_H
#define	_DNS_RESPONSE_H

#include "message.h"

namespace dns {
/**
 *  Class that represents the DNS Response and is able to code itself in its
 *  corresponding message format.
 */
class Response : public Message {
public:
    /**
     *  Response Code
     */
    enum Code { Ok=0, FormatError, ServerFailure, NameError,
                NotImplemented, Refused };

    /**
     *  Constructor.
     */
    Response() : Message(Message::Response) { }

    /**
     *  Destructor
     */
    virtual ~Response() { }

    /**
     *  Function that codes the response message in its format.
     *  @param buffer The buffer to code the query into.
     *  @return The size of the buffer coded
     */
    int code(char* buffer) throw();

    /**
     *  Function that decodes the response message in its format.
     *  @param buffer The buffer to decode the response into.
     *  @param size The size of the buffer to decode
     */
    void decode(const char* buffer, int size) throw();

    /**
     *  Returns the response message as a string text
     *  @return The string text with the response information.
     */
    std::string asString() const throw();
#if 0
    void setRCode(Code code) throw() { m_rcode = code; }
    void setName(const std::string& value) throw() { m_name = value; }
    void setType(const uint value) throw() { m_type = value; }
    void setClass(const uint value) throw() { m_class = value; }
    void setTtl(const uint value) throw() { m_ttl = value; }
    void setRdLength(const uint value) throw() { m_rdLength = value; }
    void setRdata(const std::string& value) throw() { m_rdata = value; }
#endif
    void setDnsIP(char* ip);
private:
    std::string m_name;
    uint m_type;
    uint m_class;
    ulong m_ttl;
    uint m_rdLength;
    std::string m_rdata;

    char dnsIP[64];
    void code_domain(char*& buffer, const std::string& domain) throw();
};
}
#endif	/* _DNS_RESPONSE_H */

