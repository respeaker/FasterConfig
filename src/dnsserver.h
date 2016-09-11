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



#ifndef _DNS_SERVER_H
#define	_DNS_SERVER_H

#include <netinet/in.h>

#include "exception.h"
#include "query.h"
#include "response.h"

namespace dns {

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
    DnsServer(Resolver& resolver) : m_resolver(resolver)
        { }

    /**
     *  Destructor
     */
    virtual ~DnsServer() { }

    /**
     *  Initializes the server creating a UDP datagram socket and binding it to
     *  the INADDR_ANY address and the port passed.
     *  @param port Port number where the socket is binded.
     */
    void init(int port) throw(Exception);

    /**
     *  The socket server runs in an infinite loop, waiting for queries and
     *  handling them through the @ref Resolver and sending back the responses.
     */
    void run() throw();
    
private:
    static const int BUFFER_SIZE = 1024;

    struct sockaddr_in m_address;
    int m_sockfd;

    Query m_query;
    Response m_response;

    Resolver& m_resolver;
};
}

#endif	/* _DNS_SERVER_H */

