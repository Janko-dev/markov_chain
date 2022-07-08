CC = gcc
CFLAGS = -Wall -Wextra -g

IN_WORD = word_generator.c
IN_SENT = sentence_generator.c

word:
	$(CC) $(IN_WORD) -o wgen $(CFLAGS)  

sent:
	$(CC) $(IN_SENT) -o sgen $(CFLAGS)