#pragma once
#include "Exception.h"
#include <cstdint>
#include <stdexcept>
#include <string>

namespace muduo::base {

DECLARE_EXCEPTION(UnicodeError, Exception)

inline bool     utf16_code_unit_is_bmp(char16_t const c) { return c < 0xd800 || c >= 0xe000; }
inline bool     utf16_code_unit_is_high_surrogate(char16_t const c) { return c >= 0xd800 && c < 0xdc00; }
inline bool     utf16_code_unit_is_low_surrogate(char16_t const c) { return c >= 0xdc00 && c < 0xe000; }
inline char32_t unicode_code_point_from_utf16_surrogate_pair(char16_t const high, char16_t const low) {
    if (!utf16_code_unit_is_high_surrogate(high)) {
        throw UnicodeError("invalid high surrogate");
    }
    if (!utf16_code_unit_is_low_surrogate(low)) {
        throw UnicodeError("invalid low surrogate");
    }
    return 0x10000 + ((char32_t(high) & 0x3ff) << 10) + (char32_t(low) & 0x3ff);
}

//////////////////////////////////////////////////////////////////////

/*
 * Encode a single unicode code point into a UTF-8 byte sequence.
 *
 * Return value is undefined if `cp' is an invalid code point.
 */
std::string codePointToUtf8(char32_t cp);

/*
 * Decode a single unicode code point from UTF-8 byte sequence.
 */
char32_t utf8ToCodePoint(const unsigned char *&p, const unsigned char *const e, bool skipOnError);

//////////////////////////////////////////////////////////////////////

} // namespace muduo::base