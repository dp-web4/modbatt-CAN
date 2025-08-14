# Pack Controller Communications Workflow

## Overview

This document outlines the complete communication workflow in the Modbatt system, including the pack controller's role as a dual-CAN gateway between the Modbatt app and battery modules.

## System Architecture

### Physical Network Topology
```
Modbatt App ←→ USB-CAN Adapter ←→ Pack Controller ←→ Battery Module(s)
                                    (CAN 1)         (CAN 2)
```

### Communication Flow
```
Battery Module → Pack Controller → Modbatt App → API-Bridge → Blockchain
```

### Pack Controller Role
The pack controller acts as a **CAN bridge/gateway** with:
- **CAN Interface 1**: Communicates with Modbatt app (via USB-CAN adapter)
- **CAN Interface 2**: Communicates with battery modules
- **Bridge Function**: Routes messages between the two CAN networks

## CAN Message Encryption Signaling

### CAN ID Structure (29-bit)
- **Bit 17 (MSB of 18-bit extended ID)**: Encryption flag
  - `0` = Unencrypted payload
  - `1` = Encrypted payload
- **Remaining CAN ID**: Contains controller/module ID (always unencrypted)

### Security Zones
- **CAN 1** (App ↔ Pack Controller): Initially unencrypted, then LCT encrypted
- **CAN 2** (Pack Controller ↔ Battery Modules): Initially unencrypted, then device-pairing encrypted

## System Initial State

### Pack Controller Factory Default State
- **Flash Memory**: Empty - no cryptographic keys stored
- **CAN Communication**: Unencrypted on both interfaces (encryption bit = 0)
- **Registration Status**: Unknown/unregistered with blockchain
- **Device Identity**: Hardware serial number or unique identifier only

### Modbatt App Initial State
- **Host ID Generation**: Create unique identifier for the computer running the app
  - Options: MAC address, CPU ID, motherboard serial, or generated UUID
  - Store persistently for consistent identity across app restarts
- **Registration Status**: Unregistered as blockchain component
- **Key Storage**: No keys initially stored

## Bootstrap Workflow

### Phase 1: Initial Discovery and Registration Check

#### 1.1 Device Discovery
```
Modbatt App → Pack Controller (CAN 1, Unencrypted, Bit 17 = 0)
├── Query: Device ID/Serial Number
├── Query: Current registration status
├── Query: Hardware capabilities
└── Response: Device information
```

#### 1.2 Blockchain Registration Check
```
Modbatt App → API-Bridge → Blockchain
├── POST /api/v1/components/register (if unregistered)
├── Response: component_id, lct_id, component_identity
└── Store registration details for pack controller
```

### Phase 2: Initial Key Generation and Bootstrap

#### 2.1 Component Registration Process
When pack controller is **unregistered**:

```
1. Modbatt App → API-Bridge: POST /api/v1/components/register
   ├── Request: creator, component_data, context
   └── Response: component_id, lct_id, component_identity

2. Modbatt App → API-Bridge: POST /api/v1/lct/create  
   ├── Request: creator, component_a, component_b, context
   └── Response: lct_id, device_key_half, lct_key_half, status
```

#### 2.2 Key Generation and Distribution

**Step 1: Register App as Component**
```
Modbatt App → API-Bridge: POST /api/v1/components/register
├── Request: creator, component_data (host_id), context
└── Response: app_component_id, app_lct_id, component_identity
```

**Step 2: Create LCT Relationship (App ↔ Pack Controller)**
```
Modbatt App → API-Bridge: POST /api/v1/lct/create
├── Request: creator, component_a (app_component_id), component_b (pack_controller_id), context
└── Response: lct_id, device_key_half, lct_key_half
```

**API-Bridge/Blockchain Generates:**
- **App's Device Key Half** (64 bytes) - for app ↔ pack controller communication
- **App's LCT Key Half** (64 bytes) - for Web4 operations
- **Pack Controller's Device Key Half** (64 bytes) - for pack controller ↔ app communication  
- **Pack Controller's LCT Key Half** (64 bytes) - for Web4 operations
- **LCT Relationship ID** - links app and pack controller on blockchain

#### 2.3 Initial Key Distribution
```
Modbatt App → Pack Controller (CAN 1, Unencrypted, Bit 17 = 0)
├── Pack Controller's Device Key Half (64 bytes from API-Bridge)
├── Pack Controller's LCT Key Half (64 bytes from API-Bridge)
├── App's Device Key Half (64 bytes for secure CAN communication)
├── Device Certificate/ID (pack_controller_component_id from blockchain)
└── Configuration parameters
```

#### 2.4 Key Storage
**Modbatt App Local Storage:**
```
├── App's Device Key Half (64 bytes)       (for secure CAN communication)
├── App's LCT Key Half (64 bytes)          (for Web4 operations)
├── App Component ID                       (from blockchain registration)
└── Host ID                               (unique computer identifier)
```

**Pack Controller Flash Storage:**
```
├── Pack Controller's Device Key Half (64 bytes)  (for secure CAN communication)
├── Pack Controller's LCT Key Half (64 bytes)     (for Web4 operations)
├── App's Device Key Half (64 bytes)              (for communication with app)
├── Pack Controller Component ID                  (from blockchain registration)
└── Device Configuration                          (operational parameters)
```

### Phase 3: Encrypted Communication Establishment

#### 3.1 Communication Channel Upgrade
After bootstrap keys are stored:
```
All Future Communications:
Modbatt App ←→ Pack Controller (CAN 1, Encrypted, Bit 17 = 1)
├── App combines: App's Device Key Half + Pack Controller's Device Key Half → Shared Key
├── Pack Controller combines: Pack Controller's Device Key Half + App's Device Key Half → Shared Key  
├── Both use identical Shared Key for symmetric encryption/decryption
└── Secure bidirectional communication established
```

**Key Combination Process:**
1. Each side has both device key halves (64 bytes each)
2. Combine halves using deterministic algorithm (e.g., XOR, concatenation, or cryptographic function)
3. Resulting combined key used for symmetric encryption (AES, ChaCha20, etc.)
4. Both sides can encrypt/decrypt messages using the same derived key

## Operational Workflows

### Workflow A: Device Pairing Operations

#### A.1 Battery Module Pairing Request
```
1. Modbatt App → API-Bridge: POST /api/v1/lct/create
   ├── Request: creator, component_a (pack controller), component_b (battery module), context
   └── Response: lct_id, device_key_half, lct_key_half

2. App → Pack Controller: Deliver pairing keys (CAN 1, Encrypted, Bit 17 = 1)
   ├── Device Key Half (64 bytes for pack controller ↔ battery module communication)
   ├── LCT Key Half (64 bytes for operational use)
   └── Battery Module component_id

3. Pack Controller: Store pairing keys in flash
```

#### A.2 Inter-Device Key Exchange
```
1. App creates separate LCT for battery module:
   ├── POST /api/v1/lct/create (for battery module)
   └── Receives: device_key_half, lct_key_half for battery module

2. Pack Controller → Battery Module: Exchange device key halves (CAN 2, Unencrypted, Bit 17 = 0)
   ├── Send pack controller's device key half
   └── Receive battery module's device key half

3. Both devices: Combine device key halves to form shared encryption key
4. Establish: Ongoing encrypted device-device communication (CAN 2, Encrypted, Bit 17 = 1)
```

### Workflow B: Configuration and Monitoring

#### B.1 Configuration Updates
```
Modbatt App → Pack Controller (CAN 1, Encrypted, Bit 17 = 1)
├── Operational parameters
├── Firmware updates
├── Security policy updates
└── Diagnostic commands
```

#### B.2 Status Monitoring
```
Pack Controller → Modbatt App (CAN 1, Encrypted, Bit 17 = 1)
├── Battery status data (bridged from CAN 2)
├── System health metrics
├── Error codes and alerts
└── Pairing status updates
```

#### B.3 Battery Module Communication
```
Pack Controller ←→ Battery Module(s) (CAN 2, Encrypted, Bit 17 = 1)
├── Battery telemetry data
├── Charging commands
├── Safety alerts
└── Diagnostic information
```

### Workflow C: Re-registration and Recovery

#### C.1 Key Recovery Scenario
If pack controller loses keys (flash corruption, factory reset):
```
1. Pack Controller: Revert to unencrypted communication
2. Modbatt App: Detect missing keys during communication attempt
3. App: Query blockchain for existing registration
4. If registered: Re-download and redistribute LCT keys
5. If corrupted: Initiate re-registration process
```

## Security Considerations

### Bootstrap Security
- **Initial vulnerability**: First key distribution is unencrypted
- **Mitigation**: Physical security during initial setup
- **Verification**: Cryptographic verification of received keys

### Ongoing Security
- **All communications encrypted** after bootstrap
- **Key isolation**: LCT keys separate from device pairing keys
- **Tamper detection**: Monitor for key corruption or unauthorized access

### Flash Storage Security
- **Secure storage**: Use hardware security features if available
- **Key validation**: Verify key integrity on startup
- **Backup procedures**: Secure key recovery mechanisms

## Implementation States

### Pack Controller States
```
State 1: Factory Default (No keys, CAN 1 & 2 unencrypted, Bit 17 = 0)
    ↓
State 2: Bootstrap Complete (LCT keys stored, CAN 1 encrypted, Bit 17 = 1)
    ↓
State 3: Paired (Device pairing keys available, CAN 2 encrypted, Bit 17 = 1)
    ↓
State 4: Operational (Full encrypted dual-CAN communication)
```

### Communication Security Levels
- **Level 0**: Unencrypted (bootstrap only, Bit 17 = 0)
- **Level 1**: CAN 1 encrypted (app ↔ pack controller, Bit 17 = 1)
- **Level 2**: CAN 2 encrypted (pack controller ↔ battery modules, Bit 17 = 1)
- **Level 3**: Full encrypted (both CAN networks secured, Bit 17 = 1)

## Error Handling and Recovery

### Common Failure Scenarios
1. **Key corruption**: Re-download from blockchain
2. **Registration failure**: Retry with exponential backoff
3. **Communication failure**: Fallback to lower security level temporarily
4. **Pairing failure**: Reset pairing state and retry

### Diagnostic Procedures
- **Key validation**: Verify all stored keys on startup
- **Communication test**: Validate encrypted channels
- **Blockchain sync**: Verify registration status periodically
- **Security audit**: Log all key operations for audit trail