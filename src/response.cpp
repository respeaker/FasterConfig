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

#include <iostream>
#include <sstream>

#include "logger.h"
#include "message.h"
#include "response.h"

using namespace std;
using namespace dns;

string Response::asString() const throw() {

    ostringstream text;
    text << endl << "RESPONSE { ";
    text << Message::asString();

    text << "\tname: " << m_name << endl;
    text << "\ttype: " << m_type << endl;
    text << "\tclass: " << m_class << endl;
    text << "\tttl: " << m_ttl << endl;
    text << "\trdLength: " << m_rdLength << endl;
    text << "\trdata: " << m_rdata << " }" << dec;

    return text.str();
}

void Response::decode(const char* buffer, int size) throw() {

    // Only needed for the DNS client
}

int Response::code(char* buffer) throw() {

    Logger& logger = Logger::instance();
    logger.trace("Response::code()");

    char* bufferBegin = buffer;

    code_hdr(buffer);
    buffer += HDR_OFFSET;
#if 0
    if (! m_name.compare(m_name.length() - 4 ,4,".lan")) {
        m_name = m_name.erase(m_name.length() - 4,4);
    }
#endif
    code_domain(buffer, m_name);
#if 0
    put16bits(buffer, 0x0377);
    put16bits(buffer, 0x7777);
    put16bits(buffer, 0x0b73);
    put16bits(buffer, 0x6565);
    put16bits(buffer, 0x6564);
    put16bits(buffer, 0x7374);
    put16bits(buffer, 0x7564);
    put16bits(buffer, 0x696f);
    put16bits(buffer, 0x0363);
    put16bits(buffer, 0x6f6d);
#endif
    *buffer = 0x00;
    buffer += 1;
    //put16bits(buffer, 0x0000);

    put16bits(buffer, 0x0100);
    put16bits(buffer, 0x01c0);
    put16bits(buffer, 0x0c00);
    put16bits(buffer, 0x0100);
    put16bits(buffer, 0x0100);
    put16bits(buffer, 0x0000);
    put16bits(buffer, 0x0000);

    put16bits(buffer, 0x04c0);
    put16bits(buffer, 0xa864);
    *buffer = 0x01;
#if 0
    put16bits(buffer, 0x04ac); //172.31.255.240
    put16bits(buffer, 0x1fff);
    *buffer = 0xf0;
#endif
    buffer += 1;
#if 0
    // Code Question section
    code_domain(buffer, m_name);
    put16bits(buffer, m_type);
    put16bits(buffer, m_class);

    // Code Answer section
    code_domain(buffer, m_name);
    put16bits(buffer, m_type);
    put16bits(buffer, m_class);
    put32bits(buffer, m_ttl);
    put16bits(buffer, m_rdLength);
    code_domain(buffer, m_rdata);
#endif    
    int size = buffer - bufferBegin;
    log_buffer(bufferBegin, size);

    return size;
}

void Response::code_domain(char*& buffer, const std::string& domain) throw() {

    int start(0), end; // indexes

    while ((end = domain.find('.', start)) != string::npos) {

        *buffer++ = end - start; // label length octet
        for (int i=start; i<end; i++) {

            *buffer++ = domain[i]; // label octets
        }
        start = end + 1; // Skip '.'
    }

    *buffer++ = domain.size() - start; // last label length octet
    for (int i=start; i<domain.size(); i++) {

        *buffer++ = domain[i]; // last label octets
    }

    *buffer++ = 0;
}
