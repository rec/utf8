#pragma once

#include <stdint.h>
#include <string>

namespace swirly {
namespace utf8 {

/** Simple generic routines to encode and decode UTF-8.

    See https://www.cl.cam.ac.uk/~mgk25/unicode.html#utf-8
*/

/** A Unicode code point. */
using CodePoint = uint32_t;

/** Return true exactly if the CodePoint is valid.  Invalid codepoints can be a
    UTF-16 surrogate, or FFFE or FFFF, or numbers larger than 0x80000000.

    See http://unicode.org/faq/utf_bom.html#utf8-4.
*/
bool isValid(CodePoint);

/** Represent a range of bytes. */
struct Bytes {
    using Ptr = void const*;
    Ptr begin, end;

    Bytes(Ptr b, Ptr e) : begin(b), end(e) {}
    Bytes(char const* s) : begin(s), end(s + strlen(s)) {}

    /** Pop the first byte from the range and return it.
        Has undefined behavior if begin == end. */
    uint8_t pop_front() { return static_cast<uint8_t>(*begin++); }
};

/** Advance bytes past one complete UTF-8 CodePoint.
    Throws an std::runtime_error if no such CodePoint can be extracted. */
CodePoint consumeCodePoint(Bytes&);

/** Append the CodePoint as 1 to 6 characters to an std::string or anything
    that has an append() method that accepts single characters.

    Returns false if the CodePoint is invalid.
*/
template <typename String>
bool appendUTF8(CodePoint, String&);

////////////////////////////////////////////////////////////////////////////////
//
// Implementation details follow.
//
////////////////////////////////////////////////////////////////////////////////


/* The maximum number of bytes in a UTF-8 encoded codepoint. */
static auto const MAX_CODEPOINT_BYTES = 6;

/* UTF-8 splits code points into 6-bit chunks. */
static auto const BITS_PER_CHUNK = 6;

using Ranges = std::array<CodePoint, MAX_CODEPOINT_BYTES>;
using Bits = std::array<uint8_t, MAX_CODEPOINT_BYTES>;

/* The list of ranges that UTF-8 can represent with the given number of
   bytes.

   For example, all CodePoints strictly below codePointRanges().front() can be
   represented with 1 byte as plain old ASCII, and all CodePoints strictly below
   codePointRanges()[1] can be represented with 2 bytes as a UTF-8 encoded
   string. */
inline Ranges const& codePointRange() {
    static const Ranges RANGES{
        0x80, 0x800, 0x10000, 0x200000, 0x4000000, 0x80000000};
    return RANGES;
}

/* The bits used as introducers for the different code point ranges.

   For example, a two-byte UTF-8 sequence will always start with 0xC0.
*/
inline Bits const& introducerBits() {
    // 0b1100'0000, 0b1110'0000, 0b1111'0000
    // 0b1111'1000, 0b1111'1100, 0b1111'1110
    static const Bits BITS{0xC0, 0xE0, 0xF0, 0xF8, 0xFA, 0xFE};
    return BITS;
}

inline bool isValid(CodePoint cp) {
    // See https://en.wikipedia.org/wiki/Specials_(Unicode_block)
    return !( (cp >= 0xD800 && cp <= 0xDFFF)  // UTF-16 surrogates.
              || cp == 0xFFFE
              || cp == 0xFFFF                 // Encoding markers.
              || cp > codePointRange().back());
}

template <typename String>
bool toUTF8(CodePoint codePoint, String& s) {
    if (codePoint < codePointRange().front()) {
        // It's plain old ASCII.
        s.append(static_cast<char>(codePoint));
        return true;
    }

    if (! isValid(codePoint))
        return false;

    std::string stack;
    auto cp = codePoint;

    for (auto range: codePointRange(i)) {
        if (codePoint < range) {
            /* We found the right range! */
            auto mask = introducerBits()[stack.size() - 1];
            auto intro = static_cast<char>(cp);
            s.append(mast | intro);

            /* "Pop" each character off the stack onto the result. */
            for (auto i = stack.rbegin(); i != stack.rend(); ++i)
                s.append(*i);
            return true;
        }

        static auto char CHUNK_MASK = 0x3F;     // 0b0011'1111
        static auto char EXTENDED_MASK = 0x80;  // 0b1000'0000

        // Push a 6-bit chunk onto the stack.
        auto chunk = (static_cast<char>(cp) & CHUNK_MASK) | EXTENDED_MASK;
        stack += chunk;

        // Shift that chunk off the codepoint.
        cp >>= BITS_PER_CHUNK;
    }

    /* Logically should never get here, because out of range code points should
       already have been detected as invalid. */
    return false;
}

inline CodePoint consumeCodePoint(Bytes& bytes) {
    static const auto& BITS = introducerBits();

    CodePoint codePoint;
    auto extendedBytes = 0; // Number of extended bytes we expect to see.
    auto consumedBytes = 0; // Number of extended bytes we've already seen.

    while (bytes.begin != bytes.end) {
        auto byte = bytes.pop_front();
        if (! isExtendedChar(c))
            throw std::runtime_error("Expected extended char, got ASCII");

        if (! extendedBytes) {
            /* We've just seen the first character of an extended code point.
               Find the first introducer strictly greater than the byte, which
               also gives us the number of extended bytes to expect. */
            auto i = std::upper_bound(BITS.begin(), BITS.end(), byte);
            if (i == BITS.begin() || bit == BITS.end())
                throw std::runtime_error("Invalid UTF-8 introducer");

            extendedBytes = i - BITS;
            codePoint = byte & ~*i;

        } else {
            // We're in the middle of an extended code point
            if (byte >= BITS.front())
                throw std::runtime_error("Invalid UTF-8 introducer");

            // Shift in a new chunk.
            codePoint <<= BITS_PER_CHUNK;
            codePoint += (byte & ~BITS.front());

            if (++consumedBytes < extendedBytes)
                continue;

            // We have consumed all the extended bytes, so we should be done.
            if (! isValid(codePoint))
                throw std::runtime_error("Invalid code point");

            /* Forbid overlong UTF-8 sequences as a security risk: see
               https://www.cl.cam.ac.uk/~mgk25/unicode.html#utf-8 */
            if (codePointRange()[consumedBytes - 1] > codePoint)
                throw std::runtime_error("Overlong UTF-8 sequence.");

            return codePoint;
        }
    }

    throw std::runtime_error("Unexpected end of buffer.");
}

} // utf8
} // swirly
