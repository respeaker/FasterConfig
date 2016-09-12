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

#include <stdio.h>
#include "DNSLookup.h"
using namespace std;
using namespace dns;
int main(void) {
    char szDomainName[] = "www.baidu.com";
    std::vector<ULONG> veculIPList;
    std::vector<std::string> vecstrIPList;
    std::vector<std::string> vecCNameList;
    ULONG ulTimeSpent = 0;
    Resolver m_resolver;
    CDNSLookup dnslookup(m_resolver);
    BOOL bRet = dnslookup.DNSLookup(inet_addr("114.114.114.114"), 
                                    szDomainName, 
                                    &vecstrIPList, 
                                    &vecCNameList, 
                                    1000, &ulTimeSpent);

    printf("DNSLookup result (%s):\n", szDomainName);
    if (!bRet) {
        printf("timeout!\n");
        return -1;
    }

    for (int i = 0; i != veculIPList.size(); ++i) {
        printf("IP%d(ULONG) = %lu\n", i + 1, veculIPList[i]);
    }
    for (int i = 0; i != vecstrIPList.size(); ++i) {
        printf("IP%d(string) = %s\n", i + 1, vecstrIPList[i].c_str());
    }
    for (int i = 0; i != vecCNameList.size(); ++i) {
        printf("CName%d = %s\n", i + 1, vecCNameList[i].c_str());
    }
    printf("time spent = %lums\n", ulTimeSpent);

    return 0;
}


