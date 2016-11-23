#include<iostream>
#include "dnsserver.h"
using namespace dns;
int main(){
	DnsServer m_server;
	m_server.init(8080);
	m_server.run();
	return 0;
}
