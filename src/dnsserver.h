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



#ifndef _DNS_SERVER_H
#define	_DNS_SERVER_H

#include <netinet/in.h>

#include "logger.h"

namespace dns {

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


class Resolver;

/**
 *  Server class is a socket server that receive queries and answer responses to
 *  those queries. It has a @ref Query and a @ref Response class attributtes to
 *  code and decode the messages received on the socket buffer.
 */
class DnsServer {
public:
    /**
     *  Constructor.
     *  Creates a socket Server.
     *  @param resolver The object @ref Resolver from the application.
     */
    DnsServer() { }

    /**
     *  Destructor
     */
    virtual ~DnsServer() { }

    /**
     *  Initializes the server creating a UDP datagram socket and binding it to
     *  the INADDR_ANY address and the port passed.
     *  @param port Port number where the socket is binded.
     */
    void init(int port);

    /**
     *  The socket server runs in an infinite loop, waiting for queries and
     *  handling them through the @ref Resolver and sending back the responses.
     */
    void run();

private:
    void decode_header(const char *buffer);
    void decode_domain_name(const char *buffer);
    void encode_header(char *buffer);
    void encode_domain(char *&buffer, const std::string &domain);
    int encode(char *buffer);
    int get2byte(const char *&buffer);
    void put2byte(char *&buffer, uint value);
    void dump_buffer(const char *buffer, int size);
    
private:
    static const int BUFFER_SIZE = 1024;

    struct sockaddr_in m_address;
    int m_sockfd;

    struct DNSHeader header;

    std::string domainName;

    Logger *logger;
    //Query m_query;
    //Response m_response;

    //Resolver& m_resolver;
};
}

#endif	/* _DNS_SERVER_H */

