/* протокол телефонної книги */
#ifndef PHONEBOOK_H
#define PHONEBOOK_H

#include <stdint.h>

#define CMD_ADD    0x01
#define CMD_SEARCH 0x02
#define CMD_DELETE 0x03
#define CMD_LIST   0x04

#define RSP_OK        0x00
#define RSP_NOT_FOUND 0x01
#define RSP_ERROR     0xFF

typedef struct __attribute__((packed)) {
    uint8_t  command;
    uint16_t payload_len;
} PbHeader;

#define PB_HDR_SIZE sizeof(PbHeader)

#define PB_NAME_LEN  48
#define PB_PHONE_LEN 20

typedef struct {
    char name[PB_NAME_LEN];
    char phone[PB_PHONE_LEN];
} PbEntry;

#define MAX_ENTRIES 100

#endif
