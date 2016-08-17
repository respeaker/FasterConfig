###########################################  
#Makefile for simple  dnslookup
###########################################  
INC=../../src 
LIB= -lpthread 
  
CC=g++ 
CC_FLAG=-Wall -std=c++11
  
PRG=dnslookup
OBJ=main.cpp ../../src/DNSLookup.cpp 	\
				 ../../src/query.cpp 	\
				../../src/logger.cpp 	\
				../../src/message.cpp 	\
				../../src/resolver.cpp 	\
				../../src/response.cpp 	\
				../../src/httpd.cpp
  
$(PRG):$(OBJ)  
	$(CC) -I$(INC) $(LIB) -o $@ $(OBJ)  
      
.SUFFIXES: .c .o .cpp  
.cpp.o:  
	$(CC) $(CC_FLAG) $(INC) -c $*.cpp -o $*.o  
  
.PRONY:clean  
clean:  
	@echo "Removing linked and compiled files......"  
	rm -f  $(PRG)  
