// ----------------------------------------------------------------------------
// Copyright (C) 2014 Clint Weisbrod. All rights reserved.
//
// bigfix.h
//
// THIS SOFTWARE IS PROVIDED BY CLINT WEISBROD "AS IS" AND ANY EXPRESS OR
// IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
// EVENT SHALL CLINT WEISBROD OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
// INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
// (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
// ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
// THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
// ----------------------------------------------------------------------------

#pragma once

#include <string>

class BigFix
{
 public:
    BigFix();
    BigFix(uint64_t);
    BigFix(double_t);
    BigFix(const std::string&);

    operator double_t() const;
    operator float_t() const;

    BigFix operator-() const;
    BigFix operator+=(const BigFix&);
    BigFix operator-=(const BigFix&);

    friend BigFix operator+(const BigFix&, const BigFix&);
    friend BigFix operator-(const BigFix&, const BigFix&);
    friend BigFix operator*(const BigFix&, const BigFix&);
    friend BigFix operator*(BigFix, double_t);
    friend bool operator==(const BigFix&, const BigFix&);
    friend bool operator!=(const BigFix&, const BigFix&);
    friend bool operator<(const BigFix&, const BigFix&);
    friend bool operator>(const BigFix&, const BigFix&);

    int sign() const;

    // for debugging
    void dump();
    std::string toString();

 private:
    bool isNegative() const
    {
        return hi > INT64_MAX;
    }

	static void negate128(uint64_t& hi, uint64_t& lo);

 private:
	uint64_t hi;
	uint64_t lo;
};


// Compute the additive inverse of a 128-bit twos complement value
// represented by two 64-bit values.
inline void BigFix::negate128(uint64_t& hi, uint64_t& lo)
{
    // For a twos-complement number, -n = ~n + 1
    hi = ~hi;
    lo = ~lo;
    lo++;
    if (lo == 0)
        hi++;
}

inline BigFix BigFix::operator-() const
{
    BigFix result = *this;

    negate128(result.hi, result.lo);

    return result;
}


inline BigFix BigFix::operator+=(const BigFix& a)
{
    lo += a.lo;
    hi += a.hi;

    // carry
    if (lo < a.lo)
        hi++;

    return *this;
}


inline BigFix BigFix::operator-=(const BigFix& a)
{
    lo -= a.lo;
    hi -= a.hi;

    // borrow
    if (lo > a.lo)
        hi--;

    return *this;
}


inline BigFix operator+(const BigFix& a, const BigFix& b)
{
    BigFix c;

    c.lo = a.lo + b.lo;
    c.hi = a.hi + b.hi;

    // carry
    if (c.lo < a.lo)
        c.hi++;

    return c;
}


inline BigFix operator-(const BigFix& a, const BigFix& b)
{
    BigFix c;

    c.lo = a.lo - b.lo;
    c.hi = a.hi - b.hi;

    // borrow
    if (c.lo > a.lo)
        c.hi--;

    return c;
}
