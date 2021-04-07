#ifndef UBIFORM_BASE64_H
#define UBIFORM_BASE64_H

#include <string>
#include <vector>

/*
   base64.cpp and base64.h
   https://stackoverflow.com/questions/180947/base64-decode-snippet-in-c

   Copyright (C) 2004-2008 René Nyffenegger

   This source code is provided 'as-is', without any express or implied
   warranty. In no event will the author be held liable for any damages
   arising from the use of this software.

   Permission is granted to anyone to use this software for any purpose,
   including commercial applications, and to alter it and redistribute it
   freely, subject to the following restrictions:

   1. The origin of this source code must not be misrepresented; you must not
      claim that you wrote the original source code. If you use this source code
      in a product, an acknowledgment in the product documentation would be
      appreciated but is not required.

   2. Altered source versions must be plainly marked as such, and must not be
      misrepresented as being the original source code.

   3. This notice may not be removed or altered from any source distribution.

   René Nyffenegger rene.nyffenegger@adp-gmbh.ch

*/

static const std::string base64_chars =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz"
        "0123456789+/";


static inline bool is_base64(unsigned char c) {
    return (isalnum(c) || (c == '+') || (c == '/'));
}

unsigned char* base64_encode(const unsigned char *src, size_t len,
                             size_t *out_len);


void base64_decode_to_stream(const void* data, const size_t len, std::ostream &stream);
#endif //UBIFORM_BASE64_H
