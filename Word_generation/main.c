#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define CHAR_SET 26
#define WORD_LEN 12
#define NUM_WORDS_GENERATED 20

#define rnd_float ((rand() % RAND_MAX)/(float)RAND_MAX)

int asIndex(char c){
    return c - 'a';
}

int string_to_int(char* str){
    int res = 0;
    for (int i = 0; str[i] != '\0'; i++){
        if (str[i] < '0' || str[i] > '9'){
            fprintf(stderr, "The NUM_WORDS_GENERATED argument must be a positive integer");
            exit(1);
        }
        res = 10 * res + (str[i] - '0');
    }
    return res;
}

int main(int argc, char** argv){
    
    if (argc < 2) {
        fprintf(stderr, "Please provide the necessary amount of arguments\n");
        exit(1);
    }

    int debug = 0;
    char* file_path;
    int num_words_generated; 
    argv++;
    while(--argc > 0){
        if (strcmp("-d", *argv) == 0){
            debug = 1;
            argv++;
        } else {
            file_path = *argv;
            argv++; argc--;
            num_words_generated = string_to_int(*argv);
        }
    }
    if (debug) 
        printf("\n--------------------------------------------------\nFilepath: \t\t%s\n# of generated words: \t%d\nDebug mode: \t\t%s\n--------------------------------------------------\n", 
        file_path, num_words_generated, debug ? "true" : "false");

    // Setup transition matrix
    srand(time(NULL));
    float transition_matrix[CHAR_SET][CHAR_SET] = {{0}};
    for (int i = 0; i < CHAR_SET; i++) {
        for (int j = 0; j < CHAR_SET; j++) {
            transition_matrix[i][j] = 1.0f;
        }   
    }
    
    // Read trainig data
    FILE* f = fopen(file_path, "r");
    if (f == NULL){
        fprintf(stderr, "Can't open file with given path: %s\n", file_path);
        exit(1);
    }
    size_t buf_size = 128;
    char* input = (char*)malloc(sizeof(char) * buf_size);
    size_t characters;

    // training stage
    while((characters = getline(&input, &buf_size, f)) < buf_size){
        for (int i = 0; input[i+1] != '\0'; i++){
            transition_matrix[asIndex(input[i])][asIndex(input[i+1])]++;
        }
    }
    fclose(f);

    // evaluate
    for (int i = 0; i < CHAR_SET; i++){
        float sum = 0.0f;
        for (int j = 0; j < CHAR_SET; j++){
            sum += transition_matrix[i][j];
        }
        for (int j = 0; j < CHAR_SET; j++){
            transition_matrix[i][j] = transition_matrix[i][j]/sum;
        }
    }

    
    // print
    if (debug){
        for (int i = 0; i < CHAR_SET; i++) printf("   %c ", i + 'a');
        printf("\n");
        for (int i = 0; i < CHAR_SET; i++){
            printf("%c  ", i + 'a');
            for (int j = 0; j < CHAR_SET; j++){
                printf("%2.2f ", transition_matrix[i][j]);
            }
            printf("\n");
        }
    }

    // generate words of length
    printf("\n--------------------------------------------------\n");
    char string_buffer[128];
    int word_len;
    for (int k = 0; k < num_words_generated; k++){
        word_len = rand()%WORD_LEN + 2;
        int state = rand()%CHAR_SET;
        for (int i = 0; i < word_len; i++){
            float rnd_choice = rnd_float;
            for (int j = 0; j < CHAR_SET; j++){
                if (rnd_choice < transition_matrix[state][j]){
                    string_buffer[i] = j + 'a';
                    state = j;
                    goto outer;
                }
                rnd_choice -= transition_matrix[state][j];
            }
            outer:;
        }
        string_buffer[word_len] = '\0';
        printf("generated word %2d:\t%s\n", k+1, string_buffer);
    }
    
    return 0;
}