#ifndef SHA1_HPP
#define SHA1_HPP

#include <cstdint>
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <iomanip>

using namespace std;

/**
 * @brief SHA-1 (Secure Hash Algorithm 1) implementation
 * 
 * SHA-1 produces a 160-bit (20-byte) hash value, typically rendered as 40 hexadecimal digits.
 * Used in Git for content-addressable storage - same content always produces same hash.
 * 
 * Algorithm Overview:
 * 1. Pad message to multiple of 512 bits
 * 2. Process message in 512-bit (64-byte) blocks
 * 3. Each block updates five 32-bit digest values (a, b, c, d, e)
 * 4. Final digest is concatenation of these five values
 */
class SHA1 {
public:
    /**
     * @brief Constructor - initializes digest to standard SHA-1 constants
     */
    SHA1() {
        reset();
    }

    /**
     * @brief Update hash with string data
     * @param s String to add to hash computation
     */
    void update(const string &s) {
        update(s.c_str(), s.length());
    }

    /**
     * @brief Update hash with raw byte data
     * @param data Pointer to data buffer
     * @param len Length of data in bytes
     * 
     * Accumulates data into 64-byte blocks. When a block is full,
     * it's processed immediately via transform().
     */
    void update(const char *data, size_t len) {
        for (size_t i = 0; i < len; ++i) {
            buffer[buffer_len++] = data[i];
            if (buffer_len == 64) {
                // Process complete 512-bit block
                transform(buffer);
                buffer_len = 0;
            }
        }
    }

    /**
     * @brief Finalize hash and return result as hex string
     * @return 40-character hexadecimal hash string
     * 
     * Padding process (SHA-1 specification):
     * 1. Append 0x80 byte (bit '1' followed by zeros)
     * 2. Append zeros until message length ≡ 448 (mod 512)
     * 3. Append 64-bit message length
     * 4. Process final block(s)
     */
    string final() {
        uint8_t final_block[128];  // May need up to 2 blocks for padding
        size_t final_block_len = 0;

        // Calculate total bits processed
        uint64_t total_bits = (message_len * 8) + (buffer_len * 8);

        // Step 1: Append mandatory '1' bit (0x80 = 10000000 in binary)
        final_block[final_block_len++] = 0x80;

        // If current block can't fit padding + length, need extra block
        if (buffer_len > 55) {
            // Fill rest of current block with zeros
            while (buffer_len < 64) {
                buffer[buffer_len++] = 0;
            }
            transform(buffer);
            buffer_len = 0;
        }

        // Step 2: Pad with zeros until 56 bytes (leaving 8 bytes for length)
        while (final_block_len < 56 - buffer_len) {
            final_block[final_block_len++] = 0;
        }

        // Copy remaining buffer data
        for (size_t i = 0; i < buffer_len; ++i) {
            final_block[final_block_len++] = buffer[i];
        }

        // Step 3: Append 64-bit message length (big-endian)
        for (int i = 0; i < 8; ++i) {
            final_block[final_block_len++] = (total_bits >> (56 - i * 8)) & 0xFF;
        }

        // Process final block(s)
        transform(final_block);
        if (final_block_len > 64) {
            transform(final_block + 64);  // Process second block if needed
        }

        // Convert 5 x 32-bit digest values to hexadecimal string
        stringstream ss;
        for (int i = 0; i < 5; ++i) {
            ss << hex << setw(8) << setfill('0') << digest[i];
        }

        reset();  // Prepare for reuse
        return ss.str();
    }

private:
    /**
     * @brief Reset to initial SHA-1 state
     * 
     * These magic constants are defined in the SHA-1 specification.
     * They're the first 32 bits of fractional parts of square roots of primes.
     */
    void reset() {
        digest[0] = 0x67452301;  // h0
        digest[1] = 0xEFCDAB89;  // h1
        digest[2] = 0x98BADCFE;  // h2
        digest[3] = 0x10325476;  // h3
        digest[4] = 0xC3D2E1F0;  // h4
        buffer_len = 0;
        message_len = 0;
    }

    /**
     * @brief Process a single 512-bit (64-byte) block
     * @param block Pointer to 64-byte block to process
     * 
     * Core SHA-1 algorithm:
     * 1. Expand 16 x 32-bit words to 80 x 32-bit words (message schedule)
     * 2. Initialize working variables from current digest
     * 3. Perform 80 rounds of mixing operations
     * 4. Add result to digest
     * 
     * Each round uses different logical functions and constants:
     * - Rounds 0-19:  f = (b AND c) OR ((NOT b) AND d), k = 0x5A827999
     * - Rounds 20-39: f = b XOR c XOR d,                k = 0x6ED9EBA1
     * - Rounds 40-59: f = (b AND c) OR (b AND d) OR (c AND d), k = 0x8F1BBCDC
     * - Rounds 60-79: f = b XOR c XOR d,                k = 0xCA62C1D6
     */
    void transform(const uint8_t *block) {
        uint32_t w[80];  // Message schedule (expanded from 16 to 80 words)
        
        // Step 1: Copy 16 x 32-bit words from block (big-endian)
        for (int i = 0; i < 16; ++i) {
            w[i] = (block[i * 4] << 24) | 
                   (block[i * 4 + 1] << 16) | 
                   (block[i * 4 + 2] << 8) | 
                   block[i * 4 + 3];
        }

        // Step 2: Extend 16 words to 80 words using XOR and rotation
        for (int i = 16; i < 80; ++i) {
            w[i] = left_rotate(w[i - 3] ^ w[i - 8] ^ w[i - 14] ^ w[i - 16], 1);
        }

        // Step 3: Initialize working variables with current digest
        uint32_t a = digest[0];
        uint32_t b = digest[1];
        uint32_t c = digest[2];
        uint32_t d = digest[3];
        uint32_t e = digest[4];

        // Step 4: Main loop - 80 rounds of mixing
        for (int i = 0; i < 80; ++i) {
            uint32_t f, k;
            
            // Select function and constant based on round number
            if (i < 20) {
                // Rounds 0-19: Choice function (b chooses between c and d)
                f = (b & c) | ((~b) & d);
                k = 0x5A827999;
            } else if (i < 40) {
                // Rounds 20-39: Parity function
                f = b ^ c ^ d;
                k = 0x6ED9EBA1;
            } else if (i < 60) {
                // Rounds 40-59: Majority function
                f = (b & c) | (b & d) | (c & d);
                k = 0x8F1BBCDC;
            } else {
                // Rounds 60-79: Parity function again
                f = b ^ c ^ d;
                k = 0xCA62C1D6;
            }

            // Update working variables
            // temp = ROTL(a,5) + f + e + k + w[i]
            uint32_t temp = left_rotate(a, 5) + f + e + k + w[i];
            e = d;
            d = c;
            c = left_rotate(b, 30);
            b = a;
            a = temp;
        }

        // Step 5: Add this block's hash to overall digest
        digest[0] += a;
        digest[1] += b;
        digest[2] += c;
        digest[3] += d;
        digest[4] += e;

        message_len += 64;  // Track total message length
    }

    /**
     * @brief Circular left bit rotation
     * @param value 32-bit value to rotate
     * @param count Number of bits to rotate left
     * @return Rotated value
     * 
     * Example: left_rotate(0b11010000, 3) = 0b10000110
     * Bits that fall off left end wrap around to right end
     */
    static uint32_t left_rotate(uint32_t value, size_t count) {
        return (value << count) | (value >> (32 - count));
    }

    // State variables
    uint32_t digest[5];      // Five 32-bit hash values (h0, h1, h2, h3, h4)
    uint8_t buffer[64];      // Accumulation buffer for incomplete blocks
    size_t buffer_len;       // Current number of bytes in buffer
    uint64_t message_len;    // Total message length in bytes (excluding buffer)
};

#endif
