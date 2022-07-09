#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define CHAR_SET 27
#define WORD_LEN 12

#define rnd_float() ((rand() % RAND_MAX)/(float)RAND_MAX)

int asIndex(char c){
    if (c == '\n') return CHAR_SET-1;
    return c - 'a';
}

int string_to_int(char* str){
    int res = 0;
    for (int i = 0; str[i] != '\0'; i++){
        if (str[i] < '0' || str[i] > '9'){
            fprintf(stderr, "The NUM_WORDS_GENERATED argument must be a positive integer\n");
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
    
    // Setup transition matrix
    srand(time(NULL));
    float transition_matrix[CHAR_SET][CHAR_SET] = {{0}};
    
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
        //printf("%s", input);
        for (int i = 0; (input[i+1] > 'a' && input[i+1] < 'z') || input[i+1] == '\n'; i++){
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
        if (sum == 0) continue;
        for (int j = 0; j < CHAR_SET; j++){
            transition_matrix[i][j] = transition_matrix[i][j]/sum;
        }
    }

    // print
    if (debug){
        printf("\n--------------------------------------------------\n");
        printf("Filepath: \t\t%s\n# of generated words: \t%d\nDebug mode: \t\t%s",
        file_path, num_words_generated, debug ? "true" : "false");
        printf("\n--------------------------------------------------\n");
        
        for (int i = 0; i < CHAR_SET; i++) 
            i == CHAR_SET-1 ? printf("    \\n") : printf("    %c", i + 'a');
        printf("\n");
        for (int i = 0; i < CHAR_SET; i++){
            i == CHAR_SET-1 ? printf("\\n |") : printf("%c  |", i + 'a');
            for (int j = 0; j < CHAR_SET; j++){
                printf("%2.2f ", transition_matrix[i][j]);
            }
            printf("\n");
        }
    }

    // generate words of length word_len
    printf("\n--------------------------------------------------\n");
    char string_buffer[buf_size];

    for (int k = 0; k < num_words_generated; k++){
        int state = rand()%(CHAR_SET-1);
        //printf("START: %c(%d) ", state + 'a', state);
        string_buffer[0] = state + 'a';
        int i = 1;
        for (; state != CHAR_SET-1; i++){
            float rnd_choice = rnd_float();
            for (int j = 0; j < CHAR_SET; j++){
                if (rnd_choice < transition_matrix[state][j]){
                    state = j;
                    //printf("%c(%d) ", state + 'a', state);
                    if (state == CHAR_SET-1) goto end;
                    
                    string_buffer[i] = (char)state + 'a';
                    break;
                }
                rnd_choice -= transition_matrix[state][j];
            }
        }
        end:;
        string_buffer[i] = '\0';
        printf("generated word %2d:\t%s\n", k+1, string_buffer);
    }

    // hellolopez
    // he -> ll
    // el -> lo
    // ll -> ol
    // lo -> lo
    // ol -> op
    // lo -> pe
    // op -> ez

    return 0;
}