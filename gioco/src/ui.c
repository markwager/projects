#include "../include/ui.h"

bool DisegnaBottone(Rectangle rect, const char* testo, Color color_base) {
    bool cliccato = false;
    Vector2 mousePos = GetMousePosition();
    Color colore_attuale = color_base;

    // Se il mouse è sopra il rettangolo
    if (CheckCollisionPointRec(mousePos, rect)) {
        colore_attuale = LIGHTGRAY; 
        
        // CAMBIO QUI: Usiamo Released invece di Pressed!
        if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
            cliccato = true;
        }
    }

    DrawRectangleRec(rect, colore_attuale);
    DrawRectangleLinesEx(rect, 2, BLACK);
    
    int textWidth = MeasureText(testo, 20);
    DrawText(testo, rect.x + (rect.width / 2) - (textWidth / 2), rect.y + (rect.height / 2) - 10, 20, BLACK);

    return cliccato;
}