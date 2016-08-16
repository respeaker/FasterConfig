add_executable (fasterconfig application.cpp  DNSLookup.cpp  logger.cpp  main.cpp  message.cpp  query.cpp  resolver.cpp  response.cpp  server.cpp)
install (TARGETS fasterconfig  DESTINATION /usr/bin)
ADD_CUSTOM_TARGET(uninstall COMMAND "rm" -f "/usr/bin/fasterconfig")
