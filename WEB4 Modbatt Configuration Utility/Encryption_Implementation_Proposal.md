# Modbatt System Encryption Implementation Proposal

## Document Information

- **Version**: 1.0.0
- **Date**: August 2025
- **Status**: Proposal for Review
- **Author**: Modbatt Development Team

## Overview

This document proposes a practical encryption implementation for the Modbatt system that works within the constraints of the existing API-Bridge while providing secure communication between the app, pack controller, and battery modules.

## Current Constraints

### API-Bridge Limitations
- **Fixed Key Format**: API-Bridge provides only 64-byte key halves via LCT creation
- **No Key Type Specification**: Key format and intended usage not documented
- **Immutable System**: Cannot modify API-Bridge implementation

### Available Resources
- `device_key_half` (64 bytes) - for secure CAN communication
- `lct_key_half` (64 bytes) - for Web4 operations
- Component registration and LCT relationship management

## Proposed Encryption Architecture

### 1. Hybrid Approach: API-Bridge Keys + App-Generated Components

#### API-Bridge Responsibilities
- Generate device key halves for secure device-to-device communication
- Provide LCT key halves for Web4 blockchain operations
- Manage component registration and relationships

#### App Responsibilities
- Generate additional cryptographic components not provided by API-Bridge
- Implement key derivation and combination algorithms
- Distribute complete key packages to devices
- Manage encryption parameters and security policies

### 2. Key Derivation Strategy

#### Device Key Combination
```
Input:
├── device_key_half_A (64 bytes from API-Bridge)
├── device_key_half_B (64 bytes from API-Bridge)
├── salt (32 bytes, app-generated)
└── context_info (device IDs, relationship type)

Process:
combined_material = device_key_half_A || device_key_half_B || salt

Output (via HKDF-SHA256):
├── AES-256 Encryption Key (32 bytes)
├── HMAC-SHA256 Authentication Key (32 bytes)
└── Initial IV Seed (16 bytes)
```

#### Key Properties
- **Deterministic**: Same inputs always produce same outputs
- **Unique**: Different device pairs get different keys
- **Forward Secure**: Compromise of one key doesn't affect others
- **Authenticated**: Built-in message authentication

### 3. Encryption Implementation

#### Algorithm Selection
- **Symmetric Encryption**: AES-256-GCM
  - Provides both encryption and authentication
  - Efficient for embedded systems
  - Well-tested and widely supported

#### Message Format
```
CAN Message Structure (with Bit 17 = 1):
├── CAN ID (29 bits)
│   ├── Bit 17: Encryption Flag (1 = encrypted)
│   └── Remaining: Device addressing
├── Data Length (4 bits)
└── Encrypted Payload:
    ├── IV/Nonce (12 bytes for GCM)
    ├── Encrypted Data (variable length)
    └── Authentication Tag (16 bytes)
```

#### Implementation Details
- **IV/Nonce Management**: Counter-based with device-specific base
- **Replay Protection**: Monotonic counters in encrypted payload
- **Key Rotation**: Periodic re-derivation using updated salt values

### 4. Key Distribution Workflow

#### Phase 1: App Registration and Key Generation
```
1. App generates Host ID (unique computer identifier)
2. App → API-Bridge: Register app as component
3. App generates additional crypto components:
   ├── Salt values for key derivation
   ├── Initial IV/nonce values
   └── Crypto parameters (algorithm IDs, key rotation schedule)
```

#### Phase 2: Pack Controller Setup
```
1. App → API-Bridge: Register pack controller
2. App → API-Bridge: Create LCT (app ↔ pack controller)
3. App receives:
   ├── App's device_key_half
   ├── Pack Controller's device_key_half
   └── LCT relationship details
4. App → Pack Controller (unencrypted): Distribute key package:
   ├── Pack Controller's device_key_half
   ├── App's device_key_half
   ├── Salt and IV values
   ├── Crypto parameters
   └── Key derivation instructions
```

#### Phase 3: Battery Module Pairing
```
1. App → API-Bridge: Register battery module
2. App → API-Bridge: Create LCT (pack controller ↔ battery module)
3. App receives module's device key halves
4. App → Pack Controller (encrypted): Deliver module key package
5. Pack Controller → Battery Module (unencrypted): Initial key exchange
6. Both devices derive shared encryption keys
```

### 5. Security Properties

#### Confidentiality
- **AES-256 Encryption**: Strong symmetric encryption for all CAN messages
- **Unique Keys**: Each device pair has independent encryption keys
- **Forward Secrecy**: Key rotation prevents long-term compromise

#### Authenticity
- **GCM Authentication**: Built-in message authentication and integrity
- **Device Identity**: Keys tied to blockchain-registered component IDs
- **Replay Protection**: Monotonic counters prevent message replay

#### Availability
- **Graceful Degradation**: Can fall back to unencrypted mode if keys fail
- **Error Recovery**: Key re-derivation and re-distribution procedures
- **Performance**: Optimized for real-time CAN communication requirements

### 6. Implementation Phases

#### Phase 1: Core Encryption Engine
- [ ] Implement HKDF-SHA256 key derivation
- [ ] Implement AES-256-GCM encryption/decryption
- [ ] Design CAN message format with encryption flag
- [ ] Create key storage structures

#### Phase 2: Key Management
- [ ] Implement app-side key generation (salt, IV, parameters)
- [ ] Create key distribution protocols
- [ ] Implement device key storage in flash
- [ ] Add key validation and integrity checks

#### Phase 3: Communication Integration
- [ ] Integrate encryption with CAN communication stack
- [ ] Implement encrypted message sending/receiving
- [ ] Add error handling and recovery mechanisms
- [ ] Performance optimization for real-time requirements

#### Phase 4: Security Hardening
- [ ] Implement key rotation mechanisms
- [ ] Add replay protection and sequence numbering
- [ ] Security testing and validation
- [ ] Audit logging and monitoring

## Technical Specifications

### Key Sizes and Formats
```
Device Key Half:        64 bytes (from API-Bridge)
LCT Key Half:          64 bytes (from API-Bridge)
App-Generated Salt:    32 bytes (random)
Derived AES Key:       32 bytes (256-bit)
Derived HMAC Key:      32 bytes (256-bit)
GCM IV/Nonce:         12 bytes (96-bit)
GCM Auth Tag:         16 bytes (128-bit)
```

### Performance Requirements
- **Encryption Latency**: < 1ms for 8-byte CAN frame
- **Key Derivation**: < 10ms (one-time per pairing)
- **Memory Usage**: < 1KB RAM for active keys
- **Flash Storage**: < 4KB for stored key material

### Compatibility Requirements
- **CAN 2.0B**: Extended 29-bit identifiers
- **Embedded Systems**: Limited RAM/Flash on battery modules
- **Real-time**: Deterministic encryption timing
- **Cross-platform**: Windows app, embedded pack controller

## Risk Assessment

### High Risk
- **Key Combination Algorithm**: Untested approach, may have weaknesses
- **Performance Impact**: Encryption overhead on real-time CAN communication
- **Key Distribution**: Initial unencrypted key exchange vulnerable

### Medium Risk
- **Implementation Complexity**: Multiple encryption algorithms and key management
- **Flash Storage**: Limited storage space on embedded devices
- **Key Rotation**: Complex coordination across multiple devices

### Low Risk
- **Algorithm Selection**: Well-established cryptographic primitives
- **CAN Integration**: Standard protocol with established encryption practices

## Mitigation Strategies

### Security Mitigations
- **Physical Security**: Protect initial key distribution phase
- **Key Validation**: Cryptographic verification of all received keys
- **Audit Logging**: Record all key operations for security monitoring
- **Regular Updates**: Implement key rotation and algorithm updates

### Performance Mitigations
- **Hardware Acceleration**: Use AES-NI instructions where available
- **Optimized Libraries**: Use proven cryptographic implementations
- **Selective Encryption**: Encrypt only critical message types initially
- **Benchmarking**: Continuous performance monitoring and optimization

## Success Criteria

### Functional Requirements
- [ ] Successful encrypted communication between all device pairs
- [ ] Key distribution completes without errors
- [ ] Performance meets real-time CAN requirements
- [ ] Security validation passes penetration testing

### Security Requirements
- [ ] No key material exposed in unencrypted form after bootstrap
- [ ] Message replay attacks prevented
- [ ] Device impersonation attacks prevented
- [ ] Key compromise limited to affected device pair only

## Conclusion

This proposal provides a practical path forward for implementing encryption in the Modbatt system while working within API-Bridge constraints. The hybrid approach leverages existing API-Bridge capabilities while filling gaps with app-generated components.

**Next Steps:**
1. Review and approve the proposed architecture
2. Begin Phase 1 implementation with core encryption engine
3. Validate key derivation algorithms with test data
4. Prototype CAN message encryption with performance testing

---

**Document Status**: Awaiting review and approval
**Implementation Priority**: High - Required for secure production deployment