###########################################  
#Makefile for simple  dnslookup
###########################################  
INC=../../src 
LIB= 
  
CC=g++ 
CC_FLAG=-Wall  
  
PRG=iptable_test
OBJ=main.cpp ../../src/iptable.cpp  ../../src/logger.cpp
  
$(PRG):$(OBJ)  
	$(CC) -I$(INC) $(LIB) -o $@ $(OBJ)  
      
.SUFFIXES: .c .o .cpp  
.cpp.o:  
	$(CC) $(CC_FLAG) $(INC) -c $*.cpp -o $*.o  
  
.PRONY:clean  
clean:  
	@echo "Removing linked and compiled files......"  
	rm -f  $(PRG)  
