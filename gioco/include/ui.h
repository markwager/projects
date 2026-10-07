#ifndef UI_H
#define UI_H

#include "raylib.h"
#include <stdbool.h>

// 1. Gli Stati del Gioco
typedef enum {
    STATO_MENU,
    STATO_OPZIONI,
    STATO_GIOCO,
    STATO_PAUSA
} StatoGioco;

// 2. Le Impostazioni
typedef struct {
    float volume;
    int larghezza_schermo;
    int altezza_schermo;
} Impostazioni;

// 3. Dichiarazione della funzione del bottone
bool DisegnaBottone(Rectangle rect, const char* testo, Color color_base);

#endif