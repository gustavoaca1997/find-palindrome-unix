# _Detector de Palíndromos en el Sistema de Archivos_

_Gustavo Castellanos 14-10192_
_Yuni Quintero 14-10880_
___

_Caracas, 2 de marzo de 2018._

En el siguiente proyecto se desarrolló un programa que determina si existen palíndromos en un árbol de directorios. El programa debe imprimir los palíndromos que encontró, o “No existen.” de lo contrario. Además de la implementación de los algoritmos respectivos para recorrer el árbol de directorios y la verificación de la existencia de cadenas de caracteres palíndromos, se utilizó un proceso hijo, un pipe y señales para satisfacer el requerimiento de comunicación y sincronización de procesos. 

## Pasos para compilar

1. `make clean` para reiniciar el archivo ejecutable `main`
2. `make` para ejecutar el archivo `Makefile` para indicarle al compilador cuales códigos fuentes se desean, en efecto, compilar.
3. Se procede a ejecutar el programa a través del comando `./main`. Puede haber o no argumentos de entrada, en caso de que haya, debe tener el siguiente formato: `-d <carpeta>`, donde `<carpeta>` establece el directorio desde donde se inicia el árbol; si el argumento anterior no está presente se toma el directorio actual por defecto. También se tienen los siguientes argumentos o flags: `-m <altura>` que define la altura máximo del árbol (20 por defecto) y `-f` que especifica si deben incluirse en el análisis los nombres de los archivos (por defecto no se incluyen)

## Uso de procesos

En el proyecto se tienen dos procesos presentes: 

El **proceso padre**, que se encarga de analizar el árbol de directorios mediante un recorrido de profundidad **DFS**. Éste realiza el recorrido desde el directorio raíz hasta cada una de las hojas, las cuales pueden ser o archivos regulares o directorios vacíos. Cabe destacar que depende de la presencia del argumento `-f` que definirá si se se incluye el nombre del archivo regular en el análisis o no. Por cada nodo en el camino encontrado, se concatena el nombre del directorio o archivo correspondiente a la cadena de caracteres `str` hasta encontrar una hoja. Luego, procede a “enviarle” (mediante el pipe) a su proceso hijo la cadena resultante del camino encontrado.

Para el proyecto se utilizó un único **proceso hijo**, a éste se le asignó la tarea de verificar si las cadenas que su proceso padre le envió contiene palíndromos o no y se encargará de imprimir los resultados. Hubo dos alternativas, inicialmente crear un proceso hijo por cada hoja del árbol de directorios, así, el proceso padre podría seguir recorriendo el árbol mientras los procesos hijos van verificando los strings. Sin embargo, en caso de que un directorio tenga una gran cantidad de archivos por ejemplo, esto significa un número notable de procesos hijos creados. Para simplificar la metodología, se decidió crear un sólo proceso hijo pidPal el cual a través del tiempo irá recibiendo todos los caminos que el proceso padre va encontrando.

## Uso de pipes y señales

Mediante un **pipe** o tubería, que consiste en conectar a nuestros dos procesos de forma tal que la salida del proceso padre sea la entrada del proceso hijo. Esto permite la comunicación y sincronización entre procesos. Para crear el pipe `pipePal` se emplea la función `pipe()`, una vez creado, se podrán hacer lecturas y escrituras y así intercambiar los datos. Por lo tanto, el proceso padre escribe en el pipe la cadena de caracteres haciendo `write(pipePal[1], str, strlen(str)+1)`, y el proceso hijo, por definición, lee los datos en el otro extremos del pipe `read(pipePal[0], buf, SIZE)`.

No obstante, se necesitó utilizar dos señales para cumplir con dos objetivos: `SIGLEER` para que el proceso padre le informe a su proceso hijo que la cadena de caracteres se encuentra en el pipe y puede ser leída en ese instante y la señal `SIGDFS` de que el proceso padre finalizó su recorrido del árbol de directorios y así el proceso hijo al terminar de analizar sus strings faltantes, culmine su ejecución. 

Las respectivas señales complementan la comunicación y sincronización de nuestros procesos. Como `SIGLEER` y `SIGDFS` son señales definidas por nosotros, requieren de un manejador de señales para que los procesos puedan procesar de manera adecuada sus datos. Por ejemplo, desde el manejador de señales de `SIGLEER` el proceso hijo realiza el análisis de los strings que recibió.

## Encontrar palindromos

Por otra parte, en contraste con el manejo de procesos y su sincronización y comunicación, parte del proyecto también consiste en cómo determinar que una cadena de caracteres dada tiene o no substrings palíndromos. Esto se logró mediante una modificación del **Algoritmo de Manacher**, la cual se basa en tomar cada caracter del string como pivote, que representa el centro del posible palíndromo, por lo tanto, uno por uno va verificando a sus dos caracteres adyacentes expandiéndose hasta encontrar dos caracteres distintos o se haya violado los límites de la cadena. Se garantiza encontrar todos los substrings palindromes posibles debido a que todos los caracteres se consideran como punto medio del string palíndromo. Se toma como caracteres iguales entre mayúsculas y minúsculas, letras con acentos son distintas a aquellas sin tilde, y se imprimen tantos substrings palíndromos que se encuentren, es decir, puede ser que se repitan. También se toman en cuenta los espacios es decir _“a a”_ es un palíndromo válido.

## Manejo de errores

Adicionalmente se manejan los posibles errores que puedan ocurrir creando o cerrando el pipe, escribiendo o leyendo en el pipe, configurando los manejadores de señales, determinando si un archivo es directorio o creando el proceso hijo. Si ocurrió un error, el programa termina su ejecución e imprime en consola información sobre el error.

## Complejidad

La complejidad del algoritmo es `O(max(tamaño(str))^2 + N)`, donde `N` es el número total de archivos regulares y directorios presentes en el árbol.

## Conclusión

En conclusión, hemos aprendido que para asignaciones simples que podrían realizarse con algoritmos simples de implementación puede también optimizarse utilizando procesos.
