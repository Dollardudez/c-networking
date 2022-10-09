#
# Makefile for Example
#
LIBS	= 
CFLAGS	= -g -ggdb

all:	dir nodir 

#
# Internet stream version (TCP protocol).
#
nodir:  chatClient1 chatServer1
dir:	chatClient2 chatServer2 directoryServer2

chatClient1.o chatClient1.o: structures.h inet.h
chatClient2.o chatServer2.o directoryServer2.o: structures.h inet.h

chatServer2:	chatServer2.o
	gcc $(CFLAGS) -o $@ chatServer2.o $(LIBS)

chatClient2:	chatClient2.o 
	gcc $(CFLAGS) -o $@ chatClient2.o $(LIBS)

chatServer1:	chatServer1.o
	gcc $(CFLAGS) -o $@ chatServer1.o $(LIBS)

chatClient1:	chatClient1.o 
	gcc $(CFLAGS) -o $@ chatClient1.o $(LIBS)

directoryServer2:	directoryServer2.o
	gcc $(CFLAGS) -o $@ directoryServer2.o $(LIBS)


#
# Clean up the mess we made
#
clean:
	rm *.o chatServer2 chatServer1 chatClient2 chatClient1 directoryServer2 2>/dev/null
