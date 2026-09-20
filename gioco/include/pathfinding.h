#ifndef PATHFINDING_H
#define PATHFINDING_H

#include "raylib.h"
#include "entita.h"

#define COLONNE 20
#define RIGHE 20

Vector2 calcola_prossimo_passo(Nemico mostro, Entita eroe, int mappa[RIGHE][COLONNE], int dim_tile);
bool check_coll_map(Rectangle hitb_player, int mappa[RIGHE][COLONNE], int dim_tile); //controlla collisioni per movimento in orizz e vertic nella mappa

#endif