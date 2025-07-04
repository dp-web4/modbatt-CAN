# ModBatt CAN Communication Protocols

## Table of Contents

1. [Overview](#overview)
2. [Message ID Allocation](#message-id-allocation)
3. [VCU to Pack Controller Messages](#vcu-to-pack-controller-messages)
4. [Pack Controller to VCU Messages](#pack-controller-to-vcu-messages)
5. [Module Communication Protocol](#module-communication-protocol)
6. [Data Encoding Standards](#data-encoding-standards)
7. [Timing and Synchronization](#timing-and-synchronization)
8. [Error Handling and Recovery](#error-handling-and-recovery)

## Overview

The ModBatt CAN communication protocol defines a comprehensive set of messages for battery management system communication. The protocol supports real-time telemetry, configuration management, diagnostic functions, and safety-critical operations.

### Protocol Characteristics
- **Standard**: CAN 2.0B with optional CAN-FD support
- **Identifier Type**: 11-bit standard identifiers
- **Baudrate**: 500 kbps (standard), 1 Mbps (high-speed)
- **Endianness**: Little-endian byte order
- **Error Detection**: CAN hardware CRC plus application-level checksums

### Communication Patterns
- **Command-Response**: VCU commands with pack controller acknowledgments
- **Periodic Broadcast**: Regular telemetry data transmission
- **Event-Driven**: Immediate notification of critical events
- **Request-Reply**: Parameter read/write operations

## Message ID Allocation

### ID Range Overview

```
CAN ID Ranges:
├── 0x400-0x40F: VCU to Pack Controller Commands
├── 0x410-0x41F: Pack Controller Status and State
├── 0x420-0x43F: Pack Controller Telemetry Data
├── 0x440-0x44F: Parameter and Configuration Messages
├── 0x450-0x4FF: Reserved for future expansion
└── 0x500+: Module-specific communication (if needed)
```

### Message Priority Levels

| Priority | ID Range | Message Type | Example |
|----------|----------|--------------|---------|
| Highest | 0x400-0x40F | Control Commands | State changes, emergency stop |
| High | 0x410-0x41F | Status/Safety | Pack state, fault notifications |
| Medium | 0x420-0x43F | Telemetry | Voltage, current, temperature |
| Low | 0x440-0x44F | Configuration | Parameter read/write |

## VCU to Pack Controller Messages

### 0x400: VCU Command

**Purpose**: Primary command interface for pack state control

**Format**:
```
Byte 0: Requested Pack State
  0x00 = Off
  0x01 = Standby  
  0x02 = Precharge
  0x03 = On

Byte 1: Command Flags
  Bit 0: Emergency stop
  Bit 1: Force state change
  Bit 2: Ignore safety checks (dangerous!)
  Bit 3-7: Reserved

Byte 2: Sequence Number (for command tracking)
Byte 3: Priority Level (0=normal, 255=emergency)
Bytes 4-7: Reserved
```

**Example**:
```c
// Turn pack on with normal priority
uint8_t data[8] = {0x03, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00};
canSendMessage(0x400, data, 8);
```

### 0x401: VCU Time Synchronization

**Purpose**: Synchronize pack controller real-time clock

**Format**:
```
Bytes 0-3: Unix timestamp (32-bit, little-endian)
Bytes 4-5: Milliseconds (0-999)
Byte 6: Time zone offset (signed, hours from UTC)
Byte 7: Daylight saving flag
```

### 0x402: VCU EEPROM Read Request

**Purpose**: Request parameter values from pack controller

**Format**:
```
Bytes 0-1: Parameter ID (little-endian)
Byte 2: Number of parameters to read (1-64)
Byte 3: Starting offset (for array parameters)
Bytes 4-7: Reserved
```

**Parameter IDs**:
```c
#define PARAM_MAX_CHARGE_CURRENT     0x0001
#define PARAM_MAX_DISCHARGE_CURRENT  0x0002
#define PARAM_MAX_CELL_VOLTAGE       0x0003
#define PARAM_MIN_CELL_VOLTAGE       0x0004
#define PARAM_MAX_CELL_TEMP          0x0005
#define PARAM_MIN_CELL_TEMP          0x0006
#define PARAM_BALANCE_THRESHOLD      0x0007
#define PARAM_ISOLATION_THRESHOLD    0x0008
// ... additional parameters
```

### 0x403: VCU EEPROM Write Request

**Purpose**: Update configuration parameters

**Format**:
```
Bytes 0-1: Parameter ID (little-endian)
Bytes 2-5: Parameter Value (32-bit, little-endian)
Byte 6: Write protection key (0xA5 for normal write)
Byte 7: Checksum (XOR of bytes 0-6)
```

### 0x404: Module-Specific Command

**Purpose**: Direct control of individual modules

**Format**:
```
Byte 0: Module ID (0-31)
Byte 1: Module Command
  0x00 = Get Status
  0x01 = Set State
  0x02 = Start Balancing
  0x03 = Stop Balancing
  0x04 = Reset Faults
  0x05 = Calibrate

Byte 2: Command Parameter
Bytes 3-7: Command Data (command-specific)
```

### 0x405: VCU Keep Alive

**Purpose**: Maintain communication link and prevent timeout

**Format**:
```
Byte 0: Sequence counter (increments each message)
Byte 1: VCU status flags
Bytes 2-3: VCU heartbeat interval (milliseconds)
Bytes 4-7: Reserved
```

### 0x406: Module List Request

**Purpose**: Request list of active modules

**Format**:
```
Byte 0: Request type
  0x00 = All modules
  0x01 = Active modules only
  0x02 = Faulted modules only
  
Bytes 1-7: Reserved
```

## Pack Controller to VCU Messages

### 0x410: Pack State and Status

**Purpose**: Primary pack status information

**Format**:
```
Byte 0: Current Pack State
  0x00 = Off
  0x01 = Standby
  0x02 = Precharge
  0x03 = On

Byte 1: Pack Status
  0x00 = Off (charge/discharge prohibited)
  0x01 = Empty (charge allowed/discharge prohibited)  
  0x02 = Normal (charge/discharge allowed)
  0x03 = Full (charge prohibited/discharge allowed)

Byte 2: Active Module Count
Byte 3: Fault Code (lower 8 bits)
Byte 4: Fault Code (upper 8 bits)  
Byte 5: State of Charge (0-200, 0.5% resolution)
Byte 6: State of Health (0-200, 0.5% resolution)
Byte 7: Pack Flags
  Bit 0: Balancing active
  Bit 1: Thermal management active
  Bit 2: Isolation monitoring active
  Bit 3: Emergency mode
  Bit 4-7: Reserved
```

### 0x411-0x417: Module Data Messages

These messages provide detailed information about individual modules:

#### 0x411: Module State
```
Byte 0: Module ID
Byte 1: Module State (same encoding as pack state)
Byte 2: Module Status
Byte 3: Cell Count
Byte 4: Balance Status (bit field for cells 0-7)
Byte 5: Balance Status (bit field for cells 8-15)
Byte 6: Fault Flags
Byte 7: Reserved
```

#### 0x412: Module Power Data
```
Bytes 0-1: Module Voltage (15mV resolution)
Bytes 2-3: Module Current (20mA resolution, offset -655.36A)
Byte 4: Module SOC (0.5% resolution)
Byte 5: Module SOH (0.5% resolution)
Bytes 6-7: Reserved
```

#### 0x413: Module Cell Voltages
```
Bytes 0-1: Highest Cell Voltage (1mV resolution)
Bytes 2-3: Lowest Cell Voltage (1mV resolution)
Bytes 4-5: Average Cell Voltage (1mV resolution)
Byte 6: Cell ID with highest voltage
Byte 7: Cell ID with lowest voltage
```

#### 0x414: Module Cell Temperatures
```
Bytes 0-1: Highest Cell Temperature (0.01°C resolution, offset -55.35°C)
Bytes 2-3: Lowest Cell Temperature (0.01°C resolution, offset -55.35°C)
Bytes 4-5: Average Cell Temperature (0.01°C resolution, offset -55.35°C)
Byte 6: Cell ID with highest temperature
Byte 7: Cell ID with lowest temperature
```

### 0x421-0x430: Pack Telemetry Data

Ten data frames providing comprehensive pack telemetry:

#### 0x421: Basic Pack Data
```
Bytes 0-1: Pack Voltage (15mV resolution)
Bytes 2-3: Pack Current (20mA resolution, offset -655.36A)
Byte 4: Pack SOC (0.5% resolution)
Byte 5: Pack SOH (0.5% resolution)
Bytes 6-7: Reserved
```

#### 0x422: Cell Voltage Extremes
```
Bytes 0-1: Highest Cell Voltage in Pack (1mV resolution)
Bytes 2-3: Lowest Cell Voltage in Pack (1mV resolution)
Bytes 4-5: Average Cell Voltage in Pack (1mV resolution)
Byte 6: Module ID with highest cell voltage
Byte 7: Module ID with lowest cell voltage
```

#### 0x423: Temperature Data
```
Bytes 0-1: Highest Cell Temperature (0.01°C resolution)
Bytes 2-3: Lowest Cell Temperature (0.01°C resolution)  
Bytes 4-5: Average Cell Temperature (0.01°C resolution)
Byte 6: Module ID with highest temperature
Byte 7: Module ID with lowest temperature
```

#### 0x424: Current and Power Limits
```
Bytes 0-1: Maximum Charge Current (100mA resolution)
Bytes 2-3: Maximum Discharge Current (100mA resolution)
Bytes 4-5: Maximum Charge Power (100W resolution)
Bytes 6-7: Maximum Discharge Power (100W resolution)
```

#### 0x425: Energy and Capacity
```
Bytes 0-1: Available Energy (100Wh resolution)
Bytes 2-3: Total Capacity (100Wh resolution)
Bytes 4-5: Cycle Count (total charge/discharge cycles)
Bytes 6-7: Reserved
```

### 0x440: Time Request

**Purpose**: Request current time from VCU

**Format**:
```
Bytes 0-7: All zeros (request only)
```

### 0x441: EEPROM Data Response

**Purpose**: Response to parameter read requests

**Format**:
```
Bytes 0-1: Parameter ID (echoed from request)
Bytes 2-5: Parameter Value (32-bit, little-endian)
Byte 6: Parameter Status
  0x00 = Valid
  0x01 = Invalid parameter ID
  0x02 = Read error
  0x03 = Value out of range
  
Byte 7: Checksum
```

## Module Communication Protocol

### UART-Based Module Interface

The pack controller communicates with individual modules using a UART-based protocol:

#### Frame Format
```
Start Byte: 0xAA (synchronization)
Module ID: 0x00-0x1F (32 modules max)
Command: Command/response type
Length: Data length (0-64 bytes)
Data: Command-specific payload
Checksum: XOR checksum of all preceding bytes
```

#### Commands
```c
#define CMD_STATUS_REQUEST    0x01
#define CMD_STATUS_RESPONSE   0x81
#define CMD_CELL_DATA         0x02
#define CMD_CELL_DATA_RESP    0x82
#define CMD_SET_STATE         0x03
#define CMD_SET_STATE_RESP    0x83
#define CMD_START_BALANCE     0x04
#define CMD_STOP_BALANCE      0x05
#define CMD_RESET_FAULTS      0x06
```

## Data Encoding Standards

### Voltage Encoding

```c
// Cell voltages (0-5V range with 1mV resolution)
uint16_t encodeCellVoltage(float voltage) {
    return (uint16_t)(voltage * 1000.0f);
}

float decodeCellVoltage(uint16_t raw) {
    return (float)raw / 1000.0f;
}

// Module/Pack voltages (0-1000V range with 15mV resolution)
uint16_t encodeModuleVoltage(float voltage) {
    return (uint16_t)(voltage / 0.015f);
}

float decodeModuleVoltage(uint16_t raw) {
    return (float)raw * 0.015f;
}
```

### Current Encoding

```c
// Current (±655.36A range with 20mA resolution)
uint16_t encodeCurrent(float current) {
    return (uint16_t)((current + 655.36f) / 0.02f);
}

float decodeCurrent(uint16_t raw) {
    return ((float)raw * 0.02f) - 655.36f;
}
```

### Temperature Encoding

```c
// Temperature (-55.35°C to +655°C with 0.01°C resolution)
uint16_t encodeTemperature(float temperature) {
    return (uint16_t)((temperature + 55.35f) / 0.01f);
}

float decodeTemperature(uint16_t raw) {
    return ((float)raw * 0.01f) - 55.35f;
}
```

### Percentage Encoding

```c
// SOC, SOH (0-200% with 0.5% resolution)
uint8_t encodePercentage(float percentage) {
    return (uint8_t)(percentage / 0.5f);
}

float decodePercentage(uint8_t raw) {
    return (float)raw * 0.5f;
}
```

## Timing and Synchronization

### Message Timing Requirements

| Message | Period | Timeout | Priority |
|---------|--------|---------|----------|
| VCU Keep Alive | 1000ms | 5000ms | High |
| Pack Status (0x410) | 100ms | N/A | High |
| Telemetry (0x421-0x430) | 100ms | N/A | Medium |
| Module Data (0x411-0x417) | 200ms | N/A | Medium |
| Parameter Operations | On-demand | 1000ms | Low |

### Timing Constraints

#### Critical Timing
- **Emergency Response**: < 10ms
- **State Change Acknowledgment**: < 50ms
- **Fault Notification**: < 100ms

#### Real-time Telemetry
- **Pack Status**: Every 100ms ±10ms
- **Module Data**: Every 200ms ±20ms
- **Cell Data**: Every 500ms ±50ms

#### Configuration Operations
- **Parameter Read**: Response within 1000ms
- **Parameter Write**: Acknowledgment within 500ms
- **EEPROM Operations**: Completion within 2000ms

### Clock Synchronization

```c
void synchronizeTime(uint32_t unixTime, uint16_t milliseconds) {
    RTC_TimeTypeDef sTime;
    RTC_DateTypeDef sDate;
    
    // Convert Unix timestamp to RTC format
    struct tm timeinfo;
    time_t rawTime = (time_t)unixTime;
    localtime_r(&rawTime, &timeinfo);
    
    // Set RTC time
    sTime.Hours = timeinfo.tm_hour;
    sTime.Minutes = timeinfo.tm_min;
    sTime.Seconds = timeinfo.tm_sec;
    sTime.SubSeconds = milliseconds;
    
    sDate.Date = timeinfo.tm_mday;
    sDate.Month = timeinfo.tm_mon + 1;
    sDate.Year = timeinfo.tm_year - 100; // Years since 2000
    
    HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
    HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BIN);
}
```

## Error Handling and Recovery

### Error Detection

#### CAN Bus Errors
- **Bit Errors**: Detected by CAN hardware
- **Stuff Errors**: Bit stuffing violations
- **CRC Errors**: Frame corruption detection
- **Form Errors**: Invalid frame format
- **Acknowledgment Errors**: No receiver acknowledgment

#### Application-Level Errors
- **Timeout Errors**: Missing expected messages
- **Sequence Errors**: Out-of-order messages
- **Range Errors**: Parameter values out of bounds
- **Checksum Errors**: Data corruption detection

### Error Recovery Procedures

#### Communication Recovery
```c
void handleCommunicationError(ErrorType error) {
    switch (error) {
        case ERROR_CAN_BUS_OFF:
            // Reset CAN controller
            HAL_CAN_ResetError(&hcan1);
            HAL_CAN_Start(&hcan1);
            
            // Enter safe mode until communication restored
            enterSafeMode();
            break;
            
        case ERROR_VCU_TIMEOUT:
            // VCU communication lost
            vcuTimeoutCount++;
            if (vcuTimeoutCount > MAX_VCU_TIMEOUTS) {
                // Emergency shutdown
                emergencyShutdown();
            } else {
                // Request status
                requestVCUStatus();
            }
            break;
            
        case ERROR_MODULE_TIMEOUT:
            // Module communication lost
            markModuleOffline(moduleId);
            redistributeLoad();
            break;
    }
}
```

#### Parameter Recovery
```c
void handleParameterError(uint16_t paramId) {
    // Attempt to reload from backup
    if (loadParameterFromBackup(paramId) == SUCCESS) {
        return;
    }
    
    // Use factory default
    uint32_t defaultValue = getFactoryDefault(paramId);
    setParameter(paramId, defaultValue);
    
    // Log error for service attention
    logError(ERROR_PARAMETER_CORRUPTED, paramId);
}
```

### Fault Tolerance

#### Redundant Communication
- **Dual CAN Channels**: Primary and backup communication paths
- **Message Retry**: Automatic retransmission on failure
- **Graceful Degradation**: Reduced functionality when communication limited

#### Safety Mechanisms
- **Watchdog Timer**: System reset on software failure
- **Safe State**: Default to safe conditions on error
- **Fault Isolation**: Isolate failed components to protect system

This comprehensive protocol specification ensures reliable, safe, and efficient communication throughout the ModBatt system.