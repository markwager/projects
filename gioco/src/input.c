#include "raylib.h"
#include "../include/input.h"

StatoInput leggi_input_player(void){
    StatoInput input={0};
    // Se premi W o la Freccia Su, "input.su" diventa vero
    if (IsKeyDown(KEY_W) || IsKeyDown(KEY_UP)) input.up = true;
    if (IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN)) input.down = true;
    if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT)) input.sx = true;
    if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) input.dx = true;

    return input;
}