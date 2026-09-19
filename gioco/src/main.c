#include "raylib.h"
#include <stdbool.h>
#include "../include/input.h"
#include "../include/entita.h"
#include "../include/pathfinding.h"
#include <math.h>

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
        {1,110,110,110,110,110,110,110,110,1,110,110,110,110,110,110,110,110,110,1},
        {1,110,110,110,110,110,110,110,110,1,110,110,110,1,1,1,110,110,110,1},
        {1,110,110,110,110,1,1,110,110,110,110,110,110,110,110,1,110,110,110,1},
        {1,110,110,110,110,1,1,110,110,110,110,110,110,110,110,1,110,110,110,1},
        {1,110,1,1,110,110,110,110,110,110,110,110,110,110,110,1,110,110,110,1},
        {1,110,110,1,110,110,110,110,110,110,110,1,1,110,110,110,110,110,110,1},
        {1,110,110,1,110,110,110,110,110,110,110,1,1,110,110,110,110,110,110,1},
        {1,110,110,110,110,110,110,110,110,110,110,110,110,110,110,110,110,110,110,1},
        {1,1,1,1,1,110,110,110,110,110,110,110,110,110,110,110,110,110,110,1},
        {1,110,110,110,110,110,110,110,110,1,1,1,1,1,110,110,110,110,110,1},
        {1,110,110,110,110,110,110,110,110,1,110,110,110,110,110,110,110,110,110,1},
        {1,110,110,110,1,1,110,110,110,1,110,110,110,110,110,110,110,110,110,1},
        {1,110,110,110,1,1,110,110,110,1,110,110,110,110,1,1,1,110,110,1},
        {1,110,110,110,110,110,110,110,110,1,110,110,110,110,1,110,110,110,110,1},
        {1,110,110,110,110,110,110,110,110,110,110,110,110,110,1,110,110,110,110,1},
        {1,110,110,1,1,1,1,110,110,110,110,110,110,110,1,1,1,110,110,1},
        {1,110,110,110,110,110,1,110,110,110,110,110,110,110,110,110,110,110,110,1},
        {1,110,110,110,110,110,1,110,110,110,110,110,110,110,110,110,110,110,110,1},
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}
    }; //110 è il pavimento, 1 il muro
    int dim_tile=64;
    //tileset mappa
    int col_tileset=114; //colonne
    Texture2D tileset = LoadTexture("assets/tiles_map.png");
    float vera_larghezza_tile = (float)tileset.width / col_tileset;
    float vera_altezza_tile = (float)tileset.height;

    //camera eroe
    Camera2D eroe_cam={0};
    eroe_cam.offset=(Vector2){larghezza_schermo/2.0f, altezza_schermo/2.0f}; //punto in cui camera fissa l'obiettivo
    eroe_cam.rotation=0.0f;
    eroe_cam.zoom=1.0f;

    //oggetti
    Texture2D oggetti_sprite=LoadTexture("assets/objects.png");
    int dim_ogg=64;
    const int NUM_OGGETTI=4;
    Oggetto lista_oggetti[NUM_OGGETTI];

    lista_oggetti[0].x=4*dim_tile; //riga 5 in mappa
    lista_oggetti[0].y=3*dim_tile; //colonna 3 in mappa
    lista_oggetti[0].width=dim_ogg;
    lista_oggetti[0].height=dim_ogg;
    lista_oggetti[0].attivo=true;

    lista_oggetti[1].x=10*dim_tile; 
    lista_oggetti[1].y=11*dim_tile; 
    lista_oggetti[1].width=dim_ogg;
    lista_oggetti[1].height=dim_ogg;
    lista_oggetti[1].attivo=true;

    lista_oggetti[2].x=6*dim_tile; 
    lista_oggetti[2].y=7*dim_tile; 
    lista_oggetti[2].width=dim_ogg;
    lista_oggetti[2].height=dim_ogg;
    lista_oggetti[2].attivo=true;

    lista_oggetti[3].x=9*dim_tile; 
    lista_oggetti[3].y=5*dim_tile; 
    lista_oggetti[3].width=dim_ogg;
    lista_oggetti[3].height=dim_ogg;
    lista_oggetti[3].attivo=true;

    int punteggio=0;

    //nemico
    Texture2D mostro_sprite = LoadTexture("assets/enemy.png"); 
    Nemico mostro;
    // Adatta questi valori a quanti disegnini ci sono nel tuo file (es. se ha 4 colonne e 1 riga)
    int colonne_sprite_mostro=7; 
    int righe_sprite_mostro=4;
    mostro.width = mostro_sprite.width/colonne_sprite_mostro;
    mostro.height = mostro_sprite.height/righe_sprite_mostro;
    
    mostro.x = 14 * dim_tile; // Lo facciamo nascere lontano (es. Colonna 14)
    mostro.y = 12 * dim_tile; // Riga 12
    mostro.attivo = true;

    // Mirino per l'animazione del mostro
    Rectangle frame_rec_mostro = {0.0f, 0.0f, (float)mostro.width, (float)mostro.height};
    int frame_corrente_mostro = 0;
    int count_frame_mostro = 0;
    int direzione_mostro = 0; // Memorizza dove sta guardando

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

        Rectangle rect_eroe = {
            eroe.x + margine_x, 
            eroe.y + margine_y, 
            (float)eroe.width - (margine_x * 2), 
            (float)eroe.height - margine_y
        };

       // --- LOGICA E FISICA DEL NEMICO (Movimento Fluido e Anti-Sfarfallio) ---
        if(mostro.attivo){
            float vel_mostro = 1.5f; 
            float prev_m_x = mostro.x;
            float prev_m_y = mostro.y;
            bool in_movimento = false;
            
            // Hitbox molto ridotta per scivolare dolcemente attorno agli spigoli
            float margine_m_x = 18.0f; 
            float margine_m_y = 20.0f;

            // 1. IL CERVELLO CALCOLA: qual è il centro della prossima casella utile?
            Vector2 obiettivo = calcola_prossimo_passo(mostro, eroe, mappa, dim_tile);

            // 2. MOVIMENTO FLUIDO (Diagonali permesse, fine dei tremolii!)
            float dist_x = obiettivo.x - mostro.x;
            float dist_y = obiettivo.y - mostro.y;

            // Movimento su X
            if (fabs(dist_x) > vel_mostro) { // Se è lontano, fai un passo
                mostro.x += (dist_x > 0) ? vel_mostro : -vel_mostro;
                in_movimento = true;
            } else {
                mostro.x = obiettivo.x; // FRENO ANTI-VIBRAZIONE: se è vicino, allineati perfettamente
            }
            
            // Controllo muri su X per scivolare
            Rectangle hitbox_m_x = {mostro.x + margine_m_x, mostro.y + margine_m_y, mostro.width - (margine_m_x * 2), mostro.height - margine_m_y};
            if (check_coll_map(hitbox_m_x, mappa, dim_tile)) mostro.x = prev_m_x;

            // Movimento su Y
            if (fabs(dist_y) > vel_mostro) {
                mostro.y += (dist_y > 0) ? vel_mostro : -vel_mostro;
                in_movimento = true;
            } else {
                mostro.y = obiettivo.y; // FRENO ANTI-VIBRAZIONE
            }
            
            // Controllo muri su Y per scivolare
            Rectangle hitbox_m_y = {mostro.x + margine_m_x, mostro.y + margine_m_y, mostro.width - (margine_m_x * 2), mostro.height - margine_m_y};
            if (check_coll_map(hitbox_m_y, mappa, dim_tile)) mostro.y = prev_m_y;

            // 3. ANIMAZIONE DOMINANTE (Elimina lo sfarfallio)
            if (in_movimento) {
                // Il mostro guarda fisso nell'asse dove sta camminando di più
                if (fabs(dist_x) > fabs(dist_y)) {
                    direzione_mostro = (dist_x > 0) ? 3 : 1; // 3=Destra, 1=Sinistra
                } else {
                    direzione_mostro = (dist_y > 0) ? 2 : 0; // 2=Basso, 0=Alto
                }

                count_frame_mostro++;
                if (count_frame_mostro > (60 / vel_anim)) {
                    count_frame_mostro = 0;
                    frame_corrente_mostro++;
                    if (frame_corrente_mostro >= colonne_sprite_mostro) frame_corrente_mostro = 0;
                }
            } else {
                frame_corrente_mostro = 0; 
            }
            
            frame_rec_mostro.x = (float)frame_corrente_mostro * (float)mostro.width;
            frame_rec_mostro.y = (float)direzione_mostro * (float)mostro.height;

            // 4. IL MORSO
            Rectangle rect_mostro = {mostro.x + margine_m_x, mostro.y + margine_m_y, mostro.width - (margine_m_x * 2), mostro.height - margine_m_y};
            if (CheckCollisionRecs(rect_eroe, rect_mostro)) {
                eroe.x = 100;
                eroe.y = 100;
            }
        }

        int dim_ogg_fisico=16; //quanto è grande l'oggetto fisicamente
        float offset_fisico=(dim_tile-dim_ogg_fisico)/2.0f; //calcolo per centrarlo

        for (int i = 0; i < NUM_OGGETTI; i++) {
            //controlla se l'oggetto è ancora attivo
            if (lista_oggetti[i].attivo == true) {
                
                // Creiamo temporaneamente un rettangolo (hitbox) anche per l'oggetto, per usare CheckCollisionRecs
                Rectangle rect_oggetto = {lista_oggetti[i].x+offset_fisico, lista_oggetti[i].y+offset_fisico, (float)dim_ogg_fisico, (float)dim_ogg_fisico};
                
                // Se il giocatore "tocca" l'oggetto
                if (CheckCollisionRecs(rect_eroe, rect_oggetto)) {
                    lista_oggetti[i].attivo = false; // L'oggetto viene "spento", raccolto!
                    punteggio += 1; // Aumentiamo i punti
                    // Potresti anche inserire qui un "PlaySound(suono_moneta);" in futuro!
                }
            }
        }

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

            // [Dentro BeginDrawing(), dopo aver chiuso i cicli FOR della Mappa]

            // Facciamo finta che l'oggetto da visualizzare sia il primo del foglio (quindi partiamo da x=0)
            Rectangle ritaglio_oggetto = {0.0f, 0.0f, (float)dim_ogg, (float)dim_ogg}; 
            int dim_ogg_visivo=16; //quanto lo voglio grande
            float offset=(dim_tile-dim_ogg_visivo)/2.0f; //calcolo centro esatto

            for (int i = 0; i < NUM_OGGETTI; i++){
                // Disegnamo l'oggetto SOLO se non è stato raccolto (è attivo)
                if (lista_oggetti[i].attivo == true){

                   // 4. Crei la hitbox fisica E visiva piccolina e centrata!
                    Rectangle destinazione = {
                    lista_oggetti[i].x + offset, 
                    lista_oggetti[i].y + offset, 
                    (float)dim_ogg_visivo, 
                    (float)dim_ogg_visivo
                    };

                    // 5. Raylib fa la magia: prende il 256 e lo schiaccia nel 32x32 centrato!
                     DrawTexturePro(oggetti_sprite, ritaglio_oggetto, destinazione, (Vector2){0,0}, 0.0f, WHITE);

                }
            }
            if(mostro.attivo){
                DrawTextureRec(mostro_sprite, frame_rec_mostro, (Vector2){mostro.x, mostro.y}, WHITE);
            }
            DrawTextureRec(eroe_sprite, frame_rec, (Vector2){eroe.x, eroe.y}, WHITE); //ritaglia immagine spritesheet
            EndMode2D();

            DrawText(TextFormat("PUNTEGGIO: %d", punteggio), 10, 10, 30, GREEN);

        EndDrawing(); //Manda tutto a schermo
    }

    UnloadTexture(eroe_sprite);
    UnloadTexture(tileset);
    UnloadTexture(oggetti_sprite);
    UnloadTexture(mostro_sprite);

    // ========================================================================
    // 3. CHIUSURA E PULIZIA (De-inizializzazione)
    // ========================================================================
    //Chiudi la finestra
    CloseWindow(); 

}