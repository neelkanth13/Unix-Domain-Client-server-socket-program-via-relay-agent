#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>

#define SOCKET_PATH "/tmp/socket"
#define SERVER_SOCKET_PATH "/tmp/server_socket"
#define BUFFER_SIZE 256

int main() {
    struct sockaddr_un server_addr, client_addr;
    int server_fd, client_fd, rc;
    char buffer[BUFFER_SIZE] = {0};

    // Create server socket
    server_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("socket");
        exit(1);
    }
    
    unlink(SOCKET_PATH);
    // Bind socket to socket path
    memset(&server_addr, 0, sizeof(struct sockaddr_un));
    server_addr.sun_family = AF_UNIX;
    strncpy(server_addr.sun_path, SOCKET_PATH, sizeof(server_addr.sun_path) - 1);

    rc = bind(server_fd, (struct sockaddr *)&server_addr, sizeof(struct sockaddr_un));
    if (rc < 0) {
        perror("bind");
        exit(1);
    }

    // Listen for incoming connections
    rc = listen(server_fd, 1);
    if (rc < 0) {
        perror("listen");
        exit(1);
    }

    // Accept client connection
    socklen_t client_len = sizeof(client_addr);
    client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_len);
    if (client_fd < 0) {
        perror("accept");
        exit(1);
    }

    // Connect to server
    int server_sock = socket(AF_UNIX, SOCK_STREAM, 0);
    if (server_sock < 0) {
        perror("socket");
        exit(1);
    }

    memset(&server_addr, 0, sizeof(struct sockaddr_un));
    server_addr.sun_family = AF_UNIX;
    strncpy(server_addr.sun_path, SERVER_SOCKET_PATH, sizeof(server_addr.sun_path) - 1);

    rc = connect(server_sock, (struct sockaddr *)&server_addr, sizeof(struct sockaddr_un));
    if (rc < 0) {
        perror("connect");
        exit(1);
    }

    // Start relaying messages between client and server
    while (1) {
        memset(buffer, 0, BUFFER_SIZE);

        // Read from client
        rc = read(client_fd, buffer, BUFFER_SIZE);
        if (rc < 0) {
            perror("read");
            exit(1);
        } else if (rc == 0) {
            printf("Client disconnected\n");
            break;
        }
        printf("Message received from the client: %s\n", buffer);

        // Write to server
        rc = write(server_sock, buffer, rc);
        if (rc < 0) {
            perror("write");
            exit(1);
        }
        printf("Message sent to the server: %s\n", buffer);

        // Read from server
        rc = read(server_sock, buffer, BUFFER_SIZE);
        if (rc < 0) {
            perror("read");
            exit(1);
        } else if (rc == 0) {
            printf("Server disconnected\n");
            break;
        }

        // Write to client
        rc = write(client_fd, buffer, rc);
        if (rc < 0) {
            perror("write");
            exit(1);
        }
    }

    close(client_fd);
    close(server_sock);
    close(server_fd);

    unlink(SOCKET_PATH);

    return 0;
}

