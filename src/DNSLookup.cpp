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
#include "DNSLookup.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <iostream>
#include <iomanip>
using namespace std;
CDNSLookup::CDNSLookup() :
    m_bIsInitOK(FALSE),
    m_sock(0),
    m_szDNSPacket(NULL) {
    m_bIsInitOK = Init();
}

CDNSLookup::~CDNSLookup() {
    UnInit();
}

BOOL CDNSLookup::DNSLookup(ULONG ulDNSServerIP, char *szDomainName, std::vector<ULONG> *pveculIPList, std::vector<std::string> *pvecstrCNameList, ULONG ulTimeout, ULONG *pulTimeSpent) {
    return DNSLookupCore(ulDNSServerIP, szDomainName, pveculIPList, pvecstrCNameList, ulTimeout, pulTimeSpent);
}

BOOL CDNSLookup::DNSLookup(ULONG ulDNSServerIP, char *szDomainName, std::vector<std::string> *pvecstrIPList, std::vector<std::string> *pvecstrCNameList, ULONG ulTimeout, ULONG *pulTimeSpent) {
    std::vector<ULONG> * pveculIPList = NULL;
    if (pvecstrIPList != NULL) {
        std::vector<ULONG> veculIPList;
        pveculIPList = &veculIPList;
    }

    BOOL bRet = DNSLookupCore(ulDNSServerIP, szDomainName, pveculIPList, pvecstrCNameList, ulTimeout, pulTimeSpent);

    if (bRet) {
        pvecstrIPList->clear();
        char szIP[16] = { '\0' };
        for (std::vector<ULONG>::iterator iter = pveculIPList->begin(); iter != pveculIPList->end(); ++iter) {
            BYTE *pbyIPSegment = (BYTE *)(&(*iter));
            //sprintf_s(szIP, 16, "%d.%d.%d.%d", pbyIPSegment[0], pbyIPSegment[1], pbyIPSegment[2], pbyIPSegment[3]);
            sprintf(szIP, "%d.%d.%d.%d", pbyIPSegment[0], pbyIPSegment[1], pbyIPSegment[2], pbyIPSegment[3]);
            pvecstrIPList->push_back(szIP);
        }
    }

    return bRet;
}


BOOL CDNSLookup::Init() {

    if ((m_sock = socket(AF_INET, SOCK_DGRAM, 0)) == 0) {
        return FALSE;
    }


    m_szDNSPacket = new(std::nothrow) char[DNS_PACKET_MAX_SIZE];
    if (m_szDNSPacket == NULL) {
        return FALSE;
    }

    m_usCurrentProcID = (USHORT)getpid();

    return TRUE;
}

BOOL CDNSLookup::UnInit() {

    if (m_szDNSPacket != NULL) {
        delete[] m_szDNSPacket;
    }

    return TRUE;
}

BOOL CDNSLookup::DNSLookupCore(ULONG ulDNSServerIP, char *szDomainName, std::vector<ULONG> *pveculIPList, std::vector<std::string> *pvecstrCNameList, ULONG ulTimeout, ULONG *pulTimeSpent) {
    if (m_bIsInitOK == FALSE || szDomainName == NULL) {
        return FALSE;
    }

    //配置SOCKET
    sockaddr_in sockAddrDNSServer;
    sockAddrDNSServer.sin_family = AF_INET;
    sockAddrDNSServer.sin_addr.s_addr = ulDNSServerIP;
    sockAddrDNSServer.sin_port = htons(DNS_PORT);

    //DNS查询与解析
    if (!SendDNSRequest(sockAddrDNSServer, szDomainName)
        || !RecvDNSResponse(sockAddrDNSServer, ulTimeout, pveculIPList, pvecstrCNameList, pulTimeSpent)) {
        return FALSE;
    }

    return TRUE;
}

BOOL CDNSLookup::SendDNSRequest(sockaddr_in sockAddrDNSServer, char *szDomainName) {
    char *pWriteDNSPacket = m_szDNSPacket;
    memset(pWriteDNSPacket, 0, DNS_PACKET_MAX_SIZE);

    //填充DNS查询报文头部
    DNSHeader *pDNSHeader = (DNSHeader *)pWriteDNSPacket;
    pDNSHeader->usTransID = m_usCurrentProcID;
    pDNSHeader->usFlags = htons(0x0100);
    pDNSHeader->usQuestionCount = htons(0x0001);
    pDNSHeader->usAnswerCount = 0x0000;
    pDNSHeader->usAuthorityCount = 0x0000;
    pDNSHeader->usAdditionalCount = 0x0000;

    //设置DNS查询报文内容
    USHORT usQType = htons(0x0001);
    USHORT usQClass = htons(0x0001);
    USHORT nDomainNameLen = strlen(szDomainName);
    char *szEncodedDomainName = (char *)malloc(nDomainNameLen + 2);
    if (szEncodedDomainName == NULL) {
        return FALSE;
    }
    if (!EncodeDotStr(szDomainName, szEncodedDomainName, nDomainNameLen + 2)) {
        return FALSE;
    }

    //填充DNS查询报文内容
    USHORT nEncodedDomainNameLen = strlen(szEncodedDomainName) + 1;
    memcpy(pWriteDNSPacket += sizeof(DNSHeader), szEncodedDomainName, nEncodedDomainNameLen);
    memcpy(pWriteDNSPacket += nEncodedDomainNameLen, (char *)(&usQType), DNS_TYPE_SIZE);
    memcpy(pWriteDNSPacket += DNS_TYPE_SIZE, (char *)(&usQClass), DNS_CLASS_SIZE);
    free(szEncodedDomainName);

    //发送DNS查询报文
    USHORT nDNSPacketSize = sizeof(DNSHeader) + nEncodedDomainNameLen + DNS_TYPE_SIZE + DNS_CLASS_SIZE;
    sendto(m_sock, m_szDNSPacket, nDNSPacketSize, 0, (sockaddr *)&sockAddrDNSServer, sizeof(sockAddrDNSServer));


    return TRUE;
}

BOOL CDNSLookup::RecvDNSResponse(sockaddr_in sockAddrDNSServer, ULONG ulTimeout, std::vector<ULONG> *pveculIPList, std::vector<std::string> *pvecstrCNameList, ULONG *pulTimeSpent) {
    struct timeval tpstart,tpend;
    float timeuse;
    gettimeofday(&tpstart, NULL);
    if (pveculIPList != NULL) {
        pveculIPList->clear();
    }
    if (pvecstrCNameList != NULL) {
        pvecstrCNameList->clear();
    }

    char recvbuf[1024] = { '\0' };
    char szDotName[128] = { '\0' };
    USHORT nEncodedNameLen = 0;

    while (TRUE) {
        socklen_t nSockaddrDestSize = sizeof(sockAddrDNSServer);
        //int nbytes = recvfrom(m_sockfd, buffer, BUFFER_SIZE, 0,
        //             (struct sockaddr *) &clientAddress, &addrLen);
        //接收响应报文
        unsigned int recv_size = recvfrom(m_sock, recvbuf, 1024, 0, 
                     (struct sockaddr *)&sockAddrDNSServer,
                      &nSockaddrDestSize);

            cout << "size: " << recv_size << " bytes" << endl;
            cout << "---------------------------------" << setfill('0');

            for (int i = 0; i < recv_size; i++) {
                if ((i % 10) == 0) {
                    cout << endl << setw(2) << i << ": ";
                }
                unsigned char c = recvbuf[i];
                cout << hex << setw(2) << int(c) << " " << dec;
            }
            cout << endl << setfill(' ');
            cout << "---------------------------------";
            cout << endl;
            gettimeofday(&tpend, NULL);
            DNSHeader *pDNSHeader = (DNSHeader *)recvbuf;
            USHORT usQuestionCount = 0;
            USHORT usAnswerCount = 0;

            if (pDNSHeader->usTransID == m_usCurrentProcID
                && (ntohs(pDNSHeader->usFlags) & 0xfb7f) == 0x8100 //RFC1035 4.1.1(Header section format)
                && (usQuestionCount = ntohs(pDNSHeader->usQuestionCount)) >= 0
                && (usAnswerCount = ntohs(pDNSHeader->usAnswerCount)) > 0) {
                char *pDNSData = recvbuf + sizeof(DNSHeader);

                //解析Question字段
                for (int q = 0; q != usQuestionCount; ++q) {
                    if (!DecodeDotStr(pDNSData, &nEncodedNameLen, szDotName, sizeof(szDotName))) {
                        return FALSE;
                    }
                    pDNSData += (nEncodedNameLen + DNS_TYPE_SIZE + DNS_CLASS_SIZE);
                }
                cout <<"usAnswerCount:"<< usAnswerCount << endl;
                //解析Answer字段
                for (int a = 0; a != usAnswerCount; ++a) {
                    if (!DecodeDotStr(pDNSData, &nEncodedNameLen, szDotName, sizeof(szDotName), recvbuf)) {
                        return FALSE;
                    }
                    pDNSData += nEncodedNameLen;

                    USHORT usAnswerType = ntohs(*(USHORT *)(pDNSData));
                    USHORT usAnswerClass = ntohs(*(USHORT *)(pDNSData + DNS_TYPE_SIZE));
                    ULONG usAnswerTTL = ntohl(*(ULONG *)(pDNSData + DNS_TYPE_SIZE + DNS_CLASS_SIZE));
                    USHORT usAnswerDataLen = ntohs(*(USHORT *)(pDNSData + DNS_TYPE_SIZE + DNS_CLASS_SIZE + DNS_TTL_SIZE));
                    pDNSData += (DNS_TYPE_SIZE + DNS_CLASS_SIZE + DNS_TTL_SIZE + DNS_DATALEN_SIZE);

                    if (usAnswerType == DNS_TYPE_A && pveculIPList != NULL) {
                        ULONG ulIP = *(ULONG *)(pDNSData);
                        pveculIPList->push_back(ulIP);
                    } else if (usAnswerType == DNS_TYPE_CNAME && pvecstrCNameList != NULL) {
                        if (!DecodeDotStr(pDNSData, &nEncodedNameLen, szDotName, sizeof(szDotName), recvbuf)) {
                            return FALSE;
                        }
                        pvecstrCNameList->push_back(szDotName);
                    }

                    pDNSData += (usAnswerDataLen);
                }
                timeuse=1000000*(tpend.tv_sec-tpstart.tv_sec)+ 
                tpend.tv_usec-tpstart.tv_usec; 
                timeuse/=1000000; 

                //计算DNS查询所用时间
                if (pulTimeSpent != NULL) {
                    *pulTimeSpent = (ULONG)timeuse; 
                }

                break;
            }
        }
#if 0
        //超时退出
        if (GetTickCountCalibrate() - ulSendTimestamp > ulTimeout) {
            *pulTimeSpent = ulTimeout + 1;
            return FALSE;
        }
#endif
    return TRUE;
}

/*
 * convert "www.baidu.com" to "\x03www\x05baidu\x03com"
 * 0x0000 03 77 77 77 05 62 61 69 64 75 03 63 6f 6d 00 ff
 */
BOOL CDNSLookup::EncodeDotStr(char *szDotStr, char *szEncodedStr, USHORT nEncodedStrSize) {
    USHORT nDotStrLen = strlen(szDotStr);

    if (szDotStr == NULL || szEncodedStr == NULL || nEncodedStrSize < nDotStrLen + 2) {
        return FALSE;
    }

    char *szDotStrCopy = new char[nDotStrLen + 1];
    //strcpy_s(szDotStrCopy, nDotStrLen + 1, szDotStr);
    strcpy(szDotStrCopy, szDotStr);

    char *pNextToken = NULL;
    //char *pLabel = strtok_s(szDotStrCopy, ".", &pNextToken);
    char *pLabel = strtok(szDotStrCopy, ".");
    USHORT nLabelLen = 0;
    USHORT nEncodedStrLen = 0;
    while (pLabel != NULL) {
        if ((nLabelLen = strlen(pLabel)) != 0) {
            //sprintf_s(szEncodedStr + nEncodedStrLen, nEncodedStrSize - nEncodedStrLen, "%c%s", nLabelLen, pLabel);
            sprintf(szEncodedStr + nEncodedStrLen, "%c%s", nLabelLen, pLabel);
            nEncodedStrLen += (nLabelLen + 1);
        }
        //pLabel = strtok_s(NULL, ".", &pNextToken);
        pLabel = strtok(NULL, ".");
    }

    delete[] szDotStrCopy;

    return TRUE;
}

/*
 * convert "\x03www\x05baidu\x03com\x00" to "www.baidu.com"
 * 0x0000 03 77 77 77 05 62 61 69 64 75 03 63 6f 6d 00 ff
 * convert "\x03www\x05baidu\xc0\x13" to "www.baidu.com"
 * 0x0000 03 77 77 77 05 62 61 69 64 75 c0 13 ff ff ff ff
 * 0x0010 ff ff ff 03 63 6f 6d 00 ff ff ff ff ff ff ff ff
 */
BOOL CDNSLookup::DecodeDotStr(char *szEncodedStr, USHORT *pusEncodedStrLen, char *szDotStr, USHORT nDotStrSize, char *szPacketStartPos) {
    if (szEncodedStr == NULL || pusEncodedStrLen == NULL || szDotStr == NULL) {
        return FALSE;
    }

    char *pDecodePos = szEncodedStr;
    USHORT usPlainStrLen = 0;
    BYTE nLabelDataLen = 0;
    *pusEncodedStrLen = 0;

    while ((nLabelDataLen = *pDecodePos) != 0x00) {
        if ((nLabelDataLen & 0xc0) == 0) { //普通格式，LabelDataLen + Label
            if (usPlainStrLen + nLabelDataLen + 1 > nDotStrSize) {
                return FALSE;
            }
            memcpy(szDotStr + usPlainStrLen, pDecodePos + 1, nLabelDataLen);
            memcpy(szDotStr + usPlainStrLen + nLabelDataLen, ".", 1);
            pDecodePos += (nLabelDataLen + 1);
            usPlainStrLen += (nLabelDataLen + 1);
            *pusEncodedStrLen += (nLabelDataLen + 1);
        } else { //消息压缩格式，11000000 00000000，两个字节，前2位为跳转标志，后14位为跳转的偏移
            if (szPacketStartPos == NULL) {
                return FALSE;
            }
            USHORT usJumpPos = ntohs(*(USHORT *)(pDecodePos)) & 0x3fff;
            USHORT nEncodeStrLen = 0;
            cout << "has depress format: " << usJumpPos << endl;
            if (!DecodeDotStr(szPacketStartPos + usJumpPos, &nEncodeStrLen, szDotStr + usPlainStrLen, nDotStrSize - usPlainStrLen, szPacketStartPos)) {
                return FALSE;
            } else {
                *pusEncodedStrLen += 2;
                return TRUE;
            }
        }
    }

    szDotStr[usPlainStrLen - 1] = '\0';
    *pusEncodedStrLen += 1;

    return TRUE;
}

ULONG CDNSLookup::GetTickCountCalibrate() {
#if 0
    static ULONG s_ulFirstCallTick = 0;
    static LONGLONG s_ullFirstCallTickMS = 0;


    FILETIME filetime;

    LARGE_INTEGER liCurrentTime;
    liCurrentTime.u.HighPart = filetime.dwHighDateTime;
    liCurrentTime.u.LowPart = filetime.dwLowDateTime;
    LONGLONG llCurrentTimeMS = liCurrentTime.QuadPart / 10000;

    if (s_ulFirstCallTick == 0) {
        s_ulFirstCallTick = GetTickCount();
    }
    if (s_ullFirstCallTickMS == 0) {
        s_ullFirstCallTickMS = llCurrentTimeMS;
    }

    return s_ulFirstCallTick + (ULONG)(llCurrentTimeMS - s_ullFirstCallTickMS);
#endif
    return 0;
}
