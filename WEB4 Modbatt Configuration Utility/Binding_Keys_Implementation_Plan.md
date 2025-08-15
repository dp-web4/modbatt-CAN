# Binding Keys Implementation Plan

## Document Information

- **Version**: 1.0.0
- **Date**: August 2025
- **Status**: Implementation Guide
- **Author**: System Implementation Team

## Overview

This document provides a practical, step-by-step implementation plan for securing CAN key distribution using binding keys. The plan accounts for the constraint that only the Windows Modbatt app can communicate with the API-Bridge.

## System Architecture Constraints

### Communication Flow
```
API-Bridge ←→ Modbatt Windows App ←→ Pack Controller ←→ Battery Module
     ↑              ↑                    ↑               ↑
  (HTTP/Web)    (CAN Bus)           (CAN Bus)        (CAN Bus)
```

**Key Constraint**: Only the Modbatt Windows app can talk to API-Bridge. Devices cannot directly register themselves.

## Binding Key Architecture

### Key Types Required

```
Modbatt App:
├── App Private Key (32 bytes) - Never shared
├── App Public Key (32 bytes) - Shared with API-Bridge
├── Pack Controller Public Key (32 bytes) - Received from device
└── Battery Module Public Key (32 bytes) - Received from device

Pack Controller:
├── Private Key (32 bytes) - Never shared, stored in flash
├── Public Key (32 bytes) - Sent to app during setup
├── App Public Key (32 bytes) - Received from app
└── Battery Module Public Key (32 bytes) - Received from app

Battery Module:
├── Private Key (32 bytes) - Never shared, stored in flash
├── Public Key (32 bytes) - Sent to pack controller
├── Pack Controller Public Key (32 bytes) - Received from pack controller
└── App Public Key (32 bytes) - Received from pack controller
```

## Implementation Phases

### Phase 1: Key Generation and Storage

#### 1.1 Device Firmware Updates

**Pack Controller Firmware**:
```cpp
// Add to firmware - generate once during first boot
typedef struct {
    uint8_t device_private_key[32];     // Never transmitted
    uint8_t device_public_key[32];      // Sent to app
    uint8_t app_public_key[32];         // Received from app
    uint8_t module_public_key[32];      // Received from app
    bool keys_initialized;
} binding_keys_t;

// Store in flash memory
binding_keys_t binding_keys __attribute__((section(".flash_storage")));

// Generate keys on first boot
void initialize_binding_keys() {
    if (!binding_keys.keys_initialized) {
        generate_keypair(binding_keys.device_private_key, 
                        binding_keys.device_public_key);
        binding_keys.keys_initialized = true;
        save_to_flash(&binding_keys);
    }
}
```

**Battery Module Firmware** (same structure):
```cpp
// Identical structure and initialization as pack controller
binding_keys_t binding_keys __attribute__((section(".flash_storage")));
```

#### 1.2 Modbatt App Updates

**Key Storage in Windows App**:
```cpp
// Add to app configuration
struct AppBindingKeys {
    uint8_t app_private_key[32];        // App's private key
    uint8_t app_public_key[32];         // App's public key
    uint8_t pack_controller_public_key[32];  // From pack controller
    uint8_t battery_module_public_key[32];   // From battery module
    bool keys_initialized;
};

// Store in Windows registry or config file
AppBindingKeys app_keys;
```

### Phase 2: Binding Process Implementation

#### 2.1 Device Key Exchange Protocol

**Step 1: App ↔ Pack Controller Key Exchange**
```cpp
// App sends request to pack controller
CAN_Message request;
request.id = BINDING_KEY_REQUEST;
request.data[0] = 0x01; // Request public key
SendCANMessage(request);

// Pack controller responds with public key
CAN_Message response;
response.id = BINDING_KEY_RESPONSE;
response.data[0] = 0x01; // Public key response
memcpy(&response.data[1], binding_keys.device_public_key, 32);
// Send in multiple CAN frames (32 bytes = 4 frames)

// App stores pack controller public key
memcpy(app_keys.pack_controller_public_key, received_key, 32);
```

**Step 2: Pack Controller ↔ Battery Module Key Exchange**
```cpp
// Pack controller requests module public key
CAN_Message request;
request.id = BINDING_KEY_REQUEST;
request.data[0] = 0x01; // Request public key
SendCANMessage(request);

// Battery module responds (same format as above)
// Pack controller stores module public key
memcpy(binding_keys.module_public_key, received_key, 32);
```

**Step 3: App Distributes Public Keys**
```cpp
// App sends its public key to pack controller
CAN_Message app_key_msg;
app_key_msg.id = BINDING_KEY_DISTRIBUTE;
app_key_msg.data[0] = 0x01; // App public key
memcpy(&app_key_msg.data[1], app_keys.app_public_key, 32);
SendCANMessage(app_key_msg); // Multiple frames

// App sends module public key to pack controller
CAN_Message module_key_msg;
module_key_msg.id = BINDING_KEY_DISTRIBUTE;
module_key_msg.data[0] = 0x02; // Module public key
memcpy(&module_key_msg.data[1], app_keys.battery_module_public_key, 32);
SendCANMessage(module_key_msg); // Multiple frames

// Pack controller forwards app public key to module
CAN_Message forward_app_key;
forward_app_key.id = BINDING_KEY_DISTRIBUTE;
forward_app_key.data[0] = 0x01; // App public key
memcpy(&forward_app_key.data[1], binding_keys.app_public_key, 32);
SendCANMessage(forward_app_key); // Multiple frames
```

#### 2.2 API-Bridge Registration

**App registers itself and devices with API-Bridge**:
```cpp
// App calls API-Bridge to register itself
POST /api/binding/register-component
{
    "component_type": "windows_app",
    "component_id": "modbatt_app_instance_id",
    "public_key": "base64_encoded_app_public_key",
    "device_info": {
        "host_id": "windows_computer_id",
        "app_version": "1.0.0"
    }
}

// App registers pack controller on its behalf
POST /api/binding/register-component  
{
    "component_type": "pack_controller",
    "component_id": "pack_controller_serial_number",
    "public_key": "base64_encoded_pack_controller_public_key",
    "device_info": {
        "firmware_version": "1.2.3",
        "hardware_revision": "Rev_A"
    }
}

// App registers battery module on its behalf
POST /api/binding/register-component
{
    "component_type": "battery_module", 
    "component_id": "battery_module_serial_number",
    "public_key": "base64_encoded_battery_module_public_key",
    "device_info": {
        "firmware_version": "1.1.0",
        "cell_count": 14
    }
}
```

### Phase 3: Secure Key Distribution Implementation

#### 3.1 Updated CAN Key Distribution

**When API-Bridge provides device_key_half and lct_key_half**:

```cpp
// App receives keys from API-Bridge
struct KeyMaterial {
    uint8_t device_key_half_pack[64];
    uint8_t device_key_half_module[64];
    uint8_t lct_key_half[64];
};

// App encrypts and sends to pack controller
void SendEncryptedKeys() {
    // Encrypt pack controller's keys with its public key
    uint8_t encrypted_data[128]; // Larger for encrypted payload
    encrypt_with_public_key(app_keys.pack_controller_public_key,
                           key_material.device_key_half_pack, 64,
                           encrypted_data);
    
    // Send using existing chunked protocol with encryption flag = 1
    SendChunkedEncryptedMessage(encrypted_data, 128, PACK_CONTROLLER_ID);
    
    // Encrypt module's keys with pack controller's public key
    // (so pack controller can forward them encrypted)
    uint8_t module_encrypted_data[128];
    encrypt_with_public_key(app_keys.battery_module_public_key,
                           key_material.device_key_half_module, 64,
                           module_encrypted_data);
    
    // Send encrypted module keys to pack controller for forwarding
    SendChunkedEncryptedMessage(module_encrypted_data, 128, MODULE_KEYS_FOR_FORWARD);
}
```

#### 3.2 Device Key Reception

**Pack Controller receives and decrypts**:
```cpp
void ReceiveEncryptedKeys(uint8_t* encrypted_data, size_t length) {
    uint8_t decrypted_keys[64];
    
    // Decrypt using private key
    decrypt_with_private_key(binding_keys.device_private_key,
                            encrypted_data, length,
                            decrypted_keys);
    
    // Store device key half
    memcpy(device_key_half, decrypted_keys, 64);
    
    // Now can derive AES-256 keys for high-speed communication
    derive_symmetric_keys(device_key_half);
}
```

**Pack Controller forwards to Battery Module**:
```cpp
void ForwardModuleKeys(uint8_t* encrypted_module_data, size_t length) {
    // Forward encrypted data to module (already encrypted with module's public key)
    SendChunkedEncryptedMessage(encrypted_module_data, length, BATTERY_MODULE_ID);
}
```

## Implementation Details

### Key Generation Algorithm

**Use existing crypto library** (recommend mbedTLS for embedded, Windows CryptoAPI for app):

```cpp
// Example using Curve25519 (recommended for embedded systems)
void generate_keypair(uint8_t private_key[32], uint8_t public_key[32]) {
    // Generate random private key
    generate_random_bytes(private_key, 32);
    
    // Derive public key from private key
    curve25519_derive_public_key(public_key, private_key);
}

// Encryption/Decryption
void encrypt_with_public_key(uint8_t public_key[32], 
                            uint8_t* plaintext, size_t plaintext_len,
                            uint8_t* ciphertext) {
    // Use Curve25519 + ChaCha20-Poly1305 or similar
    curve25519_encrypt(public_key, plaintext, plaintext_len, ciphertext);
}
```

### CAN Message Format Updates

**Extended ID with Encryption Flag = 1**:
```
Bit Position:  28 27 26 25 24 23 22 21 20 19 18 17 16 15 14 13 12 11 10  9  8  7  6  5  4  3  2  1  0
Field:         [      Standard 11-bit CAN ID      ] 1 A [Current Chunk] [Total Chunks] [    Device ID    ]
                                                    ↑
                                               Encryption Flag = 1
```

**Payload Format for Encrypted Messages**:
```
CAN Frame Payload (8 bytes):
├── Encrypted Data (variable, up to 8 bytes per frame)
└── May require more chunks due to encryption overhead
```

### Error Handling

```cpp
// Key validation
bool validate_received_key(uint8_t* public_key) {
    // Check for all-zeros, all-ones, or other invalid patterns
    // Verify key is on correct curve
    return curve25519_validate_public_key(public_key);
}

// Decryption failure handling
if (!decrypt_success) {
    // Log error
    // Request key retransmission
    // Fall back to physical security mode if available
}
```

## Testing and Validation

### Test Phases

**Phase 1: Key Generation Test**
- Verify unique keys generated per device
- Test key storage and retrieval from flash
- Validate public key derivation

**Phase 2: Key Exchange Test**  
- Test app ↔ pack controller key exchange
- Test pack controller ↔ module key exchange
- Verify all devices have correct public keys

**Phase 3: Encryption Test**
- Test encryption/decryption of test data
- Verify encrypted CAN transmission
- Test key reception and symmetric key derivation

**Phase 4: Integration Test**
- Complete end-to-end secure key distribution
- Performance testing with real CAN timing
- Security validation and penetration testing

## Security Properties

### What This Achieves

✅ **Confidentiality**: Device keys encrypted during transmission
✅ **Authentication**: Only devices with correct private keys can decrypt
✅ **Integrity**: Encryption provides tamper detection
✅ **Non-repudiation**: Public key cryptography provides sender verification

### Remaining Considerations

⚠️ **Binding Key Distribution**: Initial exchange of public keys still unencrypted
- Mitigation: Physical security during initial setup
- Future: Factory pre-programming of public keys

⚠️ **Key Storage Security**: Private keys stored in flash memory
- Mitigation: Use secure flash regions if available
- Future: Hardware security modules (HSM)

## Conclusion

This implementation plan provides a practical path to secure CAN key distribution using binding keys while working within the constraint that only the Windows app can communicate with API-Bridge. 

The approach:
1. **Minimizes changes** to existing architecture
2. **Uses proven cryptographic algorithms** 
3. **Provides strong security** for key transmission
4. **Maintains performance** by switching to symmetric encryption after setup

**Next Steps**: Begin with Phase 1 implementation and testing with hardcoded keys for proof of concept.

---

**Document Status**: Ready for Implementation
**Implementation Priority**: High - Required for secure production deployment
**Dependencies**: Crypto library integration, firmware updates, app modifications