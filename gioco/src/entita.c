#include "../include/entita.h"
#include <raylib.h>

Entita crea_player(int start_x, int start_y){
    Entita giocatore;
    giocatore.x = start_x;
    giocatore.y = start_y;
    giocatore.vel = 5;
    giocatore.hp = 100;    // Vita alta
    giocatore.width = 40;
    giocatore.height = 40;
    giocatore.iframes = 0; // Inizia senza invulnerabilità
    return giocatore;
}

// NUOVA: Costruttore del Mostro
Nemico crea_mostro(int colonna, int riga, int dim_tile){
    Nemico m;
    m.x = colonna * dim_tile; 
    m.y = riga * dim_tile;
    m.hp = 3;             // Bastano 3 colpi per ucciderlo
    m.attivo = true;
    m.iframes = 0;
    // La larghezza e altezza le calcoleremo nel main 
    // dividendo lo spritesheet, quindi per ora le mettiamo a 0
    m.width = 0; 
    m.height = 0;
    return m;
}

void muovi_player_x(Entita *player, StatoInput input){
    if(input.sx){
        player->x -= player->vel;
    }
    if(input.dx){
        player->x += player->vel;
    }
}

void muovi_player_y(Entita *player, StatoInput input){
    if(input.up){
        player->y -= player->vel;
    }
    if(input.down){
        player->y += player->vel;
    }
}