# CAN Extended ID Structure for Chunked Data Transmission

## Overview
This document defines the proposed CAN extended ID (29-bit) bit allocation for transmitting large data payloads (up to 128 bytes) using multiple 8-byte CAN frames.

## Extended ID Bit Allocation (29-bit)

```
Bit Position:  28 27 26 25 24 23 22 21 20 19 18 17 16 15 14 13 12 11 10  9  8  7  6  5  4  3  2  1  0
Field:         [      Standard 11-bit CAN ID      ] E A [Current Chunk] [Total Chunks] [    Device ID    ]
```

### Field Definitions

| Field | Bits | Range | Description |
|-------|------|-------|-------------|
| **Device ID** | 0-7 | 0-255 | Pack controller or module identifier |
| **Total Chunks** | 8-11 | 1-16 | Total number of chunks in the complete message |
| **Current Chunk** | 12-15 | 1-16 | Current chunk number (1-based indexing) |
| **Available (A)** | 16 | 0-1 | Available for future use |
| **Encryption Flag (E)** | 17 | 0-1 | 0=Unencrypted, 1=Encrypted payload |
| **Standard CAN ID** | 18-28 | 0-2047 | Standard 11-bit CAN identifier for compatibility |

## Usage Examples

### 64-byte Key Transmission (8 chunks)
- **Total Chunks**: 8 (bits 8-11 = 0x8)
- **Current Chunk**: 1-8 (bits 12-15 = 0x1 to 0x8)
- **Encryption Flag**: 0 (bit 17 = 0) - Keys cannot be encrypted with themselves
- **Device ID**: e.g., 0x42 (bits 0-7 = 0x42)

**Example Extended ID for chunk 3 of 8, key transmission (unencrypted), device 0x42:**
```
Standard ID: 0x000 (bits 18-28)
Encryption: 0 (bit 17)
Available: 0 (bit 16) 
Current Chunk: 3 (bits 12-15)
Total Chunks: 8 (bits 8-11)
Device ID: 0x42 (bits 0-7)

Binary: 0000000000000011100001000001000010
Hex:    0x01C842
```

### 128-byte Key Transmission (16 chunks)
- **Total Chunks**: 16 (bits 8-11 = 0xF)
- **Current Chunk**: 1-16 (bits 12-15 = 0x1 to 0xF)

## Protocol Considerations

1. **Chunk Numbering**: 1-based indexing (1 to N) for clarity
2. **Reassembly**: Receiver waits for all chunks (1 to Total Chunks) before processing
3. **Error Handling**: Missing chunks detected by gap in sequence
4. **Timeout**: Implement timeout for incomplete chunk sequences
5. **Scalability**: Supports up to 16 chunks (128 bytes total payload)

## Payload Obfuscation

To protect sensitive key material during transmission, each chunk's payload is obfuscated using byte rotation based on the current chunk number.

### Obfuscation Algorithm

**Transmission (Obfuscate):**
```cpp
uint8_t obfuscateByte(uint8_t byte, uint8_t chunkNumber) {
    uint8_t rotations = chunkNumber % 8;  // 0-7 rotations
    return (byte << rotations) | (byte >> (8 - rotations));
}
```

**Reception (Deobfuscate):**
```cpp
uint8_t deobfuscateByte(uint8_t byte, uint8_t chunkNumber) {
    uint8_t rotations = chunkNumber % 8;  // 0-7 rotations
    return (byte >> rotations) | (byte << (8 - rotations));
}
```

### Obfuscation Examples

| Chunk Number | Rotation Amount | Original Byte | Obfuscated Byte |
|--------------|-----------------|---------------|-----------------|
| 1 | 1 | 0xAB (10101011) | 0x57 (01010111) |
| 2 | 2 | 0xAB (10101011) | 0xAE (10101110) |
| 8 | 0 | 0xAB (10101011) | 0xAB (10101011) |

### Security Considerations

**Important Security Note:** This byte rotation is **obfuscation, not encryption**. Since key transmission messages have Encryption Flag = 0 (keys cannot be encrypted with themselves), obfuscation is the primary protection against CAN bus sniffing during key exchange. This obfuscation:

- Prevents raw key material from appearing in plaintext on the CAN bus
- Provides the only protection layer for unencrypted key transmission
- Makes passive sniffing attacks more difficult during key exchange
- Does NOT provide cryptographic security
- Can be reverse-engineered by someone with knowledge of the protocol
- Is particularly critical since key messages are inherently unencrypted

## Implementation Notes

- All chunks in a sequence must have identical Total Chunks value
- Device ID must remain consistent across all chunks in a sequence
- Encryption flag should be consistent across all chunks in a sequence
- Consider implementing chunk sequence ID in reserved bits for multiple concurrent transfers
- Apply obfuscation to all 8 payload bytes in each CAN frame
- Obfuscation is applied after chunking but before CAN transmission