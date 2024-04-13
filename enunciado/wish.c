/*
 PRACTICA 2 : SISTEMAS OPERATIVOS

 DANIELA ANDREA PAVAS BEDOYA
 GIOVANI STEVEN CARDONA MARIN
 
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <libgen.h>
#include <readline/readline.h>
#include <readline/history.h>

#define BUFFER_SIZE 1024
#define HISTORY_SIZE 30
#define MAX_SIZE 500
#define MAX_COMMANDS 1000

// Declaracion de funciones
void ejecutar_comando(char *command, char ***mypath);
void ejecutar_exit(char *args);
void ejecutar_cd(char *newpath);
void ejecutar_path(char *newpath, char ***mypath);
int redireccionar(char **args, char *file);

char error_message[30] = "An error has occurred\n";
char history[HISTORY_SIZE][BUFFER_SIZE];
int history_count = 0;

int main(int argc, char *argv[]) {
    // Inicialización del mypath
	char **mypath = malloc(2 * sizeof(char *));
	mypath[0] = "/bin/";
	mypath[1] = "";

	// Para capturar la entrada
	char *input_line;

	// Modo interactivo
	if (argc == 1) {
		do {
			input_line = readline("wish> ");
			
            if (!input_line) {
				// EOF o error
				break;
			}
			
            if (strlen(input_line) > 0) {
				add_history(input_line);

				// Copiar cadena de comando al búfer de historial
				if (history_count < HISTORY_SIZE) {
					strcpy(history[history_count++], input_line);
				} else {
					for (int i = 0; i < HISTORY_SIZE - 1; i++) {
						strcpy(history[i], history[i + 1]);
					}
					strcpy(history[HISTORY_SIZE - 1], input_line);
				}
			}

			// Parseo de la entrada capturada con el fin de secuencia
			input_line[strcspn(input_line, "\n")] = '\0';

			// Ejecuto el comando
			ejecutar_comando(input_line, &mypath);

		} while (1);
		free(input_line);
	} else if (argc == 2) { // Modo batch
		char commands[MAX_COMMANDS][MAX_SIZE];
		int num_commands = 0;

		FILE *fp = fopen(argv[1], "r");
		if (!fp) {
			write(STDERR_FILENO, error_message, strlen(error_message));
			return EXIT_FAILURE;
			// exit(1);
		}

		// Leer lineas del archivo
		while (fgets(commands[num_commands], MAX_SIZE, fp)) {
			num_commands++;
		}
		fclose(fp);

		// Ejecutar los comandos en orden
		for (int i = 0; i < num_commands; i++) {
			input_line = commands[i];

			// Parseo de la entrada capturada con el fin de secuencia
			input_line[strcspn(input_line, "\n")] = '\0';

			// Ejecuto el comando
			ejecutar_comando(input_line, &mypath);
		}
	} else {
		write(STDERR_FILENO, error_message, strlen(error_message));
		return EXIT_FAILURE;
	}
}

void ejecutar_comando(char *command, char ***mypath) {

    int num_com = 0;
	int fd;

    // Comandos Built-In, se validará que no se ingresen comandos como "cd /bin/ & path /bin/", ya que es incorrecto para comandos Built-In
	if ((strstr(command, "cd") != 0 || strstr(command, "path") != 0 || strstr(command, "exit") != 0) && strstr(command, "&") == 0) {

		// Separamos del comando original el nombre del comando y sus argumentos
		char *s = command;
		char *command_string = strtok_r(s, " ", &s);

		if (strcmp(command_string, "exit") == 0) {
			ejecutar_exit(s);
		} else if (strcmp(command_string, "cd") == 0) {
			ejecutar_cd(s);
		} else if (strcmp(command_string, "path") == 0) {
			ejecutar_path(s, mypath);
		}
    // Ejecución de otros comandos, no Built-In. Puede ser 1 o varios
    } else if (strstr(command, "cd") == NULL && strstr(command, "path") == NULL && strstr(command, "exit") == NULL) {

		// Extraccion de todos los posibles comandos
		num_com = 0;
		char *command_copy = strdup(command);
		char *token = strtok(command_copy, "&");
		while (token != NULL){
			// Verificamos que el comando no esté vacío
			token += strspn(token, " ");
			if (*token != '\0'){
				num_com++;
			}
			token = strtok(NULL, "&");
		}

		char *all_commands[num_com + 1];
		char *command_copy2 = strdup(command);
		token = strtok(command_copy2, "&");
		int i = 0;
		while (token != NULL) {
			/// Verificamos que el comando no esté vacío
			token += strspn(token, " ");
			if (*token != '\0') {
				all_commands[i] = token;
				// printf("Commands: %s\n",all_commands[i]);
				i++;
			}
			token = strtok(NULL, "&");
		}
		all_commands[i] = NULL;

		// Inicializamos los pids de los comandos capturados
		pid_t pids[num_com];

		// Recorrido para todos los comandos que serán lanzados
		for (i = 0; i < num_com; i++) {

			// Separamos del comando original el nombre del comando y sus argumentos
			char *s = all_commands[i];
			char *command_string = strtok_r(s, " ", &s);

			// Iniciamos comprobación del file descriptor basado en buscar las rutas del comando
			fd = -1;
			char ***mp_copy = malloc(2 * sizeof(char **)); // Crea una copia de mypath
			memcpy(mp_copy, mypath, 2 * sizeof(char **));
			char ***mp = mp_copy;
			char specificpath[MAX_SIZE];

			while ((strcmp((*mp)[0], "") != 0) && fd != 0) {
				strcpy(specificpath, *(mp[0]++));
				strncat(specificpath, command_string, strlen(command_string));
				fd = access(specificpath, X_OK);
			}
			// Si el file descriptor existe, osea la ruta del programa
			if (fd == 0) {
				// Lanzaremos cada comando como un nuevo proceso
				// En caso de error
				if ((pids[i] = fork()) < 0) {
					perror("fork");
					abort();

				} // Proceso hijo creado
				else if (pids[i] == 0) {
					// Como al hacer fork se hace una copia exacta e independiente de todo el programa padre, puedo manipular variables en el entorno del hijo
					// Extraigo los argumentos para preparar el execv()

					// Conteo de los argumentos del comando
					int num_args = 0;
					char *s_copy = strdup(s);
					char *token = strtok(s_copy, " ");
					while (token != NULL) {
						num_args++;
						token = strtok(NULL, " ");
					}

					// Guardo los argumentos en el vector myargs
					i = 1;
					char *myargs[num_args + 1];
					myargs[0] = strdup(command_string);
					char *s_copy2 = strdup(s);
					token = strtok(s_copy2, " ");
					while (token != NULL) {
						myargs[i] = token;
						token = strtok(NULL, " ");
						i++;
					}
					myargs[i] = NULL;

					// FUNCIÓN REDIRECCIÓN
					int i_found = 0, aux = 0;
					i = 0;
					do {

						if (strcmp(myargs[i], ">") == 0) {
							aux = aux + 1;
							i_found = i;
						} else if (strchr(myargs[i], '>') != NULL) {
							aux = aux + 2;
						}

						i++;
					} while (myargs[i] != NULL);

					if (aux == 1) {
						// encontro 1 >
						if (myargs[i_found + 1] == NULL) {
							write(STDERR_FILENO, error_message, strlen(error_message));
						} else if (myargs[i_found + 2] != NULL) {
							write(STDERR_FILENO, error_message, strlen(error_message));
						} else {
							char *file = malloc(MAX_SIZE * sizeof(char *));
							strcpy(file, myargs[i_found + 1]);
							myargs[i_found] = NULL;
							myargs[i_found + 1] = NULL;
							if (file != NULL){

								myargs[0] = strdup(specificpath);
								redireccionar(myargs, file);
							}
							else{
								write(STDERR_FILENO, error_message, strlen(error_message));
							}
						}
					} else if (aux > 1) {
						write(STDERR_FILENO, error_message, strlen(error_message));
					} else {
						// Lanzo el proceso hijo como un nuevo programa
						// Como me interesa ejecutar el proceso hijo como un nuevo programa, mando los argumentos capturados en el comando sobreescribiendo la imagen del proceso en cuestión
						execv(specificpath, myargs);
					}

					// Si execv() es exitoso, lo siguiente nunca se ejecutará, por precaución para una salida segura
					exit(EXIT_FAILURE);

				} // Fin del proceso hijo
			} else {
				write(STDERR_FILENO, error_message, strlen(error_message));
			}
		} // Fin del for

		// Esperamos que todos los hijos terminen
		int status;
		pid_t pid;
		for (i = 0; i < num_com; i++) {
			pid = wait(&status);
		}
	} else {// Comando incorrecto
		write(STDERR_FILENO, error_message, strlen(error_message));
	}
}

void ejecutar_exit(char *args){
	char *path = strtok_r(args, " ", &args);
	if (path != NULL) {
		write(STDERR_FILENO, error_message, strlen(error_message));
	} else{
		exit(0);
	}
}

// Implementación del comando cd para cambiar la ruta del directorio actual,sólo debe recibir 1 argumento que será el cambio a la nueva ruta
void ejecutar_cd(char *newpath){
	char *path = strtok_r(newpath, " ", &newpath);
	if (path == NULL) {
		write(STDERR_FILENO, error_message, strlen(error_message));
	} else {
		if (strtok_r(NULL, " ", &newpath) != NULL) {
			write(STDERR_FILENO, error_message, strlen(error_message));
		} else {
			if (access(path, F_OK) == 0){ // Si la ruta existe
				chdir(path);
			} else {
				write(STDERR_FILENO, error_message, strlen(error_message));
			}
		}
	}
}

// Implementación del comando path que contiene las rutas donde se buscarán los comandos ejecutables en el programa
// Puede recibir cero(Se conserva el path actual) o más argumentos(se actualiza el path actual con el numero de argumentos que se ingresen)
void ejecutar_path(char *newpath, char ***mypath) {
	char *copypath = strdup(newpath);
	int path_count = 0;
	int i = 0;
	char *path = strtok_r(copypath, " ", &copypath);

	// count number of paths to be added
	while (path != NULL) {
		path_count++;
		path = strtok_r(copypath, " ", &copypath);
	}
	// Path con cero argumentos
	if (path_count == 0) {
		i = 0;
		while (strcmp((*mypath)[i], "") != 0) {
			(*mypath)[i] = strdup("");
			i++;
		}
	} else { // Path con n argumentos
		*mypath = realloc(*mypath, (path_count + 1) * sizeof(char *));

		path = strtok_r(newpath, " ", &newpath);
		i = 0;
		while (path != NULL) {
			// Verificar si el path es igual a "bin", y si es así, cambiarlo a "/bin/"
			if (strcmp(path, "bin") == 0 || strcmp(path, "/bin") == 0 || strcmp(path, "/bin/") == 0 || strcmp(path, "./bin") == 0 || strcmp(path, "./bin/") == 0) {
				(*mypath)[i] = strdup("/bin/");
			} else {
				if (strstr(path, "./") != path) {
					char *new_path = malloc(strlen(path) + 3); // 2 para "./" y 1 para '\0'
					strcpy(new_path, "./");
					strcat(new_path, path);
					(*mypath)[i] = new_path;
				} else {
					(*mypath)[i] = strdup(path);
				}
				// Verificar si el path termina con /
				if (path[strlen(path) - 1] != '/') {
					strcat((*mypath)[i], "/");
				}
			}
			path = strtok_r(newpath, " ", &newpath);
			i++;
		}
		// set last element of mypath to ""
		(*mypath)[i] = "";
	}
}

int redireccionar(char **args, char *file) {
	pid_t pid, wpid;
	int status;
	pid = fork();

	if (pid == 0) { // Proceso hijo
		int fd = open(file, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);

		dup2(fd, STDOUT_FILENO);
		dup2(fd, STDERR_FILENO); 

		close(fd);
		execv(args[0], args);
	} else if (pid < 0) { // Error al crear hilo
		write(STDERR_FILENO, error_message, strlen(error_message));
	} else { // Proceso padre
		wait(NULL);
	}
	return 1;
}