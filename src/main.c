#include "raylib.h"
#include <stdbool.h>
#include "../include/input.h"
#include "../include/entita.h"

#define COLONNE 20
#define RIGHE 20

bool check_coll_map(Rectangle hitb_player, int mappa[RIGHE][COLONNE], int dim_tile); //controlla collisioni per movimento in orizz e vertic

bool check_coll_map(Rectangle hitb_player, int mappa[RIGHE][COLONNE], int dim_tile){
            for(int i=0; i<RIGHE; i++){ //righe
                for(int j=0; j<COLONNE; j++){ //colonne
                    int pos_j=j*dim_tile;
                    int pos_i=i*dim_tile;

                    int tipo_cella=mappa[i][j];

                    if(tipo_cella==1){
                        Rectangle hitb_wall={pos_j, pos_i, dim_tile, dim_tile}; //hitbox muro

                        if(CheckCollisionRecs(hitb_player, hitb_wall)){
                        //scontro rilevato!
                            return true;
                        }

                    }
                }
            }
            return false;
}

int main(void){
    // ========================================================================
    // 1. INIZIALIZZAZIONE (Setup) - Avviene una sola volta all'avvio
    // ========================================================================
    const int larghezza_schermo = 800;
    const int altezza_schermo = 600;

    //Crea fisicamente la finestra 
    InitWindow(larghezza_schermo, altezza_schermo, "Il mio primo Roguelike in C!");
    SetTargetFPS(60);

    //creazione pg con suo sprite
    Entita eroe=crea_player(100,100);
    Texture2D eroe_sprite=LoadTexture("assets/Hero.png");
    //adattamento del personaggio alle dim del suo sprite
    int colonne_sprite_eroe=9; //num frame per ogni animazione
    int righe_sprite_eroe=4;   //num animazioni (su, giù, ecc.)
    eroe.height=eroe_sprite.height/righe_sprite_eroe; //=64
    eroe.width=eroe_sprite.width/colonne_sprite_eroe; //=64
    Rectangle frame_rec={0.0f, 0.0f, (float)eroe.width, (float)eroe.height}; //mirino spritesheet

    int frame_corrente=0; //tiene traccia di quale tra gli 8 disegnini stiamo vedendo
    int count_frame=0; //cronometro per rallentare animazione
    int vel_anim=8; //quanti frame al secondio voglio vedere

    int mappa[RIGHE][COLONNE]={
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
        {1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,1},
        {1,0,0,0,0,0,0,0,0,1,0,0,0,1,1,1,0,0,0,1},
        {1,0,0,0,0,1,1,0,0,0,0,0,0,0,0,1,0,0,0,1},
        {1,0,0,0,0,1,1,0,0,0,0,0,0,0,0,1,0,0,0,1},
        {1,0,1,1,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,1},
        {1,0,0,1,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,1},
        {1,0,0,1,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,1},
        {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
        {1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
        {1,0,0,0,0,0,0,0,0,1,1,1,1,1,0,0,0,0,0,1},
        {1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,1},
        {1,0,0,0,1,1,0,0,0,1,0,0,0,0,0,0,0,0,0,1},
        {1,0,0,0,1,1,0,0,0,1,0,0,0,0,1,1,1,0,0,1},
        {1,0,0,0,0,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1},
        {1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,1},
        {1,0,0,1,1,1,1,0,0,0,0,0,0,0,1,1,1,0,0,1},
        {1,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,1},
        {1,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,1},
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}
    }; //0 è il pavimento, 1 il muro
    int dim_tile=64;

    Camera2D eroe_cam={0};
    eroe_cam.offset=(Vector2){larghezza_schermo/2.0f, altezza_schermo/2.0f}; //punto in cui camera fissa l'obiettivo
    eroe_cam.rotation=0.0f;
    eroe_cam.zoom=1.0f;

    //tileset mappa
    int col_tileset=114; //colonne
    Texture2D tileset = LoadTexture("assets/tiles_map.png");
    float vera_larghezza_tile = (float)tileset.width / col_tileset;
    float vera_altezza_tile = (float)tileset.height;

    // ========================================================================
    // 2. IL GAME LOOP INFINITO
    // ========================================================================
    //WindowShouldClose() diventa 'true' solo se premi ESC o la X della finestra
    while(!WindowShouldClose()){    
    
        // --- A. FASE DI INPUT & UPDATE (Fisica e Logica) ---
        StatoInput input_corrente=leggi_input_player();
        float prev_x=eroe.x; //salva pos x precedente
        muovi_player_x(&eroe, input_corrente);
        float margine_x=12.0f; //tagliamo via l'aria trasparente a destra e sinistra
        float margine_y=20.0f; //ignoriamo la testa e le spalle
        Rectangle hitbox_x={eroe.x+margine_x, eroe.y+margine_y, (float)eroe.width-(margine_x*2), (float)eroe.height-margine_y}; //rimpiccioliamo hitbox player
        if(check_coll_map(hitbox_x, mappa, dim_tile)){
            eroe.x=prev_x; //Annulla il movimento rimettendo le vecchie coordinate
        }

        float prev_y=eroe.y;
        muovi_player_y(&eroe, input_corrente);
        Rectangle hitbox_y={eroe.x+margine_x, eroe.y+margine_y, (float)eroe.width-(margine_x*2), (float)eroe.height-margine_y};
        if(check_coll_map(hitbox_y, mappa, dim_tile)){
            eroe.y=prev_y;
        }

        if(input_corrente.up || input_corrente.down || input_corrente.sx || input_corrente.dx){
            count_frame+=1;
        }

        else{
            frame_corrente=0;
            count_frame=0;
        }

        if(count_frame>(60/vel_anim)){
            count_frame=0;
            frame_corrente+=1;
        }

        if(frame_corrente>8){
            frame_corrente=0;
        }

        frame_rec.x =(float)frame_corrente*(float)eroe.width;

        if(input_corrente.up){
            frame_rec.y =0.0f*(float)eroe.height;

        }

        if(input_corrente.down){
            frame_rec.y =2.0f*(float)eroe.height;

        }

        if(input_corrente.sx){
            frame_rec.y =1.0f*(float)eroe.height;

        }

        if(input_corrente.dx){
            frame_rec.y =3.0f*(float)eroe.height;

        }

        eroe_cam.target=(Vector2){eroe.x+(eroe.width/2.0f), eroe.y+(eroe.height/2.0f)};

        // --- B. FASE DI DISEGNO (Draw) ---
        BeginDrawing(); //Iniziamo a disegnare

            // 1. Pulisce lo schermo dal frame precedente (Fondamentale!)
            // Usiamo il nero
            ClearBackground(BLACK);

            BeginMode2D(eroe_cam);
            for(int i=0; i<RIGHE; i++){ //righe
                for(int j=0; j<COLONNE; j++){ //colonne
                    int pos_j=j*dim_tile;
                    int pos_i=i*dim_tile;

                    int tipo_cella=mappa[i][j];

                    //usiamo il numero letto dalla mappa per spostare la X del ritaglio
                    float posizione_ritaglio_x = (float)tipo_cella * vera_larghezza_tile;
                    //UNICO Rectangle che si sposta da solo
                    Rectangle mirino_dinamico = {posizione_ritaglio_x, 0.0f, vera_larghezza_tile, vera_altezza_tile};
                    //UNICA riga per disegnare TUTTI i blocchi
                    DrawTextureRec(tileset, mirino_dinamico, (Vector2){pos_j, pos_i}, WHITE);
                    
                }
            }
            
            DrawTextureRec(eroe_sprite, frame_rec, (Vector2){eroe.x, eroe.y}, WHITE); //ritaglia immagine spritesheet
            EndMode2D();

            // 2. Disegna un testo al centro dello schermo (X, Y, Dimensione, Colore)
            //DrawText("Benvenuto nel Dungeon!", 220, 280, 30, GREEN);
            
            //DrawText("Premi ESC per uscire.", 280, 330, 20, DARKGRAY);

        EndDrawing(); //Manda tutto a schermo
    }

    UnloadTexture(eroe_sprite);
    UnloadTexture(tileset);

    // ========================================================================
    // 3. CHIUSURA E PULIZIA (De-inizializzazione)
    // ========================================================================
    //Chiudi la finestra
    CloseWindow(); 

}