/* інтерактивний клієнт телефонної книги */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include "phonebook.h"

#define SERVER_IP   "127.0.0.1"
#define SERVER_PORT 9400

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

static int send_command(int fd, uint8_t cmd, const void *data, uint16_t len)
{
    PbHeader hdr = { .command = cmd, .payload_len = htons(len) };
    write(fd, &hdr, PB_HDR_SIZE);
    if (len > 0 && data) {
        write(fd, data, len);
    }
    return 0;
}

int main(void)
{
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in srv;
    srv.sin_family = AF_INET;
    srv.sin_port = htons(SERVER_PORT);
    inet_pton(AF_INET, SERVER_IP, &srv.sin_addr);

    if (connect(fd, (struct sockaddr *)&srv, sizeof(srv)) == -1) {
        perror("connect"); close(fd); return EXIT_FAILURE;
    }

    printf("Команди: add <ім'я> <телефон> | search <ім'я> | delete <ім'я> | list | quit\n");

    char line[256];
    while (printf("> "), fgets(line, sizeof(line), stdin) != NULL) {
        char cmd_str[16], arg1[PB_NAME_LEN], arg2[PB_PHONE_LEN];
        int nargs = sscanf(line, "%15s %47s %19s", cmd_str, arg1, arg2);

        if (strcmp(cmd_str, "quit") == 0) break;

        if (strcmp(cmd_str, "add") == 0 && nargs == 3) {
            uint8_t payload[PB_NAME_LEN + PB_PHONE_LEN];
            memset(payload, 0, sizeof(payload));
            strncpy((char *)payload, arg1, PB_NAME_LEN - 1);
            strncpy((char *)payload + PB_NAME_LEN, arg2, PB_PHONE_LEN - 1);
            send_command(fd, CMD_ADD, payload, sizeof(payload));
        } else if (strcmp(cmd_str, "search") == 0 && nargs >= 2) {
            send_command(fd, CMD_SEARCH, arg1, strlen(arg1));
        } else if (strcmp(cmd_str, "delete") == 0 && nargs >= 2) {
            send_command(fd, CMD_DELETE, arg1, strlen(arg1));
        } else if (strcmp(cmd_str, "list") == 0) {
            send_command(fd, CMD_LIST, NULL, 0);
        } else {
            printf("Невідома команда або аргументи.\n");
            continue;
        }

        PbHeader rsp_hdr;
        if (read_full(fd, &rsp_hdr, PB_HDR_SIZE) == PB_HDR_SIZE) {
            uint16_t plen = ntohs(rsp_hdr.payload_len);
            uint8_t *payload = malloc(plen > 0 ? plen : 1);
            if (plen > 0) read_full(fd, payload, plen);

            if (rsp_hdr.command == RSP_OK) {
                if (nargs >= 1 && strcmp(cmd_str, "list") == 0) {
                    uint16_t count;
                    memcpy(&count, payload, 2);
                    count = ntohs(count);
                    printf("Знайдено записів: %u\n", count);
                    PbEntry *entries = (PbEntry *)(payload + 2);
                    for (int i = 0; i < count; i++) {
                        printf("  %d. %s — %s\n", i + 1, entries[i].name, entries[i].phone);
                    }
                } else if (nargs >= 2 && strcmp(cmd_str, "search") == 0 && plen >= sizeof(PbEntry)) {
                    PbEntry *ent = (PbEntry *)payload;
                    printf("Знайдено: %s — %s\n", ent->name, ent->phone);
                } else {
                    printf("Успішно виконано (OK).\n");
                }
            } else if (rsp_hdr.command == RSP_NOT_FOUND) {
                printf("Запис не знайдено.\n");
            } else {
                printf("Помилка виконання.\n");
            }
            free(payload);
        }
    }

    close(fd);
    return EXIT_SUCCESS;
}
