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

/*
 *                                     1  1  1  1  1  1
 *       0  1  2  3  4  5  6  7  8  9  0  1  2  3  4  5
 *     +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
 *     |                      ID                       |
 *     +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
 *     |QR|   Opcode  |AA|TC|RD|RA|   Z    |   RCODE   |
 *     +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
 *     |                    QDCOUNT                    |
 *     +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
 *     |                    ANCOUNT                    |
 *     +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
 *     |                    NSCOUNT                    |
 *     +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
 *     |                    ARCOUNT                    |
 *    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
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

/*
 * Currently defined type values for resources and queries.
 */
typedef enum __ns_type {
	ns_t_invalid = 0,	/* Cookie. */
	ns_t_a = 1,		/* Host address. */
	ns_t_ns = 2,		/* Authoritative server. */
	ns_t_md = 3,		/* Mail destination. */
	ns_t_mf = 4,		/* Mail forwarder. */
	ns_t_cname = 5,		/* Canonical name. */
	ns_t_soa = 6,		/* Start of authority zone. */
	ns_t_mb = 7,		/* Mailbox domain name. */
	ns_t_mg = 8,		/* Mail group member. */
	ns_t_mr = 9,		/* Mail rename name. */
	ns_t_null = 10,		/* Null resource record. */
	ns_t_wks = 11,		/* Well known service. */
	ns_t_ptr = 12,		/* Domain name pointer. */
	ns_t_hinfo = 13,	/* Host information. */
	ns_t_minfo = 14,	/* Mailbox information. */
	ns_t_mx = 15,		/* Mail routing information. */
	ns_t_txt = 16,		/* Text strings. */
	ns_t_rp = 17,		/* Responsible person. */
	ns_t_afsdb = 18,	/* AFS cell database. */
	ns_t_x25 = 19,		/* X_25 calling address. */
	ns_t_isdn = 20,		/* ISDN calling address. */
	ns_t_rt = 21,		/* Router. */
	ns_t_nsap = 22,		/* NSAP address. */
	ns_t_nsap_ptr = 23,	/* Reverse NSAP lookup (deprecated). */
	ns_t_sig = 24,		/* Security signature. */
	ns_t_key = 25,		/* Security key. */
	ns_t_px = 26,		/* X.400 mail mapping. */
	ns_t_gpos = 27,		/* Geographical position (withdrawn). */
	ns_t_aaaa = 28,		/* Ip6 Address. */
	ns_t_loc = 29,		/* Location Information. */
	ns_t_nxt = 30,		/* Next domain (security). */
	ns_t_eid = 31,		/* Endpoint identifier. */
	ns_t_nimloc = 32,	/* Nimrod Locator. */
	ns_t_srv = 33,		/* Server Selection. */
	ns_t_atma = 34,		/* ATM Address */
	ns_t_naptr = 35,	/* Naming Authority PoinTeR */
	ns_t_kx = 36,		/* Key Exchange */
	ns_t_cert = 37,		/* Certification record */
	ns_t_a6 = 38,		/* IPv6 address (deprecates AAAA) */
	ns_t_dname = 39,	/* Non-terminal DNAME (for IPv6) */
	ns_t_sink = 40,		/* Kitchen sink (experimentatl) */
	ns_t_opt = 41,		/* EDNS0 option (meta-RR) */
	ns_t_tsig = 250,	/* Transaction signature. */
	ns_t_ixfr = 251,	/* Incremental zone transfer. */
	ns_t_axfr = 252,	/* Transfer zone of authority. */
	ns_t_mailb = 253,	/* Transfer mailbox records. */
	ns_t_maila = 254,	/* Transfer mail agent records. */
	ns_t_any = 255,		/* Wildcard match. */
	ns_t_zxfr = 256,	/* BIND-specific, nonstandard. */
	ns_t_max = 65536
} ns_type;


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

