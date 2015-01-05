// ----------------------------------------------------------------------------
// Copyright (C) 2015 Clint Weisbrod. All rights reserved.
//
// bigfix.cpp
//
// A pretty fundamental question about rendering things on a large scale like the
// observable universe is the limited accuracy available in floating point numbers.
// The problem with floating point is that it can only represent a finite set of values
// within the infinite set of real numbers. For very large and very small numbers, the
// precision becomes hoplessly inadequate. In a universe roughly 50 billion light years
// in all directions, using floating point to represent the positions of all objects
// will quickly become inadequate. Since we will be needing to subtract two very large
// numbers in order to compute positions relative to the viewer, round-off errors will
// swamp the calculations, making the results useless.
//
// So, floating point is not an option. What about using integers? Our target platform
// for the forseeable future is 64-bit so we could use 64-bit native integers. If we use
// signed integers, the largest value we can represent 2^63-1 or roughly 9.2e18.
// What kind of spatial precision does this buy us?
//
// Given that the observable Universe is a sphere with radius on the order of 50 billion
// light years and a light year is 9.46e15 metres, we have a dimensional requirement of
// roughly 4.7e26 metres. Dividing this value by our maximum 63-bit integer size gives us
// a precision of 51413 km. No good. We want millimeter precision (or better). We have no
// choice but to require more than 63 bits.
//
// How many bits do we need to use to represent positions in the observable universe with
// millimeters?
//
// bits = log(4.7e29) / log(2) = 98.6, rounded up to 99 bits, +1 for signed, = 100 bits.
//
// We will therefore use two unsigned 64-bit integers to represent positions. Since we
// want calculations to be as fast as possible, there is no point in introducing extra
// overhead to manage exactly 100 bits, so we use 128-bit integers.
//
// 128 bits is more than adequate to represent the position of an object in the Universe
// with millimetre precision. In fact, the extra 28 bits allows us to position objects
// down to the millimeter in a universe 360 million times larger than our own. Or looked
// at another way, within our observable universe, the extra 28 bits gives us a precision
// of roughly 2.8 nanometers!
//
// Seems like a good plan, however 128-bit signed integers are not natively implemented
// on 64-bit systems so we need to implement them in software. Furthermore, we will need
// to convert (or cast) our 128-bit integers to float values. The overhead involved in
// doing this (using ttmath) turns out to be fairly hefty, requiring on the order of half
// a second to cast a million 128-bit integers to floats. Unacceptable.
//
// Investigating fixed-point implementations seems like a good idea. In particular, 64.64
// fixed-point. If we decide to represent distances in AU using 64.64 fixed-point, we can
// represent numbers as large as 2^63-1 AU or 1.38e30 metres. That's roughly 3000 times
// larger than our observable Universe. That makes me a little nervous since the accepted
// observable size probably will increase as new discoveries are made. If we choose light
// years instead, our upper-bound becomes 8.7e34 metres, or roughly 185 million times the
// size of the observable Universe. What about precision? Using 64.64 fixed-point, we can
// divide 1 LY up into 2^64 pieces, each piece would be about 5.3e-4 metres or 0.53 mm.
// Half a millimetre precision. Not bad at all.
//
// Performance analysis has been based on casting to floats. Why? Because, ultimately we
// will be subtracting two 3D vectors having 64.64 components and casting the result to a
// Vec3f. In other words, subtracting the camera's position from some object's position to
// obtain eye coordinates. If we choose the LY as our base unit, I'm worried there will
// not be adequate precision when both vectors are nearly the same. May have to dynamically
// change what the base unit is depending on where camera is.
//
// In terms of performance, my 64.64 fixed-point implementation was able to cast a million
// floats in 80 milliseconds. Hoping to reduce this number but reasonably quick.
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

#include "stdafx.h"
#include <math.h>
#include <stdio.h>
#include "bigfix.h"


static const double_t POW2_31 = 2147483648.0;
static const double_t POW2_32 = 4294967296.0;
static const double_t POW2_64 = POW2_32 * POW2_32;

static const double_t WORD0_FACTOR = 1.0 / POW2_64;
static const double_t WORD1_FACTOR = 1.0 / POW2_32;
static const double_t WORD2_FACTOR = 1.0;
static const double_t WORD3_FACTOR = POW2_32;


/*** Constructors ***/

/*
// Compute the additive inverse of a 128-bit twos complement value
// represented by two 64-bit values.
inline void negate128(uint64& hi, uint64& lo)
{
    // For a twos-complement number, -n = ~n + 1
    hi = ~hi;
    lo = ~lo;
    lo++;
    if (lo == 0)
        hi++;
}
*/


// Create a BigFix initialized to zero
BigFix::BigFix()
{
    hi = 0;
    lo = 0;
}


BigFix::BigFix(uint64_t i)
{
    hi = i;
    lo = 0;
}


BigFix::BigFix(double_t d)
{
    bool isNegative = false;

    // Handle negative values by inverting them before conversion,
    // then inverting the converted value.
    if (d < 0)
    {
	isNegative = true;
	d = -d;
    }
    
    // Need to break the number into 32-bit chunks because a 64-bit
    // integer has more bits of precision than a double.
    double_t e = floor(d * (1.0 / WORD3_FACTOR));
    if (e < POW2_31)
    {
		uint32_t w3 = (uint32_t)e;
		d -= w3 * WORD3_FACTOR;
		uint32_t w2 = (uint32_t)(d * (1.0 / WORD2_FACTOR));
		d -= w2 * WORD2_FACTOR;
		uint32_t w1 = (uint32_t)(d * (1.0 / WORD1_FACTOR));
		d -= w1 * WORD1_FACTOR;
		uint32_t w0 = (uint32_t)(d * (1.0 / WORD0_FACTOR));

		hi = ((uint64_t)w3 << 32) | w2;
		lo = ((uint64_t)w1 << 32) | w0;
    }

    if (isNegative)
        negate128(hi, lo);
}


BigFix::operator double_t() const
{
    // Handle negative values by inverting them before conversion,
    // then inverting the converted value.
    int sign = 1;
	uint64_t l = lo;
	uint64_t h = hi;

    if (isNegative())
    {
        negate128(h, l);
        sign = -1;
    }

    // Need to break the number into 32-bit chunks because a 64-bit
    // integer has more bits of precision than a double.
	uint32_t w0 = l & 0xffffffff;
	uint32_t w1 = l >> 32;
	uint32_t w2 = h & 0xffffffff;
	uint32_t w3 = h >> 32;
    double_t d;

    d = (w0 * WORD0_FACTOR +
         w1 * WORD1_FACTOR +
         w2 * WORD2_FACTOR +
         w3 * WORD3_FACTOR) * sign;

    return d;
}


BigFix::operator float_t() const
{
    return (float_t) (double_t) *this;
}


bool operator==(const BigFix& a, const BigFix& b)
{
    return a.hi == b.hi && a.lo == b.lo;
}


bool operator!=(const BigFix& a, const BigFix& b)
{
    return a.hi != b.hi || a.lo != b.lo;
}


bool operator<(const BigFix& a, const BigFix& b)
{
    if (a.isNegative() == b.isNegative())
    {
        if (a.hi == b.hi)
            return a.lo < b.lo;
        else
            return a.hi < b.hi;
    }
    else
    {
        return a.isNegative();
    }
}


bool operator>(const BigFix& a, const BigFix& b)
{
    return b < a;
}


// TODO: probably faster to do this by converting the double to fixed
// point and using the fix*fix multiplication.
BigFix operator*(BigFix f, double_t d)
{
    // Need to break the number into 32-bit chunks because a 64-bit
    // integer has more bits of precision than a double.
	uint32_t w0 = f.lo & 0xffffffff;
	uint32_t w1 = f.lo >> 32;
	uint32_t w2 = f.hi & 0xffffffff;
	uint32_t w3 = f.hi >> 32;

    return BigFix(w0 * d * WORD0_FACTOR) +
           BigFix(w1 * d * WORD1_FACTOR) +
           BigFix(w2 * d * WORD2_FACTOR) +
           BigFix(w3 * d * WORD3_FACTOR);
}


/*! Multiply two BigFix values together. This function does not check for
 *  overflow. This is not a problem in Celestia, where it is used exclusively
 *  in multiplications where one multiplicand has absolute value <= 1.0.
 */
BigFix operator*(const BigFix& a, const BigFix& b)
{
    // Multiply two fixed point values together using partial products.

	uint64_t ah = a.hi;
	uint64_t al = a.lo;
    if (a.isNegative())
        BigFix::negate128(ah, al);

	uint64_t bh = b.hi;
	uint64_t bl = b.lo;
    if (b.isNegative())
        BigFix::negate128(bh, bl);

    // Break the values down into 32-bit words so that the partial products
    // will fit into 64-bit words.
	uint64_t aw[4];
    aw[0] = al & 0xffffffff;
    aw[1] = al >> 32;
    aw[2] = ah & 0xffffffff;
    aw[3] = ah >> 32;

	uint64_t bw[4];
    bw[0] = bl & 0xffffffff;
    bw[1] = bl >> 32;
    bw[2] = bh & 0xffffffff;
    bw[3] = bh >> 32;

    // Set the high and low non-zero words; this will
    // speed up multiplicatoin of integers and values
    // less than one.
    unsigned int hiworda = ah == 0 ? 1 : 3;
    unsigned int loworda = al == 0 ? 2 : 0;
    unsigned int hiwordb = bh == 0 ? 1 : 3;
    unsigned int lowordb = bl == 0 ? 2 : 0;

	uint32_t result[8];

    unsigned int i;
    for (i = 0; i < 8; i++)
        result[i] = 0;

    for (i = lowordb; i <= hiwordb; i++)
    {
		uint32_t carry = 0;

        unsigned int j;
        for (j = loworda; j <= hiworda; j++)
        {
			uint64_t partialProd = aw[j] * bw[i];

            // This sum will never overflow. Let N = 2^32;
            // the max value of the partial product is (N-1)^2.
            // The max values of result[i + j] and carry are
            // N-1. Thus the max value of the sum is
            // (N-1)^2 + 2(N-1) = (N^2 - 2N + 1) + 2(N-1) = N^2-1
			uint64_t q = (uint64_t)result[i + j] + partialProd + (uint64_t)carry;
			carry = (uint32_t)(q >> 32);
			result[i + j] = (uint32_t)(q & 0xffffffff);
        }

        result[i + j] = carry;
    }

    // TODO: check for overflow
    // (as simple as result[0] != 0 || result[1] != 0 || highbit(result[2]))
    BigFix c;
	c.lo = (uint64_t)result[2] + ((uint64_t)result[3] << 32);
	c.hi = (uint64_t)result[4] + ((uint64_t)result[5] << 32);

    bool resultNegative = a.isNegative() != b.isNegative();
    if (resultNegative)
        return -c;
    else
        return c;
}


int BigFix::sign() const
{
    
    if (hi == 0 && lo == 0)
        return 0;
    else if (hi > INT64_MAX)
        return -1;
    else
        return 1;
}


// For debugging
void BigFix::dump()
{
    printf("%08x %08x %08x %08x",
		(uint32_t)(hi >> 32),
		(uint32_t)(hi & 0xffffffff),
		(uint32_t)(lo >> 32),
		(uint32_t)(lo & 0xffffffff));
}


#if 0
int main(int argc, char* argv[])
{
    if (argc != 3)
        return 1;

    double_t a = 0.0;
    if (sscanf(argv[1], "%lf", &a) != 1)
        return 1;

    double_t b = 0.0;
    if (sscanf(argv[2], "%lf", &b) != 1)
        return 1;

    printf("    sum:\n%f\n%f\n", a + b, (double_t) (BigFix(a) + BigFix(b)));
    printf("   diff:\n%f\n%f\n", a - b, (double_t) (BigFix(a) - BigFix(b)));
    printf("product:\n%f\n%f\n", a * b, (double_t) (BigFix(a) * BigFix(b)));
    printf("     lt: %u %u\n", a < b, BigFix(a) < BigFix(b));

    return 0;
}
#endif

static unsigned char alphabet[65] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

BigFix::BigFix(const std::string& val)
{
    static char inalphabet[256], decoder[256];
    int i, bits, c, char_count;

    for (i = (sizeof alphabet) - 1; i >= 0 ; i--)
    {
        inalphabet[alphabet[i]] = 1;
        decoder[alphabet[i]] = i;
    }

	uint16_t n[8];

    // Code from original BigFix class to convert base64 string into
    // array of 8 16-bit values.
    for (i = 0; i < 8 ; i++)
        n[i] = 0;

    char_count = 0;
    bits = 0;

    i = 0;

    for (int j = 0; j < (int) val.length(); j++)
    {
        c = val[j];
        if (c == '=')
            break;
        if (c > 255 || !inalphabet[c])
            continue;
        bits += decoder[c];
        char_count++;
        if (char_count == 4)
        {
            n[i/2] >>= 8;
            n[i/2] += (bits >> 8) & 0xff00;
            i++;
            n[i/2] >>= 8;
            n[i/2] += bits & 0xff00;
            i++;
            n[i/2] >>= 8;
            n[i/2] += (bits << 8) & 0xff00;
            i++;
            bits = 0;
            char_count = 0;
        }
        else
        {
            bits <<= 6;
        }
    }

    switch (char_count)
    {
    case 2:
        n[i/2] >>= 8;
        n[i/2] += (bits >> 2) & 0xff00;
        i++;
        break;
    case 3:
        n[i/2] >>= 8;
        n[i/2] += (bits >> 8) & 0xff00;
        i++;
        n[i/2] >>= 8;
        n[i/2] += bits & 0xff00;
        i++;
        break;
    }

    if (i & 1)
        n[i/2] >>= 8;

    // Now, convert the 8 16-bit values to a 2 64-bit values
	lo = ((uint64_t)n[0] |
		((uint64_t)n[1] << 16) |
		((uint64_t)n[2] << 32) |
		((uint64_t)n[3] << 48));
	hi = ((uint64_t)n[4] |
		((uint64_t)n[5] << 16) |
		((uint64_t)n[6] << 32) |
		((uint64_t)n[7] << 48));
}


std::string BigFix::toString()
{
    // Old BigFix class used 8 16-bit words. The bulk of this function
    // is copied from that class, so first we'll convert from two
    // 64-bit words to 8 16-bit words so that the old code can work
    // as-is.
    unsigned short n[8];

    n[0] = lo & 0xffff;
    n[1] = (lo >> 16) & 0xffff;
    n[2] = (lo >> 32) & 0xffff;
    n[3] = (lo >> 48) & 0xffff;

    n[4] = hi & 0xffff;
    n[5] = (hi >> 16) & 0xffff;
    n[6] = (hi >> 32) & 0xffff;
    n[7] = (hi >> 48) & 0xffff;

    // Conversion using code from the original BigFix class.
    std::string encoded("");
    int bits, c, char_count, started, i, j;

    char_count = 0;
    bits = 0;
    started = 0;

    // Find first significant (non null) byte
    i = 16;
    do {
        i--;
        c = n[i/2];
        if (i & 1) c >>= 8;
        c &= 0xff;
    } while ((c == 0) && (i != 0));

    if (i == 0)
        return encoded;

    // Then we encode starting by the LSB (i+1 bytes to encode)
    j = 0;
    while (j <= i)
    {
        c = n[j/2];
        if ( j & 1 ) c >>= 8;
        c &= 0xff;
        j++;
        bits += c;
        char_count++;
        if (char_count == 3)
        {
            encoded += alphabet[bits >> 18];
            encoded += alphabet[(bits >> 12) & 0x3f];
            encoded += alphabet[(bits >> 6) & 0x3f];
            encoded += alphabet[bits & 0x3f];
            bits = 0;
            char_count = 0;
        }
        else
        {
            bits <<= 8;
        }
    }

    if (char_count != 0)
    {
        bits <<= 16 - (8 * char_count);
        encoded += alphabet[bits >> 18];
        encoded += alphabet[(bits >> 12) & 0x3f];
        if (char_count != 1)
            encoded += alphabet[(bits >> 6) & 0x3f];
    }

    return encoded;
}
