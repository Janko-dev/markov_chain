#include "markov.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// check if wset of size set_size contains word 
int contains_word(const char* word, char** wset, int set_size){
    for (int i = 0; i < set_size; i++){
        if (strcmp(wset[i], word) == 0){
            return i;
        }
    }
    return -1;
}

// check if word contains character c
int contains_char(const char* word, char c){
    for (int i = 0; word[i] != '\0'; i++){
        if (word[i] == c) return 1;
    }
    return 0;
}

// read file with filepath and write to output
void read_file(const char* filepath, char* output, long output_size){
    FILE* f = fopen(filepath, "r");
    size_t len = fread(output, sizeof(char), output_size, f);
    fclose(f);
    output[len] = '\0';
}

// split input_text with given delimiters into input_array 
// and count number of words with input_sount
// Further store non-duplicate words in word_set and 
// count number of non_duplicate words with set_count  
void split_input_text(
    char* input_text, char* delims,
    char** input_array, int* input_count, 
    char** word_set, int* set_count)
{
    char* token;
    token = strtok(input_text, delims);

    while (token != NULL && *input_count < BUF_SIZE){

        input_array[*input_count] = token;
        (*input_count)++;

        // check if token already in word set
        if (contains_word(token, word_set, *set_count) != -1){
            token = strtok(NULL, DELIM);
            continue;
        }
        
        // if not found in word set, copy word to set
        word_set[*set_count] = token;
        (*set_count)++;

        token = strtok(NULL, DELIM);
    }
}

// create n x n transition matrix, with n = size param 
float** create_transition_matrix(int size){
    float** trans_matrix = (float**)malloc(size * sizeof(float*));
    for (int i = 0; i < size; i++) trans_matrix[i] = (float*)calloc(size, sizeof(float));
    return trans_matrix;
}

// destroy transition_matrix
void destroy_transition_matrix(float** trans_matrix, int size){
    for (int i = 0; i < size; i++) free(trans_matrix[i]);
    free(trans_matrix);
}

// train model by adjusting weights of transition matrix
// indices of transition matrix indicate FROM state and TO state
// transition matrix [FROM] [TO] 
void train_model(float** trans_matrix, 
char** input_array, int input_count, 
char** word_set, int set_count)
{
    for (int i = 0; i < input_count-1; i++){
        trans_matrix
            [contains_word(input_array[i], word_set, set_count)]
            [contains_word(input_array[i+1], word_set, set_count)]++;
    }
}

// Normalize weights in transition matrix to be between 0 and 1
void normalize_model(float** trans_matrix, int size){
    for (int i = 0; i < size; i++){
        float sum = 0.0f;
        for (int j = 0; j < size; j++){
            sum += trans_matrix[i][j];
        }
        if (sum == 0) continue; 
        for (int j = 0; j < size; j++){
            trans_matrix[i][j] = trans_matrix[i][j]/sum;
        }
    }
}

// Debug model by printing transition matrix 
void print_model(float** trans_matrix, int size){
    for (int i = 0; i < size; i++) printf("    %2d", i);
    printf("\n");
    for (int i = 0; i < size; i++){
        printf("%2d| ", i);
        for (int j = 0; j < size; j++){
            printf("%2.2f  ", trans_matrix[i][j]);
        }
        printf("\n");
    }
}