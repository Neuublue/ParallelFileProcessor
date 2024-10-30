#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "threading.h"
#include "file_processing.h"  // Include this to access the setter function

#define MAX_THREADS 4

typedef struct {
    char *content;
    int start;
    int end;
    int word_count;
} ThreadData;

void *count_words(void *arg) {
    ThreadData *data = (ThreadData *)arg;
    int count = 0;

    for (int i = data->start; i < data->end; i++) {
        if (isspace(data->content[i])) {
            count++;
        }
    }

    data->word_count = count;
    pthread_exit(NULL);
}

void process_file_with_threads(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Error opening file");
        return;
    }

    fseek(file, 0, SEEK_END);
    int file_size = ftell(file);
    rewind(file);

    char *content = malloc(file_size + 1);
    fread(content, 1, file_size, file);
    content[file_size] = '\0';
    fclose(file);

    pthread_t threads[MAX_THREADS];
    ThreadData thread_data[MAX_THREADS];
    int chunk_size = file_size / MAX_THREADS;

    for (int i = 0; i < MAX_THREADS; i++) {
        thread_data[i].content = content;
        thread_data[i].start = i * chunk_size;
        thread_data[i].end = (i == MAX_THREADS - 1) ? file_size : (i + 1) * chunk_size;
        thread_data[i].word_count = 0;

        pthread_create(&threads[i], NULL, count_words, &thread_data[i]);
    }

    int total_word_count = 0;
    for (int i = 0; i < MAX_THREADS; i++) {
        pthread_join(threads[i], NULL);
        total_word_count += thread_data[i].word_count;
    }

    set_last_word_count(total_word_count);  // Use the setter function
    free(content);
}
