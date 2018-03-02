#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <signal.h>

#include "pal.h"

#define SIZE 256
#define SIGLEER SIGUSR1 // señal de que se escribió un string
#define SIGDFS SIGUSR2 // señal de que terminó dfs

// Arreglo para memoization
extern int dp[SIZE][SIZE];

// Variable booleana que le avisa al proceso encargado de encontrar palindromos
// si ya no hay mas escrituras en el pipe proximas
int dfs_termino = 0;

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
pid_t pidPal;//NUEVO
//int pidsHijos = 0, pidsLength = SIZE;

// Pipes
int pipePal[2];

/**********************************************
 * MANEJADORES
 *********************************************/

/*
    Manejador de señales para leer
*/
void leer_action(int signum) {
    assert(signum == SIGLEER);

    // inicializamos tabla de dp
    memset(dp, -1, sizeof(dp));

    // buffer
    char buf[SIZE];

    // string de donde se sacaran los palindromos
    char *str;
    str = (char *) malloc(sizeof(char)*2*SIZE);
    int str_length = 2*SIZE, str_size = 0;
    str[0] = '\0';

    // leemos del pipe
    int ret;
    do {
        ret = read(pipePal[1], buf, SIZE);

        // si str está full
        if (str_size == str_length) {
            // duplicamos tamaño
            str_length *= 2;
            str = realloc(str, str_length);
        }

        // concatemos en str
        strcat(str, buf);
        str_size += SIZE;
    } while (ret == SIZE);

    // le pasamos el string a la funcion sub
    assert(str_size > 0);
    sub(str, 0, str_size-1);

    free(str);
}

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
    struct stat buf;
    int success;
    // Si no es un directorio:
    if ((success = stat(path, &buf) != 0) && errno == ENOTDIR) {
        return 0;
    }
    // Si ocurrió un error
    else if (success != 0) {
        printf("Err %d: %s\n", errno, strerror(errno));
        exit(2);
    }

    // si es un directorio
    return 1;
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
    /*if (prof == altura) {
        return;
    }*/
    // Guardamos la ruta del directorio padre
    char file[2*SIZE];
    strcpy(file, dirName);

    // Numero de directorio "hijos"
    int hijos = 0;

    // Si estamos en un directorio
    if (prof != altura && is_dir(path)) {
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
    if (hijos == 0 || !is_dir(path) || prof==altura) {

        // escribimos en el pipe
        write(pipePal[0], str, strlen(str)+1);
        // le avisamos al hijo
        assert(kill(pidPal, SIGLEER) == 0);
    }
}


/********************************
 * MAIN
*********************************/
int main(int argc, char **argv) {

    // INICIALIZACION

    // creamos el pipe
    pipe(pipePal);

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

    // PROCESO HIJO
    if ((pidPal = fork()) == 0) {
        // cierro el lado de escritura
        close(pipePal[0]);

        // setea manejadores de señales
        // Cuando leer de un PIPE
        struct sigaction leer_action;
        leer_action.sa_handler = leer_handler;
        if (sigaction(SIGLEER, &leer_action, NULL) < 0) {
            perror("sigaction");
            return 1;
        }

        // Cuando el padre terminó el DFS
        struct sigaction dfs_action;
        dfs_action.sa_handler = dfs_handler;
        if (sifaction(SIGDFS, $dfs_action, NULL) < 0) {
            perror("sigaction");
            return 1;
        }
    } 
    // PROCESO PADRE
    else {
        // cierro lado de lectura
        close(pipePal[1]);

        // comienzo dfs
        dfs(dirName, str, 0);
        
        // le avisamos al hijo que terminamos
        assert(kill(pidPal, SIGDFS));

        int status;
        waitpid(pidPal, &status, 0);
    }

    return 0;
}