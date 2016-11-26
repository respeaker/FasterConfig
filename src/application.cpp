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
#include <stdlib.h> //atoi
#include <pthread.h>

#include <stdio.h>
#include <string.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <errno.h>

#include <sys/socket.h>
#include <netdb.h>
#include <ifaddrs.h>
#include <unistd.h>
#include <glob.h>

#include "application.h"
#include "logger.h"
#include "exception.h"
#include "httpd.h"
#include "iptable.h"


using namespace dns;
using namespace std;
Application *Application::AppThreadCallBack;

void Application::getConfig() {
    AppThreadCallBack = this;

    read_config_file("/etc/fasterconfig/fasterconfig.conf");

    printf("DnsPort:%d\n", dnsPort);
    printf("DnsIP:%s\n", dnsIP);
    printf("HttpPort:%d\n", HttpPort);
    printf("ReUrl:%s\n", ReUrl);
    printf("gatewayIP:%s\n", gatewayIP);
    printf("ErrorHtml:%s\n", ErrorHtml);
    printf("staInterface:%s\n", staInterface);
    printf("apInterface:%s\n", apInterface);
/*http server simple test*/
#if 0
    Httpd *httpserver = new Httpd(gatewayIP,
                                  HttpPort);
    httpserver->setHtmlPath(ErrorHtml);
    httpserver->start();
#endif
#if 0
    m_server.init(dnsPort); 
    AppThreadCallBack->m_server.run();
#endif
}

void Application::run() {

    Iptable *iptable =  new Iptable();
    pthread_t dnsserver_pid;
    pthread_t httpserver_pid;
    int result;
    isDestroied = 1;
    isRedirected = 0;

    m_server.init(dnsPort);

    result = pthread_create(&dnsserver_pid, NULL, &do_dnsServer, NULL);
    if (result != 0) {
        printf("FATAL: Failed to create a new thread (dnsserver) - exiting\n");
        return;
    }
    result = pthread_create(&httpserver_pid, NULL, &do_httpServer, NULL);
    if (result != 0) {
        printf("FATAL: Failed to create a new thread (dnsserver) - exiting\n");
        return;
    }

    int locked;
    while (1) {
        //make sure the same rule run only once
        if (islocked() || (getNetworkStatus(staInterface) == 0)) locked = 1;
        else locked = 0;

        if (locked) {
            if (isRedirected == 1) {
                isDestroied = 0;
            }
        } else {
            if (isDestroied == 1) {
                isRedirected = 0;
            }
        }

        //printf("main process is runing .......\n");
        if (locked == 0 && isDestroied != 1) {
            isDestroied = 1;
            iptable->iptable_destroy_rule();
        }
        if (locked == 1 && isRedirected != 1) {
            isRedirected = 1;
            iptable->iptable_redirect_dns(apInterface, dnsPort);
            iptable->iptable_redirect_http(dnsIP, gatewayIP, HttpPort); 
        }
        sleep(1);
    }
}

void* Application::do_dnsServer(void *args) {
    printf("dns server is runing .......\n");
	AppThreadCallBack->m_server.set_spoof_addr(AppThreadCallBack->dnsIP);
    AppThreadCallBack->m_server.run();
}

void* Application::do_httpServer(void *args) {
    printf("http server is runing .......\n");
    Httpd *httpserver = new Httpd(AppThreadCallBack->gatewayIP,
                                  AppThreadCallBack->HttpPort);
    httpserver->setHtmlPath(AppThreadCallBack->ErrorHtml);
    httpserver->setReUrl(AppThreadCallBack->ReUrl);
    httpserver->start();
}

int Application::read_int_from_config_line(char *config_line) {
    char prm_name[1024];
    int val;
    sscanf(config_line, "%s %d\n", prm_name, &val);
    return val;
}
void Application::read_double_from_config_line(char *config_line, double *val) {
    char prm_name[1024];
    sscanf(config_line, "%s %lf\n", prm_name, val);
}
void Application::read_str_from_config_line(char *config_line, char *val) {
    char prm_name[1024];
    sscanf(config_line, "%s %s\n", prm_name, val);
}


void Application::read_config_file(const char *config_filename) {
    FILE *fp;
    char buf[64];

    if ((fp = fopen(config_filename, "r")) == NULL) {
        fprintf(stderr, "Failed to open config file %s", config_filename);
        exit(EXIT_FAILURE);
    }
    while (!feof(fp)) {
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
        if (strstr(buf, "lockfile ")) {
            read_str_from_config_line(buf, lockfile); 
        }
        if (strstr(buf, "staInterface ")) {
            read_str_from_config_line(buf, staInterface);
        }
        if (strstr(buf, "apInterface ")) {
            read_str_from_config_line(buf, apInterface);
        }
    }
}


int Application::islocked() {
    glob_t results;
    results.gl_pathc = 0;  
    glob(lockfile, 0, NULL, &results); 
    int file_found = results.gl_pathc == 1;
    globfree(&results);
    return file_found;
}
int Application::getNetworkStatus(const char *interfaceName) {
    struct ifaddrs *ifaddr, *ifa;
    int family, s;
    char host[NI_MAXHOST];

    if (getifaddrs(&ifaddr) == -1) {
        perror("getifaddrs");
        return 0;
    }


    for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr == NULL) continue;

        s = getnameinfo(ifa->ifa_addr, sizeof(struct sockaddr_in), host, NI_MAXHOST, NULL, 0, NI_NUMERICHOST);

        if ((strcmp(ifa->ifa_name, interfaceName) == 0) && (ifa->ifa_addr->sa_family == AF_INET)) {
            if (s != 0) {
                printf("getnameinfo() failed: %s\n", gai_strerror(s));
                return 0;
            }
            printf("\tInterface : <%s>\n", ifa->ifa_name);
            printf("\t  Address : <%s>\n", host);
            return 1;
        }
    }

    freeifaddrs(ifaddr);
    return 0;
}
