# Modbatt Encryption: Immediate Implementation Plan

## Document Information

- **Version**: 1.0.0
- **Date**: August 2025
- **Status**: Practical Implementation Guide
- **Context**: Getting Basic Encryption Working Now

## Overview

This document provides a **practical, step-by-step plan** to implement basic encryption in the Modbatt system using the existing codebase. This focuses on getting a **working encrypted system** first, with WEB4 sophistication to be added later.

## Current State Assessment

### ✅ **What We Already Have**
- **API-Bridge Integration Framework**: Basic structure in `web4.cpp`
- **LCT Creation Structure**: `TLctRelationship` class with key extraction
- **Key Storage Fields**: `web4DeviceKeyHalf[64]` and `web4LctKeyHalf[64]` in BMS structures
- **CAN Communication**: Existing CAN protocol with 29-bit ID support
- **Component Registration**: Basic blockchain component registration capability

### ❌ **What We Need to Complete**
- **Complete API calls** for LCT creation between components
- **Key combination algorithm** to merge key halves into encryption keys
- **AES encryption library** integration
- **CAN message encryption/decryption** functions
- **Bit 17 encryption signaling** in CAN ID

## Immediate Implementation Plan

### **Phase 1: Complete API-Bridge Integration (Week 1)**

#### **1.1 Complete LCT Creation API Calls**

**File**: `web4.cpp`
**Goal**: Extend existing `TLctRelationship` class to handle component-to-component LCT creation

```cpp
// Add to TLctRelationship class
class TLctRelationship {
private:
    // Existing fields...
    String componentA_id;
    String componentB_id;
    
public:
    // New method: Create LCT between two registered components
    bool CreateComponentLCT(const String& creatorId, 
                           const String& componentA, 
                           const String& componentB,
                           const String& context = "battery_management") {
        
        // Build API request
        StaticJsonDocument<512> requestDoc;
        requestDoc["creator"] = creatorId;
        requestDoc["component_a"] = componentA;
        requestDoc["component_b"] = componentB;
        requestDoc["context"] = context;
        
        // Make POST request to /api/v1/lct/create
        String response = PostToAPIBridge("/api/v1/lct/create", requestDoc);
        
        // Parse response and extract key halves
        return ParseLCTResponse(response);
    }
    
private:
    bool ParseLCTResponse(const String& response) {
        StaticJsonDocument<1024> doc;
        deserializeJson(doc, response);
        
        if (doc["status"] == "success") {
            LctId = TWeb4Utils::SafeJsonString(doc["lct_id"]);
            DeviceKeyHalf = TWeb4Utils::SafeJsonString(doc["device_key_half"]);
            LctKeyHalf = TWeb4Utils::SafeJsonString(doc["lct_key_half"]);
            Status = "active";
            return true;
        }
        return false;
    }
};
```

#### **1.2 App-to-Pack Controller LCT Creation**

**File**: Main application code
**Goal**: Create LCT relationship when pack controller is discovered

```cpp
// In your main app logic
bool EstablishPackControllerTrust(const String& packControllerSerial) {
    // 1. Register pack controller as component (if not already done)
    if (!RegisterPackController(packControllerSerial)) {
        return false;
    }
    
    // 2. Create LCT between app and pack controller
    TLctRelationship appToPackLCT;
    if (!appToPackLCT.CreateComponentLCT(appComponentId, 
                                        appComponentId, 
                                        packControllerComponentId)) {
        return false;
    }
    
    // 3. Store the key halves for distribution
    String appDeviceKeyHalf = appToPackLCT.GetDeviceKeyHalf();
    String packDeviceKeyHalf = appToPackLCT.GetDeviceKeyHalf(); // Same keys, different perspective
    
    // 4. Send keys to pack controller via unencrypted CAN
    return DistributeKeysToPackController(packDeviceKeyHalf, appDeviceKeyHalf);
}
```

#### **1.3 Pack Controller Key Storage**

**File**: Pack controller firmware
**Goal**: Securely store received keys in flash memory

```cpp
// Enhanced key storage structure
struct PackControllerCrypto {
    // Keys for communication with app
    char appDeviceKeyHalf[65];          // +1 for null terminator
    char packDeviceKeyHalf[65];         // +1 for null terminator
    char combinedAppKey[33];            // Derived 32-byte AES key + null
    
    // Keys for communication with modules (added later)
    char moduleDeviceKeyHalf[65];       // For module communication
    char combinedModuleKey[33];         // Derived module AES key
    
    // Identity information
    char componentId[33];               // Pack controller's blockchain ID
    char appComponentId[33];            // App's component ID
    
    // Status flags
    bool appKeysValid;                  // App communication keys loaded
    bool moduleKeysValid;               // Module communication keys loaded
    uint32_t keyVersion;                // For key rotation tracking
    uint32_t crc32;                     // Integrity check
};

// Functions to manage keys
bool StoreAppKeys(const char* appKeyHalf, const char* packKeyHalf, const char* appCompId) {
    PackControllerCrypto crypto = {0};
    
    // Copy keys
    strncpy(crypto.appDeviceKeyHalf, appKeyHalf, 64);
    strncpy(crypto.packDeviceKeyHalf, packKeyHalf, 64);
    strncpy(crypto.appComponentId, appCompId, 32);
    
    // Derive combined key
    if (!CombineKeyHalves(crypto.appDeviceKeyHalf, crypto.packDeviceKeyHalf, 
                         (uint8_t*)crypto.combinedAppKey)) {
        return false;
    }
    
    // Calculate CRC and save to flash
    crypto.crc32 = CalculateCRC32(&crypto, sizeof(crypto) - 4);
    crypto.appKeysValid = true;
    
    return WriteToFlash(CRYPTO_FLASH_ADDRESS, &crypto, sizeof(crypto));
}

bool LoadAppKeys(PackControllerCrypto* crypto) {
    if (!ReadFromFlash(CRYPTO_FLASH_ADDRESS, crypto, sizeof(*crypto))) {
        return false;
    }
    
    // Verify integrity
    uint32_t expectedCRC = CalculateCRC32(crypto, sizeof(*crypto) - 4);
    return (crypto->crc32 == expectedCRC && crypto->appKeysValid);
}
```

### **Phase 2: Key Combination and Basic Encryption (Week 2)**

#### **2.1 Key Combination Algorithm**

**File**: Crypto utilities
**Goal**: Simple but secure method to combine 64-byte halves into 32-byte AES keys

```cpp
#include <mbedtls/sha256.h>  // Or any SHA256 implementation

// Simple key combination using SHA256 (upgrade to HKDF later)
bool CombineKeyHalves(const char* keyHalf1, const char* keyHalf2, uint8_t* outputKey) {
    if (!keyHalf1 || !keyHalf2 || !outputKey) {
        return false;
    }
    
    // Combine the two 64-byte halves
    uint8_t combined[128];
    memcpy(combined, keyHalf1, 64);
    memcpy(combined + 64, keyHalf2, 64);
    
    // Hash to produce 32-byte AES-256 key
    mbedtls_sha256_context ctx;
    mbedtls_sha256_init(&ctx);
    mbedtls_sha256_starts(&ctx, 0);  // SHA256, not SHA224
    mbedtls_sha256_update(&ctx, combined, 128);
    mbedtls_sha256_finish(&ctx, outputKey);
    mbedtls_sha256_free(&ctx);
    
    // Clear combined material from memory
    memset(combined, 0, sizeof(combined));
    
    return true;
}

// Future enhancement: HKDF-based combination
bool CombineKeyHalves_HKDF(const char* keyHalf1, const char* keyHalf2, 
                          const char* salt, const char* info,
                          uint8_t* outputKey) {
    // TODO: Implement HKDF-SHA256 for production
    // For now, use simple SHA256 combination
    return CombineKeyHalves(keyHalf1, keyHalf2, outputKey);
}
```

#### **2.2 Basic AES Encryption Implementation**

**File**: Crypto utilities
**Goal**: Simple AES-256-CBC encryption (upgrade to GCM later)

```cpp
#include <mbedtls/aes.h>
#include <mbedtls/ctr_drbg.h>
#include <mbedtls/entropy.h>

// Simple AES-256-CBC encryption structure
struct EncryptedMessage {
    uint8_t iv[16];           // AES block size
    uint8_t data[64];         // Max CAN data + padding
    uint8_t actualLength;     // Original data length
    uint8_t padding;          // Padding bytes added
};

// Basic encryption function
bool EncryptCANData(const uint8_t* plaintext, size_t plaintextLen,
                   const uint8_t* key, EncryptedMessage* output) {
    if (!plaintext || !key || !output || plaintextLen > 8) {
        return false;  // CAN messages are max 8 bytes
    }
    
    mbedtls_aes_context aes;
    mbedtls_aes_init(&aes);
    
    // Set encryption key
    if (mbedtls_aes_setkey_enc(&aes, key, 256) != 0) {
        mbedtls_aes_free(&aes);
        return false;
    }
    
    // Generate random IV
    GenerateRandomBytes(output->iv, 16);
    
    // Prepare padded data (PKCS#7 padding)
    uint8_t paddedData[16] = {0};  // AES block size
    memcpy(paddedData, plaintext, plaintextLen);
    
    uint8_t padding = 16 - (plaintextLen % 16);
    for (int i = plaintextLen; i < plaintextLen + padding; i++) {
        paddedData[i] = padding;
    }
    
    // Encrypt
    uint8_t encrypted[16];
    if (mbedtls_aes_crypt_cbc(&aes, MBEDTLS_AES_ENCRYPT, 16, 
                             output->iv, paddedData, encrypted) != 0) {
        mbedtls_aes_free(&aes);
        return false;
    }
    
    // Store results
    memcpy(output->data, encrypted, 16);
    output->actualLength = plaintextLen;
    output->padding = padding;
    
    mbedtls_aes_free(&aes);
    return true;
}

// Basic decryption function
bool DecryptCANData(const EncryptedMessage* input, const uint8_t* key,
                   uint8_t* plaintext, size_t* plaintextLen) {
    if (!input || !key || !plaintext || !plaintextLen) {
        return false;
    }
    
    mbedtls_aes_context aes;
    mbedtls_aes_init(&aes);
    
    // Set decryption key
    if (mbedtls_aes_setkey_dec(&aes, key, 256) != 0) {
        mbedtls_aes_free(&aes);
        return false;
    }
    
    // Decrypt
    uint8_t decrypted[16];
    uint8_t iv_copy[16];
    memcpy(iv_copy, input->iv, 16);  // CBC modifies IV
    
    if (mbedtls_aes_crypt_cbc(&aes, MBEDTLS_AES_DECRYPT, 16,
                             iv_copy, input->data, decrypted) != 0) {
        mbedtls_aes_free(&aes);
        return false;
    }
    
    // Extract original data (remove padding)
    *plaintextLen = input->actualLength;
    memcpy(plaintext, decrypted, *plaintextLen);
    
    mbedtls_aes_free(&aes);
    return true;
}
```

#### **2.3 CAN Message Integration**

**File**: CAN communication layer
**Goal**: Integrate encryption with existing CAN protocol

```cpp
// Enhanced CAN message structure
struct EncryptedCANMessage {
    uint32_t id;              // 29-bit CAN ID (bit 17 = encryption flag)
    uint8_t dlc;              // Data length code
    union {
        uint8_t raw[8];       // Raw data for unencrypted
        struct {
            uint8_t iv[4];    // Truncated IV (first 4 bytes)
            uint8_t data[4];  // Encrypted data (truncated)
        } encrypted;
    } payload;
    bool isEncrypted;         // Local flag
};

// Set encryption bit in CAN ID
uint32_t SetEncryptionBit(uint32_t canId, bool encrypted) {
    if (encrypted) {
        return canId | (1UL << 17);  // Set bit 17
    } else {
        return canId & ~(1UL << 17); // Clear bit 17
    }
}

// Check encryption bit in CAN ID
bool IsMessageEncrypted(uint32_t canId) {
    return (canId & (1UL << 17)) != 0;
}

// Send encrypted CAN message
bool SendEncryptedCANMessage(uint32_t baseId, const uint8_t* data, size_t len) {
    PackControllerCrypto crypto;
    if (!LoadAppKeys(&crypto)) {
        return false;  // No keys available
    }
    
    // Encrypt the data
    EncryptedMessage encrypted;
    if (!EncryptCANData(data, len, (uint8_t*)crypto.combinedAppKey, &encrypted)) {
        return false;
    }
    
    // Create CAN message (simplified - may need fragmentation for full encrypted data)
    EncryptedCANMessage canMsg;
    canMsg.id = SetEncryptionBit(baseId, true);
    canMsg.dlc = 8;  // Always use full frame for encrypted data
    
    // Pack first 4 bytes of IV and first 4 bytes of encrypted data
    memcpy(canMsg.payload.encrypted.iv, encrypted.iv, 4);
    memcpy(canMsg.payload.encrypted.data, encrypted.data, 4);
    canMsg.isEncrypted = true;
    
    // Send via CAN
    return SendCANMessage(&canMsg);
}

// Receive and decrypt CAN message
bool ReceiveEncryptedCANMessage(EncryptedCANMessage* canMsg, 
                               uint8_t* decryptedData, size_t* dataLen) {
    if (!canMsg || !IsMessageEncrypted(canMsg->id)) {
        return false;  // Not an encrypted message
    }
    
    PackControllerCrypto crypto;
    if (!LoadAppKeys(&crypto)) {
        return false;  // No keys available
    }
    
    // Reconstruct encrypted message (simplified)
    EncryptedMessage encrypted;
    memcpy(encrypted.iv, canMsg->payload.encrypted.iv, 4);
    memset(encrypted.iv + 4, 0, 12);  // Zero-pad IV (not secure, but simple)
    memcpy(encrypted.data, canMsg->payload.encrypted.data, 4);
    encrypted.actualLength = 4;  // Assume 4 bytes for now
    
    // Decrypt
    return DecryptCANData(&encrypted, (uint8_t*)crypto.combinedAppKey, 
                         decryptedData, dataLen);
}
```

### **Phase 3: Module Integration (Week 3)**

#### **3.1 Pack Controller to Module LCT Creation**

**Goal**: Extend the pattern to battery modules

```cpp
// In pack controller firmware
bool EstablishModuleTrust(const String& moduleSerial) {
    // 1. Register module via app (app does the API calls)
    // 2. Request LCT creation between pack controller and module
    // 3. Receive module key halves from app
    // 4. Store module keys separately from app keys
    // 5. Test encrypted communication with module
    
    return true;  // Implementation similar to app-pack pattern
}
```

#### **3.2 Dual-Channel Encryption**

**Goal**: Pack controller manages two encrypted channels

```cpp
// Pack controller manages two key sets
enum CommunicationChannel {
    CHANNEL_APP = 0,      // App ↔ Pack Controller
    CHANNEL_MODULE = 1    // Pack Controller ↔ Modules
};

bool SendEncryptedMessage(CommunicationChannel channel, uint32_t baseId, 
                         const uint8_t* data, size_t len) {
    PackControllerCrypto crypto;
    if (!LoadAppKeys(&crypto)) return false;
    
    uint8_t* key = (channel == CHANNEL_APP) ? 
                   (uint8_t*)crypto.combinedAppKey : 
                   (uint8_t*)crypto.combinedModuleKey;
                   
    // Use appropriate key for encryption
    return EncryptAndSendCAN(key, baseId, data, len);
}
```

### **Phase 4: Testing and Validation (Week 4)**

#### **4.1 Basic Function Tests**

```cpp
// Unit tests for key functions
void TestKeyCombination() {
    char keyHalf1[65] = "1234567890123456789012345678901234567890123456789012345678901234";
    char keyHalf2[65] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789ab";
    uint8_t combinedKey[32];
    
    assert(CombineKeyHalves(keyHalf1, keyHalf2, combinedKey));
    printf("Key combination test: PASSED\n");
}

void TestEncryptionDecryption() {
    uint8_t key[32] = {0x01, 0x02, /* ... fill with test key ... */};
    uint8_t plaintext[] = {0xAA, 0xBB, 0xCC, 0xDD};
    uint8_t decrypted[8];
    size_t decryptedLen;
    
    EncryptedMessage encrypted;
    assert(EncryptCANData(plaintext, 4, key, &encrypted));
    assert(DecryptCANData(&encrypted, key, decrypted, &decryptedLen));
    assert(memcmp(plaintext, decrypted, 4) == 0);
    printf("Encryption test: PASSED\n");
}
```

#### **4.2 Integration Tests**

```cpp
// End-to-end communication test
void TestEncryptedCANCommunication() {
    // 1. Initialize pack controller with test keys
    // 2. Send encrypted message from app
    // 3. Verify pack controller receives and decrypts correctly
    // 4. Send encrypted response back to app
    // 5. Verify app receives and decrypts correctly
    printf("E2E encryption test: %s\n", success ? "PASSED" : "FAILED");
}
```

## Success Criteria

### **Minimum Viable Encryption (End of Week 4)**

- ✅ **API Integration**: App can create LCT relationships via API-Bridge
- ✅ **Key Distribution**: App sends device key halves to pack controller
- ✅ **Key Storage**: Pack controller stores keys securely in flash
- ✅ **Key Combination**: Device key halves combine into AES-256 keys
- ✅ **Basic Encryption**: CAN messages encrypt/decrypt successfully
- ✅ **CAN Integration**: Bit 17 correctly signals encrypted messages
- ✅ **Dual Channel**: Pack controller ↔ App and Pack controller ↔ Module encryption
- ✅ **Testing**: All basic functions validated

### **Deferred Features (Future Phases)**

- ⏳ **HKDF Key Derivation**: Upgrade from SHA256 to HKDF-SHA256
- ⏳ **AES-GCM**: Upgrade from CBC to GCM for authenticated encryption
- ⏳ **Key Rotation**: Automatic key refresh mechanisms
- ⏳ **T3/V3 Integration**: Trust-based encryption strength
- ⏳ **ATP/ADP Tracking**: Energy flow for crypto operations
- ⏳ **MRH Optimization**: Context-aware encryption
- ⏳ **Performance Optimization**: Hardware acceleration, timing optimization

## Implementation Notes

### **Security Considerations**
- **IV Management**: Use proper random IV generation
- **Memory Clearing**: Clear sensitive data from memory after use
- **Flash Protection**: Use flash encryption if available on hardware
- **Error Handling**: Fail securely on crypto errors

### **Performance Considerations**
- **CAN Timing**: Encryption must not violate CAN timing requirements
- **Memory Usage**: Optimize for embedded system constraints
- **Power Consumption**: Consider crypto overhead on battery systems

### **Development Tips**
- **Start Simple**: Get basic encryption working before optimizing
- **Test Incrementally**: Validate each component before integration
- **Log Everything**: Comprehensive logging for debugging crypto issues
- **Plan for Upgrades**: Structure code for easy algorithm upgrades

## Conclusion

This plan provides a **practical, step-by-step approach** to implement basic encryption in the Modbatt system. By focusing on core functionality first and deferring WEB4 sophistication, you can achieve a **working encrypted system** within one month.

The foundation will be solid enough to support future WEB4 enhancements while providing immediate security benefits for the battery management system.

**Start with Week 1 and build incrementally. Each week should produce a working milestone!**