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

#include "query.h"
#include "response.h"
#include  "resolver.h"
#include <string>
#include <vector>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdint.h>
#include <sys/time.h> 
#include <stdio.h> 
#include <math.h>
#define MAX_DOMAINNAME_LEN 255
#define DNS_PORT   53
#define DNS_TYPE_SIZE  2
#define DNS_CLASS_SIZE  2
#define DNS_TTL_SIZE  4
#define DNS_DATALEN_SIZE 2
#define DNS_TYPE_A   0x0001 //1 a host address
#define DNS_TYPE_CNAME  0x0005 //5 the canonical name for an alias
#define DNS_PACKET_MAX_SIZE (sizeof(DNSHeader) + MAX_DOMAINNAME_LEN + DNS_TYPE_SIZE + DNS_CLASS_SIZE)
#define  LONG  long 
#define  ULONG unsigned long 
#define  USHORT unsigned short
#define  LONGLONG  long long
#define  BOOL bool
#define SOCKET int
#define TRUE 1
#define FALSE 0
//#define  byte  BYTE

typedef unsigned char       BYTE;

namespace dns {

class CDNSLookup {
public:

    CDNSLookup();
    ~CDNSLookup();
    CDNSLookup(Resolver resolver);
    BOOL DNSLookup(ULONG ulDNSServerIP, char *szDomainName, std::vector<ULONG> *pveculIPList = NULL, std::vector<std::string> *pvecstrCNameList = NULL, ULONG ulTimeout = 1000, ULONG *pulTimeSpent = NULL);
    BOOL DNSLookup(ULONG ulDNSServerIP, char *szDomainName, std::vector<std::string> *pvecstrIPList = NULL, std::vector<std::string> *pvecstrCNameList = NULL, ULONG ulTimeout = 1000, ULONG *pulTimeSpent = NULL);

private:
    BOOL Init();
    BOOL UnInit();
    BOOL DNSLookupCore(ULONG ulDNSServerIP, char *szDomainName, std::vector<ULONG> *pveculIPList, std::vector<std::string> *pvecstrCNameList, ULONG ulTimeout, ULONG *pulTimeSpent);
    BOOL SendDNSRequest(sockaddr_in sockAddrDNSServer, char *szDomainName);
    BOOL RecvDNSResponse(sockaddr_in sockAddrDNSServer, ULONG ulTimeout, std::vector<ULONG> *pveculIPList, std::vector<std::string> *pvecstrCNameList, ULONG *pulTimeSpent);
    BOOL EncodeDotStr(char *szDotStr, char *szEncodedStr, USHORT nEncodedStrSize);
    BOOL DecodeDotStr(char *szEncodedStr, USHORT *pusEncodedStrLen, char *szDotStr, USHORT nDotStrSize, char *szPacketStartPos = NULL);
    ULONG GetTickCountCalibrate();

private:
    BOOL m_bIsInitOK;
    SOCKET m_sock;
    USHORT m_usCurrentProcID;
    char *m_szDNSPacket;
    struct sockaddr_in m_address;

    Query m_query;
    Response m_response;

    Resolver& m_resolver;
};
}
