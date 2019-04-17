#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

#include <sys/socket.h>
#include <arpa/inet.h>

#define BUFSZ 64

void logexit (const char *str) {
    if (errno) {
        perror(str);
    } else {
        puts(str);
    }

    exit(EXIT_FAILURE);
}

void unceaser (char *str, int size, int key) {
    for (int i = 0; i < size; i++) {
        str[i] = str[i] - key;

        if (str[i] < 97) {
            str[i] = str[i] + 26;
        }
    }
}

void *client_thread (void *param) {
	struct conn_data *data = param;
	int c = *(int*)param;

    // define temporizador de 15 segundos
    struct timeval timeout;
    timeout.tv_sec = 15;
    timeout.tv_usec = 0;

    setsockopt(c, SOL_SOCKET, SO_RCVTIMEO, (char*) &timeout, sizeof(timeout));

    char buffer[BUFSZ];
    char message[BUFSZ];

    // tamanho da mensagem recebido
    if (recv(c, buffer, 4, MSG_WAITALL) == 4) {
        uint32_t string_size = ntohl(*(uint32_t *)buffer);

        // recebe mensagem do cliente
        if (recv(c, message, string_size, MSG_WAITALL) == string_size) {

            // recebe mensagem codificada
            if (recv(c, buffer, 4, MSG_WAITALL) == 4) {
                uint32_t ceasars_cypher_key = ntohl(*(uint32_t *)buffer);

                // decodifica mensagem e envia de volta ao cliente
                unceaser(message, string_size, ceasars_cypher_key);
                send(c, message, string_size, 0);

                // imprime a mensagem na saída padrão
                message[string_size] = '\0';
                printf("%s\n", message);
                fflush(stdout);
            }
        }
    }

    // termina a conexão com o cliente
	close(c);
	pthread_exit(EXIT_SUCCESS);
}

int main (int argc, char **argv) {
    if (argc < 2) {
        logexit("missing port param");
    }

    int s = socket(AF_INET, SOCK_STREAM, 0);

    if (s != -1) {
        struct in_addr addr = {
            .s_addr = htonl(INADDR_LOOPBACK)
        };

        struct sockaddr_in client, dst = {
            .sin_family = AF_INET,
            .sin_port = htons(atoi(argv[1])),
            .sin_addr = addr
        };

        struct sockaddr* sa_dst = (struct sockaddr*) &dst;

        if (bind(s, sa_dst, sizeof(dst)) >= 0) {
            int c;

            listen(s, 5);
            socklen_t client_length = sizeof(client);

            // a combinacao ctrl+c termina a execucao
            // cada iteracao do while representa uma nova conexao de cliente
            while ((c = accept(s, (struct sockaddr*) &client,  &client_length)) >= 0) {
                // cria a thread para executar conexão de cliente
                pthread_t tid;
                pthread_create(&tid, NULL, client_thread, &c);
            }
        }
    }

    // termina o servidor
    close(s);
    exit(EXIT_SUCCESS);
}
