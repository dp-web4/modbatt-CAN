# Modbatt Device Pairing Protocol

## Overview

The Modbatt system uses a sophisticated dual-layer encryption protocol to securely pair devices (pack controller and battery module) while maintaining zero-knowledge properties for the authentication controller.

## Protocol Architecture

### LCT/Device Communication Layer

Each device in the system has a paired LCT (Local Communication Terminal) that handles secure communications. The architecture uses asymmetric key pairs where:

- Each LCT generates a private and "public" key pair
- The "public" keys are not made truly public, but only shared between paired LCT-device pairs
- All device-to-LCT messages are encrypted/decrypted using these asymmetric keys

### Device-Device Pairing Process

The pairing process involves three main phases:

#### 1. Component Registration
Each device is registered as a blockchain component:
- **Component registration**: Creates component_id and lct_id on blockchain
- **Device identity**: Establishes unique identity for each device
- **Authentication credentials**: Links device to blockchain authority

#### 2. LCT Relationship Creation
The API-Bridge creates cryptographic relationships between devices:
- **LCT creation**: POST /api/v1/lct/create between device pair
- **Device key half generation**: 64-byte key material for secure communication
- **LCT key half generation**: 64-byte key material for Web4 operations

#### 3. Key Distribution
The Modbatt app distributes keys to devices:
1. **Device key halves**: 64-byte key material from API-Bridge LCT creation
2. **Additional crypto material**: App-generated salts, IVs, and parameters
3. **Configuration data**: Device IDs, encryption algorithms, and security policies

Devices receive their complete key packages and can establish encrypted communication.

## Security Properties

### Zero-Knowledge Authentication Controller
- The API-Bridge generates device key halves but cannot derive the combined encryption keys
- Only devices with both key halves can establish secure communication
- App distributes keys but doesn't store device-to-device encryption keys

### Key Isolation
- Each device receives only its own key halves from API-Bridge
- Combined encryption keys require both devices' key material
- No single component can decrypt device communications alone

### Mutual Authentication
- Both devices must possess valid device key halves from API-Bridge
- Devices combine key halves using deterministic algorithms
- Only devices with matching key material can establish encrypted communication

## Implementation Flow

1. **Registration Phase**: App and devices register as blockchain components
2. **LCT Creation**: API-Bridge generates device key halves for device pairs
3. **Key Distribution**: App distributes device key halves and crypto parameters
4. **Key Combination**: Devices combine key halves using HKDF key derivation
5. **Encryption Setup**: Devices initialize AES-256-GCM encryption with derived keys
6. **Secure Channel**: Devices establish ongoing encrypted CAN communication
7. **Operation**: Encrypted communication with authentication and replay protection

## System Architecture: Auth Controller Implementation

In the Modbatt system, the **authentication controller** is implemented as the **API-bridge interface to the blockchain**:

### Component Roles
- **Blockchain**: Serves as the trusted authority for pairing permissions and audit trails
- **API-Bridge Interface**: Handles cryptographic operations, key generation, and pairing package formatting
- **Modbatt Configuration Utility**: Acts as the delivery mechanism, fetching packages from blockchain and distributing to devices

### System Flow
```
Blockchain (Component Registry)
    ↓ (manages component registration)
API-Bridge Interface 
    ↓ (generates device key halves via LCT creation)
Modbatt Configuration Utility 
    ↓ (distributes keys + crypto parameters)
Pack Controller ← → Battery Module
    ↓ (combine keys and establish encryption)
Secure CAN Communication
```

### Implementation Benefits
1. **Centralized Authority**: Blockchain provides authoritative source for component registration
2. **Immutable Records**: All component relationships recorded on blockchain for audit trails
3. **Key Isolation**: API-bridge generates key halves but cannot derive final encryption keys
4. **Flexible Distribution**: App handles complete key package distribution and crypto setup

## Use Case: Pack Controller and Battery Module

In the Modbatt system:
- **Pack Controller**: Receives device key halves from API-Bridge via app distribution
- **Battery Module**: Receives device key halves from pack controller during key exchange
- **Modbatt Configuration Utility**: Manages blockchain registration and distributes complete key packages
- Both devices combine key halves using HKDF and establish AES-256-GCM encrypted communication

## Security Benefits

1. **Compromise Resistance**: Single component compromise doesn't expose full communication keys
2. **Forward Secrecy**: Pairing keys are independent of ongoing device communications
3. **Audit Trail**: All pairing events can be recorded without exposing key material
4. **Scalability**: Protocol supports multiple device pairs without key management complexity