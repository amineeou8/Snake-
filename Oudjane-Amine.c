// compiler's header files
#include <stdbool.h> // bool, true, false
#include <stdlib.h> // rand
#include <stdio.h> // printf
#include <math.h> // Pour abs

// main program's header file
#include "snake_def.h"
#include "snake_dec.h"

// student name goes here
char *student = "Amine Oudjane A.K.A ClarkKent";


// Seuil de taille : on passe en strategie Hamiltonien quand on dépasse cette taille
#define SEUIL 25


// =======================================================
// MODULE : calcul de la taille du snake
// =======================================================
/*
  STRATÉGIE :
  Ce module va permettre de déterminer la taille actuelle du snake.
  Le snake étant représenté par une liste chaînée, la strategie va 
  consister à parcourir cette liste segment par segment et à compter
  le nombre total de maillons.

  c'est gr^ace à cette info qu'on pourra décider quand changer
  de strategie de déplacement.
*/
int avoir_snake_size(snake_list s) {
    int size = 0;

    // Parcours complet de la liste chaînée du snake
    while (s != NULL) {
        size++;        // On compte un segment
        s = s->next;   // Passage au segment suivant
    }

    return size;       // Taille totale du serpent
}


// =======================================================
// MODULE : distance de Manhattan
// =======================================================
/*
  STRATÉGIE :
  Ce module va calculer la distance de Manhattan entre deux positions.
  Cette distance correspond au nombre minimal de déplacements
  horizontaux et verticaux nécessaires pour relier deux cases
  car on ne peut pas aller en diagonale. Donc c'est bien adapté au jeu snake.

*/
int avoir_manhattan_dist(int x1, int y1, int x2, int y2) {
    // Somme des distances horizontale et verticale
    return abs(x1 - x2) + abs(y1 - y2);
}


// =======================================================
// MODULE : recherche du bonus
// =======================================================
/*
  STRATÉGIE :
  Ce module a pour but de localiser le bonus sur la carte.
  La stratégie va consister à parcourir toute la carte ligne par ligne
  jusqu'à trouver une case contenant un BONUS.
  Dès que le bonus est trouvé, ses coordonnées sont retournées
  via des pointeurs.
*/
void trouver_bonus(char **map, int mapxsize, int mapysize, int *bx, int *by) {
    // Initialisation : aucun bonus trouvé
    *bx = -1;
    *by = -1;

    // Parcours de la carte (y = lignes, x = colonnes)
    for (int y = 0; y < mapysize; y++) {
        for (int x = 0; x < mapxsize; x++) {

            // Si la case contient un bonus
            if (map[y][x] == BONUS) {
                *bx = x;   // Coordonnée x du bonus
                *by = y;   // Coordonnée y du bonus
                return;    // Arrêt immédiat de la recherche
            }
        }
    }
}


// =======================================================
// MODULE : vérification de sécurité d’une case
// =======================================================
/*
  STRATÉGIE :
  Ce module va vérifier si une case est sûre pour le déplacement du snake.
  Une case est considérée comme sûre si :
   - elle est dans les limites de la carte
   - elle ne contient ni mur, ni corps, ni queue du snake

  Cette fonction est utilisée avant chaque déplacement potentiel
  afin d'éviter toute collision.
*/
bool est_safe(char **map, int x, int y, int mapxsize, int mapysize) {

    // Vérification des bornes de la carte
    if (x < 0 || x >= mapxsize || y < 0 || y >= mapysize)
        return false;

    // Lecture du contenu de la case
    char c = map[y][x];

    // Interdiction des obstacles
    if (c == WALL || c == SNAKE_BODY || c == SNAKE_TAIL)
        return false;

    // La case est sûre
    return true;
}


// =======================================================
// STRATÉGIE 1 : CHASSE AU BONUS (Manhattan)
// =======================================================
/*
  STRATÉGIE :
  Cette stratégie est utilisée lorsque le snake est encore petit.
  L'objectif est de grandir rapidement en se rapprochant du bonus.

  On va donc tester les quatre directions possibles et on choisit
  celle qui minimise la distance de Manhattan jusqu'au bonus,
  tout en garantissant la sécurité du déplacement.
*/
action strategie_manhattan(char **map, int mapxsize, int mapysize, snake_list s) {

    // Position actuelle de la tête
    int hx = s->x;
    int hy = s->y;

    // Position du bonus
    int bx, by;
    trouver_bonus(map, mapxsize, mapysize, &bx, &by);

    // Aucun bonus trouvé
    if (bx == -1)
        return EAST;

    action best_action = -1;
    int min_dist = 100000;

    // Déplacements possibles
    int dx[4] = {0, 1, 0, -1};
    int dy[4] = {-1, 0, 1, 0};
    action acts[4] = {NORTH, EAST, SOUTH, WEST};

    // Test des quatre directions
    for (int i = 0; i < 4; i++) {
        int nx = hx + dx[i];
        int ny = hy + dy[i];

        // Vérification de la sécurité
        if (est_safe(map, nx, ny, mapxsize, mapysize)) {
            int d = avoir_manhattan_dist(nx, ny, bx, by);

            // On conserve le mouvement qui rapproche le plus du bonus
            if (d < min_dist) {
                min_dist = d;
                best_action = acts[i];
            }
        }
    }

    // Si une direction optimale existe
    if (best_action != -1)
        return best_action;

    // Sinon, on prend n'importe quelle direction sûre
    for (int i = 0; i < 4; i++) {
        if (est_safe(map, hx + dx[i], hy + dy[i], mapxsize, mapysize))
            return acts[i];
    }

    // Cas extrême : aucun coup sûr (coup dur aie)
    return NORTH;
}


// =======================================================
// STRATÉGIE 2 : CIRCUIT HAMILTONIEN (Zig-Zag sécurisé)
// =======================================================
/*
  STRATÉGIE :
  Cette stratégie est utilisée lorsque le snake devient long.
  L'objectif n'est plus la vitesse mais la survie car il a tendance 
  à s'enfermer et se croquer lorsque'il est trop long.

  Le snake suit un parcours en zig-zag couvrant toute la carte,
  garantissant qu'il existera toujours un chemin vers sa queue,
  ce qui évitera toute collision.
*/
action strategie_hamilton(int hx, int hy, int mapxsize, int mapysize) {

    int max_x = mapxsize - 1;
    int max_y = mapysize - 1;

    // Colonne de retour sécurisée
    if (hx == 1) {
        if (hy == 1) return EAST;
        return NORTH;
    }

    // Ligne supérieure
    if (hy == 1) {
        if (hx < max_x - 1) return EAST;
        return SOUTH;
    }

    // Zig-zag vertical
    if (hy % 2 == 0) {
        if (hx == 2) {
            if (hy == max_y - 1) return WEST;
            return SOUTH;
        }
        return WEST;
    } else {
        if (hx == max_x - 1) return SOUTH;
        return EAST;
    }
}


// =======================================================
// MODULE PRINCIPAL : IA DU SNAKE
// =======================================================
/*
  STRATÉGIE GLOBALE :
  Le comportement du snake repose sur deux stratégies distinctes :
   - une stratégie agressive (Manhattan) en début de partie
   - une stratégie sécurisée (Hamiltonienne) lorsque le snake est long

  Le choix de la stratégie dépend uniquement de la taille du snake.
*/
action snake(char **map, int mapxsize, int mapysize,
             snake_list s, action last_action) {

    int size = avoir_snake_size(s);

    // Choix dynamique de la stratégie
    if (size < SEUIL)
        return strategie_manhattan(map, mapxsize, mapysize, s);
    else
        return strategie_hamilton(s->x, s->y, mapxsize, mapysize);
}