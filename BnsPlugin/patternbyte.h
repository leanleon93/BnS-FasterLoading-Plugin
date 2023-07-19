#pragma once

struct patternbyte
{
    const unsigned char value;
    const unsigned char mask;

    patternbyte() : value(0), mask(0) {}
    patternbyte(const unsigned char value, const unsigned char mask = 0xff) : value(value), mask(mask) {}

    inline bool operator==(const unsigned char& rhs) const
    {
        return (rhs & mask) == (value & mask);
    }

    friend inline bool operator==(const unsigned char& lhs, const patternbyte& rhs)
    {
        return (lhs & rhs.mask) == (rhs.value & rhs.mask);
    }
};
