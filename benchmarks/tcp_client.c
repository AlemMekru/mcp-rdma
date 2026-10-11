#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <time.h>
#include <unistd.h>

static double elapsed_us(struct timespec a, struct timespec b) {
    return (b.tv_sec-a.tv_sec)*1e6 +
           (b.tv_nsec-a.tv_nsec)/1e3;
}

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

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <server-ip>\n", argv[0]);
        return 1;
    }

    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) return 1;

    int yes = 1;
    if (setsockopt(fd, IPPROTO_TCP, TCP_NODELAY,
                   &yes, sizeof(yes)) != 0) {
        perror("TCP_NODELAY");
        close(fd);
        return 1;
    }

    struct sockaddr_in addr = {0};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(18516);

    if (inet_pton(AF_INET, argv[1], &addr.sin_addr) != 1 ||
        connect(fd, (struct sockaddr *)&addr, sizeof(addr)) != 0) {
        perror("TCP connect");
        close(fd);
        return 1;
    }

    puts("TCP connection established.");

    FILE *csv = fopen("benchmarks/tcp_rtt.csv", "w");
    if (!csv) {
        perror("Cannot create TCP benchmark CSV");
        close(fd);
        return 1;
    }

    fprintf(csv, "iteration,rtt_us\n");

    int status = 0;
    for (int i = 0; i < 1100; i++) {
        char expected[32];
        char received[32] = {0};

        int len = snprintf(expected, sizeof(expected),
                           "ACK %d", i + 1);

        struct timespec start, end;
        clock_gettime(CLOCK_MONOTONIC, &start);

        if (transfer(fd, "RDMA-BENCH", 10, 1) != 0 ||
            transfer(fd, received, (size_t)len, 0) != 0) {
            status = 1;
            break;
        }

        clock_gettime(CLOCK_MONOTONIC, &end);

        if (memcmp(expected, received, (size_t)len) != 0) {
            status = 1;
            break;
        }

        if (i >= 100)
            fprintf(csv, "%d,%.3f\n",
                    i - 100, elapsed_us(start, end));
    }

    if (fclose(csv) != 0)
        status = 1;

    close(fd);

    puts(status ? "TCP benchmark FAILED"
                : "TCP benchmark completed");

    return status;
}
