/* серіалізує масив Person у JSON, відправляє */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include "json_message.h"

#define SERVER_IP   "127.0.0.1"
#define SERVER_PORT 9300
#define JSON_BUF    4096

int main(void)
{
    Person people[] = {
        { "Shevchenko Taras",  21, 4.50, "Kyiv"    },
        { "Franko Ivan",       22, 3.85, "Lviv"    },
        { "Ukrainka Lesia",    20, 4.95, "Lutsk"   },
        { "Skovoroda Hryhorii",25, 4.20, "Kharkiv" },
    };

    int count = sizeof(people) / sizeof(people[0]);
    char json_buf[JSON_BUF];
    int json_len = json_serialize_array(people, count, json_buf, sizeof(json_buf));

    printf("JSON (%d байт):\n%s\n", json_len, json_buf);

    int fd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in srv;
    srv.sin_family = AF_INET;
    srv.sin_port = htons(SERVER_PORT);
    inet_pton(AF_INET, SERVER_IP, &srv.sin_addr);

    if (connect(fd, (struct sockaddr *)&srv, sizeof(srv)) == -1) {
        perror("connect"); close(fd); return EXIT_FAILURE;
    }

    uint32_t net_len = htonl(json_len);
    write(fd, &net_len, 4);
    write(fd, json_buf, json_len);

    close(fd);
    return EXIT_SUCCESS;
}
