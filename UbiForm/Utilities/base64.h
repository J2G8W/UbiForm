#ifndef UBIFORM_BASE64_H
#define UBIFORM_BASE64_H

#include <string>
#include <vector>


static inline bool is_base64(unsigned char c) {
    return (isalnum(c) || (c == '+') || (c == '/'));
}

unsigned char* base64_encode(const unsigned char *src, size_t len,
                             size_t *out_len);


void base64_decode_to_stream(const void* data, const size_t len, std::ostream &stream);
#endif //UBIFORM_BASE64_H
