# makefile

all: dataserver client rmv

reqchannel.o: reqchannel.h reqchannel.cpp
	g++ -g -w -Wall -O1 -std=c++11 -c reqchannel.cpp

SafeBuffer.o: SafeBuffer.h SafeBuffer.cpp
	g++ -g -w -Wall -O1 -std=c++11 -c SafeBuffer.cpp -lpthread #-lrt

Histogram.o: Histogram.h Histogram.cpp
	g++ -g -w -Wall -O1 -std=c++11 -c Histogram.cpp -lpthread #-lrt

dataserver: dataserver.cpp reqchannel.o 
	g++ -g -w -Wall -O1 -std=c++11 -o dataserver dataserver.cpp reqchannel.o -lpthread #-lrt

client: client.cpp reqchannel.o SafeBuffer.o Histogram.o
	g++ -g -w -Wall -O1 -std=c++11 -o client client.cpp reqchannel.o SafeBuffer.o Histogram.o -lpthread #-lrt

rmv: rmv.cpp
	g++ -g -w -Wall -O1 -std=c++11 -o rmv rmv.cpp -lpthread #-lrt

clean:
	rm -rf *.o fifo* dataserver client rmv
