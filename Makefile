all:

	gcc -Wall -pthread client.c -o client 
	gcc -Wall -pthread server.c tokenizer.c -o server

clean:
	rm -rf *.o
	rm -f client
	rm -f server
