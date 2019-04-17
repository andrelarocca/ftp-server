#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <sys/time.h>
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

void ceaser (char *str, int size, int key) {
    for (int i = 0; i < size; i++) {
        str[i] = str[i] + key;

        if (str[i] > 122) {
            str[i] = str[i] - 26;
        }
    }
}

char *clear_string(char *str, int size) {
    char *buffer;
    buffer = (char*)malloc(size * sizeof(char));

    for (int i = 0; str[i] != '\0'; i++) {
        buffer[i] = str[i];
    }

    return buffer;
}

int main (int argc, char **argv) {
    if (argc < 4) {
        logexit("missing params");
    }

    char *host = argv[1];
    int port = atoi(argv[2]);
    char *file_name = argv[3];
    int buffer_size = atoi(argv[4]);

    // get start time
    struct timeval start;
    struct timeval end;
    gettimeofday(&start, NULL);

    char buffer[buffer_size];

    // create and setup connection
    int s = socket(AF_INET, SOCK_STREAM, 0);

    if (s == -1) {
        logexit("socket");
    }

    struct in_addr addr;
    inet_aton(host, &addr);

    struct sockaddr_in dst = {
        .sin_family = AF_INET,
        .sin_port = htons(port),
        .sin_addr = addr
    };

    struct sockaddr *sa_dst = (struct sockaddr *)&dst;

    if (connect(s, sa_dst, sizeof(dst))) {
        logexit("connect");
    }

    // sends file name
    send(s, file_name, strlen(file_name), 0);

    while (recv(s, buffer, buffer_size, MSG_WAITALL) == buffer_size) {
        printf("Received %d bytes of data", buffer_size);
        buffer[buffer_size] = '\0';
        printf("%s\n", buffer);
        fflush(stdout);
    }

    close(s);
    exit(EXIT_SUCCESS);
}
