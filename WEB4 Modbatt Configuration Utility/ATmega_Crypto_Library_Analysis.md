# ATmega328p Cryptographic Library Analysis

## Document Information

- **Version**: 1.0.0
- **Date**: August 2025
- **Status**: Technical Research
- **Author**: Crypto Implementation Team

## ATmega328p Memory Constraints

### Hardware Specifications
```
ATmega328p Memory:
├── Flash: 32KB (30KB usable after Arduino bootloader)
├── SRAM: 2KB (problems can occur below 1.7KB available)
└── EEPROM: 1KB (for persistent storage)
```

### Memory Budget for Crypto
```
Available for Crypto Libraries:
├── Flash: ~10-15KB (leaving room for application code)
├── SRAM: ~800-1000 bytes (during crypto operations)
└── EEPROM: 1KB (for key storage)
```

## Recommended Libraries by Algorithm

### 1. ECC-P256 + ECDSA

#### **micro-ecc** (Recommended)
**Source**: https://github.com/kmackay/micro-ecc

**Features**:
- ✅ **Supports secp256r1 (P-256)** - exact match for project requirements
- ✅ **Arduino Library Manager compatible**
- ✅ **Written in C with AVR optimizations**
- ✅ **No dynamic memory allocation**
- ✅ **8-bit microcontroller optimized**

**Memory Footprint**:
- **Flash**: ~6-8KB for ECDH + ECDSA
- **RAM**: ~200-400 bytes during operations
- **Performance**: Key generation ~4-8 seconds on ATmega328p @ 16MHz
- **ECDSA Sign/Verify**: ~1-2 seconds each

**Integration**:
```c
#include "uECC.h"

// P-256 curve (matches project requirements)
const struct uECC_Curve_t * curve = uECC_secp256r1();

// Generate binding key pair
uint8_t private_key[32];
uint8_t public_key[64]; // Uncompressed format
uECC_make_key(public_key, private_key, curve);

// Sign message
uint8_t signature[64];
uECC_sign(private_key, message_hash, 32, signature, curve);

// Verify signature  
int valid = uECC_verify(public_key, message_hash, 32, signature, curve);
```

#### **nano-ecc** (Alternative)
**Source**: https://github.com/iSECPartners/nano-ecc

**Features**:
- ✅ **Extremely small footprint** (~6KB total)
- ✅ **Supports secp256r1**
- ⚠️ **Less actively maintained**

**Memory Footprint**:
- **Flash**: ~6KB for ECDH + ECDSA
- **RAM**: ~300 bytes
- **Performance**: Similar to micro-ecc

### 2. AES-256-GCM

#### **Arduino Cryptography Library** (Recommended)
**Source**: http://rweather.github.io/arduinolibs/crypto.html

**Features**:
- ✅ **AES-256-GCM specifically supported**
- ✅ **AVR-optimized** (saves 192 bytes over standard implementations)
- ✅ **Multiple variants**: tiny, small, full versions
- ✅ **Static sbox tables in program memory** (saves RAM)

**Memory Footprint**:
- **Flash**: ~3-4KB for AES-256-GCM (tiny version)
- **RAM**: ~200-300 bytes during operation
- **Key Storage**: 32 bytes (AES-256 key)

**Integration**:
```c
#include <GCM.h>
#include <AES.h>

GCM<AES256> gcm;

// Setup
uint8_t key[32];        // AES-256 key
uint8_t iv[12];         // GCM nonce
uint8_t tag[16];        // Authentication tag

gcm.setKey(key, 32);
gcm.setIV(iv, 12);

// Encrypt
gcm.encrypt(ciphertext, plaintext, plaintext_len);
gcm.computeTag(tag, 16);

// Decrypt  
gcm.decrypt(plaintext, ciphertext, ciphertext_len);
bool valid = gcm.checkTag(tag, 16);
```

#### **tiny-AES-c** (Lightweight Alternative)
**Source**: https://github.com/kokke/tiny-AES-c

**Features**:
- ✅ **Very small** (~1KB for CTR mode)
- ✅ **AVR tested**
- ⚠️ **Manual GCM implementation required**

### 3. HKDF-SHA256

#### **Arduino Cryptography Library** (Recommended)
**Includes both SHA256 and HKDF implementations**

**Memory Footprint**:
- **Flash**: ~2-3KB for SHA256 + HKDF
- **RAM**: ~100-200 bytes during operation

**Integration**:
```c
#include <SHA256.h>
#include <HKDF.h>

SHA256 sha256;
HKDF<SHA256> hkdf;

// Key derivation
uint8_t input_key[64];    // From API-Bridge
uint8_t salt[32];         // App-generated
uint8_t output_key[32];   // Derived AES key

hkdf.setKey(input_key, 64, salt, 32);
hkdf.extract(output_key, 32, "AES-KEY");
```

#### **Microcontroller-hashes** (Alternative)
**Source**: https://github.com/oilulio/Microcontroller-hashes

**Features**:
- ✅ **ATmega328p tested**
- ✅ **Minimal footprint**
- ⚠️ **Manual HKDF implementation required**

## Complete Memory Budget Analysis

### Scenario: All Three Algorithms

**Flash Usage**:
```
micro-ecc (P-256):           ~7KB
Arduino Crypto (AES-GCM):    ~4KB  
Arduino Crypto (SHA256):     ~3KB
Total Flash:                 ~14KB

Available for app code:      ~16KB (plenty)
```

**RAM Usage (Worst Case)**:
```
ECC operations:              ~400 bytes
AES-GCM operations:          ~300 bytes
SHA256/HKDF operations:      ~200 bytes
Key storage:                 ~200 bytes
Total RAM (not concurrent):  ~500-600 bytes

Available for app:           ~1.2-1.4KB (sufficient)
```

### ✅ **VERDICT: FEASIBLE**

All required algorithms can fit on ATmega328p with careful implementation.

## Implementation Strategy

### Phase 1: Proof of Concept
```c
// Test each library individually on ATmega328p
// Measure actual flash/RAM usage
// Verify performance is acceptable
```

### Phase 2: Integration
```c
// Combine libraries with minimal overlap
// Optimize memory usage
// Test complete key distribution workflow
```

### Phase 3: Optimization
```c
// Use "tiny" versions where possible
// Store keys in EEPROM to save RAM
// Implement memory management strategies
```

## Key Storage Strategy

### EEPROM Layout
```
EEPROM (1KB):
├── Private Key (32 bytes)           // Device binding private key
├── App Public Key (64 bytes)        // For encryption to app
├── Partner Public Key (64 bytes)    // Pack controller or module
├── Device Key Half (64 bytes)       // From API-Bridge
├── Configuration (32 bytes)         // Crypto parameters
└── Reserved (768 bytes)             // Future use
```

### Flash Storage (Alternative)
```c
// Use dedicated flash sector for keys
const uint8_t private_key[32] __attribute__((section(".key_storage")));
```

## Performance Expectations

### Key Generation (One-time)
```
ECC Key Pair Generation:     ~4-8 seconds
AES Key Derivation (HKDF):   ~100-500ms
Total Setup Time:            ~5-10 seconds
```

### Runtime Operations
```
ECDSA Sign:                  ~1-2 seconds
ECDSA Verify:                ~1-2 seconds  
AES-GCM Encrypt (64 bytes):  ~10-50ms
AES-GCM Decrypt (64 bytes):  ~10-50ms
```

### CAN Key Distribution Timeline
```
1. Receive encrypted key chunks:     ~2-5 seconds
2. ECDSA verify + decrypt:           ~1-2 seconds
3. HKDF derive AES keys:             ~100ms
4. Store keys in EEPROM:             ~50ms
Total key distribution:              ~3-8 seconds
```

## Code Size Optimization Tips

### Compiler Flags
```bash
# Enable optimizations (required for micro-ecc)
-Os                    # Optimize for size
-ffunction-sections    # Enable dead code elimination
-fdata-sections        # Enable dead data elimination
-Wl,--gc-sections     # Linker garbage collection
```

### Library Configuration
```c
// Arduino Cryptography Library - minimal config
#define CRYPTO_AES_DEFAULT AES256_CIPHER
#define CRYPTO_GCM_DEFAULT GCM_CIPHER  
#define CRYPTO_SHA_DEFAULT SHA256_HASHER
// Disable unused algorithms

// micro-ecc - enable only P-256
#define uECC_SUPPORTS_secp256r1 1
#define uECC_SUPPORTS_secp192r1 0  
#define uECC_SUPPORTS_secp224r1 0
// Disable other curves
```

## Conclusion

**✅ ATmega328p CAN support all required cryptographic algorithms** with the following library combination:

1. **ECC-P256 + ECDSA**: micro-ecc library
2. **AES-256-GCM**: Arduino Cryptography Library  
3. **HKDF-SHA256**: Arduino Cryptography Library

**Memory Budget**: 
- Flash: ~14KB (fits in 30KB available)
- RAM: ~600 bytes peak (fits in 2KB available)
- Performance: Acceptable for key distribution (non-real-time)

**Recommendation**: Proceed with Arduino Cryptography Library + micro-ecc combination for ATmega328p implementation.

---

**Document Status**: Research Complete - Ready for Implementation
**Next Steps**: Download libraries and create proof-of-concept test
**Dependencies**: Arduino IDE/AVR-GCC toolchain, library installation