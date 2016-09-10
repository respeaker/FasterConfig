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
#include <pthread.h>

#include <stdio.h>
#include <string.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <errno.h>

#include "application.h"
#include "logger.h"
#include "exception.h"

using namespace dns;
using namespace std;
Application *Application::AppThreadCallBack;

void Application::parse_arguments(int argc, char **argv) throw(Exception) {
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
    AppThreadCallBack = this;

    read_config_file("/etc/fasterconfig/fasterconfig.conf");

    printf("DnsPort:%d\n", dnsPort);
    printf("DnsIP:%s\n", dnsIP);
    printf("HttpPort:%d\n", HttpPort);
    printf("ReUrl:%s\n", ReUrl);
    printf("gatewayIP:%s\n", gatewayIP);
    printf("ErrorHtml:%s\n", ErrorHtml);
}

void Application::run() throw(Exception) {

    pthread_t dnsserver_pid;
    pthread_t httpserver_pid;
    int result;
    Logger &logger = Logger::instance();
    logger.trace("Application::run()");

    //m_resolver.init(m_filename);
    m_server.init(m_port);

    result = pthread_create(&dnsserver_pid, NULL, &do_dnsServer, NULL);
    if (result != 0) {
        printf("FATAL: Failed to create a new thread (dnsserver) - exiting\n");
        //termination_handler(0);
    }
    result = pthread_create(&httpserver_pid, NULL, &do_httpServer, NULL);
    if (result != 0) {
        printf("FATAL: Failed to create a new thread (dnsserver) - exiting\n");
        //termination_handler(0);
    }
    while (1) {
        printf("main process is runing .......\n");
        sleep(1);
    }
}

void* Application::do_dnsServer(void *args) {
    printf("dns server is runing .......\n");
    AppThreadCallBack->m_server.run();
}

void* Application::do_httpServer(void *args) {
    printf("http server is runing .......\n");
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
    }
}


int Application::getLocalInfo() {
    int fd;
    int interfaceNum = 0;
    struct ifreq buf[16];
    struct ifconf ifc;
    struct ifreq ifrcopy;
    char mac[16] = { 0 };
    char ip[32] = { 0 };
    char broadAddr[32] = { 0 };
    char subnetMask[32] = { 0 };

    if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket");
        close(fd);
        return -1;
    }

    ifc.ifc_len = sizeof(buf);
    ifc.ifc_buf = (caddr_t)buf;
    if (!ioctl(fd, SIOCGIFCONF, (char *)&ifc)) {
        interfaceNum = ifc.ifc_len / sizeof(struct ifreq);
        printf("interface num = %d\n", interfaceNum);
        while (interfaceNum-- > 0) {
            printf("\ndevice name: %s\n", buf[interfaceNum].ifr_name);

            //ignore the interface that not up or not runing
            ifrcopy = buf[interfaceNum];
            if (ioctl(fd, SIOCGIFFLAGS, &ifrcopy)) {
                printf("ioctl: %s [%s:%d]\n", strerror(errno), __FILE__, __LINE__);
                close(fd);
                return -1;
            }

            //get the mac of this interface
            if (!ioctl(fd, SIOCGIFHWADDR, (char *)(&buf[interfaceNum]))) {
                memset(mac, 0, sizeof(mac));
                snprintf(mac, sizeof(mac), "%02x%02x%02x%02x%02x%02x",
                         (unsigned char)buf[interfaceNum].ifr_hwaddr.sa_data[0],
                         (unsigned char)buf[interfaceNum].ifr_hwaddr.sa_data[1],
                         (unsigned char)buf[interfaceNum].ifr_hwaddr.sa_data[2],
                         (unsigned char)buf[interfaceNum].ifr_hwaddr.sa_data[3],
                         (unsigned char)buf[interfaceNum].ifr_hwaddr.sa_data[4],
                         (unsigned char)buf[interfaceNum].ifr_hwaddr.sa_data[5]);
                printf("device mac: %s\n", mac);
            } else {
                printf("ioctl: %s [%s:%d]\n", strerror(errno), __FILE__, __LINE__);
                close(fd);
                return -1;
            }

            //get the IP of this interface
            if (!ioctl(fd, SIOCGIFADDR, (char *)&buf[interfaceNum])) {
                snprintf(ip, sizeof(ip), "%s",
                         (char *)inet_ntoa(((struct sockaddr_in *)&(buf[interfaceNum].ifr_addr))->sin_addr));
                printf("device ip: %s\n", ip);
            } else {
                printf("ioctl: %s [%s:%d]\n", strerror(errno), __FILE__, __LINE__);
                close(fd);
                return -1;
            }

            //get the broad address of this interface
            if (!ioctl(fd, SIOCGIFBRDADDR, &buf[interfaceNum])) {
                snprintf(broadAddr, sizeof(broadAddr), "%s",
                         (char *)inet_ntoa(((struct sockaddr_in *)&(buf[interfaceNum].ifr_broadaddr))->sin_addr));
                printf("device broadAddr: %s\n", broadAddr);
            } else {
                printf("ioctl: %s [%s:%d]\n", strerror(errno), __FILE__, __LINE__);
                close(fd);
                return -1;
            }

            //get the subnet mask of this interface
            if (!ioctl(fd, SIOCGIFNETMASK, &buf[interfaceNum])) {
                snprintf(subnetMask, sizeof(subnetMask), "%s",
                         (char *)inet_ntoa(((struct sockaddr_in *)&(buf[interfaceNum].ifr_netmask))->sin_addr));
                printf("device subnetMask: %s\n", subnetMask);
            } else {
                printf("ioctl: %s [%s:%d]\n", strerror(errno), __FILE__, __LINE__);
                close(fd);
                return -1;
            }
        }
    } else {
        printf("ioctl: %s [%s:%d]\n", strerror(errno), __FILE__, __LINE__);
        close(fd); 
        return -1;
    }

    close(fd);

    return 0;
}


