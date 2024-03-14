#include "battleship.h"

int *GLOBAL_SOCKET;

// carte des coups joués
char played[MAP_WIDTH][MAP_HEIGHT];
// nombre de vaisseaux ennemis à détruire
int nb_to_destroy = NB_TO_DESTROY;

// coup dans l'eau
const char SPLASH  = 'o';
// vaisseau touché
const char TOUCHED = '*';

// case vide
const char EMPTY   = '.';
// case occupée par un vaisseau
const char SHIP    = 'X';


/*
 * Ouvre une connexion TCP vers un serveur de jeu et retourne le socket créé
 *  - server : nom de la machine cible
 *  - port : port sur lequel se connecter
 */
int open_connection(char *server, char *port)
{
    struct addrinfo hints, *servinfo, *p;
	int status_getaddrinfo;
	int sockfd;

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	if((status_getaddrinfo = getaddrinfo(server, port, &hints, &servinfo)))
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

		if(connect(sockfd, p->ai_addr, p->ai_addrlen) == -1)
		{
			close(sockfd);
			perror("connect\n");
			continue;
		}

		break;
	}

	if(!p)
	{
		perror("client: erreur de connexion\n");
		exit(EXIT_FAILURE);
	}

	freeaddrinfo(servinfo);
	GLOBAL_SOCKET = &sockfd;

	return sockfd;
}


/*
 * Lecture du fichier contenant la position des vaisseaux du joueur.
 *  - filename : le nom du fichier contenant la configuration
 *  - map : le tableau qui contiendra la carte lue sous forme de chaîne de caractères après lecture du fichier
 * Attention ! on lit simplement le contenu du fichier tel quel (les différentes lignes sont séparées par un '\n')
*/
void load_map(char *filename, char *map)
{
	FILE *file = fopen(filename, "r");
	if (file == NULL)
	{
		perror("fopen");
		exit(EXIT_FAILURE);
	}

	char c;
	int i = 0;
	while ((c = fgetc(file)) != EOF)
	{
		map[i] = c;
		i++;
	}
	map[i] = '\0';
	fclose(file);
}


/*
 * Fonction envoyant le placement des vaisseaux au serveur.
 * - s : le socket vers le serveur
 * - map : la carte lue dans le fichier par la fonction load_map
*/
void send_map(int s, char *map)
{
	int len = strlen(map);
	if (send(s, map, len, 0) == -1)
	{
		perror("send");
		exit(EXIT_FAILURE);
	}

	return;
}


/*
 * Fonction envoyant le coup (x,y) au serveur via le socket s
 */
void send_move(int s, uint8_t x, uint8_t y)
{
	char move[2];
	move[0] = x;
	move[1] = y;
	if (send(s, move, 2, 0) == -1)
	{
		perror("send");
		exit(EXIT_FAILURE);
	}

	return;
}


/*
 * Fonction de réception du résultat du coup et du status de la partie
 */
void receive_result(int s, char *res, char *status)
{
	char buffer[2];
	int len = recv(s, buffer, 2, 0);
	if (len == -1)
	{
		perror("recv");
		exit(EXIT_FAILURE);
	}
	*res = buffer[0];
	*status = buffer[1];

	return;
}


/*
 * Mise à jour de la carte et du nombre de vaisseaux restant en fonction du résultat du coup envoyé au serveur
*/
void play_move(uint8_t x, uint8_t y, char res)
{
	played[x][y] = res;
	if (res == TOUCHED)
		nb_to_destroy--;
}


void sigint_handler(int sig) {
    printf("\nServer shutting down...\n");
	close(*GLOBAL_SOCKET);
    exit(EXIT_SUCCESS);
}

/*
 * Fonction principale du client
 */
int main(int argc, char *argv[])
{

	struct sigaction sa;
	sa.sa_handler = sigint_handler;
	sa.sa_flags = 0;
	sigemptyset(&sa.sa_mask);
	sigaction(SIGINT, &sa, NULL);

	// test des arguments de la commande
	if (argc != 4)
	{
		printf("Usage : %s <server> <port> <map-filename>\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	// ouverture de la connexion vers le serveur
	int s = open_connection(argv[1], argv[2]);

	// lecture du fichier contenant les positions des vaisseaux
	char map[(MAP_WIDTH+1)*MAP_HEIGHT];
	load_map(argv[3], map);

	// envoi de la carte sous la forme d'un tableau de caractères
	send_map(s, map);

	// initialisation de la carte des coups joués (on met toutes les cases à la valeur EMPTY)
	memset(played, EMPTY, MAP_WIDTH*MAP_HEIGHT);

	// boucle principale
	char end = 0;
	while (!end)
	{
		// demande des coordonnées au joueur
		uint8_t x, y;
		printf("x = ");
		scanf("%hhu", &x);
		printf("y = ");
		scanf("%hhu", &y);

		// envoi des coordonnées au serveur sous forme de deux octets
		send_move(s, x, y);

		// récupération de la réponse du serveur (résultat du coup et status de la partie)
		// on récupère les deux octets l'un après l'autre dans la fonction receive_result
		char res, status;
		receive_result(s, &res, &status);

		// mise à jour de la carte des coups joués
		play_move(x, y, res);

		// affichage des informations (on affiche la carte des coups joués)
		print_map(played, nb_to_destroy);

		// détection de la fin de partie en fonction du status envoyé par le serveur
		if (status == WIN)
		{
			// il y a un vainqueur, mais c'est l'autre
			if (nb_to_destroy > 0)
				printf("Vous avez perdu...\n");
			// il y a un vainqueur, et c'est moi
			else
				printf("Vous avez gagné\n");
			end = 1;
		}
		// match nul
		else if (res == DRAW)
		{
			printf("Match nul !\n");
		}
	}

	return EXIT_SUCCESS;
}
