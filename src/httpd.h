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


#ifndef _DNS_HTTPD_H
#define	_DNS_HTTPD_H

#include <string.h>
#include <string>
#include <stdlib.h>
#include <iostream>
#include <stdio.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <syslog.h>

#include <errno.h>
#include <stdarg.h>
#include <unistd.h>
#include <signal.h>


#include  "logger.h"


#define	HTTP_PORT 		80
#define HTTP_MAX_LEN		10240
#define HTTP_MAX_URL		1024
#define HTTP_MAX_HEADERS	1024
#define HTTP_MAX_AUTH		128
#define	HTTP_IP_ADDR_LEN	17
#define	HTTP_TIME_STRING_LEN	40
#define	HTTP_READ_BUF_LEN	4096
#define	HTTP_ANY_ADDR		NULL

#define	HTTP_GET		1
#define	HTTP_POST		2

#define	HTTP_TRUE		1
#define HTTP_FALSE		0

#define	HTTP_FILE		1
#define HTTP_C_FUNCT		2
#define HTTP_EMBER_FUNCT	3
#define HTTP_STATIC		4
#define HTTP_WILDCARD		5
#define HTTP_C_WILDCARD		6

#define HTTP_METHOD_ERROR "\n<B>ERROR : Method Not Implemented</B>\n\n"

#define httpdRequestMethod(s) 		s->request.method
#define httpdRequestPath(s)		s->request.path
#define httpdRequestContentType(s)	s->request.contentType
#define httpdRequestContentLength(s)	s->request.contentLength

#define HTTP_ACL_PERMIT		1
#define HTTP_ACL_DENY		2

#define HAVE_STDARG_H       1

/* Overencodes */
#define URL_XALPHAS     (unsigned char) 1
#define URL_XPALPHAS    (unsigned char) 2
#define ACCEPTABLE(a)   ( a!='&' && a>=32 && a<128 && ((isAcceptable[a-32]) & mask))

namespace dns { 

typedef struct {
    int method, contentLength, authLength;
    char path[HTTP_MAX_URL], query[HTTP_MAX_URL], host[HTTP_MAX_URL],       /* acv@acv.ca/wifidog: Added decoding
                                                                               of host: header if present. */
     ifModified[HTTP_MAX_URL];
    char authUser[HTTP_MAX_AUTH];
    char authPassword[HTTP_MAX_AUTH];
} httpReq;

typedef struct _httpd_var {
    char *name, *value;
    struct _httpd_var *nextValue, *nextVariable;
} httpVar;

typedef struct _httpd_content {
    char *name;
    int type, indexFlag;
    void (*function) ();
    char *data, *path;
    int (*preload) ();
    struct _httpd_content *next;
} httpContent;

typedef struct {
    int responseLength;
    httpContent *content;
    char headersSent, headers[HTTP_MAX_HEADERS], response[HTTP_MAX_URL], contentType[HTTP_MAX_URL];
} httpRes;

typedef struct _httpd_dir {
    char *name;
    struct _httpd_dir *children, *next;
    struct _httpd_content *entries;
} httpDir;

typedef struct ip_acl_s {
    int addr;
    char len, action;
    struct ip_acl_s *next;
} httpAcl;

typedef struct {
    int port, serverSock, startTime, lastError;
    char fileBasePath[HTTP_MAX_URL], *host;
    httpDir *content;
    httpAcl *defaultAcl;
    FILE *accessLog, *errorLog;
    void (*errorFunction304) (), (*errorFunction403) (), (*errorFunction404) ();
} httpd;

typedef struct {
    int clientSock, readBufRemain;
    httpReq request;
    httpRes response;
    httpVar *variables;
    char readBuf[HTTP_READ_BUF_LEN + 1], *readBufPtr, clientAddr[HTTP_IP_ADDR_LEN];
} request;


class Httpd {
public:
    /**
     *  Destructor
     */
    virtual ~Httpd();
    Httpd(const std::string &ip_address, int port);
    int start();
    int httpdAddCContent (httpd *, char *, char *, int, int (*)(), 
                          void (Httpd::*func)(httpd *, request *));
    int httpdAddFileContent (httpd *, char *, char *, int, int (*)(), char *);
    int httpdAddStaticContent (httpd *, char *, char *, int, int (*)(), char *);
    int httpdAddWildcardContent (httpd *, char *, int (*)(), char *);
    int httpdAddCWildcardContent (httpd *, char *, int (*)(), void (*)());
    int httpdAddVariable (request *, const char *, const char *);
    int httpdSetVariableValue (request *, const char *, const char *);
    request *httpdGetConnection (httpd *, struct timeval *);
    int httpdReadRequest (httpd *, request *);
    int httpdCheckAcl (httpd *, request *, httpAcl *);
    int httpdAuthenticate (request *, const char *);
    void httpdForceAuthenticate (request *, const char *);
    int httpdSetErrorFunction (httpd *, int, void (Httpd::*func)(httpd *, request *, int));

    char *httpdRequestMethodName (request *);
    char *httpdUrlEncode (const char *);

    void httpdAddHeader (request *, const char *);
    void httpdSetContentType (request *, const char *);
    void httpdSetResponse (request *, const char *);
    void httpdEndRequest (request *);

    httpd *httpdCreate (char *, int );
    void httpdFreeVariables (request *);
    void httpdDumpVariables (request *);
    void httpdOutput (request *, const char *);
    void httpdPrintf (request *, const char *, ...);
    void httpdProcessRequest (httpd *, request *);
    void httpdSendHeaders (request *);
    void httpdSendFile (httpd *, request *, const char *);
    void httpdSetFileBase (httpd *, const char *);
    void httpdSetCookie (request *, const char *, const char *);

    void httpdSetErrorLog (httpd *, FILE *);
    void httpdSetAccessLog (httpd *, FILE *);
    void httpdSetDefaultAcl (httpd *, httpAcl *);

    httpVar *httpdGetVariableByName (request *, const char *);
    httpVar *httpdGetVariableByPrefix (request *, const char *);
    httpVar *httpdGetVariableByPrefixedName (request *, const char *, const char *);
    httpVar *httpdGetNextVariableByPrefix (httpVar *, const char *);

    httpAcl *httpdAddAcl (httpd *, httpAcl *, char *, int);


    
    /**@brief Callback for libhttpd, main entry point for captive portal */
    void http_callback_404(httpd *, request *, int);
    /**@brief Callback for libhttpd */
    void http_callback_wifidog(httpd *, request *);
    /**@brief Callback for libhttpd */
    void http_callback_about(httpd *, request *);
    /**@brief Callback for libhttpd */
    void http_callback_status(httpd *, request *);
    /**@brief Callback for libhttpd, main entry point post login for auth confirmation */
    void http_callback_auth(httpd *, request *);
    /**@brief Callback for libhttpd, disconnect user from network */
    void http_callback_disconnect(httpd *, request *);

    /** @brief Sends a HTML page to web browser */
    void send_http_page(request *, const char *, const char* );

    /** @brief Sends a redirect to the web browser */
    void http_send_redirect(request *, const char *, const char *);
    /** @brief Convenience function to redirect the web browser to the authe server */
    void http_send_redirect_to_auth(request *, const char *, const char *);

private:
    char *_httpd_unescape (char *);
    char *_httpd_escape (const char *);
    char _httpd_from_hex (char);

    void _httpd_catFile (request *, const char *);
    void _httpd_send403 (httpd *, request *);
    void _httpd_send404 (httpd *, request *);
    void _httpd_send304 (httpd *, request *);
    void _httpd_sendText (request *, char *);
    void _httpd_sendFile (httpd *, request *, char *);
    void _httpd_sendStatic (httpd *, request *, char *);
    void _httpd_sendHeaders (request *, int, int);
        
    void _httpd_sanitiseUrl (char *);
    void _httpd_freeVariables (httpVar *);
    void _httpd_formatTimeString (char *, int);
    void _httpd_storeData (request *, char *);
    void _httpd_writeAccessLog (httpd *, request *);
    void _httpd_writeErrorLog (httpd *, request *, char *, char *);

    int _httpd_net_read (int, char *, int);
    int _httpd_net_write (int, char *, int);
    int _httpd_readBuf (request *, char *, int);
    int _httpd_readChar (request *, char *);
    int _httpd_readLine (request *, char *, int);
    int _httpd_checkLastModified (request *, int);
    int _httpd_sendDirectoryEntry (httpd *, request * r, httpContent *, char *);

    httpContent *_httpd_findContentEntry (request *, httpDir *, char *);
    httpDir *_httpd_findContentDir (httpd *, char *, int);
private:
    /* The internal web server */
    httpd * webserver = NULL;
    Logger& logger = Logger::instance();
    char* gw_address;
    int gw_port;



    char *hex = "0123456789ABCDEF";

    char    *LIBHTTPD_VERSION =  "0.2-fasterconfig";
    char    *LIBHTTPD_VENDOR =   "Seeed Technology Inc";
    unsigned char isAcceptable[96] =
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

protected:
};
}
#endif	/* _DNS_HTTPD_H */
