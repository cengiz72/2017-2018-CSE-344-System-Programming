all:Compile Link

Compile:
	gcc -c server.c client.c
Link:
	gcc -o server server.o -lrt -pthread -lm
	gcc -o client client.o -lrt -pthread -lm
clean:
	rm server client server.o client.o
