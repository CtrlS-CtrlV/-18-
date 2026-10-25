/* сервер: отримує JSON, парсить, виводить */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include "json_message.h"

#define PORT     9300
#define JSON_BUF 4096

int main(void)
{
    int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    int opt = 1;
    setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in srv;
    srv.sin_family = AF_INET;
    srv.sin_port = htons(PORT);
    srv.sin_addr.s_addr = INADDR_ANY;

    bind(listen_fd, (struct sockaddr *)&srv, sizeof(srv));
    listen(listen_fd, 5);

    printf("JSON-сервер слухає на порту %d...\n", PORT);
    int conn_fd = accept(listen_fd, NULL, NULL);

    uint32_t net_len;
    read(conn_fd, &net_len, 4);
    uint32_t json_len = ntohl(net_len);

    char json_buf[JSON_BUF];
    size_t total_read = 0;
    while (total_read < json_len) {
        ssize_t r = read(conn_fd, json_buf + total_read, json_len - total_read);
        if (r <= 0) break;
        total_read += r;
    }
    json_buf[json_len] = '\0';

    printf("Отримано JSON:\n%s\n\n", json_buf);

    Person people[MAX_ITEMS];
    int count = json_parse_array(json_buf, people, MAX_ITEMS);

    printf("Розпарсено %d записів:\n", count);
    for (int i = 0; i < count; i++) {
        printf("  %d. %-20s | Вік: %u | Бал: %.2f | Місто: %s\n",
               i + 1, people[i].name, people[i].age,
               people[i].score, people[i].city);
    }

    close(conn_fd);
    close(listen_fd);
    return EXIT_SUCCESS;
}
