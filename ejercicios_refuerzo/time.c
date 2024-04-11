#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <command>\n", argv[0]);
        return 1;
    }

    struct timeval start_time, end_time;

    // Obtener el tiempo de inicio
    gettimeofday(&start_time, NULL);

    pid_t pid = fork();

    if (pid < 0) {
        fprintf(stderr, "Fork failed\n");
        return 1;
    } else if (pid == 0) {  // Proceso hijo
        // Ejecutar el comando
        execvp(argv[1], &argv[1]);
        // Si execvp retorna, significa que hubo un error
        perror("execvp");
        exit(1);
    } else {  // Proceso padre
        // Esperar a que el proceso hijo termine
        wait(NULL);

        // Obtener el tiempo de finalización
        gettimeofday(&end_time, NULL);

        // Calcular el tiempo transcurrido
        double elapsed_time = (end_time.tv_sec - start_time.tv_sec) +
                              (end_time.tv_usec - start_time.tv_usec) / 1000000.0;

        // Imprimir el tiempo transcurrido
        printf("Elapsed time: %.5f seconds\n", elapsed_time);
    }

    return 0;
}