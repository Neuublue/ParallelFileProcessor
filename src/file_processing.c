#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "file_processing.h"

static int last_word_count = 0;  // Store the last word count

// Single-threaded word counting function
void process_file_single_thread(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Error opening file");
        return;
    }

    int word_count = 0;
    char ch;

    while ((ch = fgetc(file)) != EOF) {
        if (isspace(ch)) {
            word_count++;
        }
    }

    fclose(file);
    last_word_count = word_count;
}

// Getter for the last word count
int get_last_word_count() {
    return last_word_count;
}

// Setter for the last word count (used by threading.c)
void set_last_word_count(int count) {
    last_word_count = count;
}
