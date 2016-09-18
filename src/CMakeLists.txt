add_executable (fasterconfig application.cpp  DNSLookup.cpp  logger.cpp  main.cpp  message.cpp  query.cpp  resolver.cpp  response.cpp  dnsserver.cpp httpd.cpp iptable.cpp)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -O2 -pthread ")
install (TARGETS fasterconfig  DESTINATION /usr/bin)
ADD_CUSTOM_TARGET(uninstall COMMAND "rm" -f "/usr/bin/fasterconfig")
