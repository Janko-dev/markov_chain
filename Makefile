CC = gcc
CFLAGS = -Wall -Wextra -g

IN_WORD = word_generator.c
IN_NGRAM = ngram_word_generator.c
IN_SENT = sentence_generator.c

word:
	$(CC) $(IN_WORD) -o wgen $(CFLAGS)  

ngram_word:
	$(CC) $(IN_NGRAM) -o ngram $(CFLAGS)  

sent:
	$(CC) $(IN_SENT) -o sgen $(CFLAGS)