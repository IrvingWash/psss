#ifndef UTILS_H
#define UTILS_H

static inline short get_short_le(const unsigned char *data, u_long *byte_counter) {
    unsigned short value = 0;

    value |= data[(*byte_counter)++] << 0;
    value |= data[(*byte_counter)++] << 8;

    return (short) value;
}

static inline short get_short_be(const unsigned char *data, u_long *byte_counter) {
    unsigned short value = 0;

    value |= data[(*byte_counter)++] << 8;
    value |= data[(*byte_counter)++] << 0;

    return (short) value;
}

static inline long get_long_be(const unsigned char *data, u_long *byte_counter) {
    unsigned long value = 0;

    value |= data[(*byte_counter)++] << 24;
    value |= data[(*byte_counter)++] << 16;
    value |= data[(*byte_counter)++] << 8;
    value |= data[(*byte_counter)++] << 0;

    return (long) value;
};

static inline long get_long_le(const unsigned char *data, u_long *byte_counter) {
    unsigned long value = 0;

    value |= data[(*byte_counter)++] << 0;
    value |= data[(*byte_counter)++] << 8;
    value |= data[(*byte_counter)++] << 16;
    value |= data[(*byte_counter)++] << 24;

    return (long) value;
};

static inline char get_char(const unsigned char *data, u_long *byte_counter) {
    return data[(*byte_counter)++];
}

#endif
