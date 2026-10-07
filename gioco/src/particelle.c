#include "../include/particelle.h"

void GeneraSplatter(Particella array[], int max_part, float x, float y) {
    int generate = 0;
    for (int i = 0; i < max_part; i++) {
        // Cerchiamo una particella attualmente inattiva da "riciclare"
        if (!array[i].attiva) {
            array[i].x = x + 15.0f; // Centriamo un po' l'esplosione
            array[i].y = y + 20.0f;
            
            // Sparati in direzioni casuali (da -5.0 a +5.0 pixel per frame)
            array[i].vel_x = (float)GetRandomValue(-50, 50) / 10.0f; 
            array[i].vel_y = (float)GetRandomValue(-50, 50) / 10.0f;
            
            array[i].vita = GetRandomValue(30, 80); // Durata casuale
            array[i].dimensione = (float)GetRandomValue(4, 12); // Grandezza del pezzo
            
            // Scegliamo casualmente se è un pezzo rosso vivo, rosso scuro o marroncino
            int tipo_colore = GetRandomValue(0, 2);
            if (tipo_colore == 0) array[i].colore = RED;
            else if (tipo_colore == 1) array[i].colore = MAROON;
            else array[i].colore = DARKBROWN;
            
            array[i].attiva = true;
            
            generate++;
            if (generate > 40) break; // Genera 40 pezzi per ogni esplosione
        }
    }
}