#include "raylib.h"
#include "../include/input.h"
#include "../include/entita.h"

int main(void)
{
    // ========================================================================
    // 1. INIZIALIZZAZIONE (Setup) - Avviene una sola volta all'avvio
    // ========================================================================
    const int larghezza_schermo = 800;
    const int altezza_schermo = 600;

    //Crea fisicamente la finestra 
    InitWindow(larghezza_schermo, altezza_schermo, "Il mio primo Roguelike in C!");

    SetTargetFPS(60);

    Entita eroe=crea_player(400,300);

    Texture2D eroe_sprite=LoadTexture("assets/Hero.png");

    //adattamento del personaggio alle dim del suo sprite
    eroe.height=eroe_sprite.height/3;
    eroe.width=eroe_sprite.width/8;

    Rectangle frame_rec={0.0f, 0.0f, (float)eroe.width, (float)eroe.height};

    int frame_corrente=0; //tiene traccia di quale tra gli 8 disegnini stamo vedendo
    int count_frame=0; //cronometro per rallentare animazione
    int vel_anim=8; //quanti frame al secondio voglio vedere



    // ========================================================================
    // 2. IL GAME LOOP INFINITO
    // ========================================================================
    //WindowShouldClose() diventa 'true' solo se premi ESC o la X della finestra
    while(!WindowShouldClose()){    
    
        // --- A. FASE DI INPUT & UPDATE (Fisica e Logica) ---
        StatoInput input_corrente=leggi_input_player();
        muovi_player(&eroe, input_corrente);

        count_frame+=1;
        if(count_frame>(60/vel_anim)){
            count_frame=0;
            frame_corrente+=1;
        }

        if(frame_corrente>7){
            frame_corrente=0;
        }

        frame_rec.x =(float)frame_corrente*(float)eroe.width;

        // --- B. FASE DI DISEGNO (Draw) ---
        BeginDrawing(); // Diciamo alla scheda video: "Iniziamo a pitturare!"

            // 1. Pulisce lo schermo dal frame precedente (Fondamentale!)
            // Usiamo il nero
            ClearBackground(BLACK);
            
            DrawTextureRec(eroe_sprite, frame_rec, (Vector2){eroe.x, eroe.y}, WHITE);            // 2. Disegna un testo al centro dello schermo (X, Y, Dimensione, Colore)
            DrawText("Benvenuto nel Dungeon!", 220, 280, 30, GREEN);
            
            DrawText("Premi ESC per uscire.", 280, 330, 20, DARKGRAY);

        EndDrawing(); // Diciamo alla scheda video: "Ho finito, manda tutto a schermo!"
    }

    UnloadTexture(eroe_sprite);

    // ========================================================================
    // 3. CHIUSURA E PULIZIA (De-inizializzazione)
    // ========================================================================
    //Chiudi la finestra
    CloseWindow(); 

}