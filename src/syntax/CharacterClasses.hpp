#pragma once

#include <cstdint>
using namespace std;

static inline bool IsValidHex(uint8_t c)
{
    return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F');
}

static inline bool IsLetter(uint8_t c)
{
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
           c == '_' || c == '@' || c >= 127;
}

static inline bool IsDuringLetter(uint8_t c)
{
    return (
        (c >= 'a' && c <= 'z') ||
        (c >= '0' && c <= '9') ||
        (c >= 'A' && c <= 'Z') ||
        c == '_' ||
        c == '@' ||
        c >= 127);
}

static inline bool IsNumber(uint8_t c)
{
    return c >= '0' && c <= '9';
}

static inline bool IsSpace(uint8_t c)
{
    return c > 0 && c <= 32;
}

static inline bool IsSymbol(uint8_t c)
{
    return (
        c != '_' &&
        ((c >= '!' && c <= '/') ||
         (c >= ':' && c <= '?') ||
         (c >= '[' && c <= '`') ||
         (c >= '{' && c <= '~')));
}