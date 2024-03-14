all: server_battleship client_battleship

server_battleship: battleship-server.c battleship.c
	gcc -Wall -o server_battleship battleship-server.c battleship.c

client_battleship: battleship-client.c battleship.c
	gcc -Wall -o client_battleship battleship-client.c battleship.c
