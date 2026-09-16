#ifndef ENTITA_H
#define ENTITA_H

#include "input.h" //per sapere che tatsi sono stati premuti

typedef struct{
    int x;
    int y;
    int vel;
    int hp;
    int width;
    int height;
} Entita;

Entita crea_player(int start_x, int start_y); //restituisce struct player
void muovi_player(Entita *player, StatoInput input);

#endif