/* TLV-протокол */
#ifndef TLV_PROTOCOL_H
#define TLV_PROTOCOL_H

#include <stdint.h>
#include <string.h>
#include <arpa/inet.h>

#define TLV_TEXT   0x01   /* рядок UTF-8 */
#define TLV_INT32  0x02   /* 32-бітне ціле */
#define TLV_ARRAY  0x03   /* масив TLV-записів */
#define TLV_FLOAT  0x04   /* float як int32 × 1000 */

typedef struct __attribute__((packed)) {
    uint8_t  type;
    uint16_t length;
} TlvHeader;

#define TLV_HDR_SIZE sizeof(TlvHeader)

static inline size_t tlv_encode_text(uint8_t *buf, size_t buf_sz, const char *text)
{
    uint16_t len = (uint16_t)strlen(text);
    if (buf_sz < TLV_HDR_SIZE + len) return 0;

    TlvHeader hdr;
    hdr.type = TLV_TEXT;
    hdr.length = htons(len);

    memcpy(buf, &hdr, TLV_HDR_SIZE);
    memcpy(buf + TLV_HDR_SIZE, text, len);
    return TLV_HDR_SIZE + len;
}

static inline size_t tlv_encode_int32(uint8_t *buf, size_t buf_sz, int32_t value)
{
    if (buf_sz < TLV_HDR_SIZE + 4) return 0;

    TlvHeader hdr;
    hdr.type = TLV_INT32;
    hdr.length = htons(4);

    int32_t net_val = htonl(value);

    memcpy(buf, &hdr, TLV_HDR_SIZE);
    memcpy(buf + TLV_HDR_SIZE, &net_val, 4);
    return TLV_HDR_SIZE + 4;
}

static inline size_t tlv_encode_array(uint8_t *buf, size_t buf_sz,
                                    const uint8_t *data, size_t data_len)
{
    if (buf_sz < TLV_HDR_SIZE + data_len) return 0;

    TlvHeader hdr;
    hdr.type = TLV_ARRAY;
    hdr.length = htons((uint16_t)data_len);

    memcpy(buf, &hdr, TLV_HDR_SIZE);
    memcpy(buf + TLV_HDR_SIZE, data, data_len);
    return TLV_HDR_SIZE + data_len;
}

static inline size_t tlv_decode(const uint8_t *buf, size_t buf_len,
                                uint8_t *type, uint16_t *value_len, const uint8_t **value)
{
    if (buf_len < TLV_HDR_SIZE) return 0;

    TlvHeader hdr;
    memcpy(&hdr, buf, TLV_HDR_SIZE);

    *type = hdr.type;
    *value_len = ntohs(hdr.length);

    if (buf_len < TLV_HDR_SIZE + *value_len) return 0;

    *value = buf + TLV_HDR_SIZE;
    return TLV_HDR_SIZE + *value_len;
}

#endif
