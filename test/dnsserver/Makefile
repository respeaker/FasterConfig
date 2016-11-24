###########################################  
#Makefile for simple  dnslookup
###########################################  
INC=../../src 
LIB= -lpthread -lresolv
  
CC=g++ 
CC_FLAG= -std=c++11
  
PRG=dnsserver
OBJ=main.cpp ../../src/dnsserver.cpp 	\
				 ../../src/query.cpp 	\
				../../src/logger.cpp 	\
				../../src/message.cpp 	\
				../../src/resolver.cpp 	\
				../../src/response.cpp 	\
				../../src/httpd.cpp
  
$(PRG):$(OBJ)  
	$(CC) -I$(INC) -g -o $@ $(OBJ)  $(LIB) $(CC_FLAG) 
      
.SUFFIXES: .c .o .cpp  
.cpp.o:  
	$(CC) $(CC_FLAG) $(INC) -c $*.cpp -o $*.o  
  
.PRONY:clean  
clean:  
	@echo "Removing linked and compiled files......"  
	rm -f  $(PRG)  
