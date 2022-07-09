#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include "markov.h"

#define MAX_INPUT_SIZE 0xffff 
#define rnd_float() ((rand() % RAND_MAX)/(float)RAND_MAX)

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
    printf("%s ", word_set[state]);
    // if state has no further transitions, break
    // if end of sentence is encountered ('.' or '\n') increment num_sentences
    while(!done){
        // if end of sentence
        if (contains_char(word_set[state], '.') || contains_char(word_set[state], '\n')) {
            num_sentences++;
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