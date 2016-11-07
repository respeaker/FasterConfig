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
#include <cstring>
#include <sys/socket.h>
#include <errno.h>
#include <sstream>
#include <iomanip>
#include "dnsserver.h"

using namespace std;
using namespace dns;

void DnsServer::init(int dPort) {


    m_sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    m_address.sin_family = AF_INET;
    m_address.sin_addr.s_addr = INADDR_ANY;
    m_address.sin_port = htons(dPort);

    logger = &Logger::instance();

    int rbind = bind(m_sockfd, (struct sockaddr *) & m_address,
                     sizeof (struct sockaddr_in));
    
    if (rbind != 0) {
        debug(LOG_INFO,"Could not bind!!");
        return;
    }

    cout << "Listening in port: " << dPort << ", sockfd: " << m_sockfd << endl;
}

void DnsServer::run(){

    
    cout << "DNS Server running..." << endl;

    char buffer[BUFFER_SIZE];
    struct sockaddr_in clientAddress;
    socklen_t addrLen = sizeof (struct sockaddr_in);

    while (true) {

        int nbytes = recvfrom(m_sockfd, buffer, BUFFER_SIZE, 0,
                     (struct sockaddr *) &clientAddress, &addrLen);
      
#ifdef  FASTERCONFIG_DEBUG
        dump_buffer(buffer, nbytes);
#endif
        decode_header(buffer);
        decode_domain_name(buffer);
        
        memset(buffer, 0, BUFFER_SIZE);

        encode_header(buffer);
        nbytes = encode(buffer);
#ifdef FASTERCONFIG_DEBUG
        dump_buffer(buffer, nbytes);
#endif

        sendto(m_sockfd, buffer, nbytes, 0, (struct sockaddr *) &clientAddress,
               addrLen);
    }
}

/**
 * 
 * 
 * @author Baozhu (9/13/2016)
 * 
 * @param buffer 
 */
void DnsServer::decode_header(const char* buffer){    
        MSG("decode_hdr----------");
        header.usTransID = get2byte(buffer); 

        header.usFlags = get2byte(buffer); 
    
        header.usQDCOUNT = get2byte(buffer); 
        header.usANCOUNT = get2byte(buffer); 
        header.usNSCOUNT = get2byte(buffer); 
        header.usARCOUNT = get2byte(buffer); 

        debug(LOG_INFO,"usTransID:%d",header.usTransID);
        debug(LOG_INFO,"usFlags:%d",header.usFlags);
        debug(LOG_INFO,"usQDCOUNT:%d",header.usQDCOUNT);
        debug(LOG_INFO,"usANCOUNT:%d",header.usANCOUNT);
        debug(LOG_INFO,"usNSCOUNT:%d",header.usNSCOUNT);
        debug(LOG_INFO,"usARCOUNT:%d",header.usARCOUNT);
   
        MSG("decode_hdr");
}

/**
 * 
 * 
 * @author Baozhu (9/13/2016)
 * 
 * @param buffer 
 */
void DnsServer::decode_domain_name(const char* buffer) { 
    //skipe header
    buffer += sizeof(struct DNSHeader); 

    const char *decodeStr = buffer; 
    int length = 0;
    domainName.clear(); 

    while ((length = *decodeStr) != 0) {
        if ((length & 0xc0) == 0) { //normal format
            domainName.append(decodeStr + 1, length); 
            domainName.append(1,'.');
            decodeStr = decodeStr + length + 1;
        }
        else{ //compressed format,11000000 00000000, 
              //                  two bytes, 
              //                  front 2bit means jumps flag, 
              //                  last 14bit means offset bits.    
        }
    }
    debug(LOG_INFO, "domain name: %s", domainName.c_str());
}

/**
 * 
 * 
 * @author Baozhu (9/13/2016)
 * 
 * @param buffer 
 */
void DnsServer::encode_header(char *buffer) {

    put2byte(buffer, header.usTransID); 

    //flag is special
    put2byte(buffer, 0x8580);
    header.usANCOUNT  = 1;
    header.usARCOUNT  = 0;
    put2byte(buffer, header.usQDCOUNT); 
    put2byte(buffer, header.usANCOUNT); 
    put2byte(buffer, header.usNSCOUNT); 
    put2byte(buffer, header.usARCOUNT); 
}

/**
 * 
 * 
 * @author Baozhu (9/13/2016)
 * 
 * @param buffer 
 * @param domain 
 */
void DnsServer::encode_domain(char *&buffer, const std::string &domain) {

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

/** 
 * encode packeage e.g: 
 *  --------------------------------- 
 *  00: 8a 0f 85 80 00 01 00 01 00 00
 *  10: 00 00 03 77 77 77 05 62 61 69
 *  20: 64 75 03 63 6f 6d 00 00 01 00
 *  30: 01 c0 0c 00 01 00 01 00 00 00
 *  40: 00 00 04 ac 01 08 01
 *  ---------------------------------
 * 
 * @author Baozhu (9/13/2016)
 * 
 * @param buffer 
 * 
 * @return int 
 */
int DnsServer::encode(char *buffer) {

    char* bufferBegin = buffer;

    encode_header(buffer); 
    buffer += 12;
#if 0
    if (! m_name.compare(m_name.length() - 4 ,4,".lan")) {
        m_name = m_name.erase(m_name.length() - 4,4);
    }
#endif
    encode_domain(buffer, domainName); 
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
    //*buffer = 0x00;
    //buffer += 1;
    //put16bits(buffer, 0x0000);

    put2byte(buffer, 0x0100);
    put2byte(buffer, 0x01c0);
    put2byte(buffer, 0x0c00);
    put2byte(buffer, 0x0100);
    put2byte(buffer, 0x0100);
    put2byte(buffer, 0x0000);
    put2byte(buffer, 0x0000);



    put2byte(buffer, 0x04ac); //172.31.255.240
    put2byte(buffer, 0x1fff);
    *buffer = 0xf0;
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
    return size;
}
/**
 * 
 * 
 * @author Baozhu (9/13/2016)
 * 
 * @param buffer 
 * 
 * @return int 
 */
int DnsServer::get2byte(const char*& buffer) {

    int value = static_cast<unsigned char>(buffer[0]); 
    value = value << 8;
    value += static_cast<unsigned char>(buffer[1]); 
    buffer += 2;

    return value;
}
/**
 * 
 * 
 * @author Baozhu (9/13/2016)
 * 
 * @param buffer 
 * @param value 
 */
void DnsServer::put2byte(char*& buffer, uint value) {

    buffer[0] = (value & 0xFF00) >> 8;
    buffer[1] = value & 0xFF;
    buffer += 2;
}

/**
 * 
 * 
 * @author Baozhu (9/13/2016)
 * 
 * @param buffer 
 * @param size 
 */
void DnsServer::dump_buffer(const char* buffer, int size) {

    ostringstream text;

    text << "size: " << size << " bytes" << endl;
    text << "---------------------------------" << setfill('0');

    for (int i = 0; i < size; i++) {
        if ((i % 10) == 0) {
            text << endl << setw(2) << i << ": ";
        }
        unsigned char c = buffer[i]; 
        text << hex << setw(2) << int(c) << " " << dec;
    }
    text << endl << setfill(' ');
    text << "---------------------------------";
    cout << text.str() << endl;
}
