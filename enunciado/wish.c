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


#define MAX_LINE_LENGTH 1024
#define MAX_ARGS 64
#define MODE_BATCH 0
#define MODE_INTERACTIVE 1
int MODE;

// Declaracion de funciones
void execute_command(char *args[], int background);
void interactive_mode();
void batch_mode(char *batch_file);
void ejecutar_exit(char *args);
void ejecutar_cd(char *newpath);

char error_message[30] = "An error has occurred\n";

int main(int argc, char *argv[]) {
    if (argc == 1) {
        MODE = MODE_INTERACTIVE;
        interactive_mode(); // No se proporcionó un archivo de lote, ejecutar en modo interactivo
    } else if (argc == 2) {
        MODE = MODE_BATCH;
        batch_mode(argv[1]); // Se proporcionó un archivo de lote, ejecutar en modo batch
    } else {
        fprintf(stderr, "Usage: %s [batch_file]\n", argv[0]);
        exit(1);
    }

    return 0;
}

void interactive_mode() {
    char input[MAX_LINE_LENGTH];
    char *args[MAX_ARGS];

    while (1) {
        // Print prompt
        printf("wish> ");
        fflush(stdout);

        // Read input
        if (fgets(input, MAX_LINE_LENGTH, stdin) == NULL) {
            fprintf(stderr, "Error reading input\n");
            exit(1);
        }

        // Remove newline character
        input[strcspn(input, "\n")] = '\0';


        // Execute the command
        execute_command(args, 0);
    }
}

void batch_mode(char *batch_file) {
    FILE *file = fopen(batch_file, "r");
    if (file == NULL) {
        fprintf(stderr, "Error opening batch file\n");
        exit(1);
    }

    char input[MAX_LINE_LENGTH];
    char *args[MAX_ARGS];

    while (fgets(input, MAX_LINE_LENGTH, file) != NULL) {
        // Remove newline character
        input[strcspn(input, "\n")] = '\0';


        // Execute the command
        execute_command(args, 1); // Run in background mode for batch mode
    }

    fclose(file);
}

void execute_command(char *args[], int background) {
    pid_t pid = fork();

    if (pid < 0) {
        fprintf(stderr, "Fork failed\n");
        exit(1);
    } else if (pid == 0) {
        // Child process  
        if (execvp(args[0], args) < 0) {
            fprintf(stderr, "An error has occurred\n");
            exit(1);
        }
    } else {
        // Parent process
        if (!background) {
            waitpid(pid, NULL, 0); // Wait for child process to finish
        }
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