#ifndef INPUT_H
#define INPUT_H

#include <stdbool.h> //per true e false

typedef struct{
    bool up;
    bool down;
    bool sx;
    bool dx;
} StatoInput;

StatoInput leggi_input_player(void); //funzione che ti ritorna struct con i bool
//non deve avere niente in input: ecco perchè void
//questa funzione si può usare in gioco, non è nascosta

#endif