#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// the cat is in the tree

//      the  cat  is  in  tree
// the  0    .5    0   0   .5
// cat  0     0    
// is
// in
// tree

#define WORD_SIZE 50

#define rnd_float ((rand() % RAND_MAX)/(float)RAND_MAX)

int toIndex(char* word, char** wset, int set_index){
    for (int i = 0; i < set_index; i++){
        if (strcmp(wset[i], word) == 0){
            return i;
        }
    }
    fprintf(stderr, "word %s not in the set\n", word);
    exit(1);
}

int main(void){
    srand(time(NULL));

    // Input string
    char text[] = "De grote tafel is zacht van binnen, maar ik ben dat ook, dus eigenlijk is dit wel grappig.";

    // string buffer
    char* buffer[WORD_SIZE];
    int buf_index = 0;
    // set of words (no duplicates)
    char* word_set[WORD_SIZE];
    int set_index = 0;

    // splitting the input string
    char* token;
    token = strtok(text, " ");
    while (token != NULL){
        size_t word_len = strlen(token);

        // malloc and copy token to string buffer
        buffer[buf_index] = malloc(sizeof(char) * word_len);
        strcpy(buffer[buf_index], token);
        buf_index++;

        // check if token already in word set
        int in_words_set = 0;
        for (int i = 0; i < set_index; i++){
            if (strcmp(word_set[i], token) == 0){
                in_words_set = 1;
            }
        }
        if (in_words_set){
            token = strtok(NULL, " ");
            continue;
        }

        // if not found in word set, malloc, copy and add word to set
        word_set[set_index] = malloc(sizeof(char) * word_len);
        // TODO: segfault after +-16 mallocs, needs fixing
        strcpy(word_set[set_index], token);
        set_index++;

        token = strtok(NULL, " ");
    }

    // debug word set
    printf("Word_set: ");
    for (int i = 0; i < set_index; i++) printf("%s ", word_set[i]); 

    // debug string buffer
    printf("\nBuffer: ");
    for (int i = 0; i < buf_index; i++) printf("%s ", buffer[i]); 
    printf("\n");
    
    // state transition matrix
    float trans_matrix[WORD_SIZE][WORD_SIZE] = {{0}};
    
    // adjusting weights in transition matrix
    for (int i = 0; i < buf_index-1; i++){
        trans_matrix
            [toIndex(buffer[i], word_set, set_index)]
            [toIndex(buffer[i+1], word_set, set_index)]++;
    }

    // string buffer
    for (int i = 0; i < buf_index; i++) free(buffer[i]);

    // evaluate weights
    for (int i = 0; i < WORD_SIZE; i++){
        float sum = 0.0f;
        for (int j = 0; j < WORD_SIZE; j++){
            sum += trans_matrix[i][j];
        }
        if (sum == 0) continue; 
        for (int j = 0; j < WORD_SIZE; j++){
            trans_matrix[i][j] = trans_matrix[i][j]/sum;
        }
    }

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

    // Generate sentence based on markov chain model
    int done = 0;
    int state = rand()%set_index;
    printf("%s ", word_set[state]);
    while(!done){

        float rnd_choice = rnd_float;
        int selected = 0;
        for (int j = 0; j < set_index; j++){
            if (rnd_choice <= trans_matrix[state][j]){
                state = j;
                printf("%s ", word_set[state]);
                selected = 1;
                goto next;
            }
            rnd_choice -= trans_matrix[state][j];
        }
        next:;
        if (!selected) done = 1;
    }

    // clean word set
    for (int i = 0; i < set_index; i++) free(word_set[i]);

    return 0;
}