#ifndef __BATTLESHIP_H__
#define __BATTLESHIP_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <pthread.h>

// structure représentant un coup joué (simplement deux coordonnées x et y sous forme d'un entier non signé sur 8 octets)
struct move {
    uint8_t x;
    uint8_t y;
};

// paramètres du jeu
#define MAP_WIDTH  8
#define MAP_HEIGHT 8
#define NB_TO_DESTROY 8

// valeurs pour le status d'une partie
#define WIN     0
#define DRAW    1
#define ONGOING -1

// fonction d'affichage d'une carte
void print_map(char map[MAP_WIDTH][MAP_HEIGHT], int n);

#endif
