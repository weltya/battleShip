#include "battleship.h"

// constantes externes
extern const char SPLASH ;
extern const char TOUCHED;
extern const char EMPTY;
extern const char SHIP;

/*
 * Fonction d'affichage d'une carte. 
 * Que ce soit une carte de coups joués ou une carte représentant la position des vaisseaux. 
 */
void print_map(char map[MAP_WIDTH][MAP_HEIGHT], int n) 
{
	printf("    ");
	for (int i=0 ; i<MAP_WIDTH ; i++)
		printf("%d ", i);
	printf("\n");

	for (int j=0 ; j<MAP_HEIGHT ; j++)
	{
		printf("%d | ", j);
		for (int i=0 ; i<MAP_WIDTH ; i++)
		{
			if (map[i][j] == TOUCHED)
				printf("\033[31;01m%c\033[00m ", map[i][j]); 
			else
				printf("%c ", map[i][j]); 
		}
		printf("\n");
	}
	if (n>0) {
		printf("\n");
		printf("Nombre de vaisseaux restants : %d\n", n);
	}
	printf("-----------------------\n\n");
}