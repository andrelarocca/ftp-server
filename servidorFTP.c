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

int main (int argc, char **argv) {
    if (argc < 3) {
        logexit("missing params");
    }

    int port = atoi(argv[1]);
    int buffer_size = atoi(argv[2]);

    char buffer[buffer_size];
    char file_name[BUFSZ];
    FILE *file;

    int s = socket(AF_INET, SOCK_STREAM, 0);

    if (s != -1) {
        struct in_addr addr = {
            .s_addr = htonl(INADDR_LOOPBACK)
        };

        struct sockaddr_in client, dst = {
            .sin_family = AF_INET,
            .sin_port = htons(port),
            .sin_addr = addr
        };

        struct sockaddr* sa_dst = (struct sockaddr*) &dst;

        if (bind(s, sa_dst, sizeof(dst)) >= 0) {
            int c;

            listen(s, 1);
            socklen_t client_length = sizeof(client);

            if ((c = accept(s, (struct sockaddr*) &client,  &client_length)) >= 0) {
                if (recv(c, file_name, BUFSZ, MSG_WAITALL) > 0) {
                    file = fopen(file_name, "r");

                    if (file == NULL) {
                        logexit("file");
                    }

                    while (fread(buffer, buffer_size, 1, file) > 0) {
                        send(c, buffer, buffer_size, 0);
                    }
                }
            }
        }
    }

    // termina o servidor
    close(s);
    fclose(file);
    exit(EXIT_SUCCESS);
}
