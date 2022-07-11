#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>

#define MAX_INPUT_SIZE 0xffff
#define MAX_NGRAM 100 
#define BUF_SIZE 2048
#define DELIM " \n\t"

#define rnd_float() ((rand() % RAND_MAX)/(float)RAND_MAX)

// check if wset of size set_size contains word 
int contains_word(const char* word, char wset[BUF_SIZE][MAX_NGRAM], int set_size){
    for (int i = 0; i < set_size; i++){
        if (strcmp(wset[i], word) == 0){
            return i;
        }
    }
    return -1;
}

// check whether an ngram can still be made at position k until position k+n
// ngrams are specified if they are within the lowercase alphabet [a-z]
int check_ngram(const char* input, int k, int n){
    for (int j = k; j < k+n; j++){
        if ((input[j] < 'a' || input[j] > 'z')){
            return 0;
        }
    }
    return 1;
}

// read file with filepath and write to output param
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
    char** input_array, int* input_count)
{
    char* token;
    token = strtok(input_text, delims);

    while (token != NULL && *input_count < BUF_SIZE){

        input_array[*input_count] = token;
        (*input_count)++;

        token = strtok(NULL, DELIM);
    }
}

// fill ngram_set with ngrams of size n
// no duplicates are allowed in the set
void fill_ngram_set(int n,
char ngram_set[BUF_SIZE][MAX_NGRAM], int* set_count,
char** text_buffer, int buf_count)
{
    char ngram[MAX_NGRAM];
    for (int bi = 0; bi < buf_count; bi++) { // for every word in text buffer
        for (int i = 0; check_ngram(text_buffer[bi], i, n); i+=n){ // every n-gram in word
            for (int j = 0; j < n; j++){ // for 0 to n
                ngram[j] = text_buffer[bi][i+j];
            }
            ngram[n] = '\0';
            // if ngram not in ngram_set
            if (contains_word(ngram, ngram_set, *set_count) == -1){
                strcpy(ngram_set[(*set_count)++], ngram);
            }
        }
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
void train_model(float** trans_matrix, int n,
char** text_buffer, int buf_count, 
char ngram_set[BUF_SIZE][MAX_NGRAM], int set_count)
{
    char from[100];
    char to[100];
    
    for (int bi = 0; bi < buf_count; bi++) {
        //copy first k chars of word to from array
        strncpy(from, text_buffer[bi], n);
        from[n] = '\0';
        if (strlen(text_buffer[bi]) < (unsigned)n) continue;
        // start at second ngram, for all ngrams in word
        for (int i = n; check_ngram(text_buffer[bi], i, n); i+=n){
            for (int j = 0; j < n; j++){
                to[j] = text_buffer[bi][i+j];
            }
            to[n] = '\0';

            // increment transition matrix
            //printf("F:%s, T:%s\n", from, to);
            trans_matrix
                [contains_word(from, ngram_set, set_count)]
                [contains_word(to, ngram_set, set_count)]++;

            // <from> is now <to>, <to> will be next ngram
            strcpy(from, to);
        }
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

// generate words by traversing state machine and transitioning based on probability weights
void generate_words(float** trans_matrix, int n,
char ngram_set[BUF_SIZE][MAX_NGRAM], int set_count, 
int num_words, int max_word_len)
{
    printf("\n---------------------- Generated words ----------------------\n");
    for (int w = 0; w < num_words; w++){
        int count = 0;
        int state = rand()%set_count;
        printf("%s", ngram_set[state]);
        int done = 0;
        
        while(!done && count < max_word_len-n){

            float rnd_choice = rnd_float();
            int selected = 0;
            for (int j = 0; j < set_count; j++){
                if (rnd_choice <= trans_matrix[state][j]){
                    state = j;
                    printf("%s", ngram_set[state]);
                    count += n;
                    selected = 1;
                    break;
                } else {
                    rnd_choice -= trans_matrix[state][j];
                }
            }
            done = !selected;
        }
        printf("\n");
    }
}

// check if string param is positive integer value
int string_to_int(const char* str){
    int res = 0;
    for (int i = 0; str[i] != '\0'; i++){
        if (str[i] < '0' || str[i] > '9'){
            fprintf(stderr, "ERROR: expected a positive integer argument\n");
            exit(1);
        }
        res = 10 * res + (str[i] - '0');
    }
    return res;
}

int main(int argc, char** argv){

    // command line argument parsing
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
    char* file_path = *argv;                    // file path to training set
    int num_words = string_to_int(*++argv);     // number of words to generate
    int n = string_to_int(*++argv);             // n-gram parameter
    int max_word_len = string_to_int(*++argv);  // maximum word length generated

    // read file to text buffer
    srand(time(NULL));
    char text[MAX_INPUT_SIZE];
    read_file(file_path, text, MAX_INPUT_SIZE);
    
    // split raw input string in buffer of words
    char* buffer[BUF_SIZE];
    int buf_index = 0;
    split_input_text(text, DELIM, buffer, &buf_index);
    
    // fill set of ngrams (no duplicates)
    char ngram_set[BUF_SIZE][MAX_NGRAM];
    int set_index = 0;
    fill_ngram_set(n, ngram_set, &set_index, buffer, buf_index);

    // train on provided dataset and adjust weights of transition matrix
    float** trans_matrix = create_transition_matrix(set_index);
    train_model(trans_matrix, n, buffer, buf_index, ngram_set, set_index);
    normalize_model(trans_matrix, set_index);

    if (debug){
        printf("\n-------------------------------------------------------------\n");
        printf("Filepath: \t\t%s\n", file_path);
        printf("# of generated words: \t%d\n", num_words);
        printf("n-gram param: \t\t%d\n", n);
        printf("Maximum size of words: \t%d\n", max_word_len);
        printf("Debug mode: \t\t%s", debug ? "true" : "false");
    }

    // generate words of max_word_len
    generate_words(trans_matrix, n, 
                    ngram_set, set_index, 
                    num_words, max_word_len);

    // clean allocated memory
    destroy_transition_matrix(trans_matrix, set_index);
    return 0;
}