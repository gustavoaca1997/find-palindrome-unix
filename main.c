/*
    Autores: Gustavo Castellanos, Yuni Quintero

    Algoritmos usados:
        Algoritmo de Manacher modificado
*/

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
#include <assert.h>

#define SIZE 2048
#define SIGLEER SIGUSR1 // señal de que se escribió un string
#define SIGDFS SIGUSR2 // señal de que terminó dfs

#define ESCRITURA 1
#define LECTURA 0

// Variable booleana que le avisa al proceso encargado de encontrar palindromos
// si ya no hay mas escrituras en el pipe proximas
int dfs_termino = 0;

// Variable entera que indica el numero de llamadas al leer_handler
int count = 0;

// Variable que cuenta el número de palindromos encontrados
int count_pal = 0;

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
pid_t pidPal;
//int pidsHijos = 0, pidsLength = SIZE;

// Pipes
int pipePal[2];


/**********************************************
 * MANEJADORES
 *********************************************/

/*
    Manejador de señales para leer del pipe
*/
void leer_handler(int signum) {
    // si es la primera llamada
    if (count == 0) {
        // cerramos el lado del pipe para escritura
        close(pipePal[ESCRITURA]);
    }
    count++;

    assert(signum == SIGLEER);

    // buffer
    char buf[SIZE+2];

    // string de donde se sacaran los palindromos
    char str[SIZE];
    str[0] = '\0';
    // leemos del pipe
    int ret;
    do {
        ret = read(pipePal[LECTURA], buf, 1);

        if (ret == -1) {
            printf("Err %d: %s\n", errno, strerror(errno));
            exit(2);
        }

        if (ret == 0) {
            break;
        }
        // concatemos en str
        strcat(str, buf);

        // si se leyó completo un mensaje
        if (buf[0] == '\0') {
            
            // ENCONTRAR PALINDROMOS
            for (int i=0; i<strlen(str); i++) {

                // extremos del substring
                int l, r;
                for (int k=0; k<3; k++) {
                    // caso i es el centro de un substring de longitud impar
                    if (k == 0) {
                        l = i-1;
                        r = i+1;
                    }
                    // caso i es el centro izquierdo de un substring de longitud par
                    else if (k == 2) {
                        l = i;
                        r = i+1;
                    }
                    // caso i es el centro derecho de un substring de longitud par
                    else {
                        l = i-1;
                        r = i;
                    }
                    while (0 <= l && r < strlen(str) && str[l] == str[r]) {
                        if (r-l > 1) {
                            // PALINDROMO ENCONTRADO
                            // si no es el primero
                            if (count_pal > 0) {
                                printf(", ");
                            }
                            count_pal++;
                            for (int j=0; j<r-l+1; j++) {
                                printf("%c", str[l+j]);
                            }
                        }
                        l--;
                        r++;
                    }
                }
            }

            // reiniciar str
            str[0] = '\0';
        }

    } while (ret > 0);
}

/*
    Manejador de la señal que indica que ya no se escribirá más en
    el pipe
*/
void dfs_handler(int signum) {
    assert(signum == SIGDFS);
    dfs_termino = 1;

    // Si no se encontró ningún palindromo:
    if (count_pal == 0) {
        write(1, "No existen", 12);
    }
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
                    
                    dfs(file, str2, prof+1);

                    hijos++;
                }
            }
            closedir(dir);
        } else {
            printf("Error abriendo directorio.\n");
        }
    }
    
    // Checkeamos si estamos en una hoja o en la profundidad máxima
    if (hijos == 0 || !is_dir(path) || prof==altura) {

        // escribimos en el pipe
        if (write(pipePal[ESCRITURA], str, strlen(str)+1) == -1) {
            printf("Padre %d: Err %d: %s\n", getpid(), errno, strerror(errno));
            exit(2);
        }
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
    if (pipe(pipePal) == -1) {
        printf("Pipe: Err %d: %s\n", errno, strerror(errno));
        exit(2);
    }

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
    
    // setea manejadores de señales
    // Cuando leer de un PIPE
    struct sigaction leer_action;
    leer_action.sa_handler = &leer_handler;
    leer_action.sa_flags = 0;
    if (sigaction(SIGLEER, &leer_action, NULL) < 0) {
        printf("Error en el sigaction 1 en %d\n", getpid());
        perror("sigaction");
        return 1;
    }

    // Cuando el padre terminó el DFS
    struct sigaction dfs_action;
    dfs_action.sa_handler = &dfs_handler;
    dfs_action.sa_flags = 0;
    if (sigaction(SIGDFS, &dfs_action, NULL) < 0) {
        printf("Error en el sigaction 2 en %d\n", getpid());
        perror("sigaction");
        return 1;
    }
    
    // PROCESO HIJO
    pidPal = fork();
    if (pidPal == 0) {
        while(dfs_termino == 0) {
            ;
        }
    } 
    // PROCESO PADRE
    else if (pidPal > 0) {
        // cierro lado de lectura
        if (close(pipePal[LECTURA]) == -1) {
            printf("Padre %d: Err %d: %s\n", getpid(), errno, strerror(errno));
            exit(2);
        }
        // comienzo dfs
        dfs(dirName, str, 0);
        // le avisamos al hijo que terminamos
        assert(kill(pidPal, SIGDFS) == 0);

        int status;
        close(pipePal[ESCRITURA]);

        // Espera a que el hijo termine
        waitpid(pidPal, &status, 0);
        printf("\n");
    }
    // si hubo un error
    else if (pidPal == -1){
        printf("error %d en el fork\n", pidPal);
        perror("fork");
        return 1;
    }

    return 0;
}