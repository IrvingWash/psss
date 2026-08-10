#ifndef UTILS_H
#define UTILS_H

short get_short_le(const unsigned char *data, u_long *byte_counter) {
    unsigned short value = 0;

    value |= data[(*byte_counter)++] << 0;
    value |= data[(*byte_counter)++] << 8;

    return (short) value;
}

short get_short_be(const unsigned char *data, u_long *byte_counter) {
    unsigned short value = 0;

    value |= data[(*byte_counter)++] << 8;
    value |= data[(*byte_counter)++] << 0;

    return (short) value;
}

char get_char(const unsigned char *data, u_long *byte_counter) {
    return data[(*byte_counter)++];
}

#endif
