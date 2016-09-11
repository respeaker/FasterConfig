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


