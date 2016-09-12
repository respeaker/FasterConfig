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
#include <iomanip>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include "logger.h"
#include "query.h"

using namespace std;
using namespace dns;

string Query::asString() const throw() {

    ostringstream text;
    text << endl << "QUERY { ";
    text << Message::asString();
    for (int i = 0; i < usDNS.usHeader.usQDCOUNT; i++) {
        text << "   \tQuestionSection: " << i << "{" << endl;
        text << "             \tQname: " << m_qName << endl; 
        text << "             \tNameLength: " << usDNS.usQuestionSection[i].NameLength << endl;
        text << "             \tQnameBinary: ";
        for (int j = 0; j < usDNS.usQuestionSection[i].NameLength; j++) {
            uchar c = usDNS.usQuestionSection[i].usNAME[j]; 
            text << hex << setw(2) << int(c) << " " << dec;
        }
        text << endl << setfill(' ');
        text << "             \tQtype: " << usDNS.usQuestionSection[i].usTYPE << endl; 
        text << "            \tQclass: " << usDNS.usQuestionSection[i].usCLASS; 
        text << "   }" << endl;
    }
    text << " }" << dec;
    return text.str();
}

int Query::code(char* buffer) throw() {

    // Only needed for the DNS client
    return 0;
}

void Query::decode(const char* buffer, int size) throw() {

    Logger& logger = Logger::instance();
    logger.trace("Query::decode()");
    log_buffer(buffer, size);
    usCurrentProcID  = (unsigned short)getpid();
    decode_hdr(buffer);

    buffer += HDR_OFFSET;
#if 0
    if (ntohs(usDNS.usHeader.usTransID) == usCurrentProcID
                        //RFC1035 4.1.1(Header section format)
         && (((ntohs(usDNS.usHeader.usFlags) & 0xfb7f) == 0x8100 ) ||
                ((ntohs(usDNS.usHeader.usFlags) & 0xfb7f) == 0x8001 ) ||
                              ((ntohs(usDNS.usHeader.usFlags) & 0xfb7f) == 0x0001 ))
        && ntohs(usDNS.usHeader.usQDCOUNT) >= 0
        && ntohs(usDNS.usHeader.usANCOUNT) >= 0) 
#endif
        if (ntohs(usDNS.usHeader.usQDCOUNT) >= 0 ) 
        {
         cout << "malloc qusestion section1" << endl;
        //analyze qusestion section
        if (usDNS.usHeader.usQDCOUNT != 0) {
            cout << "malloc qusestion section" << endl;
            usDNS.usQuestionSection =  (struct DNSQuestionSection*)malloc(usDNS.usHeader.usQDCOUNT 
                                              * sizeof(struct DNSQuestionSection)); 
        }
        cout << "Question NUM:" <<usDNS.usHeader.usQDCOUNT << endl;
        for (int i = 0; i < usDNS.usHeader.usQDCOUNT; i++) {
            decode_qname(buffer,  usDNS.usQuestionSection[i].usNAME, 
                         &usDNS.usQuestionSection[i].NameLength); 

            for (int j = 0; j < usDNS.usQuestionSection[i].NameLength ;j++) {
                uchar c = *(usDNS.usQuestionSection[i].usNAME + j);
                 cout << hex << setw(2) << int(c) << " " << dec;
            }
            cout << endl;
            usDNS.usQuestionSection[i].usTYPE   = get16bits(buffer); 
            usDNS.usQuestionSection[i].usCLASS  = get16bits(buffer); 
        }

        //analyze answer section
        if (ntohs(usDNS.usHeader.usANCOUNT) != 0) {
            usDNS.usAnswerSection = (struct DNSAnswerSection*)malloc(usDNS.usHeader.usANCOUNT 
                                            * sizeof(struct DNSAnswerSection)); 
        }
         cout << "Question NUM:" <<usDNS.usHeader.usANCOUNT << endl;
        for (int i = 0; i < usDNS.usHeader.usANCOUNT ; i++) {
            decode_qname(buffer, usDNS.usAnswerSection[i].usNAME,
                         &usDNS.usAnswerSection[i].NameLength); 
            usDNS.usAnswerSection[i].usTYPE = get16bits(buffer);
            usDNS.usAnswerSection[i].usCLASS = get16bits(buffer);
        }
    }
        cout << m_qName << endl;
}

void Query::decode_qname(const char*& buffer,
                           char *&binaryName, unsigned int* binaryNameLength) throw() { 

    
    m_qName.clear();
    const char *decodeStr = buffer; 
    int length = 0;
    binaryName  = (char *)malloc((*decodeStr+1)*sizeof(char)); 
    MSG("m_qName1");
    int tmpBinaryNameLength = 0;
    while ((length = *decodeStr) != 0) {
        if ((length & 0xc0) == 0) { //normal format
            MSG("m_qName");
            m_qName.append(decodeStr + 1, length); 
            m_qName.append(1,'.');
            memcpy(binaryName + tmpBinaryNameLength , decodeStr, length + 1); 

            tmpBinaryNameLength += (length + 1);
            binaryName  = (char* )realloc(binaryName,tmpBinaryNameLength*sizeof(char));

            decodeStr = decodeStr + length + 1;
            *binaryNameLength = tmpBinaryNameLength;
        }
        else{ //compressed format,11000000 00000000, 
              //                  two bytes, 
              //                  front 2bit means jumps flag, 
              //                  last 14bit means offset bits.    
        }
    }

}

#if 0
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
#endif
