#
# Makefile for Example
#
LIBS	= 
CFLAGS	= -g -ggdb -std=c99

all:	udp 

#
# Internet stream version (TCP protocol).
#

udp:	server1 client1 hostent

client1.o server1.o: inet.h

server1:	server1.o 
	gcc $(CFLAGS) -o $@ server1.o $(LIBS)

client1:	client1.o 
	gcc $(CFLAGS) -o $@ client1.o $(LIBS)

hostent: hostent.o
	gcc $(CFLAGS) -o $@ hostent.o $(LIBS)

#
# Clean up the mess we made
#
clean:
	rm *.o \
	server1 client1 hostent 2>/dev/null
