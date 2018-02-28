#include <sys/stat.h>
#include <sys/types.h>
#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <unistd.h>

#define SIZE 256
/*
    Función que determina si un archivo es un directorio
    o un archivo regular.
*/
int is_dir(const char* path) {
    struct stat buf;
    stat(path, &buf);
    return S_ISDIR(buf.st_mode);
}

int main(int argc, char *argv) {
    // Nombre del directorio a tomar como raíz
    char dirName[SIZE];

    int altura = 20, // altura maxima del arbol de archivos
        d = 0,          // si el flag -d aparece
        m = 0,          // si el flag -m aparece
        f = 0;          // si el flag -f aparece

    // Si hay argumentos
    if (argc > 1) {
        for (int i=1; i<argc; i++) {
            // se define directorio
            if (strcmp("-d", argv[i]) == 0 && i+1 < argc) {
                strcpy(dirName, argv[++i]);
                d = 1;
            }
            // se define altura
            else if (strcmp("-m", argv[i]) == 0 && i+1 < argc) {
                altura = atoi(argv[++i]);
                m = 1;
            }
            // Se define si se consideraran los archivos regulares
            else if (strcmp("-f", argv[i]) == 0) {
                f = 1;
            }
        }
    }
    // Si no se ha definido un directorio, se toma el actual por defecto
    if ( d == 0 && (getcwd(dirName, SIZE)) == NULL) {
        printf("getcwd() error.\n");
        exit(1);
    }
    return 0;
}