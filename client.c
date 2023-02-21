#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>

#define SOCK_PATH "/tmp/socket"

int main(int argc, char *argv[]) {
    int s, len;
    struct sockaddr_un remote;
    char str[100];

    // Create a socket
    if ((s = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        exit(1);
    }

    printf("Trying to connect to server...\n");

    // Set up the address struct for the server
    remote.sun_family = AF_UNIX;
    strcpy(remote.sun_path, SOCK_PATH);
    len = strlen(remote.sun_path) + sizeof(remote.sun_family);

    // Connect to the server
    if (connect(s, (struct sockaddr *)&remote, len) == -1) {
        perror("connect");
        exit(1);
    }

    printf("Connected to server.\n");

    while (1) {
        printf("Enter message: ");
        fgets(str, 100, stdin);

        // Send message to relay
        if (send(s, str, strlen(str), 0) == -1) {
            perror("send");
            exit(1);
        }

        printf("Message sent to relay.\n");

        // Wait for response from relay
        if (recv(s, str, 100, 0) == -1) {
            perror("recv");
            exit(1);
        }

        printf("Received response from relay: %s\n", str);
    }

    close(s);

    return 0;
}

