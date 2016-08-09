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


#ifndef _DNS_RESOLVER_H
#define	_DNS_RESOLVER_H

#include "exception.h"

namespace dns {

class Query;
class Response;

/**
 *  Resolver is the class that handles the @ref Query and resolves the domain
 *  names contained on it. It processes the @ref Query and set the appropiate
 *  values in the @ref Response.
 */
class Resolver {
public:

    /**
     *  Constructor.
     *  Creates the Resolver.
     */
    Resolver() : m_record_list(0) { }

    /**
     *  Destructor
     */
    virtual ~Resolver() { deleteList(); }

    /**
     *  Open the hosts file and read it to stores the ipAddress-hostname pairs.
     *  @param filename Name of the file containing the pairs.
     */
    void init(const std::string& filename) throw (Exception);

    /**
     *  Process the query and sets the response to that query. @ref Record
     *  @param query @ref Query that will be processed.
     *  @param response @ref Response that will be answered.
     */
    void process(const Query& query, Response& response) throw ();

// Es raro que se quiera heredar de Resolver, pero de esta forma,
// la documentación aparece en doxygen.
protected:
    /**
     *  Extracts the ipAddress-hostname pair from a string line and adds it to
     *  the list of records.
     *  @param line Line read from the hosts file containing the ipAddress-hostname info
     */
    void store(const std::string& line) throw ();

    /**
     *  Structure to hold the ipAddress-hostname pairs.
     */
    struct Record {
        /**
         *  Constructor.
         *  @param ip String holding the IP address in dot notation.
         *  @param domain String holding the domain name.
         */
        Record(std::string& ip, std::string& domain) : ipAddress(ip),
               domainName(domain), next(0) { }

        /**
         *  Constructor
         */
        Record() : next(0) { }

        /**
         *  IP address in dot notation.
         */
        std::string ipAddress;


        /**
         *  Domain name.
         */
        std::string domainName;

        /**
         *  Pointer to next record on the list
         */
        Record* next;
    };
    /**
     *  Pointer to the start of the list of records.
     */
    Record* m_record_list;

    /**
     *  Adds new record to the list
     *  @param record Pointer to the record to add
     */
    void add(Record* record) throw ();

    /**
     *  Deletes all records from the list. Used in destructor
     */
    void deleteList() throw();


    /**
     *  Prints all records from the list.
     */
    void print_records() throw ();

    /**
     *  Covert IN-ADDR.ARPA domain to an IP addrress in dot notation
     *  @param domain The domain name
     *  @return The IP addrress formatted in dot notation.
     */
    std::string convert(const std::string& domain) throw();

    /**
     *  Finds in the list the domanin corresponding to the ipAddress
     *  @param ipAddress IP addrress in dot notation
     *  @return The domain name found. An empty string if no domain was found.
     */
    const std::string find(const std::string& ipAddress) throw ();
};
}
#endif	/* _DNS_RESOLVER_H */

