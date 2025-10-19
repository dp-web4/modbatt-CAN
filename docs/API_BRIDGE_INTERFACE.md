# Web4 API-Bridge Interface Specification

**Created**: October 18, 2025
**Purpose**: Document the REST API interface between modbatt-CAN and the Web4 API-Bridge
**Status**: Interface definition (implementation status TBD)

---

## Overview

The Web4 API-Bridge provides blockchain integration for IoT devices, enabling ModBatt battery management hardware to:
- Create unforgeable LCT (Linked Context Token) identities
- Establish cryptographic pairing relationships
- Record witnessing events
- Broadcast announcements to the Web4 network

This document defines the **interface contract** that allows alternate implementations of the API-Bridge service.

---

## Status Clarification Needed

**Question for User**: What is the status of the API-Bridge?
- [ ] Public repository (link: ________________)
- [ ] Private repository (will be made public: _____)
- [ ] Planned future implementation
- [ ] Reference implementation exists but not for public release

**Impact on modbatt-CAN Release**:
- If public: Link to API-Bridge repo in documentation
- If private/planned: Include this interface spec as reference
- If future: Provide mock endpoints for demonstration

---

## Architecture

```
┌─────────────────┐         HTTPS/REST          ┌─────────────────┐
│  modbatt-CAN    │ ────────────────────────────>│  API-Bridge     │
│  (Windows App)  │                              │  (Web Service)  │
│                 │ <────────────────────────────│                 │
└─────────────────┘         JSON Response        └─────────────────┘
                                                          │
                                                          │ Blockchain RPC
                                                          ▼
                                                  ┌─────────────────┐
                                                  │  Web4 Blockchain│
                                                  │  or Compatible  │
                                                  │  L1/L2 Network  │
                                                  └─────────────────┘
```

---

## API Endpoints

### Base URL

**Format**: `https://{host}:{port}/api/v1/`

**Example**: `https://localhost:8443/api/v1/`

### Authentication

**Method**: API Key or Bearer Token (TBD based on implementation)

**Header**: `Authorization: Bearer {token}`

---

## 1. Entity Management

### 1.1 Create Entity LCT

**Purpose**: Create a new LCT identity for a ModBatt device

**Endpoint**: `POST /entities`

**Request Body**:
```json
{
  "entity_type": "battery_module",
  "unique_id": "MODBATT-MODULE-001-ABC123",
  "hardware_serial": "ABC123XYZ456",
  "public_key": "0x04...",
  "metadata": {
    "manufacturer": "Modular Battery Technologies",
    "model": "ModBatt-Module-v2",
    "capabilities": ["cell_monitoring", "can_bus", "vuart"]
  }
}
```

**Response** (Success):
```json
{
  "status": "success",
  "lct": "lct:web4:entity:battery_module:ABC123",
  "tx_hash": "0x1234...",
  "created_at": "2025-10-18T16:00:00Z",
  "blockchain_address": "0xabcd..."
}
```

---

### 1.2 Get Entity Details

**Purpose**: Retrieve LCT details for an entity

**Endpoint**: `GET /entities/{lct}`

**Response**:
```json
{
  "lct": "lct:web4:entity:battery_module:ABC123",
  "entity_type": "battery_module",
  "unique_id": "MODBATT-MODULE-001-ABC123",
  "public_key": "0x04...",
  "created_at": "2025-10-18T16:00:00Z",
  "metadata": { ... }
}
```

---

## 2. Binding Operations

### 2.1 Create Binding

**Purpose**: Permanently bind hardware serial number to LCT

**Endpoint**: `POST /bindings`

**Request Body**:
```json
{
  "entity_lct": "lct:web4:entity:battery_module:ABC123",
  "hardware_serial": "ABC123XYZ456",
  "binding_proof": {
    "signature": "0x...",
    "timestamp": "2025-10-18T16:00:00Z"
  }
}
```

**Response**:
```json
{
  "status": "success",
  "binding_id": "binding:web4:ABC123:XYZ456",
  "tx_hash": "0x5678...",
  "immutable": true
}
```

---

## 3. Pairing Operations

### 3.1 Initiate Pairing

**Purpose**: Request pairing between two entities (e.g., Module ↔ Pack Controller)

**Endpoint**: `POST /pairings`

**Request Body**:
```json
{
  "initiator_lct": "lct:web4:entity:battery_module:ABC123",
  "target_lct": "lct:web4:entity:pack_controller:XYZ789",
  "pairing_type": "symmetric_key",
  "public_key_initiator": "0x04...",
  "metadata": {
    "protocol": "can_bus_2.0b",
    "baudrate": 250000
  }
}
```

**Response**:
```json
{
  "status": "pending",
  "pairing_id": "pairing:web4:ABC123:XYZ789",
  "challenge": "0x9abc...",
  "expires_at": "2025-10-18T16:05:00Z"
}
```

---

### 3.2 Complete Pairing

**Purpose**: Finalize pairing with cryptographic proof

**Endpoint**: `PUT /pairings/{pairing_id}`

**Request Body**:
```json
{
  "target_response": {
    "public_key": "0x04...",
    "signature": "0x..."
  },
  "shared_secret_hash": "0xdef0..."
}
```

**Response**:
```json
{
  "status": "success",
  "pairing_id": "pairing:web4:ABC123:XYZ789",
  "tx_hash": "0xaaaa...",
  "mrh_link_created": true
}
```

---

## 4. Witnessing Operations

### 4.1 Record Witnessing Event

**Purpose**: One entity witnesses another's presence/behavior

**Endpoint**: `POST /witnessing`

**Request Body**:
```json
{
  "witness_lct": "lct:web4:entity:pack_controller:XYZ789",
  "subject_lct": "lct:web4:entity:battery_module:ABC123",
  "event_type": "can_announcement",
  "timestamp": "2025-10-18T16:10:00Z",
  "observation": {
    "can_id": "0x500FF",
    "unique_id_confirmed": "MODBATT-MODULE-001-ABC123",
    "signal_strength": "good"
  }
}
```

**Response**:
```json
{
  "status": "success",
  "witnessing_id": "witness:web4:XYZ789:ABC123:001",
  "tx_hash": "0xbbbb...",
  "mrh_tensor_updated": true
}
```

---

## 5. Broadcast Operations

### 5.1 Create Broadcast Announcement

**Purpose**: Broadcast entity presence/state to Web4 network

**Endpoint**: `POST /broadcasts`

**Request Body**:
```json
{
  "entity_lct": "lct:web4:entity:battery_module:ABC123",
  "announcement_type": "module_online",
  "payload": {
    "module_id": 5,
    "state": "registered",
    "capabilities": ["cell_monitoring", "can_bus"],
    "cell_count": 32
  },
  "signature": "0x..."
}
```

**Response**:
```json
{
  "status": "success",
  "broadcast_id": "broadcast:web4:ABC123:001",
  "tx_hash": "0xcccc...",
  "propagated_to_network": true
}
```

---

## 6. Trust Tensor Operations

### 6.1 Update T3 Tensor

**Purpose**: Update trust tensor (Talent, Training, Temperament) for an entity in a role

**Endpoint**: `POST /tensors/t3`

**Request Body**:
```json
{
  "subject_lct": "lct:web4:entity:battery_module:ABC123",
  "role_lct": "lct:web4:role:cell_monitor:v1",
  "evaluator_lct": "lct:web4:entity:pack_controller:XYZ789",
  "t3_delta": {
    "talent": 0.05,
    "training": 0.02,
    "temperament": -0.01
  },
  "evidence_hash": "0xdddd...",
  "timestamp": "2025-10-18T16:15:00Z"
}
```

**Response**:
```json
{
  "status": "success",
  "t3_update_id": "t3:web4:ABC123:role_v1:001",
  "tx_hash": "0xeeee...",
  "new_t3_values": {
    "talent": 0.85,
    "training": 0.92,
    "temperament": 0.87
  }
}
```

---

### 6.2 Update V3 Tensor

**Purpose**: Update value tensor (Veracity, Validity, Value)

**Endpoint**: `POST /tensors/v3`

**Request Body**:
```json
{
  "subject_lct": "lct:web4:entity:battery_module:ABC123",
  "action_id": "action:web4:ABC123:temp_reading:001",
  "v3_delta": {
    "veracity": 0.03,
    "validity": 0.01,
    "value": 0.04
  },
  "evidence_hash": "0xffff..."
}
```

**Response**:
```json
{
  "status": "success",
  "v3_update_id": "v3:web4:ABC123:001",
  "tx_hash": "0x1111...",
  "new_v3_values": {
    "veracity": 0.95,
    "validity": 0.91,
    "value": 0.88
  }
}
```

---

## 7. Query Operations

### 7.1 Get MRH (Markov Relevancy Horizon)

**Purpose**: Retrieve the relationship graph for an entity

**Endpoint**: `GET /mrh/{entity_lct}`

**Query Parameters**:
- `depth`: Number of relationship hops (default: 1)
- `include_tensors`: Include T3/V3 values (default: true)

**Response**:
```json
{
  "entity_lct": "lct:web4:entity:battery_module:ABC123",
  "relationships": [
    {
      "type": "pairing",
      "related_lct": "lct:web4:entity:pack_controller:XYZ789",
      "paired_at": "2025-10-18T15:00:00Z",
      "role": "lct:web4:role:subordinate_module:v1",
      "t3": {
        "talent": 0.85,
        "training": 0.92,
        "temperament": 0.87
      },
      "v3": {
        "veracity": 0.95,
        "validity": 0.91,
        "value": 0.88
      }
    },
    {
      "type": "witnessing",
      "related_lct": "lct:web4:entity:cell:DEF456",
      "witnessed_at": "2025-10-18T16:00:00Z",
      "observation_count": 42
    }
  ]
}
```

---

## Error Handling

### Standard Error Response

```json
{
  "status": "error",
  "error_code": "INVALID_LCT_FORMAT",
  "message": "The provided LCT does not match the expected format",
  "details": {
    "field": "entity_lct",
    "provided": "invalid-lct",
    "expected_format": "lct:web4:entity:{type}:{id}"
  }
}
```

### Common Error Codes

| Code | HTTP Status | Description |
|------|-------------|-------------|
| `INVALID_LCT_FORMAT` | 400 | LCT string format invalid |
| `ENTITY_NOT_FOUND` | 404 | LCT does not exist |
| `BINDING_ALREADY_EXISTS` | 409 | Hardware already bound to different LCT |
| `PAIRING_EXPIRED` | 410 | Pairing challenge timeout |
| `UNAUTHORIZED` | 401 | Invalid or missing authentication |
| `BLOCKCHAIN_ERROR` | 500 | Transaction failed on blockchain |
| `RATE_LIMIT_EXCEEDED` | 429 | Too many requests |

---

## Implementation Notes

### For modbatt-CAN Integration

1. **Base URL Configuration**: Store API-Bridge base URL in configuration file
2. **Authentication**: Handle API key/token storage securely
3. **Retry Logic**: Implement exponential backoff for transient errors
4. **Caching**: Cache entity LCTs and MRH data to reduce API calls
5. **Async Operations**: Some operations (especially blockchain writes) may be async

### Mock Implementation

For demonstration purposes when API-Bridge is not available:

```cpp
// Mock API-Bridge endpoint returns success with fake data
class MockAPIBridge {
public:
    string CreateEntity(string unique_id) {
        // Return fake LCT
        return "lct:web4:entity:battery_module:" + unique_id;
    }

    bool RecordWitnessing(string witness, string subject) {
        // Log to console/file instead of blockchain
        LogEvent("WITNESS", witness + " witnessed " + subject);
        return true;
    }
};
```

---

## Security Considerations

1. **TLS Required**: All communication must use HTTPS
2. **Certificate Validation**: Verify API-Bridge SSL certificate
3. **API Key Rotation**: Support periodic key rotation
4. **Rate Limiting**: Respect API-Bridge rate limits
5. **Input Validation**: Validate all LCT formats before sending

---

## Future Enhancements

Potential additions to the API:

1. **Reputation Queries**: GET `/reputation/{entity_lct}?role={role_lct}`
2. **R7 Action Recording**: POST `/actions` with full R7 structure
3. **Event Subscriptions**: WebSocket endpoint for real-time events
4. **Batch Operations**: POST `/batch` for multiple operations in one request
5. **Provenance Verification**: GET `/provenance/{device_id}` to verify build manifests

---

## Questions for User

To complete this specification, please clarify:

1. **API-Bridge Status**:
   - [ ] Does API-Bridge already exist as a repository?
   - [ ] If yes, is it public or private?
   - [ ] If private, will it be made public alongside modbatt repos?
   - [ ] If not yet implemented, is this spec directionally correct?

2. **Authentication Method**:
   - [ ] API Key in header?
   - [ ] OAuth2/Bearer token?
   - [ ] mTLS (mutual TLS)?
   - [ ] Other?

3. **Blockchain Backend**:
   - [ ] Custom Web4 blockchain?
   - [ ] Ethereum L2?
   - [ ] Cosmos SDK chain?
   - [ ] Other?

4. **Release Strategy**:
   - [ ] Release API-Bridge alongside modbatt repos?
   - [ ] Keep API-Bridge private for now?
   - [ ] Provide mock implementation in modbatt-CAN?

---

## Conclusion

This interface specification enables:
- **Decoupled Implementation**: modbatt-CAN and API-Bridge can be developed independently
- **Alternate Implementations**: Anyone can implement a compatible API-Bridge
- **Mock/Test Implementations**: Developers can build without blockchain infrastructure
- **Documentation First**: Clear contract before implementation details

**Next Steps**:
1. User clarifies API-Bridge status
2. Finalize authentication method
3. Update modbatt-CAN documentation to reference this spec
4. Optionally: Create mock implementation for standalone testing

---

**Prepared by**: Claude (Web4 distributed consciousness network)
**Status**: Draft - Awaiting user feedback on API-Bridge status
**Location**: `modbatt_agpl_transition_kit/API_BRIDGE_INTERFACE.md`
