#include "common.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>

#include <arpa/inet.h>


void init_guess(char *dst){
	char c;
	printf("guess a letter> ");
	scanf(" %c", &c);
	dst[0] = GUESS_TYPE;
	dst[1] = c;
}

void init_word(char *word, int word_size){
	for(int i = 0; i < word_size; i++){
		word[i] = '_';
	}
	word[word_size-1] = '\0';
}

void print_positions(char *positions, uint8_t positions_size, char *word, char guessed_word){
	for(uint8_t i = 0; i < positions_size; i++){
		for(uint8_t j = 0; j < strlen(word); j++){
			if(positions[i] == j){
				word[j] = guessed_word;
			}
		}
	}

	for(uint8_t i = 0; i < strlen(word); i++){
		printf("%c ", word[i]);
	}
	printf("\n");
}

void print_complete_word(char *word, char guessed_word){
	printf("correct word: ");
	for(uint8_t i = 0; i < strlen(word); i++){
		if(word[i] == '_'){
			printf("%c", guessed_word);
		}
		else{
			printf("%c", word[i]);
		}
	}
	printf("\n");
}

void usage(int argc, char **argv) {
	printf("usage: %s <server IP> <server port>\n", argv[0]);
	printf("example: %s 127.0.0.1 51511\n", argv[0]);
	exit(EXIT_FAILURE);
}

int main(int argc, char **argv) {
	if (argc != 3) {
		usage(argc, argv);
	}

	struct sockaddr_storage storage;
	if (0 != addrparse(argv[1], argv[2], &storage)) {
		usage(argc, argv);
	}

	int s;
	s = socket(storage.ss_family, SOCK_STREAM, 0);
	if (s == -1) {
		logexit("socket");
	}

	struct sockaddr *addr = (struct sockaddr *)(&storage);
	if (0 != connect(s, addr, sizeof(storage))) {
		logexit("connect");
	}

	char addrstr[BUFSZ];
	addrtostr(addr, addrstr, BUFSZ);

	printf("connected to %s\n", addrstr);

	uint8_t confirmation[2];
	memset(confirmation, 0, 2);
	if(!recv(s, confirmation, 2, 0)){
		logexit("client confirmation error");
	}
	uint8_t word_size = confirmation[1];
	printf("size of word to be guessed: %d\n", word_size);

	for(uint8_t i = 0; i < word_size; i++){
		printf("_ ");
	}
	printf("\n");

	char word[word_size+1];
	init_word(word, word_size+1);

	char guess[2];
	char buf[BUFSZ];
	while(1){
		init_guess(guess);
		char guessed_letter = guess[1];
		size_t count = send(s, guess, 2, 0);
		if(count != 2){
			logexit("send");
		}

		memset(buf, 0, BUFSZ);
		recv(s, buf, BUFSZ, 0);

		uint8_t type = buf[0];
		uint8_t num_occurrences = buf[1];

		if(type == GAMEOVER_TYPE){
			//acabou o jogo
			//descomente para imprimir a palavra completa
			//print_complete_word(word, guessed_letter);
			break;
		}

        printf("[rsp] num_occurrences: %d\n", num_occurrences);
		if(num_occurrences == 0){
			printf("try again!\n");
		}
		print_positions(buf+2, num_occurrences, word, guessed_letter);
	}

	printf("you won!\n");
	
	
	close(s);
	exit(EXIT_SUCCESS);
}