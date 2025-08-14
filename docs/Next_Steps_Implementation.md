# Modbatt System Implementation - Next Steps

## Current State

- Device registration with blockchain completed
- Device and LCT key halves received from API-bridge interface
- Pairing protocol architecture documented

## Implementation Roadmap

### Phase 1: Key Reconstruction and Pairing

#### 1.1 Key Storage and Management
**Pack Controller Tasks:**
- Store 64-byte device key half from API-Bridge LCT creation
- Store 64-byte LCT key half from API-Bridge LCT creation
- Store app's public key for encrypted responses
- Implement secure flash storage mechanism

**Battery Module Tasks:**
- Receive and store 64-byte device key half from pack controller
- Implement secure key storage mechanism
- Prepare for encrypted CAN communication

#### 1.2 Inter-Device Key Exchange
- **CAN Bus Communication**: Exchange device key halves over CAN bus
- **Key Combination**: Implement algorithm to combine key halves into full encryption key
- **Key Validation**: Verify combined key integrity and format
- **Encryption Setup**: Initialize encryption/decryption using combined key

#### 1.3 API-Bridge Integration
- **Component Registration**: Implement calls to POST /api/v1/components/register
- **LCT Creation**: Implement calls to POST /api/v1/lct/create
- **Key Extraction**: Parse device_key_half and lct_key_half from API responses
- **Error Handling**: Handle API failures and retry logic

### Phase 2: Encrypted CAN Communication

#### 2.1 Device-to-Device Encryption (Pack Controller ↔ Battery Module)
- **Encryption Layer**: Implement message encryption/decryption over CAN bus
- **Message Format**: Define encrypted CAN frame structure
- **Error Handling**: Implement decryption failure recovery
- **Performance**: Optimize for real-time CAN communication requirements

#### 2.2 Message Types and Protocols
- **Operational Data**: Battery status, charging commands, telemetry
- **Security Messages**: Key rotation, authentication challenges
- **Diagnostic Data**: Error codes, system health monitoring

### Phase 3: Encryption Implementation

#### 3.1 Core Cryptographic Components
- **HKDF Key Derivation**: Implement HKDF-SHA256 for combining device key halves
- **AES-256-GCM Encryption**: Implement authenticated encryption for CAN messages
- **Key Combination Algorithm**: Deterministic method to combine 64-byte key halves
- **IV/Nonce Management**: Counter-based nonce generation for GCM mode

#### 3.2 CAN Message Format
- **Encryption Flag**: Use bit 17 in CAN ID to signal encrypted payload
- **Message Structure**: IV(12) + Encrypted_Data + Auth_Tag(16) format
- **Frame Handling**: Integrate encryption with existing CAN communication stack
- **Error Recovery**: Handle encryption/decryption failures gracefully

#### 3.3 Key Distribution Protocol
- **App-Generated Components**: Salt values, IV seeds, crypto parameters
- **Key Package Format**: Structured data containing all crypto material
- **Secure Transfer**: Initial unencrypted transfer followed by encrypted communication
- **Validation**: Cryptographic verification of received key packages

### Phase 4: Security and Validation

#### 4.1 Security Hardening
- **Key Storage**: Implement secure key storage on devices
- **Memory Protection**: Ensure keys are cleared from memory when not needed
- **Tamper Detection**: Implement mechanisms to detect key compromise
- **Audit Logging**: Log all encryption/decryption events

#### 4.2 Testing and Validation
- **Unit Tests**: Test encryption/decryption functions
- **Integration Tests**: Test full communication flow
- **Security Tests**: Attempt to break encryption or extract keys
- **Performance Tests**: Validate real-time communication requirements

## Implementation Decisions Required

### Cryptographic Decisions (Based on Encryption Proposal)
1. **Key Derivation**: HKDF-SHA256 with app-generated salt values ✓
2. **Encryption Algorithm**: AES-256-GCM for authenticated encryption ✓
3. **Key Combination**: Concatenate key halves + salt, then HKDF ✓
4. **Message Format**: IV(12) + Encrypted_Data + Auth_Tag(16) ✓
5. **Nonce Management**: Counter-based with device-specific base ✓

### Implementation Decisions
1. **Host ID Generation**: MAC address, CPU ID, or generated UUID?
2. **Key Storage Format**: Flash memory layout and protection mechanisms
3. **Performance Optimization**: Hardware acceleration and library selection
4. **Error Handling**: Fallback modes and recovery procedures

### Operational Decisions
1. **Key Rotation Schedule**: How often to regenerate device key halves?
2. **Diagnostic Access**: Maintain unencrypted diagnostic channels?
3. **Update Mechanism**: How to update crypto parameters over CAN?
4. **Monitoring**: What crypto events to log for security auditing?

## Priority Implementation Order

1. **High Priority**: API-Bridge integration and key storage
2. **High Priority**: HKDF key derivation and AES-256-GCM implementation
3. **High Priority**: CAN message encryption with bit 17 signaling
4. **Medium Priority**: Key distribution protocol and validation
5. **Medium Priority**: Error handling and recovery mechanisms
6. **Low Priority**: Key rotation and advanced security features

## Success Criteria

- [ ] App successfully registers as blockchain component with Host ID
- [ ] Pack controller and battery modules register and receive device key halves
- [ ] HKDF key derivation produces consistent encryption keys from key halves
- [ ] AES-256-GCM encryption/decryption works on CAN messages
- [ ] Bit 17 encryption signaling functions correctly in CAN frames
- [ ] All device pairs establish encrypted communication channels
- [ ] System meets real-time performance requirements with encryption overhead
- [ ] Security validation passes with no key material exposure

## Risk Mitigation

- **Key Loss**: Implement key backup and recovery procedures
- **Communication Failure**: Design robust retry and fallback mechanisms
- **Performance Impact**: Profile encryption overhead on embedded systems
- **Security Vulnerabilities**: Regular security audits and penetration testing