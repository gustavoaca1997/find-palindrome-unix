#include <sys/stat.h>
#include <sys/types.h>
#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#define SIZE 256

/****************************************
    VARIABLES GLOBALES
*****************************************/
// Nombre del directorio a tomar como raíz
char dirName[SIZE];

int altura = 20, // altura maxima del arbol de archivos
    d = 0,          // si el flag -d aparece
    m = 0,          // si el flag -m aparece
    f = 0;          // si el flag -f aparece

/********************************************
 * FUNCIONES
********************************************/

/*
    Función que determina si un archivo es un directorio
    o un archivo regular.

    Parametros:
    char* path: ruta del directorio actual
*/
int is_dir(const char* path) {
    struct stat buf;
    stat(path, &buf);
    return S_ISDIR(buf.st_mode);
}

/*
    Función para recorrer los directorios.

    Parametros:
    char* path: ruta del directorio actual
    char* str: cadena de caracteres con la concatenación de 
                los nombres de los directorios/archivos desde la raíz
                hasta el directorio actual.
    int prof: profundidad de la recursión
*/
void dfs(char* path, char* str, int prof) {
    // Si alcanzamos la profuncidad máxima.
    if (prof == altura) {
        return;
    }
    // Guardamos la ruta del directorio padre
    char file[2*SIZE];
    strcpy(file, dirName);

    DIR *dir;
    struct dirent *direntDir;
    dir = opendir(dirName);
    // Si se abrió correctamente
    if (dir) {
        while ((direntDir = readdir(dir)) != NULL) {
            // concatenamos con la ruta del directorio padre
            strcpy(file, path);
            strcat(file, direntDir->d_name);
            if (is_dir(file) && strcmp(".", direntDir->d_name) != 0 && strcmp("..", direntDir->d_name) != 0) {

                char str2[2*SIZE];
                strcpy(str2, str);
                strcat(str2, direntDir->d_name);

                printf("\tfile: %s\n", file);
                dfs(file, str2, prof+1);
            }
        }
    } else {
        printf("Error abriendo directorio.\n");
        exit(2);
    }
}


/********************************
 * MAIN
*********************************/
int main(int argc, char **argv) {

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

    // si al directorio le falta el ultimo /:
    int length = strlen(dirName);
    if (dirName[length-1] != '/') {
        dirName[length++] = '/';
        dirName[length] = '\0';
    }


    dfs(dirName, "", 0);

    return 0;
}