#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define PORT 8080
#define BUFF_SIZE 1024

void send_file(FILE *fp, int sockfd) {
    char buff[BUFF_SIZE] = {0};
    size_t bytes_read;

    while ((bytes_read = fread(buff, 1, BUFF_SIZE, fp))) {
        if (send(sockfd, buff, bytes_read, 0) == -1) {
            perror("Error sending file");
            exit(1);
        }
        memset(buff, 0, BUFF_SIZE);
    }
}

int main() {
    int server_fd, new_sock;
    struct sockaddr_in addr;

    int addrlen = sizeof(addr);
    FILE *fp;
    char *filename = "firmware.bin";

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket failed");
        exit(EXIT_FAILURE);
    }

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("bind() failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 3) < 0) {
        perror("listen() failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d...\n", PORT);

    if ((new_sock = accept(server_fd, (struct sockaddr *)&addr,
                           (socklen_t *)&addrlen)) < 0) {
        perror("accept() failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }
    printf("Find new connection. Sendong file...\n");

    fp = fopen(filename, "rb");
    if (fp == NULL) {
        perror("File open failed");
        close(new_sock);
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    send_file(fp, new_sock);
    printf("File sent successfully\n");

    fclose(fp);
    close(new_sock);
    close(server_fd);

    return 0;
}