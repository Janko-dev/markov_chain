#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define CHAR_SET 27
#define rnd_float() ((rand() % RAND_MAX)/(float)RAND_MAX)


int asIndex(char c){
    if (c == '\n') return CHAR_SET-1;
    return ((int)c) - 'a';
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

size_t max_line_len = 128; 
void train_model(const char* filepath, float transition_matrix[CHAR_SET][CHAR_SET]){
    FILE* f = fopen(filepath, "r");
    if (f == NULL){
        fprintf(stderr, "Can't open file with given path: %s\n", filepath);
        exit(1);
    }
    char* input = (char*)malloc(sizeof(char) * max_line_len);
    size_t characters;

    // training stage
    while((characters = getline(&input, &max_line_len, f)) < max_line_len){
        for (int i = 0; (input[i+1] > 'a' && input[i+1] < 'z') || input[i+1] == '\n'; i++){
            transition_matrix[asIndex(input[i])][asIndex(input[i+1])]++;
        }
    }
    fclose(f);
}

void normalize_weights(float transition_matrix[CHAR_SET][CHAR_SET]){
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
}

void print_model_matrix(float transition_matrix[CHAR_SET][CHAR_SET]){
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

void generate_random_words(float transition_matrix[CHAR_SET][CHAR_SET], int num){
    printf("\n--------------------------------------------------\n");
    char string_buffer[max_line_len];

    for (int k = 0; k < num; k++){
        int state = rand()%(CHAR_SET-1);
        string_buffer[0] = state + 'a';
        int i = 1;
        for (; state != CHAR_SET-1; i++){
            float rnd_choice = rnd_float();
            for (int j = 0; j < CHAR_SET; j++){
                if (rnd_choice < transition_matrix[state][j]){
                    state = j;
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
}

int main(int argc, char** argv){
    
    srand(time(NULL));
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
    int num_words_generated = string_to_int(*++argv);


    float transition_matrix[CHAR_SET][CHAR_SET] = {{0}};
    train_model(file_path, transition_matrix);
    normalize_weights(transition_matrix);

    // debug print
    if (debug){
        printf("\n--------------------------------------------------\n");
        printf("Filepath: \t\t%s\n# of generated words: \t%d\nDebug mode: \t\t%s",
        file_path, num_words_generated, debug ? "true" : "false");
        printf("\n--------------------------------------------------\n");
        
        print_model_matrix(transition_matrix);
    }

    generate_random_words(transition_matrix, num_words_generated);
    
    return 0;
}