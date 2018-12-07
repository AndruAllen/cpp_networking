all: server client

server:
	g++ ServerTCP.cpp -pthread -w -o server
client:
	g++ ClientTCP.cpp -pthread -w -o client

clean:
	rm client server
