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
#include <stdio.h>
#include "DNSLookup.h"

int main(void) {
    char szDomainName[] = "www.youku.com";
    std::vector<ULONG> veculIPList;
    std::vector<std::string> vecstrIPList;
    std::vector<std::string> vecCNameList;
    ULONG ulTimeSpent = 0;
    CDNSLookup dnslookup;
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


