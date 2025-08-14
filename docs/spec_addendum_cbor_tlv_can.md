# Spec Addendum: CBOR/TLV Schemas & CAN Frame Definitions

## 1. Cryptographic Profile & Identifiers
| Layer         | Key Type          | Algorithm         | Purpose                          | Notes |
|---------------|------------------|-------------------|-----------------------------------|-------|
| Binding (L1)  | Asymmetric (ECC) | ECC-P256 + ECDSA  | Identity attestation              | Private key on Pack Controller, public in Binding Certificate |
| Pairing (L3)  | Symmetric        | AES-256-GCM       | Session encryption/authentication | Key derived from Binding keys via HKDF-SHA256 |
| Validation    | Asymmetric (ECC) | ECC-P256 + ECDSA  | Signature on validation receipts  | May be same keypair as Binding or separate |
| Exchange (L5) | Symmetric        | AES-256-GCM       | Telemetry encryption              | Key derived from pairing key via HKDF |
| Derivation    | —                | HKDF-SHA256       | Key derivation                    | Labeled inputs per context |

**Key IDs (KID)**: 16-bit unsigned integer included in all encrypted/signed structures.  
**Versioning**: 8-bit version field in certificates and encrypted payload headers.

---

## 2. CBOR/TLV Structures

### 2.1 Binding Certificate (CBOR)
```cbor
{
  "ver": 1,                        // uint8
  "kid": 0x1234,                    // uint16
  "pub": h'04A1...',                // EC P-256 uncompressed
  "entity_type": 1,                 // uint8: 1=Pack, 2=Module, 3=Sensor
  "serial": "MOD123456",            // UTF-8 string
  "created_at": 1733942400,         // epoch seconds
  "valid_from": 1733942400,         // epoch seconds
  "expires_at": 1765478400,         // epoch seconds
  "sig": h'3045...'                 // ECDSA over all preceding fields
}
```

### 2.2 Pairing Certificate (CBOR)
```cbor
{
  "ver": 1,
  "kid": 0x5678,
  "peer_kid": 0x1234,
  "nonce": h'89AB...',
  "policy_hash": h'BEEF...',        // SHA256 digest of policy TLV
  "created_at": 1733942400,
  "valid_from": 1733942400,
  "expires_at": 1734032400,
  "sig": h'3045...'
}
```

### 2.3 Encrypted Payload Header (TLV)
| Tag | Field          | Type   | Len | Notes |
|-----|---------------|--------|-----|-------|
| 0x01| Version        | uint8  | 1   | Always 1 |
| 0x02| KID            | uint16 | 2   | Key ID |
| 0x03| Nonce          | bytes  | 12  | AES-GCM nonce |
| 0x04| Seq Num        | uint32 | 4   | Frame or message sequence |
| 0x05| Total Chunks   | uint8  | 1   | For fragmentation |
| 0x06| Chunk Index    | uint8  | 1   | 0-based |
| 0x07| MIC            | bytes  | 4   | Message Integrity Code/truncated GCM tag |

---

## 3. CAN FD Frame Definitions

All encrypted frames use **Extended Frame Format (EFF)** with encryption-level flags in ID bits 17–19:  
- `000` = Cleartext  
- `001` = Exchange-level encryption  
- `010` = Pairing-level encryption  
- `011` = Binding-level encryption  
- Others reserved.

**DLC**: 0–8 for classic CAN, up to 64 for CAN FD. Fragmentation used for payloads > DLC.

### 3.1 Announce (L1)
**MsgID**: `0x18FF1000 | node_id`  
**Payload** (CBOR): Minimal Binding cert (no sig) or hash of full cert.

### 3.2 Witness (L2)
**MsgID**: `0x18FF1100 | node_id`  
**Payload**: SHA256(cert) + signature over recent telemetry digest.

### 3.3 Paired Handshake (L3)
**MsgID**: `0x18FF1200 | node_id`  
**Payload**: Pairing cert (CBOR, may be fragmented).

### 3.4 Paired Command (L5)
**MsgID**: `0x18FF1300 | node_id`  
**Payload**: TLV header + encrypted command body.

### 3.5 Validation (L6/L7)
**MsgID**: `0x18FF1400 | node_id`  
**Payload**: CBOR structure:
```cbor
{
  "lvl": 6,             // uint8
  "result": 0,          // uint8: 0=pass, 1=fail
  "ts": 1733942400,     // epoch
  "sig": h'3045...'
}
```

---

## 4. Fragmentation & Reassembly
- Each fragment carries:
  - **Total Chunks (TC)** (1 byte)
  - **Chunk Index (CI)** (1 byte)
  - **Payload Data** (<= DLC - 2 bytes)
  - MIC covers full reassembled payload, not per-fragment.
- Receiver buffers fragments until TC chunks received, validates MIC, then processes.

---

## 5. Latency & Throughput Budget (Targets)
| Operation                  | Target (ms) | Notes |
|----------------------------|-------------|-------|
| L3 Pairing Handshake       | < 50        | w/ AES-GCM init |
| L5 Encrypted Telemetry     | < 5/frame   | AES-GCM on module |
| L5 Command ACK w/ Sig      | < 20        | ECDSA verify on pack |
| Full Validation (L6)       | < 100       | ECDSA + CBOR encode |

---

## 6. Failure & Recovery
- **Factory Reset**: Wipes all keys except immutable Binding private key.
- **Clock Skew**: Accept ±5 min; beyond triggers re-pairing.
- **Partial Pairing Timeout**: 30 s; stale certs discarded.

---

## 7. Web4 / Sensor Integration Hooks
- Treat all sensors as bound entities with Binding certs + pairing per context.
- Exchange-level telemetry for low-value trust/motion summaries.
- Pairing-level or higher for high-value/focus data (e.g., object recognition output).

---
