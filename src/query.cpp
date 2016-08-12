
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
        text << "             \tQname: " << usDNS.usQuestionSection[i].usNAME << endl; 
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

    decode_hdr(buffer);

    buffer += HDR_OFFSET;

    if (usDNS.usHeader.usTransID == usCurrentProcID
                        //RFC1035 4.1.1(Header section format)
         && usDNSFlags.usQR ? ((ntohs(usDNS.usHeader.usFlags) & 0xfb7f) == 0x8100 ):
                              ((ntohs(usDNS.usHeader.usFlags) & 0xfb7f) == 0x0001 )
        && ntohs(usDNS.usHeader.usQDCOUNT) >= 0
        && ntohs(usDNS.usHeader.usANCOUNT) >= 0) {
         cout << "malloc qusestion section1" << endl;
        //analyze qusestion section
        if (usDNS.usHeader.usQDCOUNT != 0) {
            cout << "malloc qusestion section" << endl;
            usDNS.usQuestionSection =  (struct DNSQuestionSection*)malloc(usDNS.usHeader.usQDCOUNT 
                                              * sizeof(struct DNSQuestionSection)); 
        }
        for (int i = 0; i < usDNS.usHeader.usQDCOUNT; i++) {
            decode_qname(buffer,  usDNS.usQuestionSection[i].usNAME, 
                         &usDNS.usQuestionSection[i].NameLength); 
            usDNS.usQuestionSection[i].usTYPE   = get16bits(buffer); 
            usDNS.usQuestionSection[i].usCLASS  = get16bits(buffer); 
        }

        //analyze answer section
        if (ntohs(usDNS.usHeader.usANCOUNT) != 0) {
            //usDNS.usAnswerSection =  malloc(usDNS.usHeader.usANCOUNT 
            //                                * sizeof(struct DNSAnswerSection)); 
        }
        for (int i = 0; i < usDNS.usHeader.usANCOUNT ; i++) {
            //decode_qname(buffer, usDNS.usAnswerSection[i].NameLength); 
            //m_qType = get16bits(buffer);
            //m_qClass = get16bits(buffer);
        }
    }
}

void Query::decode_qname(const char*& buffer,
                          char *binaryName, unsigned int* binaryNameLength) throw() { 

    
    m_qName.clear();
    const char *decodeStr = buffer; 
    int length = 0;
    binaryName  = (char *)malloc(sizeof(char)); 
    MSG("m_qName1");
    int tmpBinaryNameLength = 0;
    while ((length = *decodeStr) != 0) {
        if ((length & 0xc0) == 0) { //normal format
            MSG("m_qName");
            m_qName.append(decodeStr + 1, length); 
            m_qName.append(1,'.');
            binaryName  = (char* )realloc(binaryName,(length + 1)*sizeof(char));
            memcpy(binaryName + tmpBinaryNameLength , decodeStr, length + 1); 
            tmpBinaryNameLength += (length + 1);
            decodeStr = decodeStr + length + 1;
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
