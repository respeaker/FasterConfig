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

#ifndef _DNS_APPLICATION_H
#define	_DNS_APPLICATION_H

#include "dnsserver.h"

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
    Application() { }

    /**
     *  Destructor
     */
    virtual ~Application() { }

    /**
     *  Parse the port and hosts file from the arguments of main() function
     *  @param argc Number of arguments passed
     *  @param argv Array of arguments
     */
    void getConfig();

    /**
     *  Starts the application. Initialize the @ref Resolver and the @ref Server.
     */
    void run() ;


    int read_int_from_config_line(char* config_line);
    void read_double_from_config_line(char* config_line, double* val);
    void read_str_from_config_line(char* config_line, char* val) ;
    void read_config_file(const char* config_filename) ;
    int getNetworkStatus(const char *interfaceName);

    int islocked();
private:
    int m_port;
    std::string m_filename;

    DnsServer m_server;
    int dnsPort;
    char       dnsIP[64];
    int         HttpPort;
    char        ReUrl[64];
    char        gatewayIP[64];
    char        ErrorHtml[64];
    char        lockfile[64];
    char        staInterface[64];
    char        apInterface[64];


    int isRedirected;
    int isDestroied;


    static Application *AppThreadCallBack;

    static void *do_dnsServer(void *args);
    static void *do_httpServer(void *args);
};
}

#endif	/* _DNS_APPLICATION_H */

