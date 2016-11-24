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
#include <cstring>
#include <sys/socket.h>
#include <errno.h>
#include <sstream>
#include <iomanip>

#include <netinet/in.h>
#include <arpa/nameser.h>
#include <resolv.h>

#include<stdio.h>


#include "dnsserver.h"

using namespace std;
using namespace dns;

void DnsServer::init(int dPort) {


    m_sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    m_address.sin_family = AF_INET;
    m_address.sin_addr.s_addr = INADDR_ANY;
    m_address.sin_port = htons(dPort);

    logger = &Logger::instance();

    int rbind = bind(m_sockfd, (struct sockaddr *) & m_address,
                     sizeof (struct sockaddr_in));

    if (rbind != 0) {
        debug(LOG_INFO,"Could not bind!!");
        return;
    }

    cout << "Listening in port: " << dPort << ", sockfd: " << m_sockfd << endl;
}

void DnsServer::run(){


    cout << "DNS Server running..." << endl;

    char buffer[BUFFER_SIZE];
    struct sockaddr_in clientAddress;
    socklen_t addrLen = sizeof (struct sockaddr_in);

    while (true) {

        int nbytes = recvfrom(m_sockfd, buffer, BUFFER_SIZE, 0,
                     (struct sockaddr *) &clientAddress, &addrLen);

#ifdef  FASTERCONFIG_DEBUG
        dump_buffer(buffer, nbytes);
#endif
		int TransID;
		struct dns_header *dns;
		dns = (struct dns_header *)buffer;  //get dns  headers

		char name[NS_MAXDNAME];
		int name_len;

		/*get id*/
		TransID = ntohs(dns->id);
/*
       int dn_expand(unsigned char *msg,
					unsigned char *eomorig,
    				unsigned char *comp_dn,
					char *exp_dn,
					int length);
*/
		/* extract the name from the packet */
		name_len = dn_expand((u_char *)dns,
							 (u_char *)dns + nbytes,
							 (u_char *)(dns + 1),
							  name,
							 sizeof(name));

	  //skip headers
	   u_char *q = (u_char *)(buffer + sizeof(struct dns_header) + name_len);
	   /* get the type and class */
	   NS_GET16(t_type, q);
	   NS_GET16(t_class, q);



		cout << "id: " <<ntohs(dns->id) << endl;
		cout << "name: " <<name << endl;
		cout << "t_type: " <<t_type << endl;
		cout << "t_class: " <<t_class << endl;


	   /* handle only internet class */
	   if (t_class != ns_c_in){
		  cout << "Sorry, handle only internet class" << endl;
		  continue ;
	   }

	   int reply_length = 0;
	   int n_answ = 0;
	   int n_auth = 0;
	   int n_addi = 0;
	   u_char *reply_data;



	   /* we are interested only in DNS query */
	   if ( (!dns->qr) &&
			dns->opcode == ns_o_query &&
			htons(dns->num_q) == 1 &&
			htons(dns->num_answer) == 0){
		   switch (t_type) {
      case   ns_t_txt:
      case ns_t_ptr:
       case ns_t_aaaa:
		   case ns_t_a:
			   reply_data = prepare_dns_reply_a(t_type,&reply_length,&n_answ,&n_auth,&n_addi);
			   for (int i = 0;i < reply_length ; i++) {
				   printf("%x ",reply_data[i]);
			   }
			   break;
		   case ns_t_mx:
         reply_data = prepare_dns_reply_mx(t_type,&reply_length,&n_answ,&n_auth,&n_addi);
         for (int i = 0;i < reply_length ; i++) {
				   printf("%x ",reply_data[i]);
			   }
         break;
		   case ns_t_wins:
        reply_data = prepare_dns_reply_wins(t_type,&reply_length,&n_answ,&n_auth,&n_addi);
			   break;

      case ns_t_srv:
       reply_data = prepare_dns_reply_srv(t_type,&reply_length,&n_answ,&n_auth,&n_addi,0);
         break;
       default:
       reply_data = prepare_dns_reply_default(t_type,&reply_length,&n_answ,&n_auth,&n_addi);
			   break;

		   }
	   }else{
		   continue ;
	   }
	   memset(buffer, 0, BUFFER_SIZE);

	   encode_spoof_dns_header(buffer,TransID,n_answ,n_auth,n_addi);
	   nbytes = 12;
	   encode_domain(buffer+nbytes, name, &name_len);        /*encode name*/
	   nbytes += name_len;
	   encode_query_typy_and_class(buffer + nbytes, &nbytes);
	   put_block_date(buffer+nbytes,reply_data,reply_length);  /*encode reply*/
	   nbytes += reply_length;
     printf("%d\n",reply_length);
#ifdef FASTERCONFIG_DEBUG
        dump_buffer(buffer, nbytes);
#endif

        sendto(m_sockfd, buffer, nbytes, 0, (struct sockaddr *) &clientAddress,
               addrLen);
		free(reply_data);
    }
}

/*
 * checks if a spoof entry extists for the name and type
 * the answer is prepared and stored in the global lists
 *  - answer_list
 *  - authority_list
 *  - additional_list
 */
u_char* DnsServer::prepare_dns_reply_a(
								   int t_type,
								   int *dns_len,
								   int *n_answ,
								   int *n_auth,
								   int *n_addi){
	u_char* data;
	struct in_addr  reply_addr ;
	uint32_t dns_ttl;
	/* set TTL to 1 hour by default or in case something goes wrong */
	dns_ttl = htonl(3600); /* reorder bytes for network stuff */

	 /* allocate memory for the answer */
	 *dns_len = 12 + IP_ADDR_LEN;
	 data = (u_char*)calloc(*dns_len, sizeof(u_char));

	 inet_aton(spoof_addr,&reply_addr);

	 /* prepare the answer */
	 memcpy(data, "\xc0\x0c", 2);                        /* compressed name offset */
	 memcpy(data + 2, "\x00\x01", 2);                    /* type A */
	 memcpy(data + 4, "\x00\x01", 2);                    /* class */
	 memcpy(data + 6, &dns_ttl, 4);                      /* TTL */
	 memcpy(data + 10, "\x00\x04", 2);                   /* datalen */

	 memcpy(data + 12,&reply_addr.s_addr , 4);/* data */
	 *n_answ += 1;
	 return data;
}

/*
 * checks if a spoof entry extists for the name and type
 * the answer is prepared and stored in the global lists
 *  - answer_list
 *  - authority_list
 *  - additional_list
 */
u_char* DnsServer::prepare_dns_reply_mx(
								   int t_type,
								   int *dns_len,
								   int *n_answ,
								   int *n_auth,
								   int *n_addi){
    u_char* data;
    struct in_addr  reply_addr ;
    uint32_t dns_ttl;
    /* set TTL to 1 hour by default or in case something goes wrong */
    dns_ttl = htonl(3600); /* reorder bytes for network stuff */

    /* allocate memory for the answer */
    *dns_len = 12 + 2 + 7 + 17 + IP_ADDR_LEN;
    data = (u_char*)calloc(*dns_len, sizeof(u_char));
    memset(data, 0, *dns_len);
    inet_aton(spoof_addr,&reply_addr);
    /* prepare the answer */
    memcpy(data, "\xc0\x0c", 2);                          /* compressed name offset */
    memcpy(data + 2, "\x00\x0f", 2);                      /* type MX */
    memcpy(data + 4, "\x00\x01", 2);                      /* class */
    memcpy(data + 6, &dns_ttl, 4);                        /* TTL */
    memcpy(data + 10, "\x00\x09", 2);                     /* datalen */
    memcpy(data + 12, "\x00\x0a", 2);                     /* preference (highest) */
    /*
     * add "mail." in front of the domain and
     * resolve it in the additional record
     * (here `mxoffset' is pointing at)
     */
    memcpy(data + 14, "\x04mail\xc0\x0c", 7); /* mx record */
    *n_answ += 1;
    /* prepare the additinal record */
    memcpy(data + 21 , "\x04mail\xc0\x0c", 7);             /* compressed name offset */
    memcpy(data + 21 +7, "\x00\x01", 2);                 /* type A */
    memcpy(data + 21 + 9, "\x00\x01", 2);                 /* class */
    memcpy(data + 21 + 11, &dns_ttl, 4);                  /* TTL */
    memcpy(data + 21 + 15, "\x00\x04", 2);                /* datalen */
    memcpy(data + 21 + 17,&reply_addr.s_addr , 4);/* data */
    *n_addi += 1;
    return data;
}
/*
 * checks if a spoof entry extists for the name and type
 * the answer is prepared and stored in the global lists
 *  - answer_list
 *  - authority_list
 *  - additional_list
 */
u_char* DnsServer::prepare_dns_reply_wins(
								   int t_type,
								   int *dns_len,
								   int *n_answ,
								   int *n_auth,
								   int *n_addi){
    u_char* data;
    struct in_addr  reply_addr ;
    uint32_t dns_ttl;
    /* set TTL to 1 hour by default or in case something goes wrong */
    dns_ttl = htonl(3600); /* reorder bytes for network stuff */

    /* allocate memory for the answer */
    *dns_len = 12 + IP_ADDR_LEN;
    data = (u_char*)calloc(*dns_len, sizeof(u_char));
    memset(data, 0, *dns_len);
    inet_aton(spoof_addr,&reply_addr);
    /* prepare the answer */
    memcpy(data, "\xc0\x0c", 2);                        /* compressed name offset */
    memcpy(data + 2, "\xff\x01", 2);                    /* type WINS */
    memcpy(data + 4, "\x00\x01", 2);                    /* class IN */
    memcpy(data + 6, &dns_ttl, 4);                      /* TTL */
    memcpy(data + 10, "\x00\x04", 2);                   /* datalen */
    memcpy(data + 12,&reply_addr.s_addr , 4);/* data */
    return data;
}
/*
 * checks if a spoof entry extists for the name and type
 * the answer is prepared and stored in the global lists
 *  - answer_list
 *  - authority_list
 *  - additional_list
 */
u_char* DnsServer::prepare_dns_reply_srv(
								   int t_type,
								   int *dns_len,
								   int *n_answ,
								   int *n_auth,
								   int *n_addi,
                   int dn_offset){
    char tgtoffset[2];
    u_char* data;
    struct in_addr  reply_addr ;
    uint32_t dns_ttl;
    /* set TTL to 1 hour by default or in case something goes wrong */
    dns_ttl = htonl(3600); /* reorder bytes for network stuff */

    /* allocate memory for the answer */
    *dns_len = 44;
    data = (u_char*)calloc(*dns_len, sizeof(u_char));
    memset(data, 0, *dns_len);
    inet_aton(spoof_addr,&reply_addr);

    tgtoffset[0] = 0xc0; /* offset byte */
    tgtoffset[1] = 12 + dn_offset; /* offset to the actual domain name */

    /* prepare the answer */
    memcpy(data, "\xc0\x0c", 2);                    /* compressed name offset */
    memcpy(data + 2, "\x00\x21", 2);                /* type SRV */
    memcpy(data + 4, "\x00\x01", 2);                /* class IN */
    memcpy(data + 6, &dns_ttl, 4);                  /* TTL */
    memcpy(data + 10, "\x00\x0c", 2);               /* data length */
    memcpy(data + 12, "\x00\x00", 2);               /* priority */
    memcpy(data + 14, "\x00\x00", 2);               /* weight */
    memcpy(data + 16, "\x08\x00", 2);               /* port 2048 */

    /*
     * add "srv." in front of the stripped domain
     * name and resolve it in the additional
     * record (here `srvoffset' is pointing at)
     */
    memcpy(data + 18, "\x03srv", 4);                /* target */
    memcpy(data + 22, tgtoffset, 2);                /* compressed name offset */
    *n_answ += 1;
    /* prepare the additional record */
    memcpy(data +24 , "\x03srv", 4);                 /* target */
    memcpy(data +24  + 4, tgtoffset, 2);             /* compressed name offset */
    memcpy(data +24  + 6, "\x00\x01", 2);            /* type A */
    memcpy(data +24 + 8, "\x00\x01", 2);            /* class */
    memcpy(data +24 + 10, &dns_ttl, 4);             /* TTL */
    memcpy(data +24  + 14, "\x00\x04", 2);           /* datalen */
    memcpy(data +24  + 16,&reply_addr.s_addr , 4);/* data */
    *n_addi += 1;
    return data;
}
/*
 * checks if a spoof entry extists for the name and type
 * the answer is prepared and stored in the global lists
 *  - answer_list
 *  - authority_list
 *  - additional_list
 */
u_char* DnsServer::prepare_dns_reply_default(
								   int t_type,
								   int *dns_len,
								   int *n_answ,
								   int *n_auth,
								   int *n_addi){
    u_char* data;
    struct in_addr  reply_addr ;
    uint32_t dns_ttl;
    /* set TTL to 1 hour by default or in case something goes wrong */
    dns_ttl = htonl(3600); /* reorder bytes for network stuff */

    /* allocate memory for the answer */
    *dns_len = 46;
    data = (u_char*)calloc(*dns_len, sizeof(u_char));
    memset(data, 0, *dns_len);
    inet_aton(spoof_addr,&reply_addr);
    /* prepare the authorative record */
    memcpy(data, "\xc0\x0c", 2);                        /* compressed named offset */
    memcpy(data + 2, "\x00\x06", 2);                    /* type SOA */
    memcpy(data + 4, "\x00\x01", 2);                    /* class */
    memcpy(data + 6, &dns_ttl, 4);                      /* TTL (seconds) */
    memcpy(data + 10, "\x00\x22", 2);                   /* datalen */
    memcpy(data + 12, "\x03ns1", 4);                    /* primary server */
    memcpy(data + 16, "\xc0\x0c", 2);                   /* compressed name offeset */
    memcpy(data + 18, "\x05""abuse", 6);                /* mailbox */
    memcpy(data + 24, "\xc0\x0c", 2);                   /* compressed name offset */
    memcpy(data + 26, "\x51\x79\x57\xf5", 4);           /* serial */
    memcpy(data + 30, "\x00\x00\x0e\x10", 4);           /* refresh interval */
    memcpy(data + 34, "\x00\x00\x02\x58", 4);           /* retry interval */
    memcpy(data + 38, "\x00\x09\x3a\x80", 4);           /* erpire limit */
    memcpy(data + 42, "\x00\x00\x00\x3c", 4);           /* minimum TTL */
    *n_auth += 1;
    return data;
}
void DnsServer::encode_spoof_dns_header(char *buffer,
							  const uint16_t id,
							  const uint16_t answ,
							  const uint16_t auth,
							  const uint16_t addi) {

	   put2byte(buffer, id); 		/* id */
	   put2byte(buffer, 0x8400); 		/* standard reply, no error */
	   put2byte(buffer,	0x01);	    	/*number qcount*/
	   put2byte(buffer,	answ);        /*number ancount*/
	   put2byte(buffer,	auth);        /*number nscount*/
	   put2byte(buffer,	addi);        /*number qcount*/
}
void DnsServer::encode_domain(char *buffer,char *name, int* len){
/*
www.baidu.com
03 77 77 77 05 62 61 69 64 75 03 63 6f 6d
*/
	string domain = name;
	*len = 0;
    int start(0), end; // indexes

    while ((end = domain.find('.', start)) != string::npos) {

        *buffer++ = end - start; // label length octet
		*len += 1;
        for (int i=start; i<end; i++) {

            *buffer++ = domain[i]; // label octets
			*len += 1;
        }
        start = end + 1; // Skip '.'
    }

    *buffer++ = domain.size() - start; // last label length octet
	*len += 1;
    for (int i=start; i<domain.size(); i++) {

        *buffer++ = domain[i]; // last label octets
		*len += 1;
    }

	*buffer++ = 0;
	*len += 1;
}
void DnsServer::encode_query_typy_and_class(char *buffer,int *len){
	put2byte(buffer, t_type);
	put2byte(buffer, t_class);
	*len += 4;
}
/**
 *
 *
 * @author Baozhu (9/13/2016)
 *
 * @param buffer
 * @param domain
 */
void DnsServer::put_block_date(char *buffer,  u_char *data ,int len) {

	for (int i = 0; i < len ;i++) {
		*buffer++ = data[i];
	}
}

/**
 * encode packeage e.g:
 *  ---------------------------------
 *  00: 8a 0f 85 80 00 01 00 01 00 00
 *  10: 00 00 03 77 77 77 05 62 61 69
 *  20: 64 75 03 63 6f 6d 00 00 01 00
 *  30: 01 c0 0c 00 01 00 01 00 00 00
 *  40: 00 00 04 ac 01 08 01
 *  ---------------------------------
 *
 * @author Baozhu (9/13/2016)
 *
 * @param buffer
 *
 * @return int
 */
int DnsServer::encode(char *buffer) {

    char* bufferBegin = buffer;

   // encode_header(buffer);
    buffer += 12;
#if 0
    if (! m_name.compare(m_name.length() - 4 ,4,".lan")) {
        m_name = m_name.erase(m_name.length() - 4,4);
    }
#endif
   // encode_domain(buffer, domainName);
#if 0
    put16bits(buffer, 0x0377);
    put16bits(buffer, 0x7777);
    put16bits(buffer, 0x0b73);
    put16bits(buffer, 0x6565);
    put16bits(buffer, 0x6564);
    put16bits(buffer, 0x7374);
    put16bits(buffer, 0x7564);
    put16bits(buffer, 0x696f);
    put16bits(buffer, 0x0363);
    put16bits(buffer, 0x6f6d);
#endif
    //*buffer = 0x00;
    //buffer += 1;
    //put16bits(buffer, 0x0000);

    put2byte(buffer, 0x0100);
    put2byte(buffer, 0x01c0);
    put2byte(buffer, 0x0c00);
    put2byte(buffer, 0x0100);
    put2byte(buffer, 0x0100);
    put2byte(buffer, 0x0000);
    put2byte(buffer, 0x0000);



    put2byte(buffer, 0x04ac); //172.31.255.240
    put2byte(buffer, 0x1fff);
    *buffer = 0xf0;
    buffer += 1;

#if 0
    // Code Question section
    code_domain(buffer, m_name);
    put16bits(buffer, m_type);
    put16bits(buffer, m_class);

    // Code Answer section
    code_domain(buffer, m_name);
    put16bits(buffer, m_type);
    put16bits(buffer, m_class);
    put32bits(buffer, m_ttl);
    put16bits(buffer, m_rdLength);
    code_domain(buffer, m_rdata);
#endif
    int size = buffer - bufferBegin;
    return size;
}
/**
 *
 *
 * @author Baozhu (9/13/2016)
 *
 * @param buffer
 *
 * @return int
 */
int DnsServer::get2byte(const char*& buffer) {

    int value = static_cast<unsigned char>(buffer[0]);
    value = value << 8;
    value += static_cast<unsigned char>(buffer[1]);
    buffer += 2;

    return value;
}
/**
 *
 *
 * @author Baozhu (9/13/2016)
 *
 * @param buffer
 * @param value
 */
void DnsServer::put2byte(char*& buffer, uint value) {

    buffer[0] = (value & 0xFF00) >> 8;
    buffer[1] = value & 0xFF;
    buffer += 2;
}

/**
 *
 *
 * @author Baozhu (9/13/2016)
 *
 * @param buffer
 * @param size
 */
void DnsServer::dump_buffer(const char* buffer, int size) {

    ostringstream text;

    text << "size: " << size << " bytes" << endl;
    text << "---------------------------------" << setfill('0');

    for (int i = 0; i < size; i++) {
        if ((i % 10) == 0) {
            text << endl << setw(2) << i << ": ";
        }
        unsigned char c = buffer[i];
        text << hex << setw(2) << int(c) << " " << dec;
    }
    text << endl << setfill(' ');
    text << "---------------------------------";
    cout << text.str() << endl;
}
