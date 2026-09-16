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

    // Crea fisicamente la finestra 
    InitWindow(larghezza_schermo, altezza_schermo, "Il mio primo Roguelike in C!");

    SetTargetFPS(60);

    Entita eroe=crea_player(400,300);

    // ========================================================================
    // 2. IL GAME LOOP INFINITO
    // ========================================================================
    // WindowShouldClose() diventa 'true' solo se premi ESC o la X della finestra
    while(!WindowShouldClose()){    
    
        // --- A. FASE DI INPUT & UPDATE (Fisica e Logica) ---
        StatoInput input_corrente=leggi_input_player();
        muovi_player(&eroe, input_corrente);


        // --- B. FASE DI DISEGNO (Draw) ---
        BeginDrawing(); // Diciamo alla scheda video: "Iniziamo a pitturare!"

            // 1. Pulisce lo schermo dal frame precedente (Fondamentale!)
            // Usiamo il nero, il colore classico dei dungeon
            ClearBackground(BLACK);
            
            DrawRectangle(eroe.x, eroe.y, 40, 40, YELLOW);

            // 2. Disegna un testo al centro dello schermo (X, Y, Dimensione, Colore)
            //DrawText("Benvenuto nel Dungeon!", 220, 280, 30, GREEN);
            
            //DrawText("Premi ESC per uscire.", 280, 330, 20, DARKGRAY);

        EndDrawing(); // Diciamo alla scheda video: "Ho finito, manda tutto a schermo!"
    }

    // ========================================================================
    // 3. CHIUSURA E PULIZIA (De-inizializzazione)
    // ========================================================================
    // Distrugge la finestra e libera la memoria RAM usata da Raylib
    CloseWindow(); 

}