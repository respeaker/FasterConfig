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

#define MAX_BUF 4096

#define LEVEL_ERROR	"error"


/* Overencodes */
#define URL_XALPHAS     (unsigned char) 1
#define URL_XPALPHAS    (unsigned char) 2
#define ACCEPTABLE(a)   ( a!='&' && a>=32 && a<128 && ((isAcceptable[a-32]) & mask))

namespace dns { 

class Httpd;
struct httpdType;
struct requestType;
typedef struct httpdType httpd;
typedef struct requestType request;
typedef void(Httpd::*callBackTypeA)(); // void (*)())
typedef void(Httpd::*callBackTypeB)(httpd *, request *); //void (*)(httpd *, request *))
typedef void(Httpd::*callBackTypeC)(httpd *, request *, int); //void (*)(httpd *, request *, int))


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
    //void (Httpd::*function) (httpd *, request *);
    callBackTypeB function;
    char *data, *path;
    int (*preload) ();
    struct _httpd_content *next;
} httpContent;

typedef struct {
    int responseLength;
    httpContent *content;
    char headersSent;
    char headers[HTTP_MAX_HEADERS];
    char response[HTTP_MAX_URL];
    char contentType[HTTP_MAX_URL];
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

struct httpdType{
    int port, serverSock, startTime, lastError;
    char fileBasePath[HTTP_MAX_URL], *host;
    httpDir *content;
    httpAcl *defaultAcl;
    FILE *accessLog, *errorLog;
    callBackTypeC errorFunction304;
    callBackTypeC errorFunction403; 
    callBackTypeC errorFunction404;
};

struct requestType{
    int clientSock, readBufRemain;
    httpReq request;
    httpRes response;
    httpVar *variables;
    char readBuf[HTTP_READ_BUF_LEN + 1], *readBufPtr, clientAddr[HTTP_IP_ADDR_LEN];
};

/**
 * Firewall targets
 */
typedef enum {
    TARGET_DROP,
    TARGET_REJECT,
    TARGET_ACCEPT,
    TARGET_LOG,
    TARGET_ULOG
} t_firewall_target;

/**
 * Firewall rules
 */
typedef struct _firewall_rule_t {
    t_firewall_target target;   /**< @brief t_firewall_target */
    char *protocol;             /**< @brief tcp, udp, etc ... */
    char *port;                 /**< @brief Port to block/allow */
    char *mask;                 /**< @brief Mask for the rule *destination* */
    int mask_is_ipset; /**< @brief *destination* is ipset  */
    struct _firewall_rule_t *next;
} t_firewall_rule;

/**
 * Firewall rulesets
 */
typedef struct _firewall_ruleset_t {
    char *name;
    t_firewall_rule *rules;
    struct _firewall_ruleset_t *next;
} t_firewall_ruleset;

/**
 * Trusted MAC Addresses
 */
typedef struct _trusted_mac_t {
    char *mac;
    struct _trusted_mac_t *next;
} t_trusted_mac;

/**
 * Popular Servers
 */
typedef struct _popular_server_t {
    char *hostname;
    struct _popular_server_t *next;
} t_popular_server;

/**
 * Configuration structure
 */
typedef struct {
    char *configfile;       /**< @brief name of the config file */
    char *htmlmsgfile;          /**< @brief name of the HTML file used for messages */

    char *gw_id;                /**< @brief ID of the Gateway, sent to central
				     server */
    char *gw_interface;         /**< @brief Interface we will accept connections on */
    char *gw_address;           /**< @brief Internal IP address for our web
				     server */
    int gw_port;                /**< @brief Port the webserver will run on */
} golbe_config;


class Httpd {
public:
    /**
     *  Destructor
     */
    virtual ~Httpd();
    Httpd(const std::string &ip_address, int port);
    int start();
    int httpdAddCContent (httpd *, const char *,const char *, int, int (*)(), 
                          void (Httpd::*)(httpd *, request *));

    httpd *httpdCreate (char *, int );
    request *httpdGetConnection (httpd *, struct timeval *);
    httpDir* _httpd_findContentDir(httpd *server, char *dir, int createFlag);
    void send_http_page(request *r, const char *title, const char *message);
    int httpdAddVariable(request *r, const char *name, const char *value);
    void httpdOutput(request *r, const char *msg);
    httpVar* httpdGetVariableByName(request *r, const char *name);
    int _httpd_net_write(int sock, const char *buf, int len);
    void httpdSendHeaders(request *r);
    void _httpd_sendHeaders(request *r, int contentLength, int modTime);
    void _httpd_formatTimeString(char *ptr, int clock);
    int httpdCheckAcl (httpd *, request *, httpAcl *);
    int httpdReadRequest(httpd *server, request *r);
    int _httpd_readLine(request *r, char *destBuf, int len);
    int _httpd_readChar(request *r, char *cp);
    void _httpd_writeErrorLog(httpd *server, request *r, char *level,  const char *message);
    void _httpd_sanitiseUrl(char *url);
    void _httpd_storeData(request *r, char *query);
    char* _httpd_unescape(char *str);
    char _httpd_from_hex(char c);
    int _httpd_net_read(int sock, char *buf, int len);
    int _httpd_decode(char *bufcoded, char *bufplain, int outbufsize);
    void httpdProcessRequest(httpd *server, request *r);
    void _httpd_send404(httpd *server, request *r);
    void _httpd_writeAccessLog(httpd *server, request *r);
    httpContent* _httpd_findContentEntry(request *r, httpDir *dir, char *entryName);
    void _httpd_sendStatic(httpd *server, request *r, char *data);
    int _httpd_checkLastModified(request *r, int modTime);
    void _httpd_sendFile(httpd *server, request *r, const char *path);
    void _httpd_catFile(request *r, const char *path);
    int _httpd_sendDirectoryEntry(httpd *server, request *r, httpContent *entry, char *entryName);

    const char* httpdRequestMethodName(request *r);
    void _httpd_send304(httpd *server, request *r);
    void _httpd_send403(httpd *server, request *r);

    void httpdSetResponse(request *r, const char *msg);
    void _httpd_sendText(request *r, const char *msg);

    int httpdSetErrorFunction(httpd *server, int error, void (Httpd::*function)(httpd *, request *, int));

    /**@brief Callback for libhttpd, main entry point for captive portal */
    void http_callback_404(httpd *, request *, int);
    char* httpdUrlEncode(const char *str);
    /**@brief Callback for libhttpd */
    void http_callback_fasterconfig(httpd *, request *);

    void http_send_redirect(request * r, const char *url, const char *text);
    void httpdAddHeader(request * r, const char *msg);
    char * _httpd_escape(const char *str);


    char* arp_get(const char *req_ip);

    int scanCidr (char *val, u_int *result, u_int *length);
    void httpdSetDefaultAcl(httpd *server, httpAcl *acl);
    httpAcl * httpdAddAcl(httpd *server, httpAcl *acl, char *cidr, int action);
    int _isInCidrBlock(httpd *server, request *r, int addr1, int len1, int addr2, int len2);

    void _httpd_freeVariables(httpVar *var);
    void httpdFreeVariables(request *r);
    void httpdEndRequest(request *r);

    void setHtmlPath(char *path);
    void setReUrl(char *reurl);
	void setgatewayIP(char* ip);
private:
    /* The internal web server */
    httpd * webserver = NULL;
    Logger *logger;

    char htmlPath[64];
    char htmlreurl[512];


    static void* thread_httpd(void *args);

    char hex[32] ;

    char    LIBHTTPD_VERSION[32] ;
    char    LIBHTTPD_VENDOR[32] ;
    unsigned char isAcceptable[96];

    static Httpd *HttpServerCallBack;

    golbe_config config;

protected:
};
}
#endif	/* _DNS_HTTPD_H */
