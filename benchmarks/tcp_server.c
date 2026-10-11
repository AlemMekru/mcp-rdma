#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

static int transfer(int fd, void *buf, size_t len, int sending) {
    size_t done = 0;
    while (done < len) {
        ssize_t n = sending
            ? send(fd, (char *)buf + done, len - done, 0)
            : recv(fd, (char *)buf + done, len - done, 0);
        if (n <= 0) return -1;
        done += (size_t)n;
    }
    return 0;
}

int main(void) {
    int server = socket(AF_INET, SOCK_STREAM, 0);
    if (server < 0) return 1;

    int yes = 1;
    setsockopt(server, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));

    struct sockaddr_in addr = {0};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(18516);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(server, (void *)&addr, sizeof(addr)) != 0 ||
        listen(server, 1) != 0) {
        perror("TCP listen");
        close(server);
        return 1;
    }

    puts("TCP benchmark server listening on port 18516...");
    fflush(stdout);

    int client = accept(server, NULL, NULL);
    if (client < 0) {
        close(server);
        return 1;
    }

    if (setsockopt(client, IPPROTO_TCP, TCP_NODELAY,
                   &yes, sizeof(yes)) != 0) {
        perror("TCP_NODELAY");
        close(client);
        close(server);
        return 1;
    }

    int status = 0;
    for (int i = 0; i < 1100; i++) {
        char request[10];

        if (transfer(client, request, sizeof(request), 0) != 0 ||
            memcmp(request, "RDMA-BENCH", sizeof(request)) != 0) {
            status = 1;
            break;
        }

        char ack[32];
        int length = snprintf(ack, sizeof(ack), "ACK %d", i + 1);

        if (transfer(client, ack, (size_t)length, 1) != 0) {
            status = 1;
            break;
        }
    }

    close(client);
    close(server);
    puts(status ? "TCP benchmark FAILED" : "TCP benchmark completed");
    return status;
}
