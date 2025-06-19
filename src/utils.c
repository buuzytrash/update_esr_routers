// File: src/utils.c
#include "utils.h"

#include <arpa/inet.h>
#include <fcntl.h>
#include <stdbool.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <unistd.h>

bool pingable(const char *ip) {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) return false;
    struct sockaddr_in addr = {0};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(22);
    inet_aton(ip, &addr.sin_addr);
    fcntl(sock, F_SETFL, O_NONBLOCK);

    connect(sock, (struct sockaddr *)&addr, sizeof(addr));
    fd_set wf;
    FD_ZERO(&wf);
    FD_SET(sock, &wf);
    struct timeval tv = {1, 0};
    if (select(sock + 1, NULL, &wf, NULL, &tv) > 0) {
        int err = 0;
        socklen_t len = sizeof(err);
        getsockopt(sock, SOL_SOCKET, SO_ERROR, &err, &len);
        close(sock);
        return err == 0;
    }
    close(sock);
    return false;
}

void sleep_sec(int s) { sleep(s); }
