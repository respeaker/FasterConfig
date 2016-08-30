/*
2016 Copyright (c) Seeed Technology Inc.  All right reserved.
Author:Baozhu Zuo
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
#pragma GCC diagnostic ignored "-Wwrite-strings" 
#pragma GCC diagnostic ignored "-Wpmf-conversions"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>

#include "logger.h"
#include "httpd.h"

using namespace dns;
Httpd *Httpd::HttpServerCallBack;
Httpd::Httpd(const std::string &ip_address = "192.168.199.125",  int port = 321) {
    config.gw_port = port;
    config.gw_address = (char *)malloc(ip_address.length() * sizeof(char));
    strcpy(config.gw_address, ip_address.data());



    char isAcceptableTmp[96] =
        /*      Bit 0           xalpha          -- see RFC 1630
        **      Bit 1           xpalpha         -- as xalpha but with plus.
        **      Bit 2 ...       path            -- as xpalpha but with /
        */
        /*   0 1 2 3 4 5 6 7 8 9 A B C D E F */
    { 0, 7, 7, 0, 7, 0, 7, 7, 7, 7, 7, 6, 7, 7, 7, 4,       /* 2x   !"#$%&'()*+,-./ */
        7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 0, 0, 0, 0, 0, 0,     /* 3x  0123456789:;<=>?  */
        7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,     /* 4x  @ABCDEFGHIJKLMNO */
        7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 0, 0, 0, 0, 7,     /* 5X  PQRSTUVWXYZ[\]^_ */
        0, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,     /* 6x  `abcdefghijklmno */
        7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 0, 0, 0, 0, 0
    };                              /* 7X  pqrstuvwxyz{\}~ DEL */
    memcpy(hex, "0123456789ABCDEF", 16);
    memcpy(LIBHTTPD_VERSION, "0.2-fasterconfig", 32);
    memcpy(LIBHTTPD_VENDOR, "Seeed Technology Inc", 32);
    memcpy(isAcceptable, isAcceptableTmp, 96);
    logger = &Logger::instance();

    HttpServerCallBack = this;
}
Httpd::~Httpd() {
    free(config.gw_address);
}
int Httpd::start() {

    if ((webserver = httpdCreate(config.gw_address, config.gw_port)) == NULL) {
        debug(LOG_ERR, "Could not create web server: %s", strerror(errno));
        return -1;
    }

    request *r;
    void **params;
    int result;
    pthread_t tid;
    httpdAddCContent(webserver, "/", "fasterconfig", 0, NULL, &Httpd::http_callback_fasterconfig);
    httpdAddCContent(webserver, "/fasterconfig", "root", 0, NULL, &Httpd::http_callback_fasterconfig);

    debug(LOG_INFO,  "create web server:%s:%d\n",config.gw_address,config.gw_port);

    httpdSetErrorFunction(webserver, 404, &Httpd::http_callback_404);

    while (1) {
        r = httpdGetConnection(webserver, NULL);

        /* We can't convert this to a switch because there might be
         * values that are not -1, 0 or 1. */
        if (webserver->lastError == -1) {
            /* Interrupted system call */
            if (NULL != r) {
                httpdEndRequest(r);
            }
        } else if (webserver->lastError < -1) {
            /*
             * FIXME
             * An error occurred - should we abort?
             * reboot the device ?
             */
            debug(LOG_ERR, "FATAL: httpdGetConnection returned unexpected value %d, exiting.", webserver->lastError);
            //termination_handler(0);
        } else if (r != NULL) {
            /*
             * We got a connection
             *
             * We should create another thread
             */
            debug(LOG_INFO, "Received connection from %s, spawning worker thread", r->clientAddr);
            /* The void**'s are a simulation of the normal C
             * function calling sequence. */
            params = (void **)malloc(2 * sizeof(void *));
            *params = reinterpret_cast<void *>(webserver);
            *(params + 1) = r;

            result = pthread_create(&tid, NULL, &thread_httpd, (void *)params);
            if (result != 0) {
                debug(LOG_ERR, "FATAL: Failed to create a new thread (httpd) - exiting");
                //termination_handler(0);
            }
            pthread_detach(tid);
        } else {
            /* webserver->lastError should be 2 */
            /* XXX We failed an ACL.... No handling because
             * we don't set any... */
        }
    }
    return 0;
}
void* Httpd::thread_httpd(void *args)
{
	void	**params;
	httpd	*webserver;
	request	*r;
	
	params = (void **)args;
	webserver = reinterpret_cast<httpd *>(*params);
	r = reinterpret_cast<request*>(*(params + 1));
	free(params); /* XXX We must release this ourselves. */

    if (HttpServerCallBack->httpdReadRequest(webserver, r) == 0) {
         HttpServerCallBack->debug(LOG_INFO,"LOG_DEBUG........:%d",LOG_DEBUG); 
		/*
		 * We read the request fine
		 */
		HttpServerCallBack->debug(LOG_DEBUG, "Processing request from %s", r->clientAddr);
		HttpServerCallBack->debug(LOG_DEBUG, "Calling httpdProcessRequest() for %s", r->clientAddr);
		HttpServerCallBack->httpdProcessRequest(webserver, r);
		HttpServerCallBack->debug(LOG_DEBUG, "Returned from httpdProcessRequest() for %s", r->clientAddr);
	}
	else {
		HttpServerCallBack->debug(LOG_DEBUG, "No valid request received from %s", r->clientAddr);
	}
	HttpServerCallBack->debug(LOG_DEBUG, "Closing connection with %s", r->clientAddr);
	HttpServerCallBack->httpdEndRequest(r);
}


httpd* Httpd::httpdCreate(char *host, int port) {
    httpd *httpServer;
    int sock, opt;
    struct sockaddr_in addr;

    /*
     ** Create the handle and setup it's basic config
     */
    httpServer = (httpd *)malloc(sizeof(httpd));
    if (httpServer == NULL) return (NULL);
    bzero(httpServer, sizeof(httpd));
    httpServer->port = port;
    if (host == HTTP_ANY_ADDR) httpServer->host = HTTP_ANY_ADDR;
    else httpServer->host = strdup(host);
    httpServer->content = (httpDir *)malloc(sizeof(httpDir));
    bzero(httpServer->content, sizeof(httpDir));
    httpServer->content->name = strdup("");

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        free(httpServer);
        return (NULL);
    }

    httpServer->serverSock = sock;
    bzero(&addr, sizeof(addr));
    addr.sin_family = AF_INET;
    if (httpServer->host == HTTP_ANY_ADDR) {
        addr.sin_addr.s_addr = htonl(INADDR_ANY);
    } else {
        addr.sin_addr.s_addr = inet_addr(httpServer->host);
    }
    addr.sin_port = htons((u_short)httpServer->port);
    if (bind(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        close(sock);
        free(httpServer);
        return (NULL);
    }
    listen(sock, 128);
    httpServer->startTime = time(NULL);
    return (httpServer);
}
void Httpd::httpdProcessRequest(httpd *server, request *r) {
    char dirName[HTTP_MAX_URL], entryName[HTTP_MAX_URL], *cp;
    httpDir *dir;
    httpContent *entry;


    r->response.responseLength = 0;
    strncpy(dirName, httpdRequestPath(r), HTTP_MAX_URL);
    dirName[HTTP_MAX_URL - 1] = 0;
    cp = strrchr(dirName, '/');
    if (cp == NULL) {
        /* printf("Invalid request path '%s'\n", dirName); */
        return;
    }
    
    strncpy(entryName, cp + 1, HTTP_MAX_URL);
    entryName[HTTP_MAX_URL - 1] = 0;
    if (cp != dirName) *cp = 0;
    else *(cp + 1) = 0;
    dir = _httpd_findContentDir(server, dirName, HTTP_FALSE);

    debug(LOG_INFO, "dir: %s", dir->name); 
    if (!dir) {
         debug(LOG_INFO, "dir is NULL"); 
        _httpd_send404(server, r);
        _httpd_writeAccessLog(server, r);
        return;
    }
    entry = _httpd_findContentEntry(r, dir, entryName);
    if (1) {
        _httpd_send404(server, r);
        _httpd_writeAccessLog(server, r);
        return;
    }

    if (entry->preload) {
       // if ((entry->preload)(server) < 0) {
        //    _httpd_writeAccessLog(server, r);
        //    return;
        //}
    }
    debug(LOG_INFO, "entry->type:%d",entry->type);
    
    
    switch (entry->type) {
    case HTTP_C_FUNCT:
    case HTTP_C_WILDCARD:
        {
            debug(LOG_DEBUG,"r->response.headersSent:%d",r->response.headersSent);
            debug(LOG_DEBUG, " r->response.contentType:%s", r->response.contentType);
            (this->*entry->function)(server, r);
            break;
        }
    case HTTP_STATIC:
        _httpd_sendStatic(server, r, entry->data);
        break;

    case HTTP_FILE:
        _httpd_sendFile(server, r, entry->path);
        break;

    case HTTP_WILDCARD:
        if (_httpd_sendDirectoryEntry(server, r, entry, entryName) < 0) {
            _httpd_send404(server, r);
        }
        break;
    }
    _httpd_writeAccessLog(server, r);
}
int Httpd::httpdSetErrorFunction(httpd *server, int error, void (Httpd::*function)(httpd *, request *, int)) {
    char errBuf[80];

    switch (error) {
    case 304:
        server->errorFunction304 = function;
        break;
    case 403:
        server->errorFunction403 = function;
        break;
    case 404:
        server->errorFunction404 = function;
        break;
    default:
        snprintf(errBuf, 80, "Invalid error code (%d) for custom callback", error);
        _httpd_writeErrorLog(server, NULL, LEVEL_ERROR, errBuf);
        return (-1);
        break;
    }
    return (0);
}

request* Httpd::httpdGetConnection(httpd *server, struct timeval *timeout) {
    int result;
    fd_set fds;
    struct sockaddr_in addr;
    socklen_t addrLen;
    char *ipaddr;
    request *r;
    /* Reset error */
    server->lastError = 0;
    FD_ZERO(&fds);
    FD_SET(server->serverSock, &fds);
    result = 0;
    while (result == 0) {
        result = select(server->serverSock + 1, &fds, 0, 0, timeout);
        if (result < 0) {
            server->lastError = -1;
            return (NULL);
        }
        if (timeout != 0 && result == 0) {
            server->lastError = 0;
            return (NULL);
        }
        if (result > 0) {
            break;
        }
    }
    /* Allocate request struct */
    r = (request *)malloc(sizeof(request));
    if (r == NULL) {
        server->lastError = -3;
        return (NULL);
    }
    memset((void *)r, 0, sizeof(request));
    /* Get on with it */
    bzero(&addr, sizeof(addr));
    addrLen = sizeof(addr);
    r->clientSock = accept(server->serverSock, (struct sockaddr *)&addr, &addrLen);
    ipaddr = inet_ntoa(addr.sin_addr);
    if (ipaddr) {
        strncpy(r->clientAddr, ipaddr, HTTP_IP_ADDR_LEN);
        r->clientAddr[HTTP_IP_ADDR_LEN - 1] = 0;
    } else *r->clientAddr = 0;
    r->readBufRemain = 0;
    r->readBufPtr = NULL;

    /*
     ** Check the default ACL
     */
    if (server->defaultAcl) {
        if (httpdCheckAcl(server, r, server->defaultAcl)
            == HTTP_ACL_DENY) {
            httpdEndRequest(r);
            server->lastError = 2;
            return (NULL);
        }
    }
    return (r);
}


int Httpd::httpdAddCContent(httpd *server, const char *dir, const char *name, int indexFlag,
                            int (*preload)(), void (Httpd::*function)(httpd *, request *)) {
    httpDir *dirPtr;
    httpContent *newEntry;

    dirPtr = _httpd_findContentDir(server, const_cast<char*>(dir), HTTP_TRUE);
    newEntry = (httpContent *)malloc(sizeof(httpContent));
    if (newEntry == NULL) return (-1);
    bzero(newEntry, sizeof(httpContent));
    newEntry->name = strdup(name);
    newEntry->type = HTTP_C_FUNCT;
    newEntry->indexFlag = indexFlag;
    newEntry->function = function;
    newEntry->preload = preload;
    newEntry->next = dirPtr->entries;
    dirPtr->entries = newEntry;
    return (0);
}

httpDir* Httpd::_httpd_findContentDir(httpd *server, char *dir, int createFlag) {

    char buffer[HTTP_MAX_URL], *curDir;
    httpDir * curItem = new httpDir;
    httpDir *curChild = new httpDir;

    strncpy(buffer, dir, HTTP_MAX_URL);
    buffer[HTTP_MAX_URL - 1] = 0;
    curItem = server->content;
    curDir = strtok(buffer, "/");
    while (curDir) {
        curChild = curItem->children;
        while (curChild) {
            if (strcmp(curChild->name, curDir) == 0) break;
            curChild = curChild->next;
        }
        if (curChild == NULL) {
            if (createFlag == HTTP_TRUE) {
                curChild = (httpDir *)malloc(sizeof(httpDir));
                bzero(curChild, sizeof(httpDir));
                curChild->name = strdup(curDir);
                curChild->next = curItem->children;
                curItem->children = curChild;
            } else {
                return (NULL);
            }
        }
        curItem = curChild;
        curDir = strtok(NULL, "/");
    }
    
    return (curItem);
}

void Httpd::http_callback_fasterconfig(httpd *webserver, request *r) {
            debug(LOG_DEBUG,"r->response.headersSent:%d",r->response.headersSent);
            debug(LOG_DEBUG, " r->response.contentType:%s", r->response.contentType);
    send_http_page(r, "FasterConfig", "Please use the menu to navigate the features of this FasterConfig installation.");
}

void Httpd::send_http_page(request *r, const char *title, const char *message) {
    char *buffer;
    struct stat stat_info;
    int fd;
    ssize_t written;
    fd = open("/Users/Yanny/workspace/FasterConfig/fasterconfig-msg.html", O_RDONLY);
    if (fd == -1) {
        debug(LOG_CRIT, "Failed to open HTML message file /etc/fasterconfig/fasterconfig-msg.html: %s", strerror(errno));
        return;
    }

    if (fstat(fd, &stat_info) == -1) {
        debug(LOG_CRIT, "Failed to stat HTML message file: %s", strerror(errno));
        close(fd);
        return;
    }
    // Cast from long to unsigned int
    buffer = (char *)malloc((size_t)stat_info.st_size + 1);
    memset(buffer, 0, ((size_t)stat_info.st_size + 1));
    written = read(fd, buffer, (size_t)stat_info.st_size);
    if (written == -1) {
        debug(LOG_CRIT, "Failed to read HTML message file: %s", strerror(errno));
        free(buffer);
        close(fd);
        return;
    }
    close(fd);

    
    buffer[written] = 0;
    httpdAddVariable(r, "title", title);
    httpdAddVariable(r, "message", message);
    httpdAddVariable(r, "nodeID", "123");
    httpdOutput(r, buffer);
    free(buffer);
}
void Httpd::httpdOutput(request *r, const char *msg) {
    const char *src;
    char buf[HTTP_MAX_LEN], varName[80], *dest;
    int count;

    src = msg;
    dest = buf;
    count = 0;
    memset(buf, 0, HTTP_MAX_LEN);
    while (*src && count < HTTP_MAX_LEN) {
        if (*src == '$') {
            const char *tmp;
            char *cp;
            int count2;
            httpVar *curVar;

            tmp = src + 1;
            cp = varName;
            count2 = 0;
            while (*tmp && (isalnum((unsigned char)*tmp) || *tmp == '_') && count2 < 80) {
                *cp++ = *tmp++;
                count2++;
            }
            *cp = 0;
            curVar = httpdGetVariableByName(r, varName);
            if (curVar && ((count + strlen(curVar->value)) < HTTP_MAX_LEN)) {
                strcpy(dest, curVar->value);
                dest = dest + strlen(dest);
                count += strlen(dest);
                src = src + strlen(varName) + 1;
                continue;
            } else {
                *dest++ = *src++;
                count++;
                continue;
            }
        }
        *dest++ = *src++;
        count++;
    }
    *dest = 0;
    r->response.responseLength += strlen(buf);
    if (r->response.headersSent == 0 ) httpdSendHeaders(r);

    _httpd_net_write(r->clientSock, buf, strlen(buf));
}
void Httpd::httpdSendHeaders(request *r) {
    _httpd_sendHeaders(r, r->response.responseLength, 0);
}
void Httpd::_httpd_formatTimeString(char *ptr, int clock) {
    struct tm *timePtr;
    time_t t;

    t = (clock == 0) ? time(NULL) : clock;
    timePtr = gmtime(&t);
    strftime(ptr, HTTP_TIME_STRING_LEN, "%a, %d %b %Y %T GMT", timePtr);
}

void Httpd::_httpd_sendHeaders(request *r, int contentLength, int modTime) {
    char tmpBuf[80], timeBuf[HTTP_TIME_STRING_LEN];

    if (r->response.headersSent) return;

    r->response.headersSent = 1;
    _httpd_net_write(r->clientSock, "HTTP/1.0 ", 9);
    _httpd_net_write(r->clientSock, r->response.response, strlen(r->response.response));
    _httpd_net_write(r->clientSock, r->response.headers, strlen(r->response.headers));

    _httpd_formatTimeString(timeBuf, 0);
    _httpd_net_write(r->clientSock, "Date: ", 6);
    _httpd_net_write(r->clientSock, timeBuf, strlen(timeBuf));
    _httpd_net_write(r->clientSock, "\n", 1);

    _httpd_net_write(r->clientSock, "Connection: close\n", 18);
    _httpd_net_write(r->clientSock, "Content-Type: ", 14);
    _httpd_net_write(r->clientSock, r->response.contentType, strlen(r->response.contentType));
    _httpd_net_write(r->clientSock, "\n", 1);

    if (contentLength > 0) {
        _httpd_net_write(r->clientSock, "Content-Length: ", 16);
        snprintf(tmpBuf, sizeof(tmpBuf), "%d", contentLength);
        _httpd_net_write(r->clientSock, tmpBuf, strlen(tmpBuf));
        _httpd_net_write(r->clientSock, "\n", 1);

        _httpd_formatTimeString(timeBuf, modTime);
        _httpd_net_write(r->clientSock, "Last-Modified: ", 15);
        _httpd_net_write(r->clientSock, timeBuf, strlen(timeBuf));
        _httpd_net_write(r->clientSock, "\n", 1);
    }
    _httpd_net_write(r->clientSock, "\n", 1);
}

int Httpd::_httpd_net_write(int sock, const char *buf, int len) {
    debug(LOG_INFO,"%d:%s",len,buf);

    return (write(sock, buf, len));
}

httpVar* Httpd::httpdGetVariableByName(request *r, const char *name) {
    httpVar *curVar;

    curVar = r->variables;
    while (curVar) {
        if (strcmp(curVar->name, name) == 0) return (curVar);
        curVar = curVar->nextVariable;
    }
    return (NULL);
}


int Httpd::httpdAddVariable(request *r, const char *name, const char *value) {
    httpVar * curVar,*lastVar,*newVar;

    while (*name == ' ' || *name == '\t') name++;
    newVar = (httpVar *)malloc(sizeof(httpVar));
    bzero(newVar, sizeof(httpVar));
    newVar->name = strdup(name);
    if (value == NULL) 
        newVar->value = NULL;
    else
        newVar->value = strdup(value);
    lastVar = NULL;
    curVar = r->variables;
    debug(LOG_INFO,"run httpdAddVariable");
    while (curVar) {
        if (strcmp(curVar->name, name) != 0) {
            lastVar = curVar;
            curVar = curVar->nextVariable;
            continue;
        }
        while (curVar) {
            lastVar = curVar;
            curVar = curVar->nextValue;
        }
        lastVar->nextValue = newVar;
        return (0);
    }
    //debug(LOG_DEBUG,"r->response.headersSent:%d",r->response.headersSent);
    if (lastVar) lastVar->nextVariable = newVar;
    else r->variables = newVar;
    return (0);
}


/**************************************************************************
** PRIVATE ROUTINES
**************************************************************************/
int Httpd:: scanCidr(char *val, u_int *result,u_int *length)
{
    u_int res, res1, res2, res3, res4, res5;
    char *cp;

    cp = val;
    res1 = atoi(cp);
    cp = strchr(cp, '.');
    if (!cp)
        return (-1);
    cp++;
    res2 = atoi(cp);
    cp = strchr(cp, '.');
    if (!cp)
        return (-1);
    cp++;
    res3 = atoi(cp);
    cp = strchr(cp, '.');
    if (!cp)
        return (-1);
    cp++;
    res4 = atoi(cp);
    cp = strchr(cp, '/');
    if (!cp) {
        res5 = 32;
    } else {
        cp++;
        res5 = atoi(cp);
    }

    if (res1 > 255 || res2 > 255 || res3 > 255 || res4 > 255 || res5 > 32) {
        return (-1);
    }
    res = (res1 << 24) + (res2 << 16) + (res3 << 8) + res4;
    *result = res;
    *length = res5;
    return (0);
}

int Httpd::_isInCidrBlock(httpd * server, request * r, int addr1, int len1, int addr2, int len2)
{
    int count, mask;

    /* if (addr1 == 0 && len1 == 0)
       {
       return(1);
       } */

    if (len2 < len1) {
        _httpd_writeErrorLog(server, r, LEVEL_ERROR, "IP Address must be more specific than network block");
        return (0);
    }

    mask = count = 0;
    while (count < len1) {
        mask = (mask << 1) + 1;
        count++;
    }
    mask = mask << (32 - len1);
    if ((addr1 & mask) == (addr2 & mask)) {
        return (1);
    } else {
        return (0);
    }
}

/**************************************************************************
** PUBLIC ROUTINES
**************************************************************************/

httpAcl * Httpd::httpdAddAcl(httpd *server, httpAcl *acl, char *cidr, int action)
{
    httpAcl *cur;
    unsigned int addr;
    unsigned int len;

    /*
     ** Check the ACL info is reasonable
     */
    if (scanCidr(cidr, &addr, &len) < 0) {
        _httpd_writeErrorLog(server, NULL, LEVEL_ERROR, "Invalid IP address format");
        return (NULL);
    }
    if (action != HTTP_ACL_PERMIT && action != HTTP_ACL_DENY) {
        _httpd_writeErrorLog(server, NULL, LEVEL_ERROR, "Invalid acl action");
        return (NULL);
    }

    /*
     ** Find a spot to put this ACE
     */
    if (acl) {
        cur = acl;
        while (cur->next) {
            cur = cur->next;
        }
        cur->next = (httpAcl *) malloc(sizeof(httpAcl));
        cur = cur->next;
    } else {
        cur = (httpAcl *) malloc(sizeof(httpAcl));
        acl = cur;
    }

    /*
     ** Add the details and return
     */
    cur->addr = addr;
    cur->len = len;
    cur->action = action;
    cur->next = NULL;
    return (acl);
}

int Httpd::httpdCheckAcl(httpd * server, request * r, httpAcl * acl)
{
    httpAcl *cur;
    unsigned int addr;
    unsigned int len;
    int res, action;

    action = HTTP_ACL_DENY;
    res = scanCidr(r->clientAddr, &addr, &len); /* Returns -1 on conversion failure. */
    if (res == 0) {
        cur = acl;
        while (cur) {
            res = _isInCidrBlock(server, r, cur->addr, cur->len, addr, len);
            if (res == 1) {
                action = cur->action;
                break;
            }
            cur = cur->next;
        }
    }
    if (action == HTTP_ACL_DENY) {
        _httpd_send403(server, r);
        _httpd_writeErrorLog(server, r, LEVEL_ERROR, "Access denied by ACL");
    }
    return (action);
}

void Httpd::httpdSetDefaultAcl(httpd *server, httpAcl *acl)
{
    server->defaultAcl = acl;
}
void Httpd::httpdEndRequest(request *r) {
    _httpd_freeVariables(r->variables);
    shutdown(r->clientSock, 2);
    close(r->clientSock);
    free(r);
}

void Httpd::httpdFreeVariables(request *r) {
    _httpd_freeVariables(r->variables);
}

void Httpd::_httpd_freeVariables(httpVar *var) {
    httpVar * curVar,*lastVar;

    if (var == NULL) return;
    _httpd_freeVariables(var->nextVariable);
    var->nextVariable = NULL;
    curVar = var;
    while (curVar) {
        lastVar = curVar;
        curVar = curVar->nextValue;
        free(lastVar->name);
        free(lastVar->value);
        free(lastVar);
    }
    return;
}

int Httpd::httpdReadRequest(httpd *server, request *r) {
    char buf[HTTP_MAX_LEN];
    int count, inHeaders;
    char *cp, *cp2;
    //int _httpd_decode();

    /*
     ** Setup for a standard response
     */
    strcpy(r->response.headers, "Server: Seeed Technologies Embedded Server\n");
    strcpy(r->response.contentType, "text/html");
    strcpy(r->response.response, "200 Output Follows\n");
    r->response.headersSent = 0;

    /*
     ** Read the request
     */
    count = 0;
    inHeaders = 1;
    while (_httpd_readLine(r, buf, HTTP_MAX_LEN) > 0) {
        count++;

        /*
         ** Special case for the first line.  Scan the request
         ** method and path etc
         */
        if (count == 1) {
            /*
             ** First line.  Scan the request info
             */
            cp = cp2 = buf;
            while (isalpha((unsigned char)*cp2)) cp2++;
            *cp2 = 0;
            if (strcasecmp(cp, "GET") == 0) r->request.method = HTTP_GET;
            if (strcasecmp(cp, "POST") == 0) r->request.method = HTTP_POST;
            if (r->request.method == 0) {
                _httpd_net_write(r->clientSock, HTTP_METHOD_ERROR, strlen(HTTP_METHOD_ERROR));
                _httpd_net_write(r->clientSock, cp, strlen(cp));
                _httpd_writeErrorLog(server, r, LEVEL_ERROR, "Invalid method received");
                return (-1);
            }
            cp = cp2 + 1;
            while (*cp == ' ') cp++;
            cp2 = cp;
            while (*cp2 != ' ' && *cp2 != 0) cp2++;
            *cp2 = 0;
            strncpy(r->request.path, cp, HTTP_MAX_URL);
            r->request.path[HTTP_MAX_URL - 1] = 0;
            _httpd_sanitiseUrl(r->request.path);
            continue;
        }

        /*
         ** Process the headers
         */
        if (inHeaders) {
            if (*buf == 0) {
                /*
                 ** End of headers.  Continue if there's
                 ** data to read
                 */
                break;
            }

            if (strncasecmp(buf, "Authorization: ", 15) == 0) {
                cp = strchr(buf, ':');
                if (cp) {
                    cp += 2;

                    if (strncmp(cp, "Basic ", 6) != 0) {
                        /* Unknown auth method */
                    } else {
                        char authBuf[100];

                        cp = strchr(cp, ' ') + 1;
                        _httpd_decode(cp, authBuf, 100);
                        r->request.authLength = strlen(authBuf);
                        cp = strchr(authBuf, ':');
                        if (cp) {
                            *cp = 0;
                            strncpy(r->request.authPassword, cp + 1, HTTP_MAX_AUTH);
                            r->request.authPassword[HTTP_MAX_AUTH - 1] = 0;
                        }
                        strncpy(r->request.authUser, authBuf, HTTP_MAX_AUTH);
                        r->request.authUser[HTTP_MAX_AUTH - 1] = 0;
                    }
                }
            }
            /* abc@abc.xyz/fasterconfg: Added decoding of host: if
             * present. */
            if (strncasecmp(buf, "Host: ", 6) == 0) {
                cp = strchr(buf, ':');
                if (cp) {
                    cp += 2;
                    strncpy(r->request.host, cp, HTTP_MAX_URL);
                    r->request.host[HTTP_MAX_URL - 1] = 0;
                }
            }
            /* End modification */
            continue;
        }
    }

    /*
     ** Process any URL data
     */
    cp = strchr(r->request.path, '?');
    if (cp != NULL) {
        *cp++ = 0;
        strncpy(r->request.query, cp, sizeof(r->request.query));
        r->request.query[sizeof(r->request.query) - 1] = 0;
        _httpd_storeData(r, cp);
    }

    return (0);
}
int Httpd::_httpd_readLine(request *r, char *destBuf, int len) {
    char curChar, *dst;
    int count;

    count = 0;
    dst = destBuf;
    while (count < len) {
        if (_httpd_readChar(r, &curChar) < 1) return (0);
        // Fixed by Mina - if we read binary junk it's probably not a regular HTTP client
        //if (curChar == '\n')
        if (curChar == '\n' || !isascii(curChar)) {
            *dst = 0;
            return (1);
        }
        if (curChar == '\r') {
            continue;
        } else {
            *dst++ = curChar;
            count++;
        }
    }
    *dst = 0;
    return (1);
}
int Httpd::_httpd_readChar(request *r, char *cp) {
    if (r->readBufRemain == 0) {
        bzero(r->readBuf, HTTP_READ_BUF_LEN + 1);
        r->readBufRemain = _httpd_net_read(r->clientSock, r->readBuf, HTTP_READ_BUF_LEN);
        if (r->readBufRemain < 1) return (0);
        r->readBuf[r->readBufRemain] = 0;
        r->readBufPtr = r->readBuf;
    }
    *cp = *r->readBufPtr++;
    r->readBufRemain--;
    return (1);
}


void Httpd::_httpd_writeErrorLog(httpd *server, request *r, char *level, const char *message) {
    char dateBuf[30];
    struct tm *timePtr;
    time_t clock;

    if (server->errorLog == NULL) return;
    clock = time(NULL);
    timePtr = localtime(&clock);
    strftime(dateBuf, 30, "%a %b %d %T %Y", timePtr);
    if (r != NULL && *r->clientAddr != 0) {
        fprintf(server->errorLog, "[%s] [%s] [client %s] %s\n", dateBuf, level, r->clientAddr, message);
    } else {
        fprintf(server->errorLog, "[%s] [%s] %s\n", dateBuf, level, message);
    }
}

void Httpd::_httpd_sanitiseUrl(char *url) {
    char *from, *to, *last;

    /*
     ** Remove multiple slashes
     */
    from = to = url;
    while (*from) {
        if (*from == '/' && *(from + 1) == '/') {
            from++;
            continue;
        }
        *to = *from;
        to++;
        from++;
    }
    *to = 0;

    /*
     ** Get rid of ./ sequences
     */
    from = to = url;
    while (*from) {
        if (*from == '/' && *(from + 1) == '.' && *(from + 2) == '/') {
            from += 2;
            continue;
        }
        *to = *from;
        to++;
        from++;
    }
    *to = 0;

    /*
     ** Catch use of /../ sequences and remove them.  Must track the
     ** path structure and remove the previous path element.
     */
    from = to = last = url;
    while (*from) {
        if (*from == '/' && *(from + 1) == '.' && *(from + 2) == '.' && *(from + 3) == '/') {
            to = last;
            from += 3;
            continue;
        }
        if (*from == '/') {
            last = to;
        }
        *to = *from;
        to++;
        from++;
    }
    *to = 0;
}

void Httpd::_httpd_storeData(request *r, char *query) {
    char *cp, *cp2, *var, *val, *tmpVal;

    if (!query) return;

    var = (char *)malloc(strlen(query) + 1);

    cp = query;
    cp2 = var;
    bzero(var, strlen(query) + 1);
    val = NULL;
    while (*cp) {
        if (*cp == '=') {
            cp++;
            *cp2 = 0;
            val = cp;
            continue;
        }
        if (*cp == '&') {
            *cp = 0;
            tmpVal = _httpd_unescape(val);
            httpdAddVariable(r, var, tmpVal);
            cp++;
            cp2 = var;
            val = NULL;
            continue;
        }
        if (val) {
            cp++;
        } else {
            *cp2 = *cp++;
            /*
               if (*cp2 == '.')
               {
               strcpy(cp2,"_dot_");
               cp2 += 5;
               }
               else
               {
             */
            cp2++;
            /*
               }
             */
        }
    }
    if (val != NULL) {
        *cp = 0;
        tmpVal = _httpd_unescape(val);
        httpdAddVariable(r, var, tmpVal);
    }
    free(var);
}
char* Httpd::_httpd_unescape(char *str) {
    char *p = str;
    char *q = str;

    if (!str) return const_cast<char*>("");
    while (*p) {
        if (*p == '%') {
            p++;
            if (*p) *q = _httpd_from_hex(*p++) * 16;
            if (*p) *q = (*q + _httpd_from_hex(*p++));
            q++;
        } else {
            if (*p == '+') {
                *q++ = ' ';
                p++;
            } else {
                *q++ = *p++;
            }
        }
    }

    *q++ = 0;
    return str;
}
char Httpd::_httpd_from_hex(char c) {
    return c >= '0' && c <= '9' ? c - '0' : c >= 'A' && c <= 'F' ? c - 'A' + 10 : c - 'a' + 10; /* accept small letters just in case */
}
int Httpd::_httpd_net_read(int sock, char *buf, int len) {
#if defined(_WIN32)
    return (recv(sock, buf, len, 0));
#else
    /*return( read(sock, buf, len)); */
    /* XXX Select based IO */

    int nfds;
    fd_set readfds;
    struct timeval timeout;

    FD_ZERO(&readfds);
    FD_SET(sock, &readfds);
    timeout.tv_sec = 10;
    timeout.tv_usec = 0;
    nfds = sock + 1;

    nfds = select(nfds, &readfds, NULL, NULL, &timeout);

    if (nfds > 0) {
        return (read(sock, buf, len));
    }
    return (nfds);
#endif
}

int Httpd::_httpd_decode(char *bufcoded, char *bufplain, int outbufsize)
{
    static char six2pr[64] = {
        'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M',
        'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
        'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm',
        'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',
        '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '+', '/'
    };

    static unsigned char pr2six[256];

    /* single character decode */
#	define DEC(c) pr2six[(int)c]
#	define _DECODE_MAXVAL 63

    static int first = 1;

    int nbytesdecoded, j;
    register char *bufin = bufcoded;
    register char *bufout = bufplain;
    register int nprbytes;

    /*
     ** If this is the first call, initialize the mapping table.
     ** This code should work even on non-ASCII machines.
     */
    if (first) {
        first = 0;
        for (j = 0; j < 256; j++)
            pr2six[j] = _DECODE_MAXVAL + 1;
        for (j = 0; j < 64; j++)
            pr2six[(int)six2pr[j]] = (unsigned char)j;
    }

    /* Strip leading whitespace. */

    while (*bufcoded == ' ' || *bufcoded == '\t')
        bufcoded++;

    /*
     ** Figure out how many characters are in the input buffer.
     ** If this would decode into more bytes than would fit into
     ** the output buffer, adjust the number of input bytes downwards.
     */
    bufin = bufcoded;
    while (pr2six[(int)*(bufin++)] <= _DECODE_MAXVAL) ;
    nprbytes = bufin - bufcoded - 1;
    nbytesdecoded = ((nprbytes + 3) / 4) * 3;
    if (nbytesdecoded > outbufsize) {
        nprbytes = (outbufsize * 4) / 3;
    }
    bufin = bufcoded;

    while (nprbytes > 0) {
        *(bufout++) = (DEC(*bufin) << 2 | DEC(bufin[1]) >> 4);
        *(bufout++) = (DEC(bufin[1]) << 4 | DEC(bufin[2]) >> 2);
        *(bufout++) = (DEC(bufin[2]) << 6 | DEC(bufin[3]));
        bufin += 4;
        nprbytes -= 4;
    }
    if (nprbytes & 03) {
        if (pr2six[(int)bufin[-2]] > _DECODE_MAXVAL) {
            nbytesdecoded -= 2;
        } else {
            nbytesdecoded -= 1;
        }
    }
    bufplain[nbytesdecoded] = 0;
    return (nbytesdecoded);
}
void Httpd::_httpd_send404(httpd *server, request *r) {
    char msg[HTTP_MAX_URL];

    snprintf(msg, HTTP_MAX_URL, "File does not exist: %s\n", r->request.path);
    _httpd_writeErrorLog(server, r, LEVEL_ERROR, msg);

    if (server->errorFunction404) {
        debug(LOG_INFO,"start 404 function");
        /*
         * There's a custom C 404 handler defined with httpdAddC404Content
         */
        //reinterpret_cast<callBackTypeC>(server->errorFunction404)(server, r, 404);
        (this->*server->errorFunction404)(server, r, 404);
    } else {
        /*
         * Send stock 404
         */
        httpdSetResponse(r, "404 Not Found\n");
        _httpd_sendHeaders(r, 0, 0);
        _httpd_sendText(r, "<HTML><HEAD><TITLE>404 Not Found</TITLE></HEAD>\n");
        _httpd_sendText(r, "<BODY><H1>The request URL was not found!</H1>\n");
        _httpd_sendText(r, "</BODY></HTML>\n");
    }
}
void Httpd::httpdSetResponse(request *r, const char *msg) {
    strncpy(r->response.response, msg, HTTP_MAX_URL - 1);
    r->response.response[HTTP_MAX_URL - 1] = 0;
}
void Httpd::_httpd_sendText(request *r, const char *msg) {
    r->response.responseLength += strlen(msg);
    _httpd_net_write(r->clientSock, msg, strlen(msg));
}
void Httpd::_httpd_writeAccessLog(httpd *server, request *r) {
    char dateBuf[30];
    struct tm *timePtr;
    time_t clock;
    int responseCode;

    if (server->accessLog == NULL) return;
    clock = time(NULL);
    timePtr = localtime(&clock);
    strftime(dateBuf, 30, "%d/%b/%Y:%T %Z", timePtr);
    responseCode = atoi(r->response.response);
    fprintf(server->accessLog, "%s - - [%s] %s \"%s\" %d %d\n",
            r->clientAddr, dateBuf, httpdRequestMethodName(r),
            httpdRequestPath(r), responseCode, r->response.responseLength);
}

httpContent* Httpd::_httpd_findContentEntry(request *r, httpDir *dir, char *entryName) {

    httpContent *curEntry = new httpContent;
    curEntry = dir->entries;
#if 0
    while (curEntry) {
        if (curEntry->type == HTTP_WILDCARD || curEntry->type == HTTP_C_WILDCARD) break;
        if (*entryName == 0 && curEntry->indexFlag) break;
        if (strcmp(curEntry->name, entryName) == 0) break;
        curEntry = curEntry->next;
    }
#endif
    if (curEntry) r->response.content = curEntry;
    return (curEntry);
}


void Httpd::_httpd_sendStatic(httpd *server, request *r, char *data) {
    if (_httpd_checkLastModified(r, server->startTime) == 0) {
        _httpd_send304(server, r);
    }
    _httpd_sendHeaders(r, server->startTime, strlen(data));
    httpdOutput(r, data);
}
int Httpd::_httpd_checkLastModified(request *r, int modTime) {
    char timeBuf[HTTP_TIME_STRING_LEN];

    _httpd_formatTimeString(timeBuf, modTime);
    if (strcmp(timeBuf, r->request.ifModified) == 0) return (0);
    return (1);
}

void Httpd::_httpd_sendFile(httpd *server, request *r, const char *path) {
    char *suffix;
    struct stat sbuf;

    suffix = strrchr(const_cast<char*>(path), '.');
    if (suffix != NULL) {
        if (strcasecmp(suffix, ".gif") == 0) strcpy(r->response.contentType, "image/gif");
        if (strcasecmp(suffix, ".jpg") == 0) strcpy(r->response.contentType, "image/jpeg");
        if (strcasecmp(suffix, ".xbm") == 0) strcpy(r->response.contentType, "image/xbm");
        if (strcasecmp(suffix, ".png") == 0) strcpy(r->response.contentType, "image/png");
        if (strcasecmp(suffix, ".css") == 0) strcpy(r->response.contentType, "text/css");
    }
    if (stat(path, &sbuf) < 0) {
        _httpd_send404(server, r);
        return;
    }
    if (_httpd_checkLastModified(r, sbuf.st_mtime) == 0) {
        _httpd_send304(server, r);
    } else {
        _httpd_sendHeaders(r, sbuf.st_size, sbuf.st_mtime);

        _httpd_catFile(r, path);
    }
}
void Httpd::_httpd_catFile(request *r, const char *path) {
    int fd, len;
    char buf[HTTP_MAX_LEN];

    fd = open(path, O_RDONLY);
    if (fd < 0) return;
    len = read(fd, buf, HTTP_MAX_LEN);
    while (len > 0) {
        r->response.responseLength += len;
        _httpd_net_write(r->clientSock, buf, len);
        len = read(fd, buf, HTTP_MAX_LEN);
    }
    close(fd);
}

int Httpd::_httpd_sendDirectoryEntry(httpd *server, request *r, httpContent *entry, char *entryName) {
    char path[HTTP_MAX_URL];

    snprintf(path, HTTP_MAX_URL, "%s/%s", entry->path, entryName);
    _httpd_sendFile(server, r, path);
    return (0);
}

const char* Httpd::httpdRequestMethodName(request *r) {
    switch (r->request.method) {
    case HTTP_GET:
        return ("GET");
    case HTTP_POST:
        return ("POST");
    default:
        return ("INVALID");
    }
}
void Httpd::_httpd_send304(httpd *server, request *r) {
    if (server->errorFunction304) {
        (this->*server->errorFunction304)(server, r,304);
    } else {
        httpdSetResponse(r, "304 Not Modified\n");
        _httpd_sendHeaders(r, 0, 0);
    }
}

void Httpd::_httpd_send403(httpd *server, request *r) {
    if (server->errorFunction403) {
        (this->*server->errorFunction403)(server, r,403);
    } else {
        httpdSetResponse(r, "403 Permission Denied\n");
        _httpd_sendHeaders(r, 0, 0);
        _httpd_sendText(r, "<HTML><HEAD><TITLE>403 Permission Denied</TITLE></HEAD>\n");
        _httpd_sendText(r, "<BODY><H1>Access to the request URL was denied!</H1>\n");

    }
}

/** The 404 handler is also responsible for redirecting to the auth server */
void Httpd::http_callback_404(httpd *webserver, request *r, int error_code) {
    char tmp_url[MAX_BUF], *url, *mac;
    //t_auth_serv *auth_server = get_auth_server();

    memset(tmp_url, 0, sizeof(tmp_url));
    /* 
     * XXX Note the code below assumes that the client's request is a plain
     * http request to a standard port. At any rate, this handler is called only
     * if the internet/auth server is down so it's not a huge loss, but still.
     */
    //snprintf(tmp_url, (sizeof(tmp_url) - 1), "http://%s%s%s%s",
    //         r->request.host, r->request.path, r->request.query[0] ? "?" : "", r->request.query);
    snprintf(tmp_url,(sizeof(tmp_url) - 1),"http://www.baidu.com"); 
    debug(LOG_INFO,"%s",tmp_url); 
    url = httpdUrlEncode(tmp_url);
    debug(LOG_INFO,"%s",url); 
            /* Re-direct them to auth server */
            char urlFragment[MAX_BUF];
            //snprintf(urlFragment,(sizeof(urlFragment) - 1), "config/gw_address=%s&gw_port=%d&gw_id=%s&ip=%s&mac=%s&url=%s",
            //                  config.gw_address, config.gw_port, config.gw_id, r->clientAddr, mac, url);
            snprintf(urlFragment,(sizeof(urlFragment) - 1), "config");
            debug(LOG_INFO,"%s",urlFragment);
            http_send_redirect(r, tmp_url, "allow domain");
            return ; 
#if 0
            if (!(mac = arp_get(r->clientAddr))) {
                /* We could not get their MAC address */
                debug(LOG_INFO, "Failed to retrieve MAC address for ip %s, so not putting in the login request",
                      r->clientAddr);
                asprintf(&urlFragment, "config/gw_address=%s&gw_port=%d&gw_id=%s&ip=%s&url=%s",
                              config.gw_address, config.gw_port, config.gw_id, r->clientAddr, url);

                //vasprintf(&urlFragment, "config");
            } else {
                debug(LOG_INFO, "Got client MAC address for ip %s: %s", r->clientAddr, mac);
                asprintf(&urlFragment, "config/gw_address=%s&gw_port=%d&gw_id=%s&ip=%s&mac=%s&url=%s",
                              config.gw_address, config.gw_port, config.gw_id, r->clientAddr, mac, url);
                free(mac);
            }
#endif
            // if host is not in whitelist, maybe not in conf or domain'IP changed, it will go to here.
            debug(LOG_INFO, "Check host %s is in whitelist or not", r->request.host);       // e.g. www.example.com
            t_firewall_rule *rule;
            //e.g. example.com is in whitelist
            // if request http://www.example.com/, it's not equal example.com.

            for (rule = (t_firewall_rule *)malloc(sizeof(t_firewall_rule)); rule != NULL; rule = rule->next) {
                debug(LOG_INFO, "rule mask %s,request.host:%s", rule->mask,r->request.host);
                //if (strstr(r->request.host, rule->mask) == NULL) {
                //    debug(LOG_INFO, "host %s is not in %s, continue", r->request.host, rule->mask);
                //    continue;
                //}
                int host_length = strlen(r->request.host);
                int mask_length = strlen(rule->mask);
                if (host_length != mask_length) {
                    char prefix[1024] = { 0 };
                    // must be *.example.com, if not have ".", maybe Phishing. e.g. phishingexample.com
                    strncpy(prefix, r->request.host, host_length - mask_length - 1);        // e.g. www
                    strcat(prefix, ".");    // www.
                    strcat(prefix, rule->mask);     // www.example.com
                    if (strcasecmp(r->request.host, prefix) == 0) {
                        debug(LOG_INFO, "allow subdomain");
                        //fw_allow_host(r->request.host);
                        http_send_redirect(r, tmp_url, "allow subdomain");
                        free(url);
                        return;
                    }
                } else {
                    // e.g. "example.com" is in conf, so it had been parse to IP and added into "iptables allow" when wifidog start. but then its' A record(IP) changed, it will go to here.
                    debug(LOG_INFO, "allow domain again, because IP changed");
                    //fw_allow_host(r->request.host);
                    http_send_redirect(r, tmp_url, "allow domain");
                    free(url);
                    return;
                }
            }

            debug(LOG_INFO, "Captured %s requesting [%s] and re-directing them to login page", r->clientAddr, url);
            //http_send_redirect_to_auth(r, urlFragment, "Redirect to login page");

    free(url);
}

void Httpd::httpdAddHeader(request * r, const char *msg)
{
    int size;
    size = HTTP_MAX_HEADERS - 2 - strlen(r->response.headers);
    if (size > 0) {
        strncat(r->response.headers, msg, size);
        if (r->response.headers[strlen(r->response.headers) - 1] != '\n')
            strcat(r->response.headers, "\n");
    }
}



/** @brief Sends a redirect to the web browser 
 * @param r The request
 * @param url The url to redirect to
 * @param text The text to include in the redirect header and the manual redirect link title.  NULL is acceptable */
void Httpd::http_send_redirect(request * r, const char *url, const char *text)
{
    char *message = NULL;
    char *header = NULL;
    char *response = NULL;
    /* Re-direct them to auth server */
    debug(LOG_DEBUG, "Redirecting client browser to %s", url);
    asprintf(&header, "Location: %s", url);
    asprintf(&response, "302 %s\n", text ? text : "Redirecting");
    httpdSetResponse(r, response);
    httpdAddHeader(r, header);
    free(response);
    free(header);
    asprintf(&message, "Please <a href='%s'>click here</a>.", url);
    send_http_page(r, text ? text : "Redirection to message", message);
    free(message);
}

char* Httpd::httpdUrlEncode(const char *str) {
    char *new_str, *cp;

    new_str = (char *)_httpd_escape(str);
    if (new_str == NULL) {
        return (NULL);
    }
    cp = new_str;
    while (*cp) {
        if (*cp == ' ') *cp = '+';
        cp++;
    }
    return (new_str);
}
char * Httpd::_httpd_escape(const char *str){
    unsigned char mask = URL_XPALPHAS;
    const char *p;
    char *q;
    char *result;
    int unacceptable = 0;
    for (p = str; *p; p++)
        if (!ACCEPTABLE((unsigned char)*p))
            unacceptable += 2;
    result = (char *)malloc(p - str + unacceptable + 1);
    bzero(result, (p - str + unacceptable + 1));

    if (result == NULL) {
        return (NULL);
    }
    for (q = result, p = str; *p; p++) {
        unsigned char a = *p;
        if (!ACCEPTABLE(a)) {
            *q++ = '%';         /* Means hex commming */
            *q++ = hex[a >> 4];
            *q++ = hex[a & 15];
        } else
            *q++ = *p;
    }
    *q++ = 0;                   /* Terminate */
    return result;
}

