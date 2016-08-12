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
