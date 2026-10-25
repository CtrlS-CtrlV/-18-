/* структури та інтерфейс JSON-серіалізації */
#ifndef JSON_MESSAGE_H
#define JSON_MESSAGE_H

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MAX_NAME   64
#define MAX_CITY   32
#define MAX_ITEMS  10

typedef struct {
    char     name[MAX_NAME];
    uint16_t age;
    double   score;
    char     city[MAX_CITY];
} Person;

static inline int json_serialize_person(const Person *p, char *buf, size_t buf_sz)
{
    return snprintf(buf, buf_sz, "{\"name\":\"%s\",\"age\":%u,\"score\":%.2f,\"city\":\"%s\"}",
                    p->name, p->age, p->score, p->city);
}

static inline int json_serialize_array(const Person *arr, int count, char *buf, size_t buf_sz)
{
    int offset = 0;
    offset += snprintf(buf + offset, buf_sz - offset, "[");
    for (int i = 0; i < count; i++) {
        char p_buf[256];
        json_serialize_person(&arr[i], p_buf, sizeof(p_buf));
        offset += snprintf(buf + offset, buf_sz - offset, "%s%s", p_buf, (i < count - 1) ? "," : "");
    }
    offset += snprintf(buf + offset, buf_sz - offset, "]");
    return offset;
}

static inline int json_parse_person(const char *json, Person *p)
{
    const char *n_ptr = strstr(json, "\"name\":\"");
    const char *a_ptr = strstr(json, "\"age\":");
    const char *s_ptr = strstr(json, "\"score\":");
    const char *c_ptr = strstr(json, "\"city\":\"");

    if (!n_ptr || !a_ptr || !s_ptr || !c_ptr) return -1;

    n_ptr += 8;
    const char *n_end = strchr(n_ptr, '\"');
    if (!n_end) return -1;
    size_t n_len = n_end - n_ptr;
    if (n_len >= MAX_NAME) n_len = MAX_NAME - 1;
    memcpy(p->name, n_ptr, n_len);
    p->name[n_len] = '\0';

    p->age = (uint16_t)atoi(a_ptr + 6);
    p->score = atof(s_ptr + 8);

    c_ptr += 8;
    const char *c_end = strchr(c_ptr, '\"');
    if (!c_end) return -1;
    size_t c_len = c_end - c_ptr;
    if (c_len >= MAX_CITY) c_len = MAX_CITY - 1;
    memcpy(p->city, c_ptr, c_len);
    p->city[c_len] = '\0';

    return 0;
}

static inline int json_parse_array(const char *json, Person *arr, int max_items)
{
    int count = 0;
    const char *ptr = json;
    while ((ptr = strchr(ptr, '{')) != NULL && count < max_items) {
        if (json_parse_person(ptr, &arr[count]) == 0) {
            count++;
        }
        ptr++;
    }
    return count;
}

#endif
