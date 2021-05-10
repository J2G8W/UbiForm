// ------------------------------------------------------------------------------------------------
// | THE ENCODING AND DECODING FUNCTIONS PROVIDED HAVE BEEN SLIGHTLY ADJUSTED FROM ONLINE VERSIONS |
// | BUT I DO NOT CLAIM TO HAVE IMPLEMENTED THEMSELVES.                                            |
// |                                                                                               |
// | The decisions as to which implementation to us were                                           |
// |  based on https://stackoverflow.com/questions/342409/how-do-i-base64-encode-decode-in-c       |
// ------------------------------------------------------------------------------------------------



#include "base64.h"
#include <ostream>
#include <string.h>


/*
 * Base64 encoding (RFC1341)
 * Copyright (c) 2005-2011, Jouni Malinen <j@w1.fi>
 *
 * This software may be distributed under the terms of the BSD license.
 * See README for more details.
 * http://web.mit.edu/freebsd/head/contrib/wpa/src/utils/base64.c
 */


static const unsigned char base64_table[65] =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

/**
 * base64_encode - Base64 encode
 * @src: Data to be encoded
 * @len: Length of the data to be encoded
 * @out_len: Pointer to output length variable, or %NULL if not used
 * Returns: Allocated buffer of out_len bytes of encoded data,
 * or %NULL on failure
 *
 * Caller is responsible for freeing the returned buffer. Returned buffer is
 * nul terminated to make it easier to use as a C string. The nul terminator is
 * not included in out_len.
 */
unsigned char * base64_encode(const unsigned char *src, size_t len,
                              size_t *out_len)
{
    unsigned char *out, *pos;
    const unsigned char *end, *in;
    size_t olen;
    int line_len;

    olen = len * 4 / 3 + 4; /* 3-byte blocks to 4-byte */
    olen++; /* nul termination */
    if (olen < len)
        return NULL; /* integer overflow */
    // 12 is the length of {"bytes":"___"}
    olen += 12;
    out = static_cast<unsigned char *>(malloc(olen));
    if (out == NULL)
        return NULL;

    strncpy(reinterpret_cast<char *>(out), R"({"bytes":")", 10);

    end = src + len;
    in = src;
    pos = out + 10;
    while (end - in >= 3) {
        *pos++ = base64_table[in[0] >> 2];
        *pos++ = base64_table[((in[0] & 0x03) << 4) | (in[1] >> 4)];
        *pos++ = base64_table[((in[1] & 0x0f) << 2) | (in[2] >> 6)];
        *pos++ = base64_table[in[2] & 0x3f];
        in += 3;
    }

    if (end - in) {
        *pos++ = base64_table[in[0] >> 2];
        if (end - in == 1) {
            *pos++ = base64_table[(in[0] & 0x03) << 4];
            *pos++ = '=';
        } else {
            *pos++ = base64_table[((in[0] & 0x03) << 4) |
                                  (in[1] >> 4)];
            *pos++ = base64_table[(in[1] & 0x0f) << 2];
        }
        *pos++ = '=';
    }

    *pos++ = '"';
    *pos++ = '}';
    *pos = '\0';
    if (out_len)
        *out_len = pos - out;
    return out;
}




/*
 * Decode function openly provided without any license by 'polfosol' on StackOverflow, thread is:
 * https://stackoverflow.com/questions/180947/base64-decode-snippet-in-c/13935718
 */

static const int B64index[256] = { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
                                   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
                                   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 62, 63, 62, 62, 63, 52, 53, 54, 55,
                                   56, 57, 58, 59, 60, 61,  0,  0,  0,  0,  0,  0,  0,  0,  1,  2,  3,  4,  5,  6,
                                   7,  8,  9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25,  0,
                                   0,  0,  0, 63,  0, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
                                   41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51 };

void base64_decode_to_stream(const void* data, const size_t len, std::ostream &stream)
{
    if (len == 0) return ;

    unsigned char *p = (unsigned char*) data;
    size_t j = 0,
            pad1 = len % 4 || p[len - 1] == '=',
            pad2 = pad1 && (len % 4 > 2 || p[len - 2] != '=');
    const size_t last = (len - pad1) / 4 << 2;
    std::string result(last / 4 * 3 + pad1 + pad2, '\0');
    unsigned char *str = (unsigned char*) &result[0];

    for (size_t i = 0; i < last; i += 4)
    {
        int n = B64index[p[i]] << 18 | B64index[p[i + 1]] << 12 | B64index[p[i + 2]] << 6 | B64index[p[i + 3]];
        str[j++] = n >> 16;
        str[j++] = n >> 8 & 0xFF;
        str[j++] = n & 0xFF;
    }
    if (pad1)
    {
        int n = B64index[p[last]] << 18 | B64index[p[last + 1]] << 12;
        str[j++] = n >> 16;
        if (pad2)
        {
            n |= B64index[p[last + 2]] << 6;
            str[j++] = n >> 8 & 0xFF;
        }
    }
    stream.write(reinterpret_cast<const char *>(str), result.size());
}


