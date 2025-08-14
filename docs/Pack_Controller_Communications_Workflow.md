# Pack Controller Communications Workflow

## Overview

This document outlines the complete communication workflow in the Modbatt system using the three-tier transaction protocol: Binding, Pairing, and Validation.

## System Architecture

### Physical Network Topology
```
Modbatt App ←→ USB-CAN Adapter ←→ Pack Controller ←→ Battery Module(s)
                                    (CAN 1)         (CAN 2)
```

### Logical Protocol Layers
```
Binding Layer (Permanent Identity) → Pairing Layer (Context Trust) → Validation Layer (Event Attestation)
```

### Pack Controller Role
The pack controller acts as a **CAN bridge/gateway** with:
- **CAN Interface 1**: Communicates with Modbatt app (via USB-CAN adapter)
- **CAN Interface 2**: Communicates with battery modules
- **Bridge Function**: Routes messages between networks with protocol translation
- **Security Gateway**: Manages different encryption levels per context

## CAN Message Encryption Signaling

### Extended CAN ID Structure (29-bit)
- **Bit 17**: General encryption flag (0 = clear, 1 = encrypted)
- **Bits 18-19**: Encryption level when Bit 17 = 1
  - `00` = Exchange key (lightweight symmetric)
  - `01` = Pairing key (64-byte symmetric)
  - `10` = Binding key (asymmetric)
  - `11` = Reserved for future use
- **Remaining bits**: Device/message identifiers (always unencrypted)

### Encryption Hierarchy
1. **Binding Keys** (Asymmetric) - Validation events, identity verification
2. **Pairing Keys** (Symmetric 64-byte) - Command/control, configuration
3. **Exchange Keys** (Lightweight) - Routine telemetry, status updates
4. **Clear** - Public announcements, discovery

## System Initial State

### Factory Default State
**Pack Controller:**
- No cryptographic keys stored
- No blockchain registration
- Hardware serial number only
- CAN communication unencrypted

**Battery Module:**
- No cryptographic keys stored
- Module serial number only
- Awaiting binding transaction

**Modbatt App:**
- Host ID generated (MAC address, CPU ID, or UUID)
- No blockchain registration initially
- No keys stored

## Phase 1: Binding Transactions

### 1.1 Device Discovery
```
Modbatt App → Pack Controller (CAN 1, Unencrypted)
├── Announce (Level 1 Validation)
├── Query: Device serial number
├── Query: Binding status
└── Response: Device information + access data
```

### 1.2 Binding Establishment

**Step 1: Authority Validation**
Auth controller (API-Bridge) validates binding authority through external means

**Step 2: Access Data Collection**
```json
{
  "pack_controller": {
    "device_id": "PC-12345",
    "protocol": "can",
    "can_interface": 1,
    "endpoint": "0x100"  // CAN base address
  },
  "app": {
    "host_id": "APP-HOST-67890",
    "protocol": "http",
    "endpoint": "http://localhost:8080/api"
  }
}
```

**Step 3: Key Generation**
```
Auth Controller → Pack Controller: POST /api/binding/generate-keys
├── Request: App's access data
└── Response: Pack controller's semi-public key

Auth Controller → App: POST /api/binding/generate-keys
├── Request: Pack controller's access data
└── Response: App's semi-public key
```

**Step 4: Semi-Public Key Distribution**
```
Auth Controller → Pack Controller: App's semi-public key
Auth Controller → App: Pack controller's semi-public key
Both store: Partner's access data + semi-public key
```

**Step 5: Binding Validation** (Level 7)
```json
{
  "type": "binding_validation",
  "binding_id": "uuid",
  "validation_type": "establishment",
  "entities": ["pack_controller", "app"],
  "blockchain_proof": "tx_hash"
}
```

### 1.3 Module Binding
Similar process for Pack Controller ↔ Battery Module binding

## Phase 2: Pairing Transactions

### 2.1 Operational Context Pairing

**Context Examples:**
- `battery_management`: Normal charge/discharge operations
- `diagnostics`: Troubleshooting and monitoring
- `firmware_update`: Software updates
- `emergency`: Safety-critical operations

### 2.2 Pairing Certificate Generation

**Auth Controller creates certificate with:**
```json
{
  "pairing_certificate": {
    "clear_metadata": {
      "pairing_id": "uuid",
      "context": "battery_management",
      "entities": [
        {
          "entity_id": "pack_controller",
          "access_data": {...}
        },
        {
          "entity_id": "battery_module_1",
          "access_data": {...}
        }
      ],
      "timing": {
        "expires_at": "2024-12-31T23:59:59Z"
      },
      "encryption_policy": {
        "critical_required": ["command", "control"],
        "unencrypted_allowed": ["status", "telemetry"]
      }
    },
    "encrypted_sections": {
      "pack_controller": "encrypted_with_pc_semi_public_key",
      "battery_module": "encrypted_with_bm_semi_public_key",
      "handshake_validation": "encrypted_with_full_pairing_key"
    }
  }
}
```

### 2.3 Key Exchange and Handshake

**Step 1: Certificate Distribution**
```
App → Pack Controller (CAN 1, Binding key encrypted, Bits 17=1, 18-19=10)
├── Pairing certificate
└── Acknowledgment required
```

**Step 2: Key Half Exchange**
```
Pack Controller ↔ Battery Module (CAN 2, Exchange key)
├── Exchange key halves using exchange key from certificate
├── Reconstruct full pairing key
└── Decrypt handshake validation section
```

**Step 3: Handshake Completion** (Level 6 Validation)
```
Pack Controller + Battery Module → Auth Controller
├── Pairing ID
├── Validation nonce (from handshake section)
├── Both entity signatures
└── Completion timestamp
```

## Phase 3: Operational Communication

### 3.1 Message Classification and Encryption

**Critical Operations** (Pairing Key Required):
```
Pack Controller ↔ Battery Module (CAN 2, Bits 17=1, 18-19=01)
├── Charge/discharge commands
├── Safety limit configuration
├── Cell balancing commands
└── Emergency shutdown
```

**Routine Monitoring** (Exchange Key or Clear):
```
Battery Module → Pack Controller (CAN 2, Bits 17=1, 18-19=00 or 17=0)
├── Voltage readings (every 100ms)
├── Temperature data (every 500ms)
├── Current measurements (every 100ms)
└── Status heartbeat (every 1000ms)
```

**Validation Events** (Binding Key):
```
Any Entity → Network (Bits 17=1, 18-19=10)
├── State change announcements
├── Anomaly detection alerts
├── Audit trail entries
└── Escalation requests
```

### 3.2 Validation Hierarchy in Operation

**Level 1: Announce** - Battery module broadcasts availability
**Level 2: Witness** - Pack controller witnesses module state change
**Level 3: Signed Handshake** - Initial contact without pairing
**Level 4: Paired Handshake** - Session re-establishment
**Level 5: Paired Command** - Operational commands
**Level 6: Pairing Validation** - Auth controller involved
**Level 7: Binding Validation** - Identity verification

### 3.3 Escalation Example
```
Normal telemetry (Level 1) → Anomaly detected → 
Escalation request (target_level: 5) → 
Paired command for diagnostic mode → 
Further anomaly → Escalation (target_level: 7) → 
Full binding validation with auth controller
```

## Security State Machine

### Pack Controller States
```
State 0: Factory Default
    ↓ [Binding Transaction]
State 1: Bound (Semi-public keys established)
    ↓ [Pairing Transaction]
State 2: Paired (Context-specific keys active)
    ↓ [Operational]
State 3: Active (All encryption levels available)
    ↓ [Validation Events]
State 4: Validated (Continuous attestation)
```

### Communication Security Levels
```
Level 0: Discovery (No encryption, public announce)
Level 1: Exchange (Lightweight symmetric, routine data)
Level 2: Paired (64-byte symmetric, commands)
Level 3: Bound (Asymmetric, validation events)
```

## Error Handling and Recovery

### Key Loss Recovery
1. Device announces key loss (Level 1 validation)
2. Escalation to binding validation (Level 7)
3. Auth controller initiates re-binding
4. New semi-public keys generated
5. Existing pairings invalidated
6. Re-pairing required for operational contexts

### Pairing Expiration
1. Monitor pairing certificate expiration
2. Request new pairing before expiration
3. Seamless key transition using new certificate
4. Old pairing keys securely deleted

### Anomaly Response
1. Detect anomalous behavior
2. Generate escalation request
3. Increase validation level dynamically
4. Log all escalation events
5. Potential pairing revocation if severe

## Implementation Checkpoints

### Binding Implementation
- [ ] `/api/binding/generate-keys` endpoint on all devices
- [ ] Semi-public key generation and storage
- [ ] Access data management
- [ ] Binding validation (Level 7) support

### Pairing Implementation
- [ ] Certificate parsing and validation
- [ ] Three-section decryption logic
- [ ] Key half exchange protocol
- [ ] Handshake completion validation

### Validation Implementation
- [ ] All 7 validation levels
- [ ] Escalation request handling
- [ ] Signature generation/verification
- [ ] Audit trail logging

### CAN Integration
- [ ] Extended ID encryption signaling
- [ ] Multi-level encryption support
- [ ] Protocol-aware message routing
- [ ] Performance optimization

## Security Audit Checklist

- [ ] No private keys ever transmitted
- [ ] Semi-public keys only to authorized parties
- [ ] Pairing keys require both halves
- [ ] Auth controller maintains zero-knowledge
- [ ] All validation events logged
- [ ] Escalation mechanism tested
- [ ] Key storage secured
- [ ] Communication patterns analyzed