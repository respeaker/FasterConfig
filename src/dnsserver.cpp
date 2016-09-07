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
#include <cstring>
#include <sys/socket.h>
#include <errno.h>

#include "logger.h"
#include "server.h"
#include "resolver.h"

using namespace std;
using namespace dns;

void DnsServer::init() throw (Exception) {

    Logger& logger = Logger::instance();
    logger.trace("Server::init()");

    m_sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    m_address.sin_family = AF_INET;
    m_address.sin_addr.s_addr = INADDR_ANY;
    m_address.sin_port = htons(dPort);

    int rbind = bind(m_sockfd, (struct sockaddr *) & m_address,
                     sizeof (struct sockaddr_in));
    
    if (rbind != 0) {
        string text("Could not bind: ");
        text += strerror(errno);
        Exception e(text);
        throw(e);
    }

    cout << "Listening in port: " << port << ", sockfd: " << m_sockfd << endl;
}

void DnsServer::run() throw () {

    Logger& logger = Logger::instance();
    logger.trace("Server::run()");
    
    cout << "DNS Server running..." << endl;

    char buffer[BUFFER_SIZE];
    struct sockaddr_in clientAddress;
    socklen_t addrLen = sizeof (struct sockaddr_in);

    while (true) {

        int nbytes = recvfrom(m_sockfd, buffer, BUFFER_SIZE, 0,
                     (struct sockaddr *) &clientAddress, &addrLen);
       
        m_query.decode(buffer, nbytes);
        m_query.asString();

        m_resolver.process(m_query, m_response);

        m_response.asString();
        memset(buffer, 0, BUFFER_SIZE);
        nbytes = m_response.code(buffer);

        sendto(m_sockfd, buffer, nbytes, 0, (struct sockaddr *) &clientAddress,
               addrLen);
    }
}

void DnsServer::SetDnsPort(int port){
    dPort = port;
}
