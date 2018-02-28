#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#include "pal.h"

#define SIZE 256

extern int dp[SIZE][SIZE];

/****************************************
    VARIABLES GLOBALES
*****************************************/
// Nombre del directorio a tomar como raíz
char dirName[SIZE];

int altura = 20, // altura maxima del arbol de archivos
    d = 0,          // si el flag -d aparece
    m = 0,          // si el flag -m aparece
    f = 0;          // si el flag -f aparece

// Arreglo dinámico con los PID de los procesos hijos
pid_t *procesosHijos;
int pidsHijos = 0, pidsLength = SIZE;

/********************************************
 * FUNCIONES
********************************************/

/*
    Función que agrega un / a una ruta si éste carece de un último /.
    
    Parametros:
    char* path: ruta a verificar
*/
void path_this(char* path) {
    int length = strlen(path);
    if (path[length-1] != '/') {
        path[length++] = '/';
        path[length] = '\0';
    }
}

/*
    Funcion que elimina los / de una ruta de un directorio.

    Parametros:
    char* path: ruta a eliminarle los /
    char* str: string donde se guardará el resultado.
*/
void dispath_this(char* path, char* str) {
    int j = 0;
    for (int i=0; i<strlen(path); i++) {
        if (path[i] != '/') {
            str[j++] = path[i];
        }
    }
    str[j] = '\0';
}

/*
    Función que determina si un archivo es un directorio
    o un archivo regular.

    Parametros:
    char* path: ruta del directorio actual
*/
int is_dir(char* path) {
    struct stat *buf = (struct stat*) malloc(sizeof(struct stat)) ;
    stat(path, buf);
    int ret = S_ISDIR(buf->st_mode);
    return ret;
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

    // Numero de directorio "hijos"
    int hijos = 0;

    // Si estamos en un directorio
    if (is_dir(path)) {
        // Abrimos directorio
        DIR *dir;
        struct dirent *direntDir;
        dir = opendir(path);
        // Si se abrió correctamente
        if (dir) {
            while ((direntDir = readdir(dir)) != NULL) {
                // concatenamos con la ruta del directorio padre
                strcpy(file, path);
                strcat(file, direntDir->d_name);

                // si a file le falta el ultimo /
                path_this(file);

                // Si el archivo es un directorio y distinto de . y ..
                if ((is_dir(file) || (f == 1)) && strcmp(direntDir->d_name, ".") != 0 && strcmp(direntDir->d_name, "..") != 0) {
                    char str2[2*SIZE];
                    strcpy(str2, str);
                    strcat(str2, direntDir->d_name);

                    //printf("directorio: %s\n", file);
                    
                    dfs(file, str2, prof+1);

                    hijos++;
                }
            }
            closedir(dir);
        } else {
            printf("Error abriendo directorio.\n");
        }
    }
    
    // Checkeamos si estamos en una hoja
    if (hijos == 0 || !is_dir(path)) {
        pid_t pidPal;

        // Si estamos en el proceso hijo
        if ((pidPal = fork()) == 0) {

        	memset(dp, -1, sizeof(dp));

            printf("Pasando %s\n", str);
            printf("Número de palindromos encontrados: %d\n", sub(str, 0, strlen(str)-1));
            exit(0);
        } else {
            // guardamos el PID en el arreglo de PID
            procesosHijos[pidsHijos++] = pidPal;

            // si ya alcanzamos la capacidad máxima
            if (pidsHijos == pidsLength) {
                // aumentamos el doble en la capacidad maxima
                pidsLength = 2*pidsLength;
                procesosHijos = realloc(procesosHijos, pidsLength);
            }
        }
    }
}


/********************************
 * MAIN
*********************************/
int main(int argc, char **argv) {

    // INICIALIZACION
    procesosHijos = (pid_t *) malloc(sizeof(pid_t)*SIZE);

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

    // agrega / si al directorio le falta el ultimo /:
    path_this(dirName);

    // Quita los / del directorio
    char str[SIZE];
    dispath_this(dirName, str);
    
    dfs(dirName, str, 0);

    int status;
    for (int i=0; i<pidsHijos; i++) {
        waitpid(procesosHijos[i], &status, 0);
    }

    return 0;
}