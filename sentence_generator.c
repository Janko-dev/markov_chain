#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>

#define MAX_INPUT_SIZE 0xffff 
#define BUF_SIZE 2048
#define DELIM " \n\t"

#define rnd_float() ((rand() % RAND_MAX)/(float)RAND_MAX)

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
    if (f == NULL) {
        fprintf(stderr, "Can't open file with given path: %s\n", filepath);
        exit(1);
    }
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

int string_to_int(const char* str){
    int res = 0;
    for (int i = 0; str[i] != '\0'; i++){
        if (str[i] < '0' || str[i] > '9'){
            fprintf(stderr, "The MAX_SENTENCES argument must be a positive integer\n");
            exit(1);
        }
        res = 10 * res + (str[i] - '0');
    }
    return res;
}

int main(int argc, char** argv){

    if (argc < 3) {
        fprintf(stderr, "Please provide the necessary amount of arguments\n");
        exit(1);
    }

    ++argv;
    int debug = 0;
    if (strcmp(*argv, "-d") == 0){
        debug = 1;
        ++argv;
    }
    char* file_path = *argv;
    int max_sentences = string_to_int(*++argv);

    srand(time(NULL));
    char text[MAX_INPUT_SIZE];
    read_file(file_path, text, MAX_INPUT_SIZE);
    
    // string buffer
    char* buffer[BUF_SIZE];
    int buf_index = 0;

    // set of words (no duplicates)
    char* word_set[BUF_SIZE];
    int set_index = 0;

    // split raw input string in buffer of words and set of words
    split_input_text(text, DELIM, buffer, &buf_index, word_set, &set_index);
    // create transition matrix of the size of word_set
    float** trans_matrix = create_transition_matrix(set_index);
    // train model on order of words in buffer
    train_model(trans_matrix, buffer, buf_index, word_set, set_index);
    // normalize weights in range(0, 1)
    normalize_model(trans_matrix, set_index);

    if (debug){
        printf("\n--------------------------------------------------\n");
        printf("Filepath: \t\t%s\n# generated sentences: \t%d\nDebug mode: \t\t%s", 
        file_path, max_sentences, debug ? "true" : "false");
        printf("\n--------------------------------------------------\n");
        // print transition matrix as table
        print_model(trans_matrix, set_index);
    }

    // determine first state
    int state = 0;
    for (int i = 0; i < set_index; i++){
        if (isupper(word_set[i][0]) && rand()%2 == 1) {
            state = i;
            break;
        }
    }

    int done = 0;
    int num_sentences = 0;
    printf("\n%s ", word_set[state]);
    // if state has no further transitions, break
    // if end of sentence is encountered ('.', '?' or '\n') increment num_sentences
    while(!done){
        // if end of sentence
        if (contains_char(word_set[state], '.') || contains_char(word_set[state], '?') || contains_char(word_set[state], '\n')) {
            num_sentences++;
            printf("\n");
            if (num_sentences >= max_sentences) break;
        }
        float rnd_choice = rnd_float();
        int selected = 0;
        for (int j = 0; j < set_index && !selected; j++){
            if (rnd_choice <= trans_matrix[state][j]){
                state = j;
                printf("%s ", word_set[state]);
                selected = 1;
            } else {
                rnd_choice -= trans_matrix[state][j];
            }
        }
        done = !selected;
    }

    destroy_transition_matrix(trans_matrix, set_index);
    return 0;
}