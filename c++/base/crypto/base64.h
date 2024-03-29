#pragma once
#include <iostream>
#include <vector>
#define BUFFSIZE 4096
#define CHARS_PER_LINE 72
#define decoding_size 80

namespace base64 {
namespace detail {

typedef enum {
    step_a,
    step_b,
    step_c,
    step_d,
} base64_decodestep;

typedef struct {
    base64_decodestep step;
    char              plainchar;
} base64_decodestate;

typedef enum { step_A, step_B, step_C } base64_encodestep;

typedef struct {
    base64_encodestep step;
    char              result;
    int               stepcount;
} base64_encodestate;

static void base64_init_encodestate(base64_encodestate *state_in) {
    state_in->step      = step_A;
    state_in->result    = 0;
    state_in->stepcount = 0;
}

static char base64_encode_value(char value_in) {
    if (value_in > 63)
        return '=';
    std::string encoding = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    return encoding[ (int)value_in ];
}

static int base64_encode_blockend(char *code_out, base64_encodestate *state_in);

static int base64_decode_value(char value_in) {
    value_in -= 43;
    if (value_in < 0 || value_in >= decoding_size)
        return -1;
    std::vector<int> decoding = {62, -1, -1, -1, 63, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, -1, -1, -1, -2, -1,
                                 -1, -1, 0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12, 13, 14, 15, 16, 17,
                                 18, 19, 20, 21, 22, 23, 24, 25, -1, -1, -1, -1, -1, -1, 26, 27, 28, 29, 30, 31,
                                 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51};
    return decoding[ (int)value_in ];
}

static void base64_init_decodestate(base64_decodestate *state_in) {
    state_in->step      = step_a;
    state_in->plainchar = 0;
}

static int base64_decode_block(const char *code_in, const int length_in, char *plaintext_out,
                               base64_decodestate *state_in) {
    const char *codechar  = code_in;
    char *      plainchar = plaintext_out;
    char        fragment;

    *plainchar = state_in->plainchar;

    switch (state_in->step) {
        while (1) {
            case step_a:
                do {
                    if (codechar == code_in + length_in) {
                        state_in->step      = step_a;
                        state_in->plainchar = *plainchar;
                        return plainchar - plaintext_out;
                    }
                    fragment = (char)base64_decode_value(*codechar++);
                } while (fragment < 0);
                *plainchar = (fragment & 0x03f) << 2;
            case step_b:
                do {
                    if (codechar == code_in + length_in) {
                        state_in->step      = step_b;
                        state_in->plainchar = *plainchar;
                        return plainchar - plaintext_out;
                    }
                    fragment = (char)base64_decode_value(*codechar++);
                } while (fragment < 0);
                *plainchar++ |= (fragment & 0x030) >> 4;
                *plainchar = (fragment & 0x00f) << 4;
            case step_c:
                do {
                    if (codechar == code_in + length_in) {
                        state_in->step      = step_c;
                        state_in->plainchar = *plainchar;
                        return plainchar - plaintext_out;
                    }
                    fragment = (char)base64_decode_value(*codechar++);
                } while (fragment < 0);
                *plainchar++ |= (fragment & 0x03c) >> 2;
                *plainchar = (fragment & 0x003) << 6;
            case step_d:
                do {
                    if (codechar == code_in + length_in) {
                        state_in->step      = step_d;
                        state_in->plainchar = *plainchar;
                        return plainchar - plaintext_out;
                    }
                    fragment = (char)base64_decode_value(*codechar++);
                } while (fragment < 0);
                *plainchar++ |= (fragment & 0x03f);
        }
    }
    /* control should not reach here */
    return plainchar - plaintext_out;
}

static int base64_encode_block(const char *plaintext_in, int length_in, char *code_out, base64_encodestate *state_in) {
    const char *      plainchar    = plaintext_in;
    const char *const plaintextend = plaintext_in + length_in;
    char *            codechar     = code_out;
    char              result;
    char              fragment;

    result = state_in->result;

    switch (state_in->step) {
        while (true) {
            case step_A:
                if (plainchar == plaintextend) {
                    state_in->result = result;
                    state_in->step   = step_A;
                    return codechar - code_out;
                }
                fragment    = *plainchar++;
                result      = (fragment & 0x0fc) >> 2;
                *codechar++ = base64_encode_value(result);
                result      = (fragment & 0x003) << 4;
            case step_B:
                if (plainchar == plaintextend) {
                    state_in->result = result;
                    state_in->step   = step_B;
                    return codechar - code_out;
                }
                fragment = *plainchar++;
                result |= (fragment & 0x0f0) >> 4;
                *codechar++ = base64_encode_value(result);
                result      = (fragment & 0x00f) << 2;
            case step_C:
                if (plainchar == plaintextend) {
                    state_in->result = result;
                    state_in->step   = step_C;
                    return codechar - code_out;
                }
                fragment = *plainchar++;
                result |= (fragment & 0x0c0) >> 6;
                *codechar++ = base64_encode_value(result);
                result      = (fragment & 0x03f) >> 0;
                *codechar++ = base64_encode_value(result);

                ++(state_in->stepcount);
                if (state_in->stepcount == CHARS_PER_LINE / 4) {
                    *codechar++         = '\n';
                    state_in->stepcount = 0;
                }
        }
    }
    /* control should not reach here */
    return codechar - code_out;
}

static int base64_encode_blockend(char *code_out, base64_encodestate *state_in) {
    char *codechar = code_out;

    switch (state_in->step) {
        case step_B:
            *codechar++ = base64_encode_value(state_in->result);
            *codechar++ = '=';
            *codechar++ = '=';
            break;
        case step_C:
            *codechar++ = base64_encode_value(state_in->result);
            *codechar++ = '=';
            break;
        case step_A:
            break;
    }
    *codechar++ = '\0';

    return codechar - code_out;
}
} // namespace detail

struct decoder {
    detail::base64_decodestate _state;
    int                        _BUFFSIZE;

    decoder(int BUFFSIZE_in = BUFFSIZE)
        : _BUFFSIZE(BUFFSIZE_in) {}

    int decode(char value_in) { return detail::base64_decode_value(value_in); }

    int decode(const char *code_in, const int length_in, char *plaintext_out) {
        return detail::base64_decode_block(code_in, length_in, plaintext_out, &_state);
    }

    int decode_str(const char *plaintext, size_t len, char *output) {
        if (output == nullptr) {
            return -1;
        }
        detail::base64_init_decodestate(&_state);
        int decodeLength = decode(plaintext, len, output);
        detail::base64_init_decodestate(&_state);
        return decodeLength;
    }

    void decode(std::istream &istream_in, std::ostream &ostream_in) {
        detail::base64_init_decodestate(&_state);
        const int N         = _BUFFSIZE;
        char *    code      = new char[ N ];
        char *    plaintext = new char[ N ];
        int       codelength;
        int       plainlength;

        do {
            istream_in.read((char *)code, N);
            codelength  = istream_in.gcount();
            plainlength = decode(code, codelength, plaintext);
            ostream_in.write((const char *)plaintext, plainlength);
        } while (istream_in.good() && codelength > 0);
        detail::base64_init_decodestate(&_state);

        delete[] code;
        delete[] plaintext;
    }
};

struct encoder {
    detail::base64_encodestate _state;
    int                        _BUFFSIZE;

    encoder(int BUFFSIZE_in = BUFFSIZE)
        : _BUFFSIZE(BUFFSIZE_in) {}

    int encode(char value_in) { return detail::base64_encode_value(value_in); }

    int encode(const char *code_in, const int length_in, char *plaintext_out) {
        return detail::base64_encode_block(code_in, length_in, plaintext_out, &_state);
    }

    int encode_end(char *plaintext_out) { return detail::base64_encode_blockend(plaintext_out, &_state); }

    int encode_str(const char *plaintext, size_t len, char *output) {
        if (output == nullptr) {
            return -1;
        }
        detail::base64_init_encodestate(&_state);
        int  codeLength    = encode(plaintext, len, output);
        char last_str[ 3 ] = {0};
        int  last_len      = encode_end(last_str);
        for (int idx = 0; idx < last_len; idx++) {
            *(output + idx + codeLength) = last_str[ idx ];
        }
        detail::base64_init_encodestate(&_state);
        return codeLength + last_len;
    }

    void encode(std::istream &istream_in, std::ostream &ostream_in) {
        detail::base64_init_encodestate(&_state);
        //
        const int N         = _BUFFSIZE;
        char *    plaintext = new char[ N ];
        char *    code      = new char[ 2 * N ];
        int       plainlength;
        int       codelength;

        do {
            istream_in.read(plaintext, N);
            plainlength = istream_in.gcount();
            codelength  = encode(plaintext, plainlength, code);
            ostream_in.write(code, codelength);
        } while (istream_in.good() && plainlength > 0);

        codelength = encode_end(code);
        ostream_in.write(code, codelength - 1);
        detail::base64_init_encodestate(&_state);

        delete[] code;
        delete[] plaintext;
    }
};

} // namespace base64