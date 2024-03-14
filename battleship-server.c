#include "battleship.h"

// les deux cartes contenant le placement des vaisseaux des joueurs
char maps[2][MAP_WIDTH][MAP_HEIGHT];
// les deux sockets pour communiquer avec chaque joueur
int sockets[2];

// coup dans l'eau
const char SPLASH  = 'o';
// vaisseau touché
const char TOUCHED = '*';

// case vide
const char EMPTY   = '.';
// case occupée par un vaisseau
const char SHIP    = 'X';


/*
 * Récupération de la carte du joueur numéro 'player'
 *  - on lit les informations reçues sur le socket 'sockets[player]'
 *  - on copie le contenu dans la carte du joueur 'maps[player]'
 * 
 * Attention ! les données reçues sont sous forme d'un tableau de caractères (les différentes lignes sont séparées par un '\n')
*/
void get_map(int player)
{
	// réception de la carte
	char buffer[MAP_WIDTH*MAP_HEIGHT];
	int len = recv(sockets[player], buffer, MAP_WIDTH*MAP_HEIGHT, 0);
	if (len == -1)
	{
		perror("recv");
		exit(EXIT_FAILURE);
	}
	buffer[len] = '\0';

	// copie dans la variable maps
	for(size_t i=0; i<MAP_WIDTH; i++)
	{
		for(size_t j=0; j<MAP_HEIGHT; j++)
		{
			maps[player][i][j] = buffer[i*MAP_WIDTH+j];
		}
	}

	

	// affichage de la carte
	printf("Carte du joueur %d :\n", player);
	for(size_t j=0; j<MAP_HEIGHT; j++)
	{
		for(size_t i=0; i<MAP_WIDTH; i++)
		{
			printf("%c", maps[player][i][j]);
		}
		printf("\n");
	}

	printf("\n");

	return;
}


/*
 * Lecture d'un coup envoyé par un joueur 'player' dans le socket correspondant.
 */
void get_move(int player, struct move *m) 
{
	char buffer[2];
	int len = recv(sockets[player], buffer, 2, 0);
	if (len == -1)
	{
		perror("recv");
		exit(EXIT_FAILURE);
	}
	m->x = buffer[0];
	m->y = buffer[1];
	printf("Coup du joueur %d : %d-%d\n", player, m->x, m->y);
	return;
}


/*
 * Le joueur 'player' joue un coup 'move'
 *  - vérification des coordonnées (pas de nombre négatif, pas supérieur à la taille de la carte) 
 *  - vérification du coup (un vaisseau est touché ou non) - case EMPTY ou SHIP
 *  - si la case est 'EMPTY' on envoie la valeur SPLASH au client sinon on envoie TOUCHED et on "supprime" le vaisseau de la case (on met la valeur EMPTY)
*/
char play_move(int player, struct move m)
{
	char status = SPLASH;
	printf("%d: %d-%d\n", player,  m.x, m.y);
	if (m.x >= 0 && m.x < MAP_WIDTH && m.y >=0 && m.y < MAP_HEIGHT)
	{
		if (maps[(player+1)%2][m.x][m.y] == SHIP) {
			maps[(player+1)%2][m.x][m.y] = EMPTY;
			status = TOUCHED;
		}	
	}
	return status;
}


/*
 * Envoi du résultat du coup au joueur et du status de la partie. Les deux octets sont envoyés l'un après l'autre.
 *  - le résultat du coup peut être SPLASH ou TOUCHED
 *  - le status peut être WIN s'il y a un vainqueur, DRAW si c'est un match nul ou -1 sinon
 */
void send_result(int player, char res, char status)
{
	char buffer[2];
	buffer[0] = res;
	buffer[1] = status;
	int len = send(sockets[player], buffer, 2, 0);
	if (len == -1)
	{
		perror("send");
		exit(EXIT_FAILURE);
	}
	return;
}


/*
 * Fonction calculant le nombre de vaisseaux restant à détruire pour chaque joueur.
 * On compte simplement combien de cases des deux cartes sont encore à la valeur SHIP.
 * Rappel important : la case est mise à EMPTY une fois touchée
 * Si un joueur obtient un score de 0, c'est qu'il a gagné.
 * Le status de la partie est renvoyée : WIN s'il y a un vainqueur, DRAW un match nul et ONGOING sinon
 */
char score()
{
	int nb_alive_for_0 = 0;
	int nb_alive_for_1 = 0;

	for (int j=0 ; j<MAP_HEIGHT ; j++)
	{
		for (int i=0 ; i<MAP_WIDTH ; i++)
		{
			if (maps[0][i][j] == SHIP)
				nb_alive_for_0++;
			if (maps[1][i][j] == SHIP)
				nb_alive_for_1++;
		}
	}
	
	if (nb_alive_for_0 == 0 && nb_alive_for_1 == 0)
	{
		printf("Match nul !\n");
		return DRAW;
	}
	else if (nb_alive_for_0 == 0)
	{
		printf("Joueur 1 a gagné !\n");
		return WIN;
	}
	else if (nb_alive_for_1 == 0)
	{
		printf("Joueur 0 a gagné !\n");
		return WIN;
	}

	printf("Score : %d - %d\n", nb_alive_for_0, nb_alive_for_1);
	return ONGOING;
}

/*
 * Fonction ouvrant une connexion TCP sur le port passé en paramètre et attendant la connexion de deux clients.
 * Les numéros des sockets de deux connexions clientes sont stockées dans le tableeau 'sockets' (sockets[0] pour le joueur 0 et sockets[1] pour le joueur 1).
 * La première étape consiste à écrire les fonctions open_connection du serveur de jeu et du client.
Celle du serveur crée un socket TCP se mettant en attente de connexions sur un port passé en paramètre à la com-
mande serveur. Lors d’une réception de demande de connexion, le serveur stocke le socket créé dans la variable
sockets qui contiendra donc les deux sockets vers les deux joueurs : sockets[0] vers le premier joueur et sockets[1]
vers le second.
*/
void open_connection(char* port) {
	struct addrinfo hints, *servinfo, *p;
	struct sockaddr_storage their_addr;
	socklen_t sin_size;
	int status_getaddrinfo;
	int sockfd;

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	if((status_getaddrinfo = getaddrinfo(NULL, port, &hints, &servinfo)))
	{
		perror("getaddrinfo\n");
		exit(EXIT_FAILURE);
	}

	for(p = servinfo; p!=NULL; p=p->ai_next)
	{
		if((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1)
		{
			perror("socket\n");
			continue;
		}

		if(bind(sockfd, p->ai_addr, p->ai_addrlen) == -1)
		{
			close(sockfd);
			perror("bind\n");
			continue;
		}

		break;
	}

	if(!p)
	{
		perror("serveur: erreur de bind\n");
		exit(EXIT_FAILURE);
	}

	if(listen(sockfd, 5) == -1)
	{
		perror("listen\n");
		exit(EXIT_FAILURE);
	}

	printf("Serveur en attente de connexion\n");
	sin_size = sizeof their_addr;
	sockets[0] = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
	if(sockets[0] == -1)
	{
		perror("accept\n");
		exit(EXIT_FAILURE);
	}

	printf("Première connexion établie\n");

	sockets[1] = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
	if(sockets[1] == -1)
	{
		perror("accept\n");
		exit(EXIT_FAILURE);
	}

	printf("Deuxième connexion établie\n");

	freeaddrinfo(servinfo);
	printf("Connexion établie\n");
	close(sockfd);

	return;
}

void sigint_handler(int sig) {
    printf("\nServer shutting down...\n");
	close(sockets[0]);
	close(sockets[1]);
    exit(EXIT_SUCCESS);
}

/*
 * Fonction principale du serveur
 */
int main(int argc, char *argv[])
{
	struct sigaction sa;
	sa.sa_handler = sigint_handler;
	sa.sa_flags = 0;
	sigemptyset(&sa.sa_mask);
	sigaction(SIGINT, &sa, NULL);

	// test des arguments de la commande
	if (argc != 2)
	{
		printf("Usage : %s <port>\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	// ouverture de la connexion TCP
	open_connection(argv[1]);

	// réception des placements des vaisseaux
	get_map(0);
	get_map(1);

	struct move m0, m1;
	char stat0, stat1;
	char end = 0; 
	while (!end)
	{
		// réception des coups des joueurs
		get_move(0, &m0);
		get_move(1, &m1);

		// évaluation des coups des joueurs et envoi des résultats
		stat0 = play_move(0, m0);
		stat1 = play_move(1, m1);

		// affichage du score et vérification de fin de partie
		char status = score();
		
		// envoi du résultat du coup et du status de la partie
		send_result(0, stat0, status);
		send_result(1, stat1, status);

		// un des joueurs a gagné
		if (status == WIN || status == DRAW)
			end = 1;
	}

	return EXIT_SUCCESS;
}