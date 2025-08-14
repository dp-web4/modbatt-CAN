# Modbatt Device Pairing Protocol

## Overview

The Modbatt system implements three distinct types of cryptographic transactions to establish and maintain secure relationships between devices, entities, and their digital identities (LCTs). Each transaction type serves a specific purpose in the trust ecosystem.

## Transaction Types

### 1. Binding Transactions
**Purpose**: Permanently associate an LCT with a device, entity, or another LCT through high-trust cryptographic binding.

**Characteristics**:
- High-trust operation requiring external authority
- One-time operation (typically at manufacturing or first boot)
- Creates immutable linkage between digital identity and physical/logical entity
- Establishes root of trust for all future transactions
- Cannot be undone without destroying the LCT

**Key Architecture**:
- Uses asymmetric key pairs with "semi-public" keys (not truly public)
- Semi-public keys only disclosed to authentication controller and bound partner
- Private keys never leave the entity

**Binding Process**:

1. **Authority Establishment**
   - Authentication controller obtains binding authority through external means
   - Authority includes permission to bind specific LCT to specific entity

2. **Access Data Collection**
   - Controller obtains access data for both entities:
     - For blockchain LCT: chain_id, token_id, RPC endpoint
     - For device: device_id, network address, protocol type
     - For other entities: URL, API endpoint, unique identifiers
   - Access data must be sufficient for any party to locate and contact the entity

3. **Key Generation Request**
   - Controller calls each entity's key generation API:
   ```
   POST /api/binding/generate-keys
   {
     "partner_access_data": {
       "type": "device|lct|entity",
       "chain_id": "...",
       "device_id": "...",
       "endpoint": "..."
     }
   }
   
   Response:
   {
     "semi_public_key": "base64_encoded_key",
     "binding_id": "unique_binding_identifier"
   }
   ```
   - Each entity internally stores:
     - Partner's access data
     - Generated private key (never exposed)
     - Binding context

4. **Binding Completion**
   - Controller distributes semi-public keys:
     - Sends Entity A's semi-public key to Entity B
     - Sends Entity B's semi-public key to Entity A
   - Creates binding record:
   ```
   {
     "binding_id": "uuid",
     "entity_a": {
       "access_data": {...},
       "semi_public_key": "..."
     },
     "entity_b": {
       "access_data": {...},
       "semi_public_key": "..."
     },
     "timestamp": "...",
     "authority": "controller_signature"
   }
   ```

5. **Blockchain Recording**
   - Binding event recorded in auth controller's private blockchain
   - May broadcast witness event (without semi-public keys):
   ```
   {
     "binding_id": "uuid",
     "entity_a_hash": "hash(access_data)",
     "entity_b_hash": "hash(access_data)",
     "timestamp": "...",
     "witness_signature": "..."
   }
   ```

**Post-Binding Communication**:
- Bound entities can communicate directly using:
  - Stored partner access data (to locate)
  - Semi-public/private key pairs (to encrypt/authenticate)
- Auth controller can communicate with either entity using their semi-public keys

### 2. Pairing Transactions
**Purpose**: Authorize two or more entities to trust each other within a specific context through cryptographic pairing certificates.

**Prerequisites**:
- All entities must be previously bound to corresponding LCTs
- Auth controller must have validated authority to create pairing
- Auth controller must have access to binding records and semi-public keys

**Characteristics**:
- Context-specific trust (not absolute trust)
- Time-bounded with expiration options
- Revocable through validation transactions
- Creates operational trust relationship for specific purposes

**Pairing Certificate Structure**:

```json
{
  "pairing_certificate": {
    "clear_metadata": {
      "pairing_id": "uuid",
      "context": "battery_management|diagnostics|firmware_update|etc",
      "entities": [
        {
          "entity_id": "entity_a_id",
          "access_data": {
            "type": "device|lct",
            "endpoint": "...",
            "device_id": "..."
          }
        },
        {
          "entity_id": "entity_b_id",
          "access_data": {
            "type": "device|lct",
            "endpoint": "...",
            "device_id": "..."
          }
        }
      ],
      "timing": {
        "created_at": "timestamp",
        "expires_at": "timestamp|null",
        "valid_from": "timestamp",
        "max_uses": "number|null"
      },
      "restrictions": {
        "allowed_operations": ["read", "write", "execute"],
        "data_types": ["telemetry", "configuration"],
        "rate_limits": {}
      },
      "encryption_policy": {
        "critical_required": ["command", "control", "configuration", "authentication"],
        "critical_optional": ["telemetry", "diagnostics"],
        "unencrypted_allowed": ["status", "heartbeat", "basic_telemetry"]
      }
    },
    "encrypted_sections": {
      "entity_a": "base64_encrypted_with_entity_a_semi_public_key",
      "entity_b": "base64_encrypted_with_entity_b_semi_public_key",
      "handshake_validation": "base64_encrypted_with_full_pairing_key"
    }
  }
}
```

**Entity Encrypted Section Contents** (decrypted by each entity using their private key):
```json
{
  "symmetric_key_half": "32_bytes_base64",
  "clear_metadata_hash": "sha256_of_clear_metadata",
  "exchange_key": "symmetric_key_for_key_exchange"
}
```

**Handshake Validation Section Contents** (encrypted with full pairing key, only readable after successful key exchange):
```json
{
  "validation_nonce": "unique_random_value",
  "auth_controller_endpoint": "url_for_validation_callback",
  "validation_token": "signed_token_for_auth",
  "completion_instructions": {
    "required_fields": ["pairing_id", "validation_nonce", "entity_signatures"],
    "timeout_seconds": 3600
  }
}
```

**Pairing Process**:

1. **Authority Validation**
   - Auth controller validates its authority to create pairing
   - Retrieves binding records for all entities
   - Obtains semi-public keys from binding records

2. **Symmetric Key Generation**
   - Generates full symmetric key for the pairing (64 bytes)
   - Splits into two halves (32 bytes each)
   - Generates exchange key for secure key half exchange

3. **Certificate Creation**
   - Assembles clear metadata with all pairing parameters
   - Computes SHA256 hash of clear metadata
   - For each entity, encrypts with their semi-public key:
     - Their half of the symmetric key
     - Clear metadata hash (for integrity verification)
     - Exchange key (for subsequent key exchange)
   - Creates handshake validation section encrypted with full pairing key:
     - Validation nonce and callback endpoint
     - Only decryptable after entities complete key exchange

4. **Certificate Distribution**
   - Asynchronously delivers certificate to each entity
   - Each entity receives the complete certificate
   - Entities decrypt their section using private key
   - Entities verify clear metadata hash matches

5. **Key Exchange and Handshake Completion**:
   - Entities use exchange key to securely share key halves
   - Combine halves to reconstruct full symmetric key (pairing key)
   - Decrypt handshake validation section using pairing key
   - Send validation message to auth controller:
   ```json
   {
     "pairing_id": "from_clear_metadata",
     "validation_nonce": "from_handshake_section",
     "entity_signatures": {
       "entity_a": "signature_using_binding_private_key",
       "entity_b": "signature_using_binding_private_key"
     },
     "timestamp": "completion_time"
   }
   ```
   - Auth controller validates completion and records handshake success
   - Store pairing key for duration of pairing validity

**Communication After Pairing**:

The reconstructed pairing key is used selectively based on communication criticality:

**Critical Communication** (must use pairing key encryption):
- Command and control messages
- Coordination of functionality between entities
- Configuration changes
- Authentication challenges/responses
- Any data requiring security within the pairing context

**Routine Communication** (encryption optional based on policy):
- Status monitoring
- Non-sensitive telemetry
- Heartbeat signals
- Basic operational data

The encryption policy in the clear metadata specifies which message types require encryption, allowing entities to optimize performance by not encrypting routine, high-frequency, non-sensitive data while ensuring critical operations remain secure.

**Post-Pairing State Summary**:

After successful pairing completion, each party holds:

**Paired Entities** (each has):
- Partner's access data (from clear metadata)
- Complete pairing clear metadata (context, restrictions, encryption policy)
- Full pairing key (reconstructed from both halves)
- Exchange key (for initial key half exchange)
- Their own private key from binding (never shared)
- Partner's semi-public key (from binding, used for encrypted sections)

**Authentication Controller** (retains):
- All pairing clear metadata
- Both entities' access data
- Both entities' semi-public keys (from binding records)
- Both entities' bound LCT access data
- Both entities' LCT semi-public keys
- Pairing record with certificate hash
- Does NOT retain: pairing key, exchange keys, or private keys

This distributed knowledge enables:
- Direct entity-to-entity secure communication
- Auth controller validation of pairing status
- Future validation transactions using retained keys
- Revocation capability through validation events

**Encryption Hierarchy by Value and Cost**:

The system provides three tiers of encryption matched to event value and computational constraints:

1. **Binding Keys (Asymmetric)** - High-value events
   - Highest computational cost
   - Used for: validation transactions, certificate signing, identity verification
   - Examples: pairing authorization, revocation, critical attestations
   - Provides: non-repudiation, strong authentication

2. **Pairing Key (Symmetric, 64 bytes)** - Medium-value events
   - Moderate computational cost
   - Used for: command and control, configuration changes
   - Examples: operational commands, settings updates, coordination messages
   - Provides: efficient bulk encryption for operational data

3. **Exchange Key (Symmetric, lightweight)** - Low-value encrypted events
   - Lowest computational cost
   - Used for: sensitive telemetry, diagnostic data
   - Examples: performance metrics, non-critical logs, status updates
   - Provides: basic confidentiality with minimal overhead

This tiered approach allows resource-constrained devices to apply appropriate security levels based on message importance while maintaining performance.

**Security Properties**:
- Auth controller cannot decrypt entity communications (doesn't retain symmetric key)
- Each entity can only decrypt its own section
- Clear metadata allows any observer to understand pairing context and encryption requirements
- Exchange keys enable secure peer-to-peer key half sharing
- Selective encryption balances security with performance

### 3. Validation Transactions
**Purpose**: Provide contextualized confirmation of events, transactions, or provenance.

**Characteristics**:
- Acts as witness or attestation
- Creates audit trail
- Can be chained for multi-party validation
- Provides non-repudiation

**Implementation**:
```
VALIDATE(event_id, validator_lct_id, context, signature) → validation_receipt
```

## Protocol Architecture

### LCT/Device Communication Layer

Each device in the system has a bound LCT (Linked Context Token) that manages blockchain relationships and cryptographic material. The LCT tokens provide:

- Permanent binding to physical/logical entities
- Context-specific pairing authorizations
- Validation capabilities for events and transactions
- Immutable audit trail of all interactions

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