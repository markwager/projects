#ifndef PARTICELLE_H
#define PARTICELLE_H

#include "raylib.h"

// Definiamo qui il limite massimo così è accessibile ovunque
#define MAX_PARTICELLE 200

// Struttura per un singolo pezzetto di sangue/carne
typedef struct {
    float x, y;
    float vel_x, vel_y;
    int vita;        // Quanti frame dura prima di sparire o fermarsi
    Color colore;
    float dimensione;
    bool attiva;
} Particella;

// Firma della funzione
void GeneraSplatter(Particella array[], int max_part, float x, float y);

#endif