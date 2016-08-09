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

#ifndef _DNS_APPLICATION_H
#define	_DNS_APPLICATION_H

#include "exception.h"
#include "server.h"
#include "resolver.h"

namespace dns {

/**
 *  Application class is a terminal application that parses arguments from
 *  command line. It has a socket @ref Server to receive queries and answer
 *  responses to those queries, and a @ref Resolver that handles the query and
 *  resolves the domain names contained on it.
 */
class Application {
public:
    /**
     *  Constructor.
     *  Creates a Domain Server Application started from a terminal.
     */
    Application() : m_server(m_resolver) { }

    /**
     *  Destructor
     */
    virtual ~Application() { }

    /**
     *  Parse the port and hosts file from the arguments of main() function
     *  @param argc Number of arguments passed
     *  @param argv Array of arguments
     */
    void parse_arguments(int argc, char** argv) throw (Exception);

    /**
     *  Starts the application. Initialize the @ref Resolver and the @ref Server.
     */
    void run() throw(Exception);

private:
    int m_port;
    std::string m_filename;

    Resolver m_resolver;
    Server m_server;
};
}

#endif	/* _DNS_APPLICATION_H */

