#ifndef _MARKOV_H
#define _MARKOV_H

#define BUF_SIZE 2048
#define DELIM " \n\t"

int contains_word(const char* word, char** wset, int set_size);
int contains_char(const char* word, char c);

void read_file(const char* filepath, char* output, long output_size);
void split_input_text(char* input_text, char* delims, char** input_array, int* input_count, char** word_set, int* set_count);

float** create_transition_matrix(int size);
void destroy_transition_matrix(float** trans_matrix, int size);
void train_model(float** trans_matrix, char** input_array, int input_count, char** word_set, int set_count);
void normalize_model(float** trans_matrix, int size);

void print_model(float** trans_matrix, int size);

#endif  //_MARKOV_H