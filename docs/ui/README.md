# User Interface Guide

## Table of Contents

1. [Overview](#overview)
2. [Getting Started](#getting-started)
3. [Main Interface Elements](#main-interface-elements)
4. [Connection Setup](#connection-setup)
5. [Real-time Monitoring](#real-time-monitoring)
6. [Configuration Operations](#configuration-operations)
7. [Diagnostic Features](#diagnostic-features)
8. [Troubleshooting](#troubleshooting)

## Overview

The ModBatt Configuration Tool provides an intuitive Windows interface for monitoring and configuring modular battery systems. This guide covers the day-to-day operation of the application for field technicians, test engineers, and system integrators.

### Key User Roles

**Field Service Technicians**
- Battery pack commissioning and maintenance
- Parameter configuration and validation
- Fault diagnosis and troubleshooting

**Test Engineers**  
- Development testing and validation
- Performance analysis and optimization
- Protocol compliance verification

**System Integrators**
- Initial system setup and configuration
- Integration testing and validation
- Documentation and training support

## Getting Started

### System Requirements

**Hardware Requirements:**
- Windows 7/8/10/11 (32-bit or 64-bit)
- PEAK PCAN interface (USB, PCIe, or ISA)
- Available USB port (for PCAN-USB)
- Minimum 4GB RAM, 1GB free disk space

**Software Requirements:**
- PEAK PCAN driver installation
- .NET Framework 4.0 or later
- Administrative privileges for driver installation

### First-Time Setup

#### 1. Install PCAN Drivers

```
Prerequisites:
1. Download PEAK PCAN driver package
2. Connect PCAN hardware to computer
3. Run driver installer as administrator
4. Restart computer if prompted
```

#### 2. Launch Application

```
Application Startup:
1. Double-click modbatt.exe
2. Allow firewall access if prompted
3. Wait for application initialization
4. Verify PCAN hardware detection
```

#### 3. Verify Hardware Detection

The application automatically scans for available PCAN channels on startup. Check that your hardware appears in the channel selection dropdown.

## Main Interface Elements

### Application Layout

```
┌─────────────────────────────────────────────────────────────────┐
│ ModBatt Configuration Tool                              [_][□][X] │
├─────────────────────────────────────────────────────────────────┤
│ ① Hardware Setup Section                                        │
│ ┌─────────────────────────────────────────────────────────────┐ │
│ │ Channel Selection | Baudrate | Connection Controls         │ │
│ └─────────────────────────────────────────────────────────────┘ │
├─────────────────────────────────────────────────────────────────┤
│ ② Data Display Section                                          │
│ ┌───────────────────────────┬─────────────────────────────────┐ │
│ │ ③ Pack Data               │ ④ Module Data                   │ │
│ │   • Voltage/Current       │   • Individual Module Info     │ │
│ │   • SOC/SOH              │   • Cell-level Data             │ │
│ │   • Status Information    │   • Balancing Status            │ │
│ └───────────────────────────┴─────────────────────────────────┘ │
├─────────────────────────────────────────────────────────────────┤
│ ⑤ Configuration Section    │ ⑥ Message Log Section            │
│ ┌─────────────────────────┬─────────────────────────────────┐ │
│ │ Parameter Management    │ Live CAN Traffic Display        │ │
│ │ • Read/Write Operations │ • Message Filtering              │ │
│ │ • Value Validation      │ • Timing Analysis                │ │
│ └─────────────────────────┴─────────────────────────────────┘ │
└─────────────────────────────────────────────────────────────────┘
```

### Interface Sections Explained

#### ① Hardware Setup Section
Controls for CAN hardware connection and configuration

#### ② Data Display Section  
Real-time battery system telemetry and status information

#### ③ Pack Data Panel
Aggregated data from the entire battery pack

#### ④ Module Data Panel
Detailed information for individual battery modules

#### ⑤ Configuration Section
Tools for reading and writing system parameters

#### ⑥ Message Log Section
Live CAN message traffic display and analysis

## Connection Setup

### Step 1: Select Hardware Channel

```
Channel Selection Process:
1. Click [Refresh] to scan for hardware
2. Select channel from dropdown:
   - PCAN-USB1, PCAN-USB2, etc.
   - PCAN-PCIe1, PCAN-PCIe2, etc.
3. Verify channel status (Available/Busy)
```

**Available Channel Types:**
- **PCAN-USB**: Most common, plug-and-play
- **PCAN-PCIe**: High-performance, multiple channels  
- **PCAN-ISA**: Legacy systems only

### Step 2: Configure Baudrate

```
Baudrate Options:
• 125 kbps - Low speed, high reliability
• 250 kbps - Medium speed applications  
• 500 kbps - Standard for automotive (recommended)
• 1 Mbps   - High speed, short distances only
```

**Recommended Settings:**
- **Development/Testing**: 500 kbps
- **Production Systems**: 500 kbps or 1 Mbps
- **Long Cable Runs**: 250 kbps or lower

### Step 3: Establish Connection

```
Connection Process:
1. Verify channel and baudrate selection
2. Click [Connect] button
3. Watch for connection status:
   - Success: "Connected to PCAN-USB1"
   - Failure: Error message with details
4. Verify message reception in log panel
```

**Connection Indicators:**
- **Green Status**: Successfully connected
- **Red Status**: Connection failed or lost
- **Message Counter**: Shows live message reception

## Real-time Monitoring

### Pack-Level Monitoring

#### Primary Telemetry Display

```
Pack Data Panel:
┌─────────────────────────────────┐
│ Voltage:      51.2 V            │ ← Total pack voltage
│ Current:     -23.7 A            │ ← Pack current (+ charge, - discharge)
│ SOC:           78 %             │ ← State of charge
│ SOH:           94 %             │ ← State of health  
│ Status:     Normal              │ ← Operational status
│ State:         On               │ ← Current pack state
│ Modules:       4/4              │ ← Active/Total modules
│ Faults:     None                │ ← Active fault conditions
└─────────────────────────────────┘
```

#### Status Interpretation

**Pack States:**
- **Off**: Pack is disconnected, no power available
- **Standby**: Pack ready, contactors open
- **Precharge**: Contactors closing, voltage equalizing  
- **On**: Pack operational, full power available

**Pack Status:**
- **Normal**: All systems operating within limits
- **Warning**: Operating with minor issues
- **Fault**: Major fault condition, protection active
- **Emergency**: Critical fault, immediate shutdown

**Current Sign Convention:**
- **Positive (+)**: Battery charging (power flowing in)
- **Negative (-)**: Battery discharging (power flowing out)

### Module-Level Monitoring

#### Individual Module Display

```
Module Data Panel:
┌───────────────────────────────┐
│ Module: [2] ▼                 │ ← Module selection dropdown
│ Voltage:      12.8 V          │ ← Module terminal voltage
│ Current:      -5.9 A          │ ← Module current contribution
│ Temperature:   24.3 °C        │ ← Average module temperature
│ SOC:            77 %          │ ← Module state of charge
│ Cells:         4/4            │ ← Active/Total cells
│ Balance:    Inactive          │ ← Cell balancing status
│ Status:      Normal           │ ← Module health status
└───────────────────────────────┘
```

#### Cell-Level Information

```
Cell Voltage Extremes:
┌───────────────────────────────┐
│ Highest:     3.21 V (Cell 3)  │ ← Maximum cell voltage
│ Lowest:      3.18 V (Cell 1)  │ ← Minimum cell voltage  
│ Average:     3.20 V           │ ← Mean cell voltage
│ Delta:       0.03 V           │ ← Voltage spread
└───────────────────────────────┘

Cell Temperature Extremes:
┌───────────────────────────────┐
│ Highest:    25.1 °C (Cell 4)  │ ← Hottest cell temperature
│ Lowest:     23.8 °C (Cell 2)  │ ← Coolest cell temperature
│ Average:    24.3 °C           │ ← Mean temperature
│ Delta:       1.3 °C           │ ← Temperature spread
└───────────────────────────────┘
```

### Data Update Rates

**Refresh Frequencies:**
- **Pack Status**: Every 100ms
- **Pack Telemetry**: Every 100ms
- **Module Data**: Every 200ms
- **Cell Details**: Every 500ms
- **Parameter Values**: On-demand

## Configuration Operations

### Parameter Management

#### Reading Parameters

```
Parameter Read Process:
1. Select parameter from dropdown:
   - Max Charge Current
   - Max Discharge Current  
   - Cell Voltage Limits
   - Temperature Limits
   - Balancing Thresholds
   
2. Click [Get] button

3. Wait for response (typically <1 second)

4. Value appears in display field

5. Check status message for success/failure
```

#### Writing Parameters

```
Parameter Write Process:
1. Select parameter from dropdown

2. Enter new value in input field
   - Respect units (A, V, °C, %)
   - Stay within valid ranges
   
3. Click [Set] button

4. Confirm write operation if prompted

5. Verify new value with [Get] operation

6. Check for error messages
```

### Parameter Categories

#### Current Limits
```
Max Charge Current:     50.0 A
Max Discharge Current: 100.0 A
Emergency Limit:       150.0 A
```

#### Voltage Limits  
```
Max Cell Voltage:      4.20 V
Min Cell Voltage:      2.50 V
Pack Cutoff High:     67.2 V
Pack Cutoff Low:      40.0 V
```

#### Temperature Limits
```
Max Cell Temperature:  60.0 °C
Min Cell Temperature: -20.0 °C
Thermal Derating:      45.0 °C
```

#### Balancing Configuration
```
Balance Threshold:     0.05 V
Balance Current:       0.20 A
Balance Timeout:      300 sec
```

### Safety Considerations

**⚠️ Critical Safety Notes:**

1. **Parameter Validation**: The system validates all parameter values. Invalid values will be rejected.

2. **Factory Defaults**: Corrupted parameters automatically revert to safe factory defaults.

3. **Write Protection**: Some parameters require special unlock sequences for safety.

4. **Impact Assessment**: Consider system impact before changing operational limits.

5. **Backup Configuration**: Record current values before making changes.

## Diagnostic Features

### Message Traffic Analysis

#### Live Message Display

```
CAN Message Log:
┌─────────────────────────────────────────────────────────────┐
│ Time     ID   Type  DLC  Data                   Period      │
├─────────────────────────────────────────────────────────────┤
│ 12:34:56 410  STD   8    02 01 04 00 00 9C 78  100.0ms     │
│ 12:34:56 421  STD   8    FF 0C 80 7F 9C 78 00  100.1ms     │
│ 12:34:57 422  STD   8    8A 0C 86 0C 88 0C 01  100.0ms     │
│ 12:34:57 411  STD   8    01 02 01 04 F0 F0 00  200.2ms     │
└─────────────────────────────────────────────────────────────┘
```

#### Message Analysis Tools

**Filtering Options:**
- Show only specific message IDs
- Hide periodic messages
- Display only changed data
- Filter by message type (data/parameter/diagnostic)

**Timing Analysis:**
- Message period calculation
- Jitter detection
- Missing message alerts
- Burst traffic identification

### Fault Diagnosis

#### Fault Code Display

```
Active Faults:
┌─────────────────────────────────────────────────────┐
│ Code │ Description                    │ Module        │
├─────────────────────────────────────────────────────┤
│ 0x01 │ Cell Overvoltage              │ Module 2      │
│ 0x04 │ Temperature High              │ Module 1      │  
│ 0x10 │ Communication Timeout         │ Module 4      │
│ 0x20 │ Isolation Fault               │ Pack Level    │
└─────────────────────────────────────────────────────┘
```

#### Diagnostic Procedures

**Communication Diagnostics:**
1. Verify message reception rates
2. Check for CAN bus errors
3. Validate message content
4. Test parameter read/write operations

**System Health Checks:**
1. Monitor voltage/current limits
2. Track temperature trends
3. Verify balancing operation
4. Check isolation resistance

## Troubleshooting

### Common Issues and Solutions

#### Connection Problems

**Issue: Cannot Connect to PCAN Hardware**
```
Troubleshooting Steps:
1. Check hardware connection (USB cable, power)
2. Verify driver installation
3. Try different USB port
4. Restart application
5. Check Device Manager for conflicts
6. Test with PCAN-View utility
```

**Issue: Connection Drops Frequently**
```
Possible Causes:
• USB power management settings
• CAN bus termination issues
• Electrical noise interference
• Hardware failure

Solutions:
• Disable USB selective suspend
• Check 120Ω termination resistors
• Use shielded cables
• Replace PCAN hardware
```

#### Data Reception Issues

**Issue: No Messages Received**
```
Diagnostic Steps:
1. Verify CAN bus wiring (CANH, CANL, GND)
2. Check baudrate configuration
3. Confirm pack controller power
4. Validate termination resistors
5. Test with oscilloscope if available
```

**Issue: Garbled or Invalid Data**
```
Possible Causes:
• Baudrate mismatch
• Bus timing issues
• Multiple device conflicts
• Electrical interference

Solutions:
• Match baudrates exactly
• Use quality CAN cables
• Isolate test setup
• Check for proper grounding
```

#### Parameter Configuration Issues

**Issue: Cannot Write Parameters**
```
Common Causes:
• Parameter value out of range
• Write protection enabled
• Communication error
• EEPROM corruption

Solutions:
• Check parameter limits
• Verify unlock sequence
• Test basic communication
• Reset to factory defaults
```

### Error Code Reference

**PCAN Error Codes:**
- `PCAN_ERROR_OK`: Operation successful
- `PCAN_ERROR_XMTFULL`: Transmit buffer full
- `PCAN_ERROR_OVERRUN`: Receive buffer overrun
- `PCAN_ERROR_BUSLIGHT`: Bus error (light)
- `PCAN_ERROR_BUSHEAVY`: Bus error (heavy)
- `PCAN_ERROR_BUSOFF`: Bus off condition

**Application Error Messages:**
- "Invalid parameter value": Check value range and format
- "Communication timeout": Verify CAN bus connection
- "Hardware not found": Check PCAN installation
- "Access denied": Run as administrator

### Best Practices

**Daily Operation:**
1. Start with connection verification
2. Monitor pack status continuously
3. Log configuration changes
4. Verify parameter write operations
5. Save diagnostic logs for analysis

**Maintenance Procedures:**
1. Regular parameter backup
2. Communication performance checks
3. Hardware connection inspection
4. Software updates as available
5. Documentation of all changes

This user interface guide provides comprehensive coverage of the ModBatt Configuration Tool operation for all user roles and experience levels.