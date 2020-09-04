#include "common.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>

#include <arpa/inet.h>


void init_guess(uint8_t *dst){
	uint8_t c;
	printf("guess a letter> ");
	scanf(" %c", &c);
	dst[0] = GUESS_TYPE;
	dst[1] = c;
}

void print_positions(uint8_t *positions, uint8_t positions_size){
	char buf[BUFSZ];
	
	printf("positions: ");
	for(uint8_t i = 0; i < positions_size; i++){
		sprintf(buf, "%d", positions[i] + 1);
		printf("%s ", buf);
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
	printf("size of word to be guessed: %d\n", confirmation[1]);

	uint8_t guess[2];
	uint8_t buf[BUFSZ];
	while(1){
		init_guess(guess);
		size_t count = send(s, guess, 2, 0);
		if(count != 2){
			logexit("send");
		}

		memset(buf, 0, BUFSZ);
		recv(s, buf, BUFSZ, 0);

		uint8_t type = buf[0];
		if(type == GAMEOVER_TYPE){
			//acabou o jogo
			break;
		}
		uint8_t num_occurrences = buf[1];
        printf("[rsp] num_occurrences: %d\n", num_occurrences);
		//so imprime as posicoes se acertar
		if(num_occurrences > 0) {
			print_positions(buf+2, num_occurrences);
		}	
	}

	printf("you won!\n");
	
	
	close(s);
	exit(EXIT_SUCCESS);
}