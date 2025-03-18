#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define PORT 8080
#define BUFF_SIZE 1024

void receive_file(int sockfd, const char *filename) {
    FILE *fp = fopen(filename, "wb");
    if (fp == NULL) {
        perror("File open failed");
        exit(1);
    }

    char buff[BUFF_SIZE] = {0};
    ssize_t bytes_received;

    while ((bytes_received = recv(sockfd, buff, BUFF_SIZE, 0))) {
        if (bytes_received == -1) {
            perror("Error receiving file");
            exit(1);
        }

        fwrite(buff, 1, bytes_received, fp);
        memset(buff, 0, BUFF_SIZE);
    }

    fclose(fp);
}

int main() {
    int sockfd;
    struct sockaddr_in server_addr;
    const char *filename = "received_firmware.bin";

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) <= 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    if (inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr) <= 0) {
        perror("Invalid address/Address not supported");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) <
        0) {
        perror("Connection failed");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    printf("Connected to server\n");

    receive_file(sockfd, filename);
    printf("File received and saves as %s\n", filename);

    close(sockfd);

    return 0;
}