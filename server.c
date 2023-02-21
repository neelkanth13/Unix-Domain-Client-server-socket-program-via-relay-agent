#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>

#define SOCKET_PATH "/tmp/server_socket"
#define BUFFER_SIZE 1024

int main() {
    int server_fd, client_fd;
    struct sockaddr_un server_address, client_address;
    char buffer[BUFFER_SIZE];

    // Remove any existing socket file
    unlink(SOCKET_PATH);

    // Create server socket
    server_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (server_fd == -1) {
        perror("Failed to create server socket");
        exit(EXIT_FAILURE);
    }

    // Set server address
    memset(&server_address, 0, sizeof(server_address));
    server_address.sun_family = AF_UNIX;
    strcpy(server_address.sun_path, SOCKET_PATH);

    // Bind server socket
    if (bind(server_fd, (struct sockaddr *)&server_address, sizeof(server_address)) == -1) {
        perror("Failed to bind server socket");
        exit(EXIT_FAILURE);
    }

    // Listen for client connections
    if (listen(server_fd, 1) == -1) {
        perror("Failed to listen for client connections");
        exit(EXIT_FAILURE);
    }

    // Wait for client connection
    printf("Waiting for client connection...\n");
    socklen_t client_address_len = sizeof(client_address);
    client_fd = accept(server_fd, (struct sockaddr *)&client_address, &client_address_len);
    if (client_fd == -1) {
        perror("Failed to accept client connection");
        exit(EXIT_FAILURE);
    }
    printf("Client connected\n");

    while (1) {
        // Receive message from client
        memset(buffer, 0, BUFFER_SIZE);
        if (recv(client_fd, buffer, BUFFER_SIZE, 0) == -1) {
            perror("Failed to receive message from client");
            exit(EXIT_FAILURE);
        }
        printf("Received message from client: %s\n", buffer);

        // Send message to relay
        if (send(client_fd, buffer, strlen(buffer), 0) == -1) {
            perror("Failed to send message to relay");
            exit(EXIT_FAILURE);
        }
        printf("Sent message to relay\n");
    }

    // Close sockets
    close(client_fd);
    close(server_fd);

    return 0;
}

