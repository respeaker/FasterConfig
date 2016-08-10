DNS server  test.

Quick guide
-----------

Open a terminal and run:
prompt> make
prompt> ./go.bash

Open another terminal and run:
prompt> ./dig-test.bash

The file 'dnsServer.log' will be created with the tracing info.


Info about files and folders
----------------------------

Makefile : Generated for Linux with 'build'(default) y 'clean' targets.
Makefile.Manual : Simpler makefile for other systems or environments.
go.bash : Starts dnsserver with listenig port 9000 and file 'hosts'.
dig-test.bash : Start 'dig' with parameters: @127.0.0.1 -x 193.180.17.140 -p 9000
hosts : Textfile with ipAddress-hostname mappings.
doxygen.conf : Configuration file for Doxygen.
src/ : Source code folder
uml/ : UML class and sequence diagrams of dnsserver.
nbproject/ : Netbeans IDE configuration files folder.


Design Criteria
---------------

In the server design, simplicity and stability have been priorized over speed and
efficiency (the implementation of the host/ipAddress records using a linked list
is a clear example) and only UDP transport is implemented.

Following items can be implemented to improve those last two matters:

 - Change the record link list by a hash table.
 - Avoid when possible to copy strings, using pointer to strings, etc...
 - Improve Logger class with different trace levels: debug, warning, error, to
   reduce trace dump size.

To grasp easily the server design, it is recomended to take a look at the UML
class and sequence diagrams, then look up Doxygen documentation and finally
jump into the C++ source code.

Query package
-----------------------
![](https://github.com/respeaker/FasterConfig/blob/master/pic/query.png)

Response package
----------------------
![](https://github.com/respeaker/FasterConfig/blob/master/pic/response.png)

Doxygen 
-----------------------

Doxygen documentation can be generated and clean with the following make targets:

prompt> make -f Makefile.Manual doxy
prompt> make -f Makefile.Manual doxy-clean

and it can reached in the following relative path:

    doxygen/html/index.html

Enjoy it!
Baozhu
