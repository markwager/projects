#include "../include/entita.h"

Entita crea_player(int start_x, int start_y){
    Entita giocatore;
    giocatore.x=start_x;
    giocatore.y=start_y;
    giocatore.vel=5;
    giocatore.hp=100;
    return giocatore;
}

void muovi_player(Entita *player, StatoInput input){
    if(input.up)
        player->y-=player->vel;
    
    if(input.down)
        player->y+=player->vel;

    if(input.sx)
        player->x-=player->vel;

    if(input.dx)
        player->x+=player->vel;
    
}