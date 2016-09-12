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



#ifndef _DNS_IPTABLE_H
#define	_DNS_IPTABLE_H

#include <string.h>
#include <string>
#include <stdlib.h>
#include <iostream>
#include "logger.h"


#define WD_SHELL_PATH "/bin/sh"
/*@{*/
/**Iptable chain names used by WifiDog */
#define CHAIN_OUTGOING  "fasterconfig_Outgoing"
#define CHAIN_TO_INTERNET "fasterconfig_Internet"
#define CHAIN_TO_ROUTER "fasterconfig_Router"
#define CHAIN_INCOMING  "fasterconfig_Incoming"
#define CHAIN_PREROUTING "fasterconfig_prerouter"
#define CHAIN_POSTROUTING "fasterconfig_postrouting"


namespace dns {
class Iptable {
public:
    /**
 *  Destructor
 */
    virtual ~Iptable();
    Iptable();

    int iptables_do_command(const char *format, ...);
    int execute(const char *cmd_line, int quiet);
    void iptabel_set_NewChain();
    void iptable_destroy_rule();
    void iptable_redirect_dns(char *interface, int rePort);
    void iptable_redirect_http(char *destIP, char *srcIP, int srcPort);

    int isBlock;

private:
    Logger *logger;

protected:
};
}
#endif	/* _DNS_IPTABLE_H */
