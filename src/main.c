#include "raylib.h"

int main(void)
{
    // ========================================================================
    // 1. INIZIALIZZAZIONE (Setup) - Avviene una sola volta all'avvio
    // ========================================================================
    const int larghezza_schermo = 800;
    const int altezza_schermo = 600;

    // Crea fisicamente la finestra 
    InitWindow(larghezza_schermo, altezza_schermo, "Il mio primo Roguelike in C!");

    // Diciamo al gioco di girare a un massimo di 60 Frame al Secondo (FPS)
    SetTargetFPS(60); 

    // ========================================================================
    // 2. IL GAME LOOP INFINITO
    // ========================================================================
    // WindowShouldClose() diventa 'true' solo se premi ESC o la X della finestra
    while(!WindowShouldClose()){    
    
        // --- A. FASE DI INPUT & UPDATE (Fisica e Logica) ---
        // (Qui in futuro richiamerai: leggi_input(); muovi_eroe(); ecc.)


        // --- B. FASE DI DISEGNO (Draw) ---
        BeginDrawing(); // Diciamo alla scheda video: "Iniziamo a pitturare!"

            // 1. Pulisce lo schermo dal frame precedente (Fondamentale!)
            // Usiamo il nero, il colore classico dei dungeon
            ClearBackground(BLACK); 

            // 2. Disegna un testo al centro dello schermo (X, Y, Dimensione, Colore)
            DrawText("Benvenuto nel Dungeon!", 220, 280, 30, GREEN);
            
            DrawText("Premi ESC per uscire.", 280, 330, 20, DARKGRAY);

        EndDrawing(); // Diciamo alla scheda video: "Ho finito, manda tutto a schermo!"
    }

    // ========================================================================
    // 3. CHIUSURA E PULIZIA (De-inizializzazione)
    // ========================================================================
    // Distrugge la finestra e libera la memoria RAM usata da Raylib
    CloseWindow(); 

    return 0;
}