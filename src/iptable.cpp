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
#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>
#include <errno.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>

#include <net/if.h>

#include <fcntl.h>
#include <net/ethernet.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <netpacket/packet.h>

#include <string.h>
#include <netdb.h>


#include "iptable.h"

using namespace dns;
using namespace std;
Iptable::Iptable() {
     logger = &Logger::instance();
     iptable_destroy_rule(); 
     iptabel_set_NewChain();
     isBlock = 0;
}
Iptable::~Iptable() {

}
/** @internal 
 * */
int Iptable::iptables_do_command(const char *format, ...)
{
    va_list vlist;
    char *fmt_cmd;
    char *cmd;
    int rc;
    char fw_quiet = 1;

    va_start(vlist, format);
    vasprintf(&fmt_cmd, format, vlist);
    va_end(vlist);

    asprintf(&cmd, "iptables %s", fmt_cmd);
    free(fmt_cmd);

    debug(LOG_DEBUG, "Executing command: %s", cmd);

    rc = execute(cmd, fw_quiet);

    if (rc != 0) {
        // If quiet, do not display the error
        if (fw_quiet == 0)
            debug(LOG_ERR, "iptables command failed(%d): %s", rc, cmd);
        else if (fw_quiet == 1)
            debug(LOG_DEBUG, "iptables command failed(%d): %s", rc, cmd);
    }

    free(cmd);

    return rc;
}
/** Fork a child and execute a shell command, the parent
 * process waits for the child to return and returns the child's exit()
 * value.
 * @return Return code of the command
 */
int Iptable::execute(const char *cmd_line, int quiet)
{
    int pid, status, rc;

    const char *new_argv[4];
    new_argv[0] = WD_SHELL_PATH;
    new_argv[1] = "-c";
    new_argv[2] = cmd_line;
    new_argv[3] = NULL;

    pid = fork();
    if (pid == 0) {             /* for the child process:         */
        /* We don't want to see any errors if quiet flag is on */
        if (quiet)
            close(2);
        if (execvp(WD_SHELL_PATH, (char *const *)new_argv) == -1) { /* execute the command  */
            debug(LOG_ERR, "execvp(): %s", strerror(errno));
        } else {
            debug(LOG_ERR, "execvp() failed");
        }
        exit(1);
    }

    /* for the parent:      */
    debug(LOG_DEBUG, "Waiting for PID %d to exit", pid);
    rc = waitpid(pid, &status, 0);
    debug(LOG_DEBUG, "Process PID %d exited", rc);
    
    if (-1 == rc) {
        debug(LOG_ERR, "waitpid() failed (%s)", strerror(errno));
        return 1; /* waitpid failed. */
    }

    if (WIFEXITED(status)) {
        return (WEXITSTATUS(status));
    } else {
        /* If we get here, child did not exit cleanly. Will return non-zero exit code to caller*/
        debug(LOG_DEBUG, "Child may have been killed.");
        return 1;
    }
}
void Iptable::iptabel_set_NewChain() {
        /* Create new chains */
    iptables_do_command("-t nat -N " CHAIN_OUTGOING);
    iptables_do_command("-t nat -N " CHAIN_PREROUTING); 
    iptables_do_command("-t nat -N " CHAIN_INCOMING); 
    iptables_do_command("-t nat -N " CHAIN_POSTROUTING);

    iptables_do_command("-t nat -A OUTPUT       -j " CHAIN_OUTGOING);
    iptables_do_command("-t nat -A PREROUTING   -j " CHAIN_PREROUTING); 
    iptables_do_command("-t nat -A INPUT        -j " CHAIN_INCOMING); 
    iptables_do_command("-t nat -A POSTROUTING  -j " CHAIN_POSTROUTING);    
     
}

void Iptable::iptable_destroy_rule() {
    iptables_do_command("-t nat -F " CHAIN_OUTGOING);
    iptables_do_command("-t nat -F " CHAIN_PREROUTING); 
    iptables_do_command("-t nat -F " CHAIN_INCOMING); 
    iptables_do_command("-t nat -F " CHAIN_POSTROUTING); 

    /*delete custom chain*/
    iptables_do_command("-t nat -X " CHAIN_OUTGOING);
    iptables_do_command("-t nat -X " CHAIN_PREROUTING); 
    iptables_do_command("-t nat -X " CHAIN_INCOMING); 
    iptables_do_command("-t nat -X " CHAIN_POSTROUTING); 
}

/**
 * iptables -t nat -I PREROUTING -i br-lan -p udp -m udp
 * --dport 53 -j REDIRECT --to-ports 9000 
 *  
 * iptables -t nat -I PREROUTING -i br-lan -p tcp -m tcp --dport 
 * 53 -j REDIRECT --to-ports 9000 
 *  
 * iptables -t nat -A OUTPUT -p udp --dport 9000 -j DNAT --to 
 * 127.0.0.1:53 
 *  
 * iptables -t nat -A OUTPUT -p tcp --dport 9000 -j DNAT --to 
 * 127.0.0.1:53 
 * 
 * @author Baozhu (9/11/2016)
 * 
 * @param interface 
 * @param rePort 
 */
void Iptable::iptable_redirect_dns(char *interface, int rePort) {
    char *command = NULL;
    asprintf(&command, " -t nat -I " CHAIN_PREROUTING
             " -i %s -p udp -m udp --dport 53 -j REDIRECT --to-ports %d", interface, rePort);
    iptables_do_command(command);

    asprintf(&command, " -t nat -I " CHAIN_PREROUTING
             " -i %s -p tcp -m tcp --dport 53 -j REDIRECT --to-ports %d", interface, rePort);
    iptables_do_command(command);

    asprintf(&command, " -t nat -A " CHAIN_OUTGOING
             " -p udp --dport %d -j DNAT --to 127.0.0.1:53", rePort);
    iptables_do_command(command);

    asprintf(&command, " -t nat -A " CHAIN_OUTGOING
             " -p tcp --dport %d -j DNAT --to 127.0.0.1:53", rePort);
    iptables_do_command(command);
    free(command);
}

/**
 * iptables -t nat -A POSTROUTING -s 172.31.255.240 -j SNAT 
 * --to-source 192.168.100.1 
 *  
 * iptables -t nat -A OUTPUT -d 172.31.255.240 -j DNAT 
 * --to-destination 192.168.100.1 
 *  
 * iptables -t nat -A PREROUTING -p tcp --dport 80 -j REDIRECT 
 * --to-ports 2049 
 *  
 * iptables -t nat -A OUTPUT -d localhost -p tcp --dport 80 -j 
 * REDIRECT --to-ports 2049 
 *  
 * @author Baozhu (9/11/2016)
 * 
 * @param destIP 
 * @param srcIP 
 * @param destPort 
 * @param srcPort 
 */
void Iptable::iptable_redirect_http(char *destIP,char *srcIP, int srcPort) {
    char *command = NULL;
/*
    asprintf(&command, " -t nat -A " CHAIN_POSTROUTING
             " -s %s -j SNAT --to-source %s",destIP, srcIP); 
    iptables_do_command(command);

    asprintf(&command, " -t nat -A " CHAIN_OUTGOING
             "  -d %s -j DNAT --to-destination %s", destIP, srcIP); 
    iptables_do_command(command);
*/
    asprintf(&command, " -t nat -A " CHAIN_PREROUTING
             "  -d  %s  -p tcp --dport 80 -j REDIRECT --to-ports %d", destIP , srcPort); 
    iptables_do_command(command);
/*
    asprintf(&command, " -t nat -A " CHAIN_OUTGOING
             "  -d localhost -p tcp --dport 80 -j REDIRECT --to-ports  %d", srcPort); 
    iptables_do_command(command);
*/
    free(command);
}
