/* клієнт: серіалізує та відправляє запис */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include "student_record.h"

#define SERVER_IP   "127.0.0.1"
#define SERVER_PORT 9200

int main(void)
{
    StudentRecord students[] = {
        { "Shevchenko Taras",   21, 450, 1001 },
        { "Franko Ivan",        22, 385, 1002 },
        { "Ukrainka Lesia",     20, 495, 1003 },
    };

    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd == -1) { perror("socket"); return EXIT_FAILURE; }

    struct sockaddr_in srv;
    memset(&srv, 0, sizeof(srv));
    srv.sin_family = AF_INET;
    srv.sin_port   = htons(SERVER_PORT);
    inet_pton(AF_INET, SERVER_IP, &srv.sin_addr);

    if (connect(fd, (struct sockaddr *)&srv, sizeof(srv)) == -1) {
        perror("connect");
        close(fd);
        return EXIT_FAILURE;
    }

    int count = sizeof(students) / sizeof(students[0]);

    for (int i = 0; i < count; i++) {
        uint8_t wire[WIRE_SIZE];
        serialize_student(&students[i], wire, sizeof(wire));
        
        write(fd, wire, WIRE_SIZE);
        printf("Відправлено: %s (ID=%u, GPA=%.2f)\n",
               students[i].name, students[i].student_id,
               students[i].gpa_x100 / 100.0);
    }

    shutdown(fd, SHUT_RDWR);
    close(fd);
    return EXIT_SUCCESS;
}
