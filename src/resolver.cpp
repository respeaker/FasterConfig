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


#include <string>
#include <iostream>
#include <fstream>

#include "logger.h"
#include "resolver.h"
#include "query.h"
#include "response.h"

using namespace std;
using namespace dns;

void Resolver::init(const std::string& filename) throw (Exception) {

    Logger& logger = Logger::instance();
    string text("Resolver::init() | filename: ");
    text += filename;
    logger.trace(text);

    ifstream file(filename.data());

    if (!file) {

        string text("Could not open file: ");
        text += filename;
        logger.error(text);
        Exception e(text);
        throw (e);
    }

    string line;
    while (!file.eof()) {

        getline(file, line);
        store(line);
    }

    file.close();

    print_records();
}

void Resolver::store(const std::string& line) throw () {

    string::size_type ipAddresEndPos = line.find_first_of(" ");
    if (ipAddresEndPos == string::npos) return;

    string::size_type domainNameStartPos = line.find_last_of(" ");
    if (domainNameStartPos == string::npos) return;
    domainNameStartPos += 1;

    string ipAddress(line, 0, ipAddresEndPos);
    string domainName(line, domainNameStartPos, line.length());

    try {
        Record* record = new Record(ipAddress, domainName);
        add(record);
    }
    catch (exception& e) {
        Logger& logger = Logger::instance();
        logger.error("Could not create Record object");
    }
}

void Resolver::add(Record* newone) throw() {

    Logger& logger = Logger::instance();
    string text("Resolver::add() | Record: ");
    text += newone->ipAddress.data();
    text += "-";
    text += newone->domainName.data();
    logger.trace(text);


    Record* record = m_record_list;
    if (record == 0) {
        m_record_list = newone;
        return;
    }

    while (record->next != 0) {
        record = record->next;
    }
    record->next = newone;
}

void Resolver::deleteList() throw() {

    Record* record = m_record_list;
    while (record != 0) {
        Record* next = record->next;
        delete record;
        record = next;
    }
}

void Resolver::print_records() throw() {

    cout << "Reading records from file..." << endl;

    Record* record = m_record_list;
    if (record == 0) {
        cout << "No records on list." << endl;
        return;
    }

    while (record != 0) {
        cout << "Record: " << record->ipAddress.data();
        cout << " - " << record->domainName.data() << endl;
        record = record->next;
    }
    cout << endl;
}

const string Resolver::find(const std::string& ipAddress) throw () {

    Logger& logger = Logger::instance();
    string text("Resolver::find() | ipAddres: ");
    text += ipAddress;

    string domainName;

    Record* record = m_record_list;
    while (record != 0) {
        
        if (record->ipAddress == ipAddress) {

            domainName = record->domainName;
            break;
        }
        record = record->next;
    }

    text += " ---> ";
    text += domainName;
    logger.trace(text);

    return domainName;
}

void Resolver::process(const Query& query, Response& response) throw () {

    Logger& logger = Logger::instance();
    string text("Resolver::process()");
    text += query.asString();
    logger.trace(text);
#if 0
    string qName = query.getQName();
    string ipAddress = convert(qName);
    //string domainName = find( ipAddress );
    string domainName = "112.80.248.73";

    response.setID( query.getID() );
    response.setQdCount(1);
    response.setAnCount(1);
    response.setName( query.getQName() );
    response.setType( query.getQType() );
    response.setClass( query.getQClass() );
    response.setRdata(domainName);

    cout << endl << "Query for: " << ipAddress;
    cout << endl << "Response with: ";

    if (domainName.empty()) {
        cout << "NameError" << endl;
        response.setRCode(Response::NameError);
        response.setRdLength(1); // null label
    }
    else {
        cout << domainName << endl;
        response.setRCode(Response::Ok);
        response.setRdLength(domainName.size()+2); // + initial label length & null label
    }
#endif
    text = "Resolver::process()";
    text += response.asString();
    logger.trace(text);
}

string Resolver::convert(const std::string& qName) throw() {
#if 0
    int pos = qName.find(".in-addr.arpa");
    if (pos == string::npos) return string();

    string tmp(qName, 0, pos);
    string ipAddress;
    while ((pos = tmp.rfind('.')) != string::npos) {

        ipAddress.append(tmp, pos+1, tmp.size());
        ipAddress.append(".");
        tmp.erase(pos, tmp.size());
    }
    ipAddress.append(tmp, 0, tmp.size());
#endif
    string ipAddress = "172.16.1.1";
    return ipAddress;
}


