#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>    // For pipes and fork
#include <sys/wait.h>  // For wait()
#include <time.h>      // For measuring execution time
#include "file_processing.h"
#include "threading.h"

// List of files to process
const char *files[] = {
    "data/bib", "data/paper1", "data/paper2",
    "data/progc", "data/progl", "data/progp",
    "data/trans"
};
#define FILE_COUNT 7

// Function to measure execution time
double get_execution_time(void (*func)(const char *), const char *filename) {
    clock_t start = clock();
    func(filename);
    clock_t end = clock();
    return ((double)(end - start)) / CLOCKS_PER_SEC;
}

int main() {
    int pipes[FILE_COUNT][2];  // Pipes for communication between parent and children

    // Create pipes for each file
    for (int i = 0; i < FILE_COUNT; i++) {
        if (pipe(pipes[i]) == -1) {
            perror("Pipe creation failed");
            exit(EXIT_FAILURE);
        }
    }

    // Create child processes for each file
    for (int i = 0; i < FILE_COUNT; i++) {
        pid_t pid = fork();
        if (pid < 0) {
            perror("Fork failed");
            exit(EXIT_FAILURE);
        } else if (pid == 0) {  // Child process
            close(pipes[i][0]);  // Close read end of the pipe

            // Measure time and word count for single-threaded execution
            double single_thread_time = get_execution_time(process_file_single_thread, files[i]);
            int single_thread_count = get_last_word_count();

            // Measure time and word count for multi-threaded execution
            double multi_thread_time = get_execution_time(process_file_with_threads, files[i]);
            int multi_thread_count = get_last_word_count();

            // Write the results to the pipe
            write(pipes[i][1], &single_thread_count, sizeof(single_thread_count));
            write(pipes[i][1], &single_thread_time, sizeof(single_thread_time));
            write(pipes[i][1], &multi_thread_count, sizeof(multi_thread_count));
            write(pipes[i][1], &multi_thread_time, sizeof(multi_thread_time));

            close(pipes[i][1]);  // Close write end of the pipe
            exit(EXIT_SUCCESS);  // Child process exits
        }
    }

    // Parent process: Read results from child processes
    for (int i = 0; i < FILE_COUNT; i++) {
        close(pipes[i][1]);  // Close write end of the pipe

        int single_thread_count, multi_thread_count;
        double single_thread_time, multi_thread_time;

        // Read results from the pipe
        read(pipes[i][0], &single_thread_count, sizeof(single_thread_count));
        read(pipes[i][0], &single_thread_time, sizeof(single_thread_time));
        read(pipes[i][0], &multi_thread_count, sizeof(multi_thread_count));
        read(pipes[i][0], &multi_thread_time, sizeof(multi_thread_time));

        close(pipes[i][0]);  // Close read end of the pipe

        // Print the results
        printf("Processing file: %s\n", files[i]);
        printf("Single-threaded count: %d, Time: %.6f seconds\n", single_thread_count, single_thread_time);
        printf("Multi-threaded count: %d, Time: %.6f seconds\n\n", multi_thread_count, multi_thread_time);
    }

    // Wait for all child processes to complete
    for (int i = 0; i < FILE_COUNT; i++) {
        wait(NULL);
    }

    return 0;
}
