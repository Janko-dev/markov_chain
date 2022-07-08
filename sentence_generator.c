#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>

#define BUF_SIZE 2048
#define WORD_LEN 40
#define MAX_INPUT_SIZE 0xffff 
#define DELIM " \n\t"

#define rnd_float() ((rand() % RAND_MAX)/(float)RAND_MAX)

int toIndex(char* word, char wset[BUF_SIZE][WORD_LEN], int set_index){
    for (int i = 0; i < set_index; i++){
        if (strcmp(wset[i], word) == 0){
            return i;
        }
    }
    return -1;
}

int contains(char* word, char c){
    for (int i = 0; word[i] != '\0'; i++){
        if (word[i] == c) return 1;
    }
    return 0;
}

int string_to_int(char* str){
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
    FILE* f = fopen(file_path, "r");
    char text[MAX_INPUT_SIZE];
    size_t len = fread(text, sizeof(char), MAX_INPUT_SIZE, f);
    text[len] = '\0';
    fclose(f);
    
    // string buffer
    char buffer[BUF_SIZE][WORD_LEN];
    int buf_index = 0;
    
    // set of words (no duplicates)
    char word_set[BUF_SIZE][WORD_LEN];
    int set_index = 0;

    // splitting the input string with delimers
    char* token;
    token = strtok(text, DELIM);

    while (token != NULL && buf_index < BUF_SIZE){

        strcpy(buffer[buf_index], token);
        buf_index++;

        // check if token already in word set
        if (toIndex(token, word_set, set_index) != -1){
            token = strtok(NULL, DELIM);
            continue;
        }
        
        // if not found in word set, copy word to set
        strcpy(word_set[set_index], token);
        set_index++;

        token = strtok(NULL, DELIM);
    }
    
    // alloc state transition matrix
    float** trans_matrix = (float**)malloc(set_index * sizeof(float*));
    for (int i = 0; i < set_index; i++) trans_matrix[i] = (float*)calloc(set_index, sizeof(float));

    // adjusting weights in transition matrix
    for (int i = 0; i < buf_index-1; i++){
        trans_matrix
            [toIndex(buffer[i], word_set, set_index)]
            [toIndex(buffer[i+1], word_set, set_index)]++;
    }

    // evaluate weights
    for (int i = 0; i < set_index; i++){
        float sum = 0.0f;
        for (int j = 0; j < set_index; j++){
            sum += trans_matrix[i][j];
        }
        if (sum == 0) continue; 
        for (int j = 0; j < set_index; j++){
            trans_matrix[i][j] = trans_matrix[i][j]/sum;
        }
    }

    if (debug){
        printf("\n--------------------------------------------------\nFilepath: \t\t%s\n# generated sentences: \t%d\nDebug mode: \t\t%s\n--------------------------------------------------\n", 
        file_path, max_sentences, debug ? "true" : "false");

        // debug print transition matrix
        for (int i = 0; i < set_index; i++) printf("    %2d", i);
        printf("\n");
        for (int i = 0; i < set_index; i++){
            printf("%2d| ", i);
            for (int j = 0; j < set_index; j++){
                printf("%2.2f  ", trans_matrix[i][j]);
            }
            printf("\n");
        }
    }

    // Generate sentence based on markov chain model
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
    while(!done){
        // if end of sentence
        if (contains(word_set[state], '.') || contains(word_set[state], '\n')) {
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

    // free transition matrix
    for (int i = 0; i < set_index; i++) free(trans_matrix[i]);
    free(trans_matrix);

    return 0;
}