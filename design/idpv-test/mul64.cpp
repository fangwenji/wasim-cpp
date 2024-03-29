#include <strings.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <assert.h>

#define DOUBLE_MANTISSA_BITS 52
#define DOUBLE_EXPONENT_BITS 11
#define DOUBLE_SIGN_BIT 1

#define DOUBLE_EXPONENT_MASK 0x7FF0000000000000
#define DOUBLE_MANTISSA_MASK 0x000FFFFFFFFFFFFF
#define DOUBLE_SIGN_MASK 0x8000000000000000

#define DOUBLE_BIAS 1023
#define DOUBLE_EXPONENT_SHIFT 52

typedef union {
    double f;
    __uint64_t i;
} double_cast;

__uint64_t doubleMulParts(__uint64_t a1, __uint64_t a2) {
    __uint64_t total = 0;
    for (int bit = 0; bit < DOUBLE_MANTISSA_BITS; bit++) {
        if (a1 & ((__uint64_t)1 << bit)) {
            total += (a2 << bit);
        }
    }
    return total;
}

__uint64_t muldf3_classical(__uint64_t a1, __uint64_t a2) {
    __uint64_t sign = (a1 & DOUBLE_SIGN_MASK) ^ (a2 & DOUBLE_SIGN_MASK);
    __int64_t exp1 = (a1 & DOUBLE_EXPONENT_MASK) >> DOUBLE_EXPONENT_SHIFT;
    __int64_t exp2 = (a2 & DOUBLE_EXPONENT_MASK) >> DOUBLE_EXPONENT_SHIFT;
    __uint64_t mant1 = (a1 & DOUBLE_MANTISSA_MASK) | ((__uint64_t)1 << DOUBLE_MANTISSA_BITS);
    __uint64_t mant2 = (a2 & DOUBLE_MANTISSA_MASK) | ((__uint64_t)1 << DOUBLE_MANTISSA_BITS);
    
    if(!a1 || !a2) return 0;

    __int64_t exp = exp1 + exp2 - DOUBLE_BIAS;
    __uint64_t result = doubleMulParts(mant1, mant2);
    
    // Normalize and round the result (simplified for demonstration)
    // This is a complex process involving checking for overflow, underflow,
    // and the need for rounding. The full process is omitted for brevity.

    result = (sign | (exp << DOUBLE_EXPONENT_SHIFT) | (result & DOUBLE_MANTISSA_MASK));
    return result;
}

double __muldf3(double a, double b) {
    double_cast af, bf, cf;

    af.f = a;
    bf.f = b;
    cf.i = muldf3_classical(af.i, bf.i);
    return cf.f;
}

int main() {
    double a ; 
    double b ;
    double result = __muldf3(a, b);

    assert(result == 3.0);
    return 0;
}
