#include <stdint.h>
#include <stdio.h>
#include <assert.h>

// Function to mimic the behavior of the Verilog module.
void spec(uint16_t A, uint16_t B, uint8_t M, uint8_t N, uint64_t *O) {
    // Ensure that M and N are within the range of 0 to 15 since the shift amount
    // cannot be greater than the bit width of A and B.
    M &= 0xF;
    N &= 0xF;

    // Perform the shift operations, promoting A and B to a larger type
    // to avoid overflow during the shift if necessary.
    uint32_t D = (uint32_t)A << M;
    uint32_t E = (uint32_t)B << N;

    // Perform the multiplication and store the result in O.
    *O = (uint64_t)D * (uint64_t)E;
}

int main() {
    uint16_t A = unknown(); // Example values for A
    uint16_t B = unknown(); // Example values for B
    uint8_t M = unknown();       // Example values for M
    uint8_t N = unknown();       // Example values for N
    uint64_t O = unknown();

    // Call the spec function.
    spec(A, B, M, N, &O);
    
    // Output the result.
    assert(O==1);
    return 0;
}