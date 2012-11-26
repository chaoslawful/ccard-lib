#include <stdint.h>
#include <string.h>
#include "lookup3hash.h"

uint32_t lookup3(const uint32_t *k, uint32_t offset, uint32_t length, uint32_t initval)
{
    uint32_t a, b, c;
    uint32_t i = offset;
    a = b = c = 0xdeadbeef + (length << 2) + initval;

    while (length > 3) {
        a += k[i];
        b += k[i + 1];
        c += k[i + 2];

        a -= c;  a ^= (c << 4)  | (c >> -4);   c += b;
        b -= a;  b ^= (a << 6)  | (a >> -6);   a += c;
        c -= b;  c ^= (b << 8)  | (b >> -8);   b += a;
        a -= c;  a ^= (c << 16) | (c >> -16);  c += b;
        b -= a;  b ^= (a << 19) | (a >> -19);  a += c;
        c -= b;  c ^= (b << 4)  | (b >> -4);   b += a;

        length -= 3;
        i += 3;
    }

    switch(length) {
        case 3 : c += k[i + 2];  // fall through
        case 2 : b += k[i + 1];  // fall through
        case 1 : a += k[i + 0];  // fall through
            c ^= b; c -= (b << 14) | (b >> -14);
            a ^= c; a -= (c << 11) | (c >> -11);
            b ^= a; b -= (a << 25) | (a >> -25);
            c ^= b; c -= (b << 16) | (b >> -16);
            a ^= c; a -= (c << 4)  | (c >> -4);
            b ^= a; b -= (a << 14) | (a >> -14);
            c ^= b; c -= (b << 24) | (b >> -24);
        case 0:
            break;
    }
    return c;
}

uint32_t lookup3ycs(const uint32_t *k, uint32_t offset, uint32_t length, uint32_t initval)
{
    return lookup3(k, offset, length, initval - (length << 2));
}

uint32_t lookup3ycs_2(const char *s, uint32_t start, uint32_t end, uint32_t initval)
{
    uint32_t a, b, c;
    uint32_t i = start;
    a = b = c = 0xdeadbeef + initval;
    uint8_t mixed = 1;  // have the 3 state variables been adequately mixed?

    for(;;) {
        if (i >= end) break;
        mixed = 0;
        char ch;
        /* ch = s.charAt(i++);
        a += Character.isHighSurrogate(ch) && i< end ? Character.toCodePoint(ch, s.charAt(i++)) : ch;
        if (i>= end) break;
        ch = s.charAt(i++);
        b += Character.isHighSurrogate(ch) && i< end ? Character.toCodePoint(ch, s.charAt(i++)) : ch;
        if (i>= end) break;
        ch = s.charAt(i++);
        c += Character.isHighSurrogate(ch) && i< end ? Character.toCodePoint(ch, s.charAt(i++)) : ch;
        if (i>= end) break; */
        ch = s[i++];
        a += ch;
        if (i >= end) break;
        ch = s[i++];
        b += ch;
        if (i >= end) break;
        ch = s[i++];
        c += ch;
        if (i >= end) break;

        a -= c;  a ^= (c << 4)  | (c >> -4);   c += b;
        b -= a;  b ^= (a << 6)  | (a >> -6);   a += c;
        c -= b;  c ^= (b << 8)  | (b >> -8);   b += a;
        a -= c;  a ^= (c << 16) | (c >> -16);  c += b;
        b -= a;  b ^= (a << 19) | (a >> -19);  a += c;
        c -= b;  c ^= (b << 4)  | (b >> -4);   b += a;

        mixed = 1;
    }

    if (mixed == 0) {
        c ^= b; c -= (b <<14) | (b >> -14);
        a ^= c; a -= (c <<11) | (c >> -11);
        b ^= a; b -= (a <<25) | (a >> -25);
        c ^= b; c -= (b <<16) | (b >> -16);
        a ^= c; a -= (c <<4)  | (c >> -4);
        b ^= a; b -= (a <<14) | (a >> -14);
        c ^= b; c -= (b <<24) | (b >> -24);
    }

    return c;
}

uint64_t lookup3ycs64(const char *s, uint32_t start, uint32_t end, uint64_t initval)
{
    uint32_t a, b, c;
    uint32_t i = start;
    a = b = c = 0xdeadbeef + (uint32_t)initval;
    c += (uint32_t)(initval >> 32);

    uint8_t mixed = 1;  // have the 3 state variables been adequately mixed?
    for(;;) {
        if (i >= end) break;
        mixed = 0;
        char ch;
        /* ch = s.charAt(i++);
        a += Character.isHighSurrogate(ch) && i< end ? Character.toCodePoint(ch, s.charAt(i++)) : ch;
        if (i>= end) break;
        ch = s.charAt(i++);
        b += Character.isHighSurrogate(ch) && i< end ? Character.toCodePoint(ch, s.charAt(i++)) : ch;
        if (i>= end) break;
        ch = s.charAt(i++);
        c += Character.isHighSurrogate(ch) && i< end ? Character.toCodePoint(ch, s.charAt(i++)) : ch;
        if (i>= end) break; */
        ch = s[i++];
        a += ch;
        if (i >= end) break;
        ch = s[i++];
        b += ch;
        if (i >= end) break;
        ch = s[i++];
        c += ch;
        if (i >= end) break;

        a -= c;  a ^= ( c << 4)  | (c >> -4);   c += b;
        b -= a;  b ^= ( a << 6)  | (a >> -6);   a += c;
        c -= b;  c ^= ( b << 8)  | (b >> -8);   b += a;
        a -= c;  a ^= ( c << 16) | (c >> -16);  c += b;
        b -= a;  b ^= ( a << 19) | (a >> -19);  a += c;
        c -= b;  c ^= ( b << 4)  | (b >> -4);   b += a;

        mixed = 1;
    }

    if (mixed == 0) {
        c ^= b; c -= (b << 14) | (b >> -14);
        a ^= c; a -= (c << 11) | (c >> -11);
        b ^= a; b -= (a << 25) | (a >> -25);
        c ^= b; c -= (b << 16) | (b >> -16);
        a ^= c; a -= (c << 4)  | (c >> -4);
        b ^= a; b -= (a << 14) | (a >> -14);
        c ^= b; c -= (b << 24) | (b >> -24);
    }

    return c + ((uint64_t)b << 32);
}

uint64_t lookup3ycs64_2(const char *s)
{
    return lookup3ycs64(s, 0, strlen(s), -1);
}
