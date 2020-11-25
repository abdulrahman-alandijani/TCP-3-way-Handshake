server: server.c 
	gcc -w server.c -o server
	chmod +x server

client: client.c
	gcc -w client.c -o client
	chmod +x client

clean:
	rm server client