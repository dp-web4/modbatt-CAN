# Enhanced CAN Large Message Protocol (ECLMP)

## Design Goals
- Support messages up to 64KB (practical CAN bus limit)
- Maintain 100% payload efficiency for small messages
- Robust flow control and error recovery
- Backward compatible with existing ModBatt protocol
- Adaptive to message size

## Protocol Overview

### Dual-Mode Operation

**Mode 1: Small Messages (≤128 bytes)**
- Uses existing ModBatt chunked protocol
- 100% efficiency, simple implementation
- Direct extended ID chunk encoding

**Mode 2: Large Messages (>128 bytes)**
- Session-based protocol with enhanced features
- Windowed flow control
- Advanced error recovery

## Extended ID Structure (Enhanced)

```
Bit Position:  28 27 26 25 24 23 22 21 20 19 18 17 16 15 14 13 12 11 10  9  8  7  6  5  4  3  2  1  0
Mode 1 (≤128): [      Standard 11-bit CAN ID      ] E A [Current Chunk] [Total Chunks] [    Device ID    ]
Mode 2 (>128): [      Standard 11-bit CAN ID      ] E M [   Session ID  ] [ Window ID ] [    Device ID    ]
```

### Field Definitions (Mode 2)

| Field | Bits | Range | Description |
|-------|------|-------|-------------|
| **Device ID** | 0-7 | 0-255 | Pack controller or module identifier |
| **Window ID** | 8-11 | 0-15 | Current transmission window (16 chunks per window) |
| **Session ID** | 12-15 | 0-15 | Message session identifier |
| **Mode Flag (M)** | 16 | 0-1 | 0=Small mode, 1=Large mode |
| **Encryption Flag (E)** | 17 | 0-1 | 0=Unencrypted, 1=Encrypted payload |
| **Standard CAN ID** | 18-28 | 0-2047 | Standard 11-bit CAN identifier |

## Large Message Protocol Flow

### 1. Session Initiation Frame
```
Payload:
Byte 0: 0xSI (Session Initiate)
Bytes 1-2: Total message length (16-bit, up to 65535 bytes)
Byte 3: Chunks per window (1-16)
Byte 4: Chunk size (1-8 bytes)
Bytes 5-7: Message checksum/hash
```

### 2. Data Transmission Windows
- Send up to 16 chunks per window
- Each chunk uses Window ID + implicit chunk number within window
- Full 8-byte payload per chunk

### 3. Window Acknowledgment
```
Payload:
Byte 0: 0xWA (Window ACK)
Byte 1: Window ID being acknowledged
Bytes 2-3: Bitmap of received chunks (16 bits)
Bytes 4-7: Running checksum
```

### 4. Flow Control
- Receiver sends ACK after each window
- Sender waits for ACK before next window
- Automatic retry of missing chunks only

## Protocol Advantages

### Efficiency
- **Small messages**: 100% efficiency (unchanged)
- **Large messages**: 99%+ efficiency (minimal overhead)
- **Adaptive**: Protocol complexity scales with message size

### Scalability
- Supports up to **1048560 bytes** (65535 × 16 windows × 8 bytes max theoretical)
- Practical limit: **~4KB** for reasonable transmission time
- Window-based approach reduces memory requirements

### Reliability
- **Selective retransmission**: Only resend missing chunks
- **Session management**: Handle multiple concurrent transfers
- **Integrity checking**: Built-in checksums
- **Timeout handling**: Per-window timeouts with exponential backoff

### Flow Control
- **Windowed transmission**: Prevents buffer overflow
- **Adaptive window sizing**: Adjust based on channel conditions
- **Back-pressure**: Receiver controls transmission rate

## Implementation Example

```cpp
class ECLMPProtocol {
    enum Mode { SMALL_MESSAGE = 0, LARGE_MESSAGE = 1 };
    
    struct SessionState {
        uint8_t sessionId;
        uint16_t totalLength;
        uint8_t chunksPerWindow;
        uint8_t currentWindow;
        std::vector<bool> receivedChunks;
        uint32_t checksum;
    };
    
public:
    TPCANStatus SendMessage(const uint8_t* data, uint16_t length);
    bool ProcessReceivedFrame(const TPCANMsg& msg);
    
private:
    TPCANStatus SendSmallMessage(const uint8_t* data, uint8_t length);
    TPCANStatus SendLargeMessage(const uint8_t* data, uint16_t length);
    void HandleWindowAck(const TPCANMsg& msg);
    void RetransmitMissingChunks(uint8_t sessionId, uint8_t windowId, uint16_t bitmap);
};
```

## Performance Comparison

| Message Size | ModBatt | ISO-TP | ECLMP | Efficiency |
|--------------|---------|---------|--------|------------|
| 64 bytes | 8 frames | 10 frames | 8 frames | 100% |
| 128 bytes | 16 frames | 19 frames | 16 frames | 100% |
| 512 bytes | N/A | 74 frames | 65 frames | 99.2% |
| 2048 bytes | N/A | 293 frames | 257 frames | 99.6% |

## Backward Compatibility

- Mode bit (16) distinguishes protocol versions
- Existing ModBatt devices ignore Mode 2 messages
- Enhanced devices support both modes automatically
- Graceful degradation to simple chunking

## Future Enhancements

1. **Compression Integration**: Built-in payload compression
2. **Priority Levels**: Express vs. standard transmission
3. **Multicast Support**: One-to-many transmission
4. **Quality of Service**: Guaranteed delivery with SLA
5. **Encryption Integration**: Seamless with crypto protocols

## Conclusion

ECLMP provides the best of both worlds:
- **Maintains 100% efficiency** for current use cases
- **Scales elegantly** to large messages
- **Adds robustness** without complexity for simple cases
- **Future-proof** design for evolving requirements