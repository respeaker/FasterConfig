###########################################  
#Makefile for simple  dnslookup
###########################################  
INC=../../src 
LIB= 
  
CC=g++ 
CC_FLAG=-Wall  
  
PRG=dnslookup
OBJ=main.cpp ../../src/DNSLookup.cpp ../../src/query.cpp ../../src/logger.cpp ../../src/message.cpp ../../src/resolver.cpp ../../src/response.cpp 
  
$(PRG):$(OBJ)  
	$(CC) -I$(INC) $(LIB) -o $@ $(OBJ)  
      
.SUFFIXES: .c .o .cpp  
.cpp.o:  
	$(CC) $(CC_FLAG) $(INC) -c $*.cpp -o $*.o  
  
.PRONY:clean  
clean:  
	@echo "Removing linked and compiled files......"  
	rm -f  $(PRG)  
