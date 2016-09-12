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
   // *buffer = 0x00;
   // buffer += 1;
    //put16bits(buffer, 0x0000);

    put16bits(buffer, 0x0100);
    put16bits(buffer, 0x01c0);
    put16bits(buffer, 0x0c00);
    put16bits(buffer, 0x0100);
    put16bits(buffer, 0x0100);
    put16bits(buffer, 0x0000);
    put16bits(buffer, 0x0000);
#if 0
    put16bits(buffer, 0x04c0);
    put16bits(buffer, 0xa864);
    *buffer = 0x01;
#endif
#if 1
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

void Response::setDnsIP(char *ip){
    strcpy(dnsIP,ip); 
}
