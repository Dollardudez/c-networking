#
# Makefile for Example
#
LIBS	= 
CFLAGS	= -g -ggdb -std=c99

all:	udp tcp 

#
# Internet stream version (TCP protocol).
#

udp:	server2 client2 hostent
tcp:	server3 client3 hostent

client2.o server2.o: inet.h
client3.o server3.o: inet.h

server2:	server2.o 
	gcc $(CFLAGS) -o $@ server2.o $(LIBS)

client2:	client2.o 
	gcc $(CFLAGS) -o $@ client2.o $(LIBS)

server3:	server3.o
	gcc $(CFLAGS) -o $@ server3.o $(LIBS)

client3:	client3.o
	gcc $(CFLAGS) -o $@ client3.o $(LIBS)

hostent: hostent.o
	gcc $(CFLAGS) -o $@ hostent.o $(LIBS)

#
# Clean up the mess we made
#
clean:
	rm *.o \
	server2 client2 server3 client3 hostent 2>/dev/null