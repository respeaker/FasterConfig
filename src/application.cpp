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
#include <stdlib.h> //atoi

#include "application.h"
#include "logger.h"
#include "exception.h"

using namespace dns;
using namespace std;

void Application::parse_arguments(int argc, char** argv) throw (Exception) {
#if 0
    if (argc != 3) {

        string text("Usage: dnsServer <port> <hostsFile>\n");
        text += "Example: dnsServer 9000 hosts\n";
        Exception e(text);
        throw (e);
    }

    m_port = atoi(argv[1]);
    if (m_port < 1 || m_port > 65535) {
        
        string text("Error: Invalid port number.\n");
        Exception e(text);
        throw (e);
    }

    m_filename.assign(argv[2]);
#endif
    read_config_file("/etc/fasterconfig/fasterconfig.conf");

    printf("DnsPort:%d\n", dnsPort); 
    printf("DnsIP:%s\n", dnsIP); 
    printf("HttpPort:%d\n", HttpPort); 
    printf("ReUrl:%s\n", ReUrl); 
    printf("gatewayIP:%s\n", gatewayIP); 
    printf("ErrorHtml:%s\n", ErrorHtml); 
}

void Application::run() throw (Exception) {

    Logger& logger = Logger::instance();
    logger.trace("Application::run()");

    m_resolver.init(m_filename);
    m_server.init(m_port);
    m_server.run();
}

int Application::read_int_from_config_line(char* config_line) {    
    char prm_name[1024];
    int val;
    sscanf(config_line, "%s %d\n", prm_name, &val);
    return val;
}
void Application::read_double_from_config_line(char* config_line, double* val) {    
    char prm_name[1024];
    sscanf(config_line, "%s %lf\n", prm_name, val);
}
void Application::read_str_from_config_line(char* config_line, char* val) {    
    char prm_name[1024];
    sscanf(config_line, "%s %s\n", prm_name, val);
}


void Application::read_config_file(const char* config_filename) {
    FILE *fp;
    char buf[64];

    if ((fp=fopen(config_filename, "r")) == NULL) {
        fprintf(stderr, "Failed to open config file %s", config_filename);
        exit(EXIT_FAILURE);
    }
    while(! feof(fp)) {
        fgets(buf, 1024, fp);
        if (buf[0] == '#' || strlen(buf) < 4) {
            continue;
        }
        if (strstr(buf, "DnsPort ")) {
            dnsPort = read_int_from_config_line(buf);
        }
        if (strstr(buf, "DnsIP ")) {
            read_str_from_config_line(buf, dnsIP);
        }
        if (strstr(buf, "HttpPort ")) {
            HttpPort = read_int_from_config_line(buf);
        }
        if (strstr(buf, "ReUrl ")) {
            read_str_from_config_line(buf, ReUrl);
        }
        if (strstr(buf, "gatewayIP ")) {
            read_str_from_config_line(buf, gatewayIP);  
        }
        if (strstr(buf, "ErrorHtml ")) {
            read_str_from_config_line(buf, ErrorHtml);
        }
    }
}

