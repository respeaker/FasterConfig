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

private:
    Logger *logger;

protected:
};
}
#endif	/* _DNS_IPTABLE_H */
