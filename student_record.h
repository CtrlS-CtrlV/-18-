/* визначення структури та wire format */
#ifndef STUDENT_RECORD_H
#define STUDENT_RECORD_H

#include <stdint.h>
#include <arpa/inet.h>
#include <string.h>

#define NAME_LEN 32

typedef struct {
    char     name[NAME_LEN];   /* ім'я студента (фіксований розмір) */
    uint16_t age;              /* вік */
    uint32_t gpa_x100;         /* середній бал × 100 (напр., 385 = 3.85) */
    uint32_t student_id;       /* ідентифікатор */
} StudentRecord;

/* Wire format: NAME_LEN + 2 + 4 + 4 = 42 байти */
#define WIRE_SIZE (NAME_LEN + 2 + 4 + 4)

static inline size_t serialize_student(const StudentRecord *rec, uint8_t *buf, size_t buf_sz)
{
    if (buf_sz < WIRE_SIZE) return 0;

    size_t offset = 0;
    
    /* 1. Копіюємо ім'я (фіксований розмір NAME_LEN) */
    memset(buf, 0, NAME_LEN);
    strncpy((char *)buf, rec->name, NAME_LEN - 1);
    offset += NAME_LEN;

    /* 2. Вік (uint16_t) -> network byte order (htons) */
    uint16_t net_age = htons(rec->age);
    memcpy(buf + offset, &net_age, sizeof(net_age));
    offset += sizeof(net_age);

    /* 3. GPA (uint32_t) -> network byte order (htonl) */
    uint32_t net_gpa = htonl(rec->gpa_x100);
    memcpy(buf + offset, &net_gpa, sizeof(net_gpa));
    offset += sizeof(net_gpa);

    /* 4. Student ID (uint32_t) -> network byte order (htonl) */
    uint32_t net_id = htonl(rec->student_id);
    memcpy(buf + offset, &net_id, sizeof(net_id));
    offset += sizeof(net_id);

    return WIRE_SIZE;
}

static inline int deserialize_student(const uint8_t *buf, size_t len, StudentRecord *rec)
{
    if (len < WIRE_SIZE) return -1;

    size_t offset = 0;

    /* 1. Ім'я */
    memcpy(rec->name, buf + offset, NAME_LEN);
    rec->name[NAME_LEN - 1] = '\0'; /* Гарантований нуль-термінатор */
    offset += NAME_LEN;

    /* 2. Вік */
    uint16_t net_age;
    memcpy(&net_age, buf + offset, sizeof(net_age));
    rec->age = ntohs(net_age);
    offset += sizeof(net_age);

    /* 3. GPA */
    uint32_t net_gpa;
    memcpy(&net_gpa, buf + offset, sizeof(net_gpa));
    rec->gpa_x100 = ntohl(net_gpa);
    offset += sizeof(net_gpa);

    /* 4. Student ID */
    uint32_t net_id;
    memcpy(&net_id, buf + offset, sizeof(net_id));
    rec->student_id = ntohl(net_id);
    offset += sizeof(net_id);

    return 0;
}

#endif
