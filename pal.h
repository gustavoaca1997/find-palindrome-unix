#ifndef PAL_H
#define PAL_H
/*
    Función que ve si un string es un palindromo
*/
int pal(char *str, int i, int j);

/*
    Función que se llama recursivamente para encontrar
    todos los palindromos
*/
int sub(char *str, int i, int j);
#endif