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
#include <iomanip>
#include <netinet/in.h>

#include "logger.h"
#include "message.h"

using namespace dns;
using namespace std;

string Message::asString() const throw() {

    ostringstream text;
    text << "ID: " << showbase << hex << usDNS.usHeader.usTransID << endl << noshowbase; 
    text << "\tfields: [ QR: " << m_qr << " opCode: " << m_opcode << " ]" << endl; 
    text << "\tQDcount: " << usDNS.usHeader.usQDCOUNT << endl;
    text << "\tANcount: " << usDNS.usHeader.usANCOUNT << endl;
    text << "\tNScount: " << usDNS.usHeader.usNSCOUNT << endl;
    text << "\tARcount: " << usDNS.usHeader.usARCOUNT << endl;
    return text.str();
}

void Message::decode_hdr(const char* buffer) throw () {
    struct  DNSHeader tmpHeader;
    
        MSG("decode_hdr----------");
    tmpHeader.usTransID = get16bits(buffer); 

    tmpHeader.usFlags = get16bits(buffer);
    usDNSFlags.usQR     = tmpHeader.usFlags & QR_MASK; 
    usDNSFlags.usOpcode = tmpHeader.usFlags & OPCODE_MASK; 
    usDNSFlags.usAA     = tmpHeader.usFlags & AA_MASK; 
    usDNSFlags.usTC     = tmpHeader.usFlags & TC_MASK; 
    usDNSFlags.usRD     = tmpHeader.usFlags & RD_MASK; 
    usDNSFlags.usRA     = tmpHeader.usFlags & RA_MASK;    
    tmpHeader.usQDCOUNT = get16bits(buffer); 
    tmpHeader.usANCOUNT = get16bits(buffer);
    tmpHeader.usNSCOUNT = get16bits(buffer);
    tmpHeader.usARCOUNT = get16bits(buffer);
    usDNS.usHeader = tmpHeader;
    m_qr        = tmpHeader.usFlags & QR_MASK;
    m_opcode    = tmpHeader.usFlags & OPCODE_MASK;
    MSG("decode_hdr");
}

void Message::code_hdr(char* buffer) throw () {

    put16bits(buffer, usDNS.usHeader.usTransID); 

    
    int fields = (m_qr << 15);
    fields += (m_opcode << 14);
    //...
    //fields += m_rcode;
    //put16bits(buffer, fields);
    put16bits(buffer, 0x8580);

    put16bits(buffer, usDNS.usHeader.usQDCOUNT);
    put16bits(buffer, usDNS.usHeader.usANCOUNT);
    put16bits(buffer, usDNS.usHeader.usNSCOUNT);
    put16bits(buffer, usDNS.usHeader.usANCOUNT);
}                          

void Message::log_buffer(const char* buffer, int size) throw () {

    ostringstream text;

    text << "Message::log_buffer()" << endl;
    text << "size: " << size << " bytes" << endl;
    text << "---------------------------------" << setfill('0');

    for (int i = 0; i < size; i++) {
        if ((i % 10) == 0) {
            text << endl << setw(2) << i << ": ";
        }
        uchar c = buffer[i];
        text << hex << setw(2) << int(c) << " " << dec;
    }
    text << endl << setfill(' ');
    text << "---------------------------------";

    Logger& logger = Logger::instance();
    logger.trace(text);
}

int Message::get16bits(const char*& buffer) throw () {

    int value = static_cast<uchar> (buffer[0]);
    value = value << 8;
    value += static_cast<uchar> (buffer[1]);
    buffer += 2;

    return value;
}

void Message::put16bits(char*& buffer, uint value) throw () {

    buffer[0] = (value & 0xFF00) >> 8;
    buffer[1] = value & 0xFF;
    buffer += 2;
}

void Message::put32bits(char*& buffer, ulong value) throw () {

    buffer[0] = (value & 0xFF000000) >> 24;
    buffer[1] = (value & 0xFF0000) >> 16;
    buffer[2] = (value & 0xFF00) >> 16;
    buffer[3] = (value & 0xFF) >> 16;
    buffer += 4;
}
