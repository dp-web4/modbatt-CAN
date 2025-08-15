# Secure CAN Key Distribution - Implementation Roadmap

## Document Information

- **Version**: 1.0.0
- **Date**: August 2025
- **Status**: Project Roadmap
- **Author**: Implementation Team

## Project Status Summary

### ✅ **Research Complete**
- Security architecture analyzed and solution identified
- Compatible crypto libraries found for all platforms
- Documentation gaps identified and corrected
- Memory feasibility confirmed for ATmega328p

### 🎯 **Implementation Goal**
Implement secure CAN key distribution using binding keys to encrypt device_key_half and lct_key_half transmission, enabling the existing AES-256-GCM symmetric encryption system.

## Phase 1: Foundation & Proof of Concept (2-3 weeks)

### 1.1 Crypto Library Integration

**ATmega328p (Battery Module)**:
```
□ Install Arduino Cryptography Library
□ Install micro-ecc library  
□ Create basic test sketch:
  - Generate ECC-P256 key pair
  - Test AES-256-GCM encrypt/decrypt
  - Test HKDF-SHA256 key derivation
  - Measure flash/RAM usage
□ Verify performance is acceptable
```

**STM32 (Pack Controller)**:
```
□ Integrate mbedTLS or similar
□ Create test program:
  - Generate ECC-P256 key pair
  - Test AES-256-GCM encrypt/decrypt  
  - Test HKDF-SHA256 key derivation
□ Verify compatibility with ATmega
```

**Windows App (Modbatt.exe)**:
```
□ Integrate Windows CryptoAPI or OpenSSL
□ Create test functions:
  - Generate ECC-P256 key pair
  - Test AES-256-GCM encrypt/decrypt
  - Test HKDF-SHA256 key derivation
□ Verify same algorithms work across platforms
```

### 1.2 Cross-Platform Compatibility Test

```
□ Generate key pairs on all three platforms
□ Test encryption: Platform A → Platform B
□ Test signatures: Platform A signs → Platform B verifies
□ Confirm identical algorithms produce compatible results
□ Document any platform-specific issues
```

### 1.3 Key Storage Implementation

**ATmega328p**:
```
□ Implement EEPROM key storage functions
□ Add key generation on first boot
□ Add key validation/corruption detection
□ Test key persistence across power cycles
```

**STM32**:
```
□ Implement flash memory key storage
□ Add key generation on first boot
□ Add key validation/corruption detection
□ Test key persistence
```

**Windows App**:
```
□ Implement registry/config file storage
□ Add key generation on first run
□ Add backup/restore functionality
□ Test key persistence
```

## Phase 2: CAN Protocol Implementation (3-4 weeks)

### 2.1 Update CAN Extended ID Chunked Protocol

```
□ Update CAN_Extended_ID_Chunked_Protocol.md:
  - Change encryption flag from 0 to 1
  - Document binding key encryption
  - Remove "keys cannot be encrypted with themselves"
  - Add encryption overhead considerations
  
□ Define new CAN message types:
  - BINDING_KEY_REQUEST
  - BINDING_KEY_RESPONSE  
  - BINDING_KEY_DISTRIBUTE
  - ENCRYPTED_KEY_CHUNKS
```

### 2.2 Binding Key Exchange Protocol

**ATmega328p (Battery Module)**:
```
□ Implement key request/response handlers
□ Add public key transmission (multi-frame)
□ Add public key reception and storage
□ Test with pack controller
```

**STM32 (Pack Controller)**:
```
□ Implement key exchange with app
□ Implement key exchange with battery module
□ Add key forwarding capability
□ Add public key storage for multiple devices
```

**Windows App**:
```
□ Implement CAN key collection from devices
□ Add device registration with API-Bridge
□ Implement key distribution to devices
□ Add binding coordination logic
```

### 2.3 Encrypted Key Distribution

```
□ Implement chunked encrypted transmission
□ Add encryption overhead handling
□ Implement decryption and key reconstruction
□ Add error handling and retransmission
□ Test complete key distribution workflow
```

## Phase 3: Integration & Testing (2-3 weeks)

### 3.1 API-Bridge Integration

```
□ Modify app to register devices with API-Bridge
□ Test LCT creation with device public keys
□ Verify device_key_half and lct_key_half reception
□ Test encrypted key distribution end-to-end
```

### 3.2 Existing System Integration

```
□ Integrate with existing CAN communication stack
□ Update message handlers for encrypted/unencrypted modes
□ Implement graceful fallback mechanisms
□ Test with existing pack controller and module firmware
```

### 3.3 Security Validation

```
□ Test CAN bus sniffing resistance
□ Verify key material never appears in plaintext
□ Test replay attack prevention
□ Validate device authentication
□ Performance testing under load
```

## Phase 4: Production Hardening (2-3 weeks)

### 4.1 Error Handling & Recovery

```
□ Implement key corruption detection
□ Add key regeneration capabilities
□ Implement secure key rotation
□ Add diagnostic and logging capabilities
```

### 4.2 Security Hardening

```
□ Add secure random number generation
□ Implement anti-tampering measures
□ Add secure boot verification (if possible)
□ Code obfuscation for key handling
```

### 4.3 Documentation & Training

```
□ Update all technical documentation
□ Create installation/setup guides
□ Develop troubleshooting procedures
□ Create security audit documentation
```

## Development Environment Setup

### Required Tools

**ATmega328p Development**:
```
□ Arduino IDE or PlatformIO
□ AVR-GCC toolchain
□ micro-ecc library
□ Arduino Cryptography Library
□ Logic analyzer or CAN sniffer for testing
```

**STM32 Development**:
```
□ STM32CubeIDE or similar
□ ARM-GCC toolchain  
□ mbedTLS library
□ STM32 HAL libraries
□ CAN interface for testing
```

**Windows Development**:
```
□ C++ Builder (existing)
□ Windows SDK
□ OpenSSL or CryptoAPI headers
□ CAN interface libraries (existing)
```

### Test Infrastructure

```
□ CAN bus test setup with all three devices
□ Logic analyzer for protocol verification
□ Network sniffing tools for security testing
□ Automated test scripts
□ Performance measurement tools
```

## Critical Path Dependencies

### Immediate Blockers
1. **Crypto library testing** - Must verify all algorithms work correctly
2. **Memory constraints** - Confirm ATmega328p can handle full implementation
3. **CAN timing** - Ensure encrypted operations don't break real-time requirements

### Integration Dependencies
1. **API-Bridge access** - Need test environment for device registration
2. **Existing firmware** - Must not break current pack controller/module functionality
3. **CAN protocol compatibility** - Maintain backward compatibility where possible

## Risk Mitigation

### High Risk Items
```
□ ATmega328p memory exhaustion
  - Mitigation: Implement minimal configurations first
  
□ Performance degradation
  - Mitigation: Profile all crypto operations early
  
□ API-Bridge integration issues  
  - Mitigation: Create mock API-Bridge for testing
```

### Medium Risk Items
```
□ Cross-platform compatibility issues
  - Mitigation: Extensive cross-platform testing
  
□ Key storage corruption
  - Mitigation: Implement redundant storage and validation
```

## Success Criteria

### Phase 1 Success
- [ ] All crypto libraries working on all platforms
- [ ] Cross-platform key exchange successful
- [ ] Memory usage within acceptable limits

### Phase 2 Success  
- [ ] Encrypted CAN key distribution working
- [ ] Binding key exchange protocol functional
- [ ] No regression in existing functionality

### Phase 3 Success
- [ ] Complete API-Bridge integration
- [ ] Security validation passes
- [ ] Performance meets requirements

### Phase 4 Success
- [ ] Production-ready code
- [ ] Security audit complete
- [ ] Documentation complete

## Timeline Summary

```
Week 1-3:   Phase 1 - Foundation & Proof of Concept
Week 4-7:   Phase 2 - CAN Protocol Implementation  
Week 8-10:  Phase 3 - Integration & Testing
Week 11-13: Phase 4 - Production Hardening

Total: ~3 months for complete implementation
```

## Team Assignments

### Embedded Development (ATmega328p + STM32)
- [ ] Crypto library integration
- [ ] Key storage implementation
- [ ] CAN protocol updates
- [ ] Performance optimization

### Windows Application Development
- [ ] Crypto library integration
- [ ] API-Bridge integration
- [ ] Key coordination logic
- [ ] User interface updates

### Testing & Validation
- [ ] Security testing
- [ ] Performance testing  
- [ ] Integration testing
- [ ] Documentation

## Next Immediate Actions (This Week)

### High Priority
1. **Download and test crypto libraries** on ATmega328p
2. **Create simple ECC key generation test** 
3. **Measure actual memory usage** vs estimates
4. **Test basic AES-256-GCM** encryption/decryption

### Medium Priority  
1. **Set up cross-platform development environment**
2. **Create unified crypto API header** for all platforms
3. **Design key storage data structures**
4. **Plan CAN message format updates**

### Documentation Updates
1. **Update CAN Extended ID Chunked Protocol** document
2. **Update Encryption Implementation Proposal** 
3. **Create crypto library integration guide**

---

**Document Status**: Ready for Implementation Start
**Implementation Priority**: Critical - Foundational security requirement
**Dependencies**: Development environment setup, library downloads
**Next Review**: After Phase 1 completion (3 weeks)