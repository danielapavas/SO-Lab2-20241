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

// Declaracion de funciones
void ejecutar_comando(char *command, char ***mypath);
void interactive_mode();
void batch_mode(char *batch_file);
void ejecutar_exit(char *args);
void ejecutar_cd(char *newpath);

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
			
            if (strlen(input_line) > 0){
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
	} else {
		write(STDERR_FILENO, error_message, strlen(error_message));
		return EXIT_FAILURE;
	}
}

void ejecutar_comando(char *command, char ***mypath) {
    // Comandos Built-In, se validará que no se ingresen comandos como "cd /bin/ & path /bin/", ya que es incorrecto para comandos Built-In
	if ((strstr(command, "cd") != 0 || strstr(command, "path") != 0 || strstr(command, "exit") != 0) && strstr(command, "&") == 0) {

		// Separamos del comando original el nombre del comando y sus argumentos
		char *s = command;
		char *command_string = strtok_r(s, " ", &s);

		if (strcmp(command_string, "exit") == 0)
		{
			ejecutar_exit(s);
		}
		else if (strcmp(command_string, "cd") == 0)
		{
			ejecutar_cd(s);
		}
	} else {
		write(STDERR_FILENO, error_message, strlen(error_message));
	}
}

void ejecutar_exit(char *args)
{
	char *path = strtok_r(args, " ", &args);
	if (path != NULL)
	{
		write(STDERR_FILENO, error_message, strlen(error_message));
	}
	else
	{
		exit(0);
	}
}

// Implementación del comando cd para cambiar la ruta del directorio actual,sólo debe recibir 1 argumento que será el cambio a la nueva ruta
void ejecutar_cd(char *newpath)
{
	char *path = strtok_r(newpath, " ", &newpath);
	if (path == NULL)
	{
		write(STDERR_FILENO, error_message, strlen(error_message));
	}
	else
	{
		if (strtok_r(NULL, " ", &newpath) != NULL)
		{
			write(STDERR_FILENO, error_message, strlen(error_message));
		}
		else
		{
			if (access(path, F_OK) == 0) // Si la ruta existe
			{
				chdir(path);
			}
			else
			{
				write(STDERR_FILENO, error_message, strlen(error_message));
			}
		}
	}
}