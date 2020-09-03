#include "common.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/socket.h>
#include <sys/types.h>

int set_protocol(int *proto){
    *proto = 0;
    printf("choose the protocol version (4/6)> ");
    scanf("%d", proto);
    if(*proto == 4 || *proto == 6){
        return 0;
    }
    return -1;
}

int set_palavra(char *dst){
    memset(dst, 0, BUFSZ);
    printf("choose a word to be guessed> ");
    if(!fgets(dst, BUFSZ-1, stdin)){
        return -1;
    }
    dst[strlen(dst)-1] = '\0';
    return 0;
}

int send_confirmation(int csock, uint8_t word_size){
    uint8_t confirmation[2];
    memset(confirmation, 0, 2);
    confirmation[0] = CONFIRMATION_TYPE;
    confirmation[1] = word_size;
    size_t count = send(csock, confirmation, 2, 0);
    if(count != 2){
        return -1;
    }
    return 0;
}

int terminate(int csock){
    char flag[1];
    flag[0] = GAMEOVER_TYPE;
    if(1 != send(csock, flag, 1, 0)){
        return -1;
    }
    printf("[rsp] response size: %ld bytes\n", sizeof(flag));
    return 0;
}

uint8_t get_occurrences(char guessed_letter, char *word){
    uint8_t num_occurrences = 0;
    uint8_t word_size = strlen(word);

    for(uint8_t i = 0; i < word_size; i++){
        if(word[i] == guessed_letter){
            num_occurrences++;
        }
    }

    return num_occurrences;
}

void init_response(uint8_t *response, uint8_t num_occurrences, char *word, char guessed_letter){
    uint8_t word_size = strlen(word);

    response[0] =  RESPONSE_TYPE;
    response[1] = num_occurrences;
    for(uint8_t i = 0, j = 0; i < word_size; i++){
        if(word[i] == guessed_letter){
            response[2+j] = i;
            j++;
        }
    }
}

void usage(int argc, char **argv) {
    printf("usage: %s <server port>\n", argv[0]);
    printf("example: %s 51511\n", argv[0]);
    exit(EXIT_FAILURE);
}

int main(int argc, char **argv) {
    if (argc != 2) {
        usage(argc, argv);
    }

    char word[BUFSZ] = "protocolo";
    /*descomente para ler a palavra do teclado
    if(0 != set_palavra(word)){
        logexit("error reading word");
    }*/
    uint8_t word_size = (uint8_t)strlen(word);
    printf("size of word:%d bytes\n", word_size);

    int proto = 6;
    /*descomente para ler o protocolo do teclado
    if(0 != set_protocol(&proto)){
        printf("please choose a valid protocol\n");
        exit(EXIT_FAILURE);
    }*/

    struct sockaddr_storage storage;
    if (0 != server_sockaddr_init(proto, argv[1], &storage)) {
        usage(argc, argv);
    }

    int s;
    s = socket(storage.ss_family, SOCK_STREAM, 0);
    if (s == -1) {
        logexit("socket");
    }

    int enable = 1;
    if (0 != setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int))) {
        logexit("setsockopt");
    }

    struct sockaddr *addr = (struct sockaddr *)(&storage);
    if (0 != bind(s, addr, sizeof(storage))) {
        logexit("bind");
    }

    if (0 != listen(s, MAX_CONNECTIONS)) {
        logexit("listen");
    }

    char addrstr[BUFSZ];
    addrtostr(addr, addrstr, BUFSZ);
    printf("bound to %s, waiting connections\n", addrstr);

    struct sockaddr_storage cstorage;
    struct sockaddr *caddr = (struct sockaddr *)(&cstorage);
    socklen_t caddrlen = sizeof(cstorage);

    while (1) {
        int csock = accept(s, caddr, &caddrlen);
        if (csock == -1) {
            logexit("accept");
        }

        char caddrstr[BUFSZ];
        addrtostr(caddr, caddrstr, BUFSZ);
        printf("[log] connection from %s\n", caddrstr);

        if (0 != send_confirmation(csock, word_size)) {
            logexit("server confirmation error");
        }
        uint8_t letters_guessed = 0;
        while (1) {
            char guess[2];
            memset(guess, 0, 2);
            if (!recv(csock, guess, 2, 0)) {
                // connection terminated
                break;
            }
            char guessed_letter = guess[1];
            printf("[msg] recieved: %c\n", guessed_letter);

            uint8_t num_occurrences = get_occurrences(guessed_letter, word);
            letters_guessed += num_occurrences;

            // win condition
            if (letters_guessed == word_size) {
                terminate(csock);
                break;
            }

            // 2 bytes para o cabecalho e 1 byte para cada posicao achada
            uint8_t response_size = 2 + num_occurrences;
            uint8_t response[response_size];
            init_response(response, num_occurrences, word, guessed_letter);
            printf("[rsp] response size: %ld bytes\n", sizeof(response));

            size_t count = send(csock, response, sizeof(response), 0);
            if (response_size != count) {
                logexit("server response error");
            }
        }

        close(csock);
    }
    close(s);
    printf("server closed!\n");
    exit(EXIT_SUCCESS);
}