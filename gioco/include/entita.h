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

typedef struct {
    float x;
    float y;
    float width;
    float height;
    bool attivo; //se è true l'oggetto esiste. Se è false, è stato raccolto e scompare.
    int tipo; //distinguere tra chiavi e pozioni in futuro
} Oggetto;

typedef struct {
    float x;
    float y;
    float width;
    float height;
    bool attivo;
} Nemico;

Entita crea_player(int start_x, int start_y); //restituisce struct player

//per i movimenti indipendenti rendendoli più fluidi
void muovi_player_x(Entita *player, StatoInput input); 
void muovi_player_y(Entita *player, StatoInput input);

#endif