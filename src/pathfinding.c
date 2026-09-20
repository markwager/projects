#include "../include/pathfinding.h"
#include <stdbool.h>
#include <math.h> //serve per fabs()

Vector2 calcola_prossimo_passo(Nemico mostro, Entita eroe, int mappa[RIGHE][COLONNE], int dim_tile){
    //1. CALCOLO SUI PIEDI: Usiamo la base dello sprite per non finire mai dentro i muri (mappatura pixel-matrice)
    int start_c = (int)(mostro.x + mostro.width / 2.0f) / dim_tile;
    int start_r = (int)(mostro.y + mostro.height - 5.0f) / dim_tile; // -5px per stare sicuri sul pavimento
    
    int target_c = (int)(eroe.x + eroe.width / 2.0f) / dim_tile;
    int target_r = (int)(eroe.y + eroe.height - 5.0f) / dim_tile;

    if (start_r == target_r && start_c == target_c) {
        return (Vector2){eroe.x, eroe.y};
    }

    //preparazione della memoria

    bool visited[RIGHE][COLONNE] = {false}; //caselle visitate
    int parent_r[RIGHE][COLONNE];
    int parent_c[RIGHE][COLONNE]; //traccia stile pollicino: annotazione da quale cella si proviene

    //pulizia rigorosa della memoria (Evita percorsi fantasma)
    for(int i = 0; i < RIGHE; i++){
        for(int j = 0; j < COLONNE; j++){
            parent_r[i][j] = -1;
            parent_c[i][j] = -1;
        }
    }

    int queue_r[1000]; //coda aumentata per massima sicurezza: sarebbe coda di attesa della BFS
    int queue_c[1000];
    int head = 0, tail = 0;

    queue_r[tail] = start_r;
    queue_c[tail] = start_c;
    tail++;
    visited[start_r][start_c] = true;

    int dr[] = {-1, 1, 0, 0}; 
    int dc[] = {0, 0, -1, 1};
    bool found = false;

    // 2. BFS
    while (head < tail) {
        int curr_r = queue_r[head];
        int curr_c = queue_c[head];
        head++;

        if (curr_r == target_r && curr_c == target_c) {
            found = true;
            break;
        }

        for (int i = 0; i < 4; i++) {
            int nr = curr_r + dr[i];
            int nc = curr_c + dc[i];

            if (nr >= 0 && nr < RIGHE && nc >= 0 && nc < COLONNE) {
                if (mappa[nr][nc] != 1 && !visited[nr][nc]) { 
                    visited[nr][nc] = true;
                    parent_r[nr][nc] = curr_r; 
                    parent_c[nr][nc] = curr_c;
                    
                    if (tail < 1000) {
                        queue_r[tail] = nr;
                        queue_c[tail] = nc;
                        tail++;
                    }
                }
            }
        }
    }

    if (!found) return (Vector2){eroe.x, eroe.y};

    // 3. RICOSTRUZIONE DEL PERCORSO: percorso a ritroso
    int curr_r = target_r;
    int curr_c = target_c;

    while (parent_r[curr_r][curr_c] != start_r || parent_c[curr_r][curr_c] != start_c) {
        int pr = parent_r[curr_r][curr_c];
        int pc = parent_c[curr_r][curr_c];
        curr_r = pr;
        curr_c = pc;
    }

    float next_x = (curr_c * dim_tile) + (dim_tile / 2.0f) - (mostro.width / 2.0f);
    float next_y = (curr_r * dim_tile) + (dim_tile / 2.0f) - (mostro.height / 2.0f);

    return (Vector2){next_x, next_y};
}