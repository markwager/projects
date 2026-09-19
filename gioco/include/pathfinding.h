#ifndef PATHFINDING_H
#define PATHFINDING_H

#include "raylib.h"
#include "entita.h"

#define COLONNE 20
#define RIGHE 20

Vector2 calcola_prossimo_passo(Nemico mostro, Entita eroe, int mappa[RIGHE][COLONNE], int dim_tile);

#endif