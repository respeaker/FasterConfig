# The SOURCEDIR macro specifies the relative path for source files.
# The CXXSOURCES macro contains a list of source files.
# The CXXOBJECTS macro converts CXXSOURCES macro into a list of object files.
# The CXXFLAGS macro contains a list of options passed to the compiler.
# The CXX macro defines the C++ compiler.
# The LDFLAGS macro contains all of the library and library
# directory information to be passed to the linker.
#

SOURCEDIR = ./src
CXXSOURCES = main.cpp application.cpp logger.cpp message.cpp \
             query.cpp resolver.cpp response.cpp dnsserver.cpp \
			 httpd.cpp  iptable.cpp

CXXOBJECTS = $(CXXSOURCES:.cpp=.o)  # expands to list of object files
CXXFLAGS = -g $(INCLUDEDIRS)
CXX = arm-linux-gnueabihf-g++
LIBS = -lpthread
LDFLAGS = $(LIBDIRS) $(LIBS)

#
# Default target
#
		
build:	$(CXXOBJECTS)
	$(CXX) -o fasterconfig $(CXXOBJECTS) $(LDFLAGS)

#
# Object targets:
#

main.o: $(SOURCEDIR)/main.cpp
	$(CXX) $(CXXFLAGS) -c -o main.o $(SOURCEDIR)/main.cpp

application.o: $(SOURCEDIR)/application.cpp
	$(CXX) $(CXXFLAGS) -c -o application.o $(SOURCEDIR)/application.cpp

logger.o: $(SOURCEDIR)/logger.cpp
	$(CXX) $(CXXFLAGS) -c -o logger.o $(SOURCEDIR)/logger.cpp

message.o: $(SOURCEDIR)/message.cpp
	$(CXX) $(CXXFLAGS) -c -o message.o $(SOURCEDIR)/message.cpp

query.o: $(SOURCEDIR)/query.cpp
	$(CXX) $(CXXFLAGS) -c -o query.o $(SOURCEDIR)/query.cpp

resolver.o: $(SOURCEDIR)/resolver.cpp
	$(CXX) $(CXXFLAGS) -c -o resolver.o $(SOURCEDIR)/resolver.cpp

response.o: $(SOURCEDIR)/response.cpp
	$(CXX) $(CXXFLAGS) -c -o response.o $(SOURCEDIR)/response.cpp

dnsserver.o: $(SOURCEDIR)/dnsserver.cpp
	$(CXX) $(CXXFLAGS) -c -o dnsserver.o $(SOURCEDIR)/dnsserver.cpp

httpd.o: $(SOURCEDIR)/httpd.cpp
	$(CXX) $(CXXFLAGS) -c -o httpd.o $(SOURCEDIR)/httpd.cpp

iptable.o: $(SOURCEDIR)/iptable.cpp
	$(CXX) $(CXXFLAGS) -c -o iptable.o $(SOURCEDIR)/iptable.cpp
#
# Clean target
#

clean:
	$(RM) -f $(CXXOBJECTS) fasterconfig fasterconfig.log

#
# Doxygen target
#

doxy:
	doxygen doxygen.conf

doxy-clean:
	$(RM) -rf doxygen
