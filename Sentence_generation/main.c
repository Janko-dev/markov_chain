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

#define BUF_SIZE 2048
#define WORD_LEN 40

#define rnd_float ((rand() % RAND_MAX)/(float)RAND_MAX)

int toIndex(char* word, char wset[BUF_SIZE][WORD_LEN], int set_index){
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
    FILE* f = fopen("test.txt", "r");
    char text[0xffff];
    size_t len = fread(text, sizeof(char), 0xffff, f);
    text[len] = '\0';
    //printf("TEST: %s\n", text);
    //char text[] = "Both versions convey a topic; it's pretty easy to predict that the paragraph will be about epidemiological evidence, but only the second version establishes an argumentative point and puts it in context. The paragraph doesn't just describe the epidemiological evidence; it shows how epidemiology is telling the same story as etiology. Similarly, while Version A doesn't relate to anything in particular, Version B immediately suggests that the prior paragraph addresses the biological pathway (i.e. etiology) of a disease and that the new paragraph will bolster the emerging hypothesis with a different kind of evidence. As a reader, it's easy to keep track of how the paragraph about cells and chemicals and such relates to the paragraph about populations in different places.";

    // string buffer
    char buffer[BUF_SIZE][WORD_LEN];
    int buf_index = 0;
    // set of words (no duplicates)
    char word_set[BUF_SIZE][WORD_LEN];
    int set_index = 0;

    // splitting the input string
    char* token;
    token = strtok(text, " ");

    while (token != NULL && buf_index < BUF_SIZE){
        //size_t word_len = strlen(token);
        
        //printf("TEST: %s - %d\tbuf_index: %d\tset_index: %d\n", token, word_len, buf_index, set_index);

        // malloc and copy token to string buffer
        //buffer[buf_index] = (char*)malloc(sizeof(char) * word_len);
        // if (buffer[buf_index] == NULL){
        //     fprintf(stderr, "Error: can't allocate enough resources for the string buffer\n");
        //     exit(1);
        // }
        strcpy(buffer[buf_index], token);
        //printf("TEST: %s\n", buffer[buf_index]);
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
        //word_set[set_index] = (char*)malloc(sizeof(char) * word_len);
        //printf("LEN: %d\n", sizeof(char) * word_len);
        // if (word_set[set_index] == NULL){
        //     fprintf(stderr, "Error: can't allocate enough resources for the word set\n");
        //     exit(1);
        // }
        // TODO: segfault after +-16 mallocs, needs fixing
        strcpy(word_set[set_index], token);
        set_index++;

        token = strtok(NULL, " ");
    }

    // debug word set
    // printf("Word_set: ");
    // for (int i = 0; i < set_index; i++) printf("%s ", word_set[i]); 

    // // debug string buffer
    // printf("\n\nBuffer: ");
    // for (int i = 0; i < buf_index; i++) printf("%s ", buffer[i]); 
    // printf("\n\n");
    
    // state transition matrix
    float** trans_matrix = (float**)malloc(set_index * sizeof(float*));
    for (int i = 0; i < set_index; i++) trans_matrix[i] = (float*)calloc(set_index, sizeof(float));
    //[WORD_SIZE][WORD_SIZE] = {{0}};

    // adjusting weights in transition matrix
    for (int i = 0; i < buf_index-1; i++){
        trans_matrix
            [toIndex(buffer[i], word_set, set_index)]
            [toIndex(buffer[i+1], word_set, set_index)]++;
    }

    // string buffer
    // for (int i = 0; i < buf_index; i++) free(buffer[i]);
    // free(buffer);

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

    // debug print transition matrix
    // for (int i = 0; i < set_index; i++) printf("    %2d", i);
    // printf("\n");
    // for (int i = 0; i < set_index; i++){
    //     printf("%2d| ", i);
    //     for (int j = 0; j < set_index; j++){
    //         printf("%2.2f  ", trans_matrix[i][j]);
    //     }
    //     printf("\n");
    // }

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

    // // clean word set
    // for (int i = 0; i < set_index; i++) free(word_set[i]);
    // free(word_set);

    return 0;
}