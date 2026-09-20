#include "raylib.h"
#include <stdbool.h>
#include "../include/input.h"
#include "../include/entita.h"
#include "../include/pathfinding.h"
#include "../include/ui.h"
#include "../include/particelle.h"
#include <stdio.h>
#include <math.h>

int main(void){
    // ========================================================================
    // 1. INIZIALIZZAZIONE (Setup) - Avviene una sola volta all'avvio
    // ========================================================================
    Impostazioni settings = {0.5f, 800, 600};

    //Crea fisicamente la finestra
    InitWindow(settings.larghezza_schermo, settings.altezza_schermo, "Il mio primo Roguelike in C!");
    SetExitKey(0); //Disabilita la chiusura automatica con ESC!
    SetTargetFPS(60);

    InitAudioDevice(); // Per il volume
    SetMasterVolume(settings.volume);

    StatoGioco stato_corrente = STATO_MENU;

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
    // ... setup del mostro ...
    Particella sistema_sangue[MAX_PARTICELLE] = {0};

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
    eroe_cam.offset = (Vector2){settings.larghezza_schermo/2.0f, settings.altezza_schermo/2.0f}; 
    eroe_cam.rotation = 0.0f;
    eroe_cam.zoom = 1.0f; // <-- MANDATO A CAPO, FONDAMENTALE!

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
    // USIAMO LA NOSTRA NUOVA FUNZIONE (Colonna 14, Riga 12)
    Nemico mostro = crea_mostro(14, 12, dim_tile);
    
    // Adattamento al suo spritesheet
    int colonne_sprite_mostro = 7; 
    int righe_sprite_mostro = 4;
    mostro.width = mostro_sprite.width / colonne_sprite_mostro;
    mostro.height = mostro_sprite.height / righe_sprite_mostro;
    
    mostro.x = 14 * dim_tile; // Lo facciamo nascere lontano (es. Colonna 14)
    mostro.y = 12 * dim_tile; // Riga 12
    mostro.attivo = true;

    // Mirino per l'animazione del mostro
    Rectangle frame_rec_mostro = {0.0f, 0.0f, (float)mostro.width, (float)mostro.height};
    int frame_corrente_mostro = 0;
    int count_frame_mostro = 0;
    int direzione_mostro = 0; // Memorizza dove sta guardando

    // --- VARIABILI DEL COMBATTIMENTO E DELLA SPADA ---
    int direzione_eroe=2; // 0=Su, 1=Sx, 2=Giù, 3=Dx (Inizia guardando verso il basso)
    bool sta_attaccando=false;
    int timer_attacco=0;
    Rectangle hitbox_spada = {0};
    int shake_timer = 0; // <-- NUOVO: Timer per il terremoto

    Texture2D spada_sprite = LoadTexture("assets/sword.png"); 
    bool ha_spada = false;     //l'eroe parte a mani vuote
    bool spada_a_terra = true; //la spada si trova fisicamente sul pavimento

    // Calcoliamo quanto è grande un singolo frame della spada 
    // (Presumiamo abbia le stesse 9 colonne e 4 righe del tuo eroe)
    int colonne_spada=6; 
    int righe_spada=1;
    float larghezza_spada = (float)spada_sprite.width/colonne_spada;
    float altezza_spada = (float)spada_sprite.height/righe_spada;
    // Scegliamo dove far apparire la spada (es. riga 2, colonna 3)
    Rectangle rect_spada_terra={3*dim_tile, 2*dim_tile, larghezza_spada, altezza_spada};


    // ========================================================================
    // 2. IL GAME LOOP INFINITO
    // ========================================================================
    while(!WindowShouldClose()){ 
        
        switch(stato_corrente){

            // ==========================================
            // STATO: MENU PRINCIPALE
            // ==========================================
            // ==========================================
            // STATO: MENU PRINCIPALE
            // ==========================================
            case STATO_MENU: {
                BeginDrawing();
                ClearBackground(DARKGRAY);
                DrawText("IL MIO ROGUELIKE", 250, 100, 40, WHITE);

                // --- NUOVA PARTITA ---
                if (DisegnaBottone((Rectangle){ 300, 200, 200, 50 }, "NUOVA PARTITA", GRAY)) {
                    
                    // 1. Ripristiniamo l'eroe
                    eroe = crea_player(100, 100);
                    eroe.height = eroe_sprite.height / righe_sprite_eroe; 
                    eroe.width = eroe_sprite.width / colonne_sprite_eroe; 
                    
                    // 2. Ripristiniamo il mostro
                    mostro = crea_mostro(14, 12, dim_tile);
                    mostro.width = mostro_sprite.width / colonne_sprite_mostro;
                    mostro.height = mostro_sprite.height / righe_sprite_mostro;
                    mostro.x = 14 * dim_tile; 
                    mostro.y = 12 * dim_tile; 
                    mostro.attivo = true;

                    // 3. Ripristiniamo la progressione
                    punteggio = 0;
                    ha_spada = false;
                    spada_a_terra = true;
                    sta_attaccando = false;
                    timer_attacco = 0;
                    shake_timer = 0;

                    // 4. Riattiviamo tutti gli oggetti sulla mappa
                    for(int i = 0; i < NUM_OGGETTI; i++) {
                        lista_oggetti[i].attivo = true;
                    }

                    // Pulisci il sangue dal pavimento!
                    for (int i = 0; i < MAX_PARTICELLE; i++) {
                        sistema_sangue[i].attiva = false;
                    }

                    // Ora che tutto è pulito, possiamo iniziare a giocare!
                    stato_corrente = STATO_GIOCO; 
                }
                
                // --- CARICA PARTITA ---
                if (DisegnaBottone((Rectangle){ 300, 270, 200, 50 }, "CARICA PARTITA", GRAY)) {
                    FILE *file = fopen("salvataggio.txt", "r");
                    if (file != NULL) { 
                        int m_attivo, p_spada, p_terra, ogg_attivo;
                        float temp_ex, temp_ey, temp_mx, temp_my; // Variabili sicure per le coordinate
                        
                        // Leggiamo coordinate Eroe e le assegniamo
                        fscanf(file, "%f %f %d", &temp_ex, &temp_ey, &eroe.hp);
                        eroe.x = temp_ex;
                        eroe.y = temp_ey;

                        // Leggiamo coordinate Mostro
                        fscanf(file, "%f %f %d %d", &temp_mx, &temp_my, &mostro.hp, &m_attivo);
                        mostro.x = temp_mx;
                        mostro.y = temp_my;
                        mostro.attivo = (bool)m_attivo;

                        // Leggiamo progressione
                        fscanf(file, "%d", &punteggio);
                        fscanf(file, "%d %d", &p_spada, &p_terra);
                        ha_spada = (bool)p_spada;
                        spada_a_terra = (bool)p_terra;
                        
                        // Leggiamo oggetti
                        for(int i = 0; i < NUM_OGGETTI; i++) {
                            fscanf(file, "%d", &ogg_attivo);
                            lista_oggetti[i].attivo = (bool)ogg_attivo;
                        }
                        
                        fclose(file);
                        stato_corrente = STATO_GIOCO; 
                    }
                }

                if (DisegnaBottone((Rectangle){ 300, 340, 200, 50 }, "OPZIONI", GRAY)) {
                    stato_corrente = STATO_OPZIONI; 
                }
                if (DisegnaBottone((Rectangle){ 300, 410, 200, 50 }, "ESCI", GRAY)) {
                    goto esci_dal_gioco; 
                }
                EndDrawing();
            } break;

            // ==========================================
            // STATO: OPZIONI
            // ==========================================
            case STATO_OPZIONI: {
                BeginDrawing();
                ClearBackground(DARKGRAY);
                
                DrawText("OPZIONI", 330, 50, 40, WHITE);

                // --- GESTIONE VOLUME ---
                DrawText(TextFormat("VOLUME: %d%%", (int)(settings.volume * 100)), 200, 150, 20, WHITE);
                if (DisegnaBottone((Rectangle){ 400, 140, 50, 40 }, "-", GRAY)) {
                    settings.volume -= 0.1f;
                    if (settings.volume < 0.0f) settings.volume = 0.0f;
                    SetMasterVolume(settings.volume);
                }
                if (DisegnaBottone((Rectangle){ 460, 140, 50, 40 }, "+", GRAY)) {
                    settings.volume += 0.1f;
                    if (settings.volume > 1.0f) settings.volume = 1.0f;
                    SetMasterVolume(settings.volume);
                }

                // --- GESTIONE RISOLUZIONE ---
                DrawText("RISOLUZIONE:", 200, 250, 20, WHITE);
                if (DisegnaBottone((Rectangle){ 400, 240, 100, 40 }, "800x600", GRAY)) {
                    settings.larghezza_schermo = 800;
                    settings.altezza_schermo = 600;
                    SetWindowSize(settings.larghezza_schermo, settings.altezza_schermo);
                }
                if (DisegnaBottone((Rectangle){ 520, 240, 120, 40 }, "1024x768", GRAY)) {
                    settings.larghezza_schermo = 1024;
                    settings.altezza_schermo = 768;
                    SetWindowSize(settings.larghezza_schermo, settings.altezza_schermo);
                }

                // Bottone "INDIETRO" per tornare al menu
                if (DisegnaBottone((Rectangle){ 300, 400, 200, 50 }, "INDIETRO", GRAY)) {
                    stato_corrente = STATO_MENU;
                }

                EndDrawing();
            } break;

            // ==========================================
            // STATO: GIOCO VERO E PROPRIO
            // ==========================================
            case STATO_GIOCO: {

                // Se premiamo ESC: SALVIAMO e torniamo al MENU
                if (IsKeyPressed(KEY_ESCAPE)) {
                    stato_corrente = STATO_PAUSA;
                }

                // --- A. FASE DI INPUT & UPDATE (Fisica e Logica) ---
                StatoInput input_corrente = leggi_input_player();
                
                // --- GESTIONE INVULNERABILITÀ ---
                if (eroe.iframes > 0) eroe.iframes--;
                if (mostro.iframes > 0) mostro.iframes--;
                
                float prev_x = eroe.x; 
                muovi_player_x(&eroe, input_corrente);
                float margine_x = 12.0f; 
                float margine_y = 20.0f; 
                
                Rectangle hitbox_x = {eroe.x + margine_x, eroe.y + margine_y, (float)eroe.width - (margine_x * 2), (float)eroe.height - margine_y}; 
                if (check_coll_map(hitbox_x, mappa, dim_tile)) {
                    eroe.x = prev_x; 
                }
                
                float prev_y = eroe.y;
                muovi_player_y(&eroe, input_corrente);
                Rectangle hitbox_y = {eroe.x + margine_x, eroe.y + margine_y, (float)eroe.width - (margine_x * 2), (float)eroe.height - margine_y};
                if (check_coll_map(hitbox_y, mappa, dim_tile)) {
                    eroe.y = prev_y;
                }
                
                if (input_corrente.up || input_corrente.down || input_corrente.sx || input_corrente.dx) {
                    count_frame += 1;
                } else {
                    frame_corrente = 0;
                    count_frame = 0;
                }
                if (count_frame > (60 / vel_anim)) {
                    count_frame = 0;
                    frame_corrente += 1;
                }
                if (frame_corrente >= colonne_sprite_eroe) {
                    frame_corrente = 0;
                }
                frame_rec.x = (float)frame_corrente * (float)eroe.width;

                // dove incollare spada e dove scagliare colpo
                if (input_corrente.up) { frame_rec.y = 0.0f * (float)eroe.height; direzione_eroe = 0; }
                if (input_corrente.down) { frame_rec.y = 2.0f * (float)eroe.height; direzione_eroe = 2; }
                if (input_corrente.sx) { frame_rec.y = 1.0f * (float)eroe.height; direzione_eroe = 1; }
                if (input_corrente.dx) { frame_rec.y = 3.0f * (float)eroe.height; direzione_eroe = 3; }

                // aggiornamento camera
                eroe_cam.target = (Vector2){eroe.x + (eroe.width / 2.0f), eroe.y + (eroe.height / 2.0f)};

                // --- GESTIONE SCREEN SHAKE ---
                if (shake_timer > 0) {
                    shake_timer--; 
                    int offset_casuale_x = GetRandomValue(-8, 8);
                    int offset_casuale_y = GetRandomValue(-8, 8);
                    eroe_cam.offset = (Vector2){ (800 / 2.0f) + offset_casuale_x, (600 / 2.0f) + offset_casuale_y };                } else {
                    eroe_cam.offset = (Vector2){ 800 / 2.0f, 600 / 2.0f };                }

                Rectangle rect_eroe = {
                    eroe.x + margine_x, 
                    eroe.y + margine_y, 
                    (float)eroe.width - (margine_x * 2), 
                    (float)eroe.height - margine_y
                };

                // A. RACCOLTA DELLA SPADA
                if (spada_a_terra){
                    if (CheckCollisionRecs(rect_eroe, rect_spada_terra)) {
                        spada_a_terra = false; 
                        ha_spada = true;       
                    }
                }

                // B. L'ATTACCO
                if (IsKeyPressed(KEY_SPACE) && !sta_attaccando && ha_spada) {
                    sta_attaccando = true;
                    timer_attacco = 15; 
                    
                    float raggio_spada = 60.0f; 
                    float spessore = 50.0f;     
                    
                    float centro_x = eroe.x + (eroe.width / 2.0f);
                    float centro_y = eroe.y + (eroe.height / 2.0f);

                    if (direzione_eroe == 0) hitbox_spada = (Rectangle){centro_x - spessore/2, centro_y - raggio_spada, spessore, raggio_spada}; 
                    else if (direzione_eroe == 2) hitbox_spada = (Rectangle){centro_x - spessore/2, centro_y, spessore, raggio_spada}; 
                    else if (direzione_eroe == 1) hitbox_spada = (Rectangle){centro_x - raggio_spada, centro_y - spessore/2, raggio_spada, spessore}; 
                    else if (direzione_eroe == 3) hitbox_spada = (Rectangle){centro_x, centro_y - spessore/2, raggio_spada, spessore}; 

                    Rectangle rect_mostro_reale = {mostro.x + 18.0f, mostro.y + 20.0f, mostro.width - 36.0f, mostro.height - 20.0f};
                    if (mostro.attivo && CheckCollisionRecs(hitbox_spada, rect_mostro_reale) && mostro.iframes == 0) {
                        mostro.hp -= 1;
                        mostro.iframes = 30; 
                        if (mostro.hp <= 0) {
                            mostro.attivo = false;
                            GeneraSplatter(sistema_sangue, MAX_PARTICELLE, mostro.x, mostro.y); 
                        }
                    }
                }

                if (sta_attaccando) {
                    timer_attacco--;
                    if (timer_attacco <= 0) sta_attaccando = false; 
                }

                // --- LOGICA E FISICA DEL NEMICO ---
                if (mostro.attivo) {
                    float vel_mostro = 1.5f; 
                    float prev_m_x = mostro.x;
                    float prev_m_y = mostro.y;
                    bool in_movimento = false;
                    
                    float margine_m_x = 18.0f; 
                    float margine_m_y = 20.0f;

                    Vector2 obiettivo = calcola_prossimo_passo(mostro, eroe, mappa, dim_tile);
                    float dist_x = obiettivo.x - mostro.x;
                    float dist_y = obiettivo.y - mostro.y;

                    if (fabs(dist_x) > vel_mostro) { 
                        mostro.x += (dist_x > 0) ? vel_mostro : -vel_mostro;
                        in_movimento = true;
                    } else {
                        mostro.x = obiettivo.x; 
                    }
                    
                    Rectangle hitbox_m_x = {mostro.x + margine_m_x, mostro.y + margine_m_y, mostro.width - (margine_m_x * 2), mostro.height - margine_m_y};
                    if (check_coll_map(hitbox_m_x, mappa, dim_tile)) mostro.x = prev_m_x;

                    if (fabs(dist_y) > vel_mostro) {
                        mostro.y += (dist_y > 0) ? vel_mostro : -vel_mostro;
                        in_movimento = true;
                    } else {
                        mostro.y = obiettivo.y; 
                    }
                    
                    Rectangle hitbox_m_y = {mostro.x + margine_m_x, mostro.y + margine_m_y, mostro.width - (margine_m_x * 2), mostro.height - margine_m_y};
                    if (check_coll_map(hitbox_m_y, mappa, dim_tile)) mostro.y = prev_m_y;

                    if (in_movimento) {
                        if (fabs(dist_x) > fabs(dist_y)) {
                            direzione_mostro = (dist_x > 0) ? 3 : 1; 
                        } else {
                            direzione_mostro = (dist_y > 0) ? 2 : 0; 
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

                    Rectangle rect_mostro = {mostro.x + margine_m_x, mostro.y + margine_m_y, mostro.width - (margine_m_x * 2), mostro.height - margine_m_y};
                    if (CheckCollisionRecs(rect_eroe, rect_mostro) && eroe.iframes == 0) {
                        eroe.hp -= 25; 
                        eroe.iframes = 60; 
                        shake_timer = 15; 
                        punteggio -= 2;
                        if (punteggio < 0) punteggio = 0;

                        if (eroe.hp <= 0) {
                            GeneraSplatter(sistema_sangue, MAX_PARTICELLE, eroe.x, eroe.y);
                            eroe.x = 100;
                            eroe.y = 100;
                            eroe.hp = 100; 
                            mostro.x = 14 * dim_tile; 
                            mostro.y = 12 * dim_tile; 
                        }
                    }
                }

                // interazione oggetti
                int dim_ogg_fisico = 16; 
                float offset_fisico = (dim_tile - dim_ogg_fisico) / 2.0f; 
                for (int i = 0; i < NUM_OGGETTI; i++) {
                    if (lista_oggetti[i].attivo == true) {
                        Rectangle rect_oggetto = {lista_oggetti[i].x + offset_fisico, lista_oggetti[i].y + offset_fisico, (float)dim_ogg_fisico, (float)dim_ogg_fisico};
                        if (CheckCollisionRecs(rect_eroe, rect_oggetto)) {
                            lista_oggetti[i].attivo = false; 
                            punteggio += 1; 
                        }
                    }
                }

                // --- AGGIORNAMENTO FISICA SPLATTER ---
                for (int i = 0; i < MAX_PARTICELLE; i++) {
                    if (sistema_sangue[i].attiva) {
                        if (sistema_sangue[i].vita > 0) {
                            sistema_sangue[i].x += sistema_sangue[i].vel_x;
                            sistema_sangue[i].y += sistema_sangue[i].vel_y;
                            
                            // "Frizione" per farle rallentare e fermare a terra
                            sistema_sangue[i].vel_x *= 0.90f;
                            sistema_sangue[i].vel_y *= 0.90f;
                            
                            sistema_sangue[i].vita--;
                        }
                    }
                }

                // --- B. FASE DI DISEGNO (Draw) ---
                BeginDrawing(); 
                    ClearBackground(BLACK);

                    BeginMode2D(eroe_cam);
                    // disegno mappa
                    for (int i=0; i<RIGHE; i++) { 
                        for (int j=0; j<COLONNE; j++) { 
                            int pos_j = j * dim_tile;
                            int pos_i = i * dim_tile;
                            int tipo_cella = mappa[i][j];

                            float posizione_ritaglio_x = (float)tipo_cella * vera_larghezza_tile;
                            Rectangle mirino_dinamico = {posizione_ritaglio_x, 0.0f, vera_larghezza_tile, vera_altezza_tile};
                            DrawTextureRec(tileset, mirino_dinamico, (Vector2){pos_j, pos_i}, WHITE);
                        }
                    }

                    // --- DISEGNO SANGUE SUL PAVIMENTO ---
                    for (int i = 0; i < MAX_PARTICELLE; i++) {
                        if (sistema_sangue[i].attiva) {
                            DrawRectangle(
                                (int)sistema_sangue[i].x, 
                                (int)sistema_sangue[i].y, 
                                (int)sistema_sangue[i].dimensione, 
                                (int)sistema_sangue[i].dimensione, 
                                sistema_sangue[i].colore
                            );
                        }
                    }

                    Rectangle ritaglio_oggetto = {0.0f, 0.0f, (float)dim_ogg, (float)dim_ogg}; 
                    int dim_ogg_visivo = 16; 
                    float offset = (dim_tile - dim_ogg_visivo) / 2.0f; 

                    for (int i = 0; i < NUM_OGGETTI; i++) {
                        if (lista_oggetti[i].attivo == true) {
                            Rectangle destinazione = {
                                lista_oggetti[i].x + offset, 
                                lista_oggetti[i].y + offset, 
                                (float)dim_ogg_visivo, 
                                (float)dim_ogg_visivo
                            };
                            DrawTexturePro(oggetti_sprite, ritaglio_oggetto, destinazione, (Vector2){0,0}, 0.0f, WHITE);
                        }
                    }
                    
                    // --- GESTIONE COLORI LAMPEGGIO (DANNO) ---
                    Color colore_mostro = WHITE;
                    if (mostro.iframes > 0 && (mostro.iframes / 5) % 2 == 0) colore_mostro = RED;
                    
                    Color colore_eroe = WHITE;
                    if (eroe.iframes > 0 && (eroe.iframes / 5) % 2 == 0) colore_eroe = RED;

                    if (mostro.attivo) {
                        DrawTextureRec(mostro_sprite, frame_rec_mostro, (Vector2){mostro.x, mostro.y}, colore_mostro); 
                    }
                    
                    // 1. SPADA A TERRA
                    if (spada_a_terra) {
                        Rectangle ritaglio_terra = {0.0f, 0.0f, larghezza_spada, altezza_spada};
                        DrawTextureRec(spada_sprite, ritaglio_terra, (Vector2){rect_spada_terra.x, rect_spada_terra.y}, WHITE);
                        DrawTextureRec(eroe_sprite, frame_rec, (Vector2){eroe.x, eroe.y}, colore_eroe);
                    } 
                    // 2. SPADA EQUIPAGGIATA
                    else if (ha_spada) {
float offset_spada_x = 0.0f;
                        float offset_spada_y = 0.0f;
                        float ritaglio_x = 0.0f; 
                        float larghezza_mirino = larghezza_spada;
                        float altezza_mirino = altezza_spada;

                        // --- NUOVO: ONDEGGIAMENTO BRACCIO ---
                        // Creiamo dei piccoli scostamenti in pixel per ogni frame (da 0 a 8)
                        // NOTA: Dovrai aggiustare questi numeri (es. 2, -2, ecc.) guardando il tuo sprite!
                        float mov_braccio_x[9] = {0, -2, -4, -2, 0, 2, 4, 2, 0}; 
                        float mov_braccio_y[9] = {0, -1, -2, -1, 0, -1, -2, -1, 0}; // Per il saltello
                        
                        float extra_x = 0;
                        float extra_y = 0;

                        // Applichiamo il movimento SOLO se stiamo camminando e NON stiamo attaccando
                        if (!sta_attaccando && count_frame > 0) {
                            extra_x = mov_braccio_x[frame_corrente];
                            extra_y = mov_braccio_y[frame_corrente];
                        }

                        // --- LOGICA DI DIREZIONE, SPECCHIO E FENDENTE ---
                        if (direzione_eroe == 0) { 
                            // SU
                            ritaglio_x = (sta_attaccando ? 3.0f : 2.0f) * larghezza_spada; 
                            offset_spada_x = 40.0f; 
                            offset_spada_y = 19.0f + extra_y; // Su e giù
                        } else if (direzione_eroe == 2) { 
                            // GIÙ
                            ritaglio_x = (sta_attaccando ? 3.0f : 2.0f) * larghezza_spada; 
                            altezza_mirino = -altezza_spada; 
                            offset_spada_x = 12.0f; 
                            offset_spada_y = 40.0f + extra_y; // Su e giù
                        } else if (direzione_eroe == 3) { 
                            // DESTRA
                            ritaglio_x = (sta_attaccando ? 3.0f : 0.0f) * larghezza_spada; 
                            offset_spada_x = 28.0f + extra_x; // Avanti e indietro
                            offset_spada_y = 28.0f + extra_y; // Su e giù
                        } else if (direzione_eroe == 1) { 
                            // SINISTRA (Invertiamo extra_x perché il braccio va verso sinistra!)
                            ritaglio_x = (sta_attaccando ? 3.0f : 0.0f) * larghezza_spada; 
                            larghezza_mirino = -larghezza_spada; 
                            offset_spada_x = 0.0f - extra_x; 
                            offset_spada_y = 28.0f + extra_y; 
                        }

                        Rectangle frame_rec_spada = {ritaglio_x, 0.0f, larghezza_mirino, altezza_mirino};

                        // --- Z-INDEX ---
                        if (direzione_eroe == 0) {
                            DrawTextureRec(spada_sprite, frame_rec_spada, (Vector2){eroe.x + offset_spada_x, eroe.y + offset_spada_y}, WHITE);
                            DrawTextureRec(eroe_sprite, frame_rec, (Vector2){eroe.x, eroe.y}, colore_eroe);
                        } else {
                            DrawTextureRec(eroe_sprite, frame_rec, (Vector2){eroe.x, eroe.y}, colore_eroe);
                            DrawTextureRec(spada_sprite, frame_rec_spada, (Vector2){eroe.x + offset_spada_x, eroe.y + offset_spada_y}, WHITE);
                        }
                    }
                    
                    EndMode2D(); 

                    // --- 5. INTERFACCIA UTENTE (UI) ---
                    DrawText(TextFormat("HP EROE: %d", eroe.hp), 10, 10, 20, RED);
                    if (mostro.attivo) {
                        DrawText(TextFormat("HP MOSTRO: %d", mostro.hp), 200, 10, 20, RED);
                    }
                    DrawText(TextFormat("PUNTEGGIO: %d", punteggio), 10, 40, 20, GREEN);
                    DrawText("Premi S per salvare | ESC per il Menu", 450, 10, 15, LIGHTGRAY);
                EndDrawing(); 
            } break; // <--- ATTENZIONE! QUESTO BREAK È FONDAMENTALE

            // ==========================================
            // STATO: MENU DI PAUSA
            // ==========================================
            case STATO_PAUSA: {
                
                // Se premiamo di nuovo ESC, togliamo la pausa e torniamo a giocare
                if (IsKeyPressed(KEY_ESCAPE)) {
                    stato_corrente = STATO_GIOCO;
                }

                BeginDrawing();
                ClearBackground(DARKGRAY);
                
                DrawText("GIOCO IN PAUSA", 270, 100, 40, WHITE);

                // Bottone 1: RIPRENDI
                if (DisegnaBottone((Rectangle){ 300, 200, 200, 50 }, "RIPRENDI", GRAY)) {
                    stato_corrente = STATO_GIOCO; 
                }
                
                // Bottone 2: SALVA PARTITA (Ecco la vera logica di salvataggio!)
                if (DisegnaBottone((Rectangle){ 300, 270, 200, 50 }, "SALVA PARTITA", GRAY)) {
                    FILE *file = fopen("salvataggio.txt", "w");
                    if (file != NULL) {
                        // Forziamo (float) così il salvataggio è super sicuro
                        fprintf(file, "%f %f %d\n", (float)eroe.x, (float)eroe.y, eroe.hp);
                        fprintf(file, "%f %f %d %d\n", (float)mostro.x, (float)mostro.y, mostro.hp, (int)mostro.attivo);
                        fprintf(file, "%d\n", punteggio);
                        fprintf(file, "%d %d\n", (int)ha_spada, (int)spada_a_terra);
                        
                        for(int i = 0; i < NUM_OGGETTI; i++) {
                            fprintf(file, "%d ", (int)lista_oggetti[i].attivo);
                        }
                        fclose(file);
                        
                        // Piccolo trucco visivo per capire che ha salvato
                        DrawText("PARTITA SALVATA!", 320, 170, 20, GREEN);
                    }
                }

                // Bottone 3: ESCI AL MENU PRINCIPALE
                if (DisegnaBottone((Rectangle){ 300, 340, 200, 50 }, "MENU PRINCIPALE", GRAY)) {
                    stato_corrente = STATO_MENU; 
                }

                EndDrawing();
            } break;

        } // Fine Switch

    } // Fine While

esci_dal_gioco: 
    // Questa etichetta serve per saltare fuori dal while se premi ESCI dal menu

    // ====================================================================
    // 3. CHIUSURA E PULIZIA 
    // ====================================================================
    UnloadTexture(eroe_sprite);
    UnloadTexture(tileset);
    UnloadTexture(oggetti_sprite);
    UnloadTexture(mostro_sprite);
    UnloadTexture(spada_sprite);
    CloseAudioDevice();
    CloseWindow(); 
}