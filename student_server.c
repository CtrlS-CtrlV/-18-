/* сервер: отримує та десеріалізує записи */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include "student_record.h"

#define PORT 9200

static ssize_t read_full(int fd, void *buf, size_t n)
{
    size_t total = 0;
    while (total < n) {
        ssize_t r = read(fd, (char *)buf + total, n - total);
        if (r == -1) {
            if (errno == EINTR) continue;
            return -1;
        }
        if (r == 0) return (ssize_t)total;
        total += r;
    }
    return (ssize_t)total;
}

int main(void)
{
    int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd == -1) { perror("socket"); return EXIT_FAILURE; }

    int opt = 1;
    setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in srv;
    memset(&srv, 0, sizeof(srv));
    srv.sin_family      = AF_INET;
    srv.sin_port        = htons(PORT);
    srv.sin_addr.s_addr = INADDR_ANY;

    if (bind(listen_fd, (struct sockaddr *)&srv, sizeof(srv)) == -1) {
        perror("bind"); close(listen_fd); return EXIT_FAILURE;
    }
    if (listen(listen_fd, 5) == -1) {
        perror("listen"); close(listen_fd); return EXIT_FAILURE;
    }

    printf("Студент-сервер слухає на порту %d...\n", PORT);

    int conn_fd = accept(listen_fd, NULL, NULL);
    if (conn_fd == -1) { perror("accept"); close(listen_fd); return EXIT_FAILURE; }

    uint8_t wire[WIRE_SIZE];

    while (read_full(conn_fd, wire, WIRE_SIZE) == WIRE_SIZE) {
        StudentRecord rec;
        if (deserialize_student(wire, WIRE_SIZE, &rec) == 0) {
            printf("Отримано: %-20s | ID: %u | Вік: %u | GPA: %.2f\n",
                   rec.name, rec.student_id, rec.age, rec.gpa_x100 / 100.0);
        }
    }

    printf("Клієнт відключився.\n");
    close(conn_fd);
    close(listen_fd);
    return EXIT_SUCCESS;
}
