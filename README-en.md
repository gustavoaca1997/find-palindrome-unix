# _Palindrome Detector in the File System_

_Gustavo Castellanos 14-10192_
_Yuni Quintero 14-10880_
___

_Caracas, march 2, 2018._

In the next project, a program was developed that determines whether palindromes exist in a directory tree. The program should print the palindromes it found, or "They do not exist." on the contrary. In addition to the implementation of the respective algorithms to traverse the directory tree and the verification of the existence of palindromic character strings, a child process, a pipe and signals were used to satisfy the communication and process synchronization requirement.

## Steps to compile

1. `make clean` to restart the executable file `main`
2. `make` to execute the `Makefile` file to tell the compiler which source codes you actually want to compile.
3. The program is executed through the command `./main`. There may or may not be input arguments, if there are, it must be in the following format: `-d <folder>`, where `<folder>` sets the directory from where the tree starts; if the previous argument is not present the current directory is taken by default. There are also the following arguments or flags: `-m <height>` that define the maximum height of the tree (20 by default) and `-f` that specifies whether the names of the files should be included in the analysis (they're not included by default)

## Process usage

There are two processes present in the project:

The **parent process**, which is in charge of parsing the directory tree through a depth scan **DFS**. This makes the journey from the root directory to each of the sheets, which can be either regular files or empty directories. It should be noted that it depends on the presence of the `-f` argument that will define whether the name of the regular file is included in the analysis or not. For each node in the path found, the name of the directory or file corresponding to the character string `str` is concatenated until a sheet is found. Then, it proceeds to "send" (through the pipe) to its child process the string resulting from the path found. In order for the program to traverse the directory and file tree we use the C structure `dirent` and the functions `opendir` and `readdir`.

A single **child process** was used for the project, it was assigned the task of verifying whether the strings sent by its parent process contain palindromes or not and it will print the results. There were two alternatives, initially creating a child process for each leaf of the directory tree, thus, the parent process could continue traversing the tree while the child processes verify the strings. However, in case a directory has a large number of files for example, this means a significant number of child processes created. To simplify the methodology, it was decided to create a single pidPal child process which, over time, will receive all the paths that the parent process finds.

## Pipes and signals usage

Through a **pipe**, which consists of connecting our two processes in such a way that the output of the parent process is the input of the child process. This allows communication and synchronization between processes. To create the pipe `pipePal` the function `pipe()` is used, once created, reads and writes can be made and thus the data can be exchanged. Therefore, the parent process writes the character string to the pipe by doing `write(pipePal[1],str,strlen(str)+1)`, and the child process, by definition, reads the data at the other end del pipe `read(pipePal[0],buf,SIZE)`.

However, it was necessary to use two signals to meet two objectives: `SIGLEER` so that the parent process informs its child process that the string of characters is in the pipe and can be read at that moment and the `SIGDFS signal` that the parent process finished its journey through the directory tree and thus the child process, when it finishes analyzing its missing strings, ends its execution.

The respective signals complement the communication and synchronization of our processes. As `SIGLEER` and `SIGDFS` are signals defined by us, they require a signal handler so that the processes can properly process their data. For example, from the signal handler of `SIGLEER` the child process performs the analysis of the strings it received.

## Find palindromes

On the other hand, in contrast to the management of processes and their synchronization and communication, part of the project also consists of how to determine whether or not a given character string has palindromic substrings. This was achieved through a modification of the **Manacher Algorithm**, which is based on taking each character of the string as a pivot, which represents the center of the possible palindrome, therefore, one by one it verifies its two adjacent characters expanding until two different characters are found or string limits have been violated. It is guaranteed to find all possible palindromic substrings because all characters are considered as the midpoint of the palindrome string. It is taken as equal characters between uppercase and lowercase, letters with accents are different from those without accents, and as many palindromic substrings that are found are printed, that is, they may be repeated. Spaces are also taken into account, ie _“a a“_ is a valid palindrome.

## Error handling

Additionally, possible errors that may occur creating or closing the pipe, writing or reading in the pipe, configuring the signal handlers, determining if a file is a directory or creating the child process are handled. If an error occurred, the program ends its execution and prints information about the error to the console.

## Complexity

The complexity of the algorithm is `O(max(tamaño(str))^2 + N)`, where `N` is the total number of regular files and directories present in the tree.

## Conclusion

In conclusion, we have learned that for simple assignments that could be done with simple implementation algorithms it can also be optimized using processes.
