#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/time.h>

#define MAX_LINE_LENGTH 1024
#define MAX_ARGS 64
#define MODE_BATCH 0
#define MODE_INTERACTIVE 1
int MODE;

// Function declarations
void parse_command(char *command, char *args[]);
void execute_command(char *args[], int background);
void interactive_mode();
void batch_mode(char *batch_file);

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

        // Parse the command line arguments
        parse_command(input, args);

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

        // Parse the command line arguments
        parse_command(input, args);

        // Execute the command
        execute_command(args, 1); // Run in background mode for batch mode
    }

    fclose(file);
}

void parse_command(char *command, char *args[]) {
    // Check for exit command
    if (strcmp(command, "exit") == 0) {
        exit(0);
    }

    // Parse the command line into arguments
    char *token;
    int arg_count = 0;

    token = strtok(command, " \t\n");
    while (token != NULL && arg_count < MAX_ARGS - 1) {
        args[arg_count++] = token;
        token = strtok(NULL, " \t\n");
    }
    args[arg_count] = NULL; // Null-terminate the argument list

    if (strcmp(args[0], "cd") == 0 && arg_count != 2) {
        fprintf(stderr, "An error has occurred\n");
        exit(0);
    }
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