# Modbatt Configuration Tool (modbatt-CAN)

Windows GUI application for configuring and monitoring Modular Battery Pack Controllers and Modules via CAN bus.

## Purpose

This tool allows you to:
- Monitor real-time pack and module data via CAN bus
- Configure EEPROM parameters on the Pack Controller
- Control pack and module states (Off/Standby/Precharge/On)
- View detailed module information (voltage, current, temperature, SOC, SOH)
- Inspect CAN bus traffic and debug communication issues
- Interface with Pack Controller and Module hardware during development and testing

## Hardware Requirements

- Windows PC (Windows 10/11 recommended)
- PCAN-USB adapter (or compatible PEAK CAN interface)
- CAN bus connection to Pack Controller
- 500 kBit/s CAN baudrate (standard for battery system)

## Software Requirements

- Borland C++ Builder (for building from source)
- PCAN-Basic drivers installed
- Visual C++ Runtime (for running pre-built executable)

## Building the Application

**IMPORTANT**: This project uses Borland C++ Builder and must be built in Windows.

1. Open `modbatt.cbproj` in Borland C++ Builder
2. Ensure PCAN-Basic API is installed (headers and libraries included)
3. Build configuration:
   - Debug: For development and testing
   - Release: For production use
4. Build the project (Project → Build)

**Note**: Cannot build from WSL or Linux. User must compile manually in Windows environment.

## User Interface Guide

### Main Window Layout

The application window is organized into functional sections:

```
┌──────────────────────────────────────────────────────┐
│ Hardware Settings & Connection Controls               │
├──────────────┬───────────────────────────────────────┤
│ Pack Data    │ Module Data                           │
│ (left panel) │ (right panel)                         │
│              │                                       │
│ - State      │ - Selected Module Details            │
│ - Voltage    │ - Voltage/Current/Temp               │
│ - Current    │ - SOC/SOH                            │
│ - SOC/SOH    │ - Fault Status                       │
│ - Statistics │ - Cell Information                   │
├──────────────┴───────────────────────────────────────┤
│ CAN Message Traffic & Info Log                       │
├──────────────────────────────────────────────────────┤
│ Connection Status Bar                                │
└──────────────────────────────────────────────────────┘
```

### Top Section - Hardware Settings & Connection

#### Hardware Group (Left Side)

**CAN Channel Dropdown**
- Select PCAN-USB adapter channel
- Auto-detects available PEAK CAN interfaces
- Typical: "PCAN-USB 1"

**Hardware Type Dropdown**
- Select hardware interface type
- Options: ISA, PCI, USB (default: USB)
- Most users: Select "Plug & Play"

**I/O Port Dropdown**
- I/O port configuration for non-PnP hardware
- Usually disabled for PCAN-USB adapters

**Interrupt Dropdown**
- Interrupt configuration for non-PnP hardware
- Usually disabled for PCAN-USB adapters

**HW Refresh Button**
- Refreshes list of available CAN channels
- Click if adapter not showing up

#### Baudrate & Connection (Center)

**Baudrate Dropdown**
- Set CAN bus speed
- **Default: 500 kBit/s** (standard for Modbatt system)
- Must match Pack Controller configuration

**CAN FD Checkbox**
- Enable CAN-FD mode (flexible data-rate)
- **Default: Unchecked** (standard CAN 2.0)
- Only check if Pack Controller supports CAN-FD

**Bitrate Text Box**
- Shows calculated bitrate for CAN-FD
- Only relevant when CAN FD enabled

**Connect Button**
- Establishes CAN bus connection
- Enables all monitoring and control features
- Status bar shows "Connected" when successful

**Release Button**
- Closes CAN bus connection
- Releases PCAN hardware
- Use before disconnecting adapter

#### Parameters & Control (Right Side)

**Parameter Dropdown**
- Select CAN parameter to configure
- Options: Device ID, filters, trace, etc.

**Active/Inactive Radio Buttons**
- Set parameter state

**Device ID or Delay Edit**
- Enter numeric value for parameter
- Context-sensitive based on selected parameter

**Parameter Get Button**
- Read current parameter value from CAN adapter

**Parameter Set Button**
- Write parameter value to CAN adapter

**Show Period Checkbox**
- Display message period instead of absolute timestamp
- Useful for analyzing message timing

### Left Panel - Pack Data

#### Pack Selection Group

**Pack ID Dropdown**
- Select Pack Controller ID (0-255)
- **Default: 0**
- Pack Controller responds to messages for this ID
- Changes CAN ID base address

**DMC Checkbox**
- Direct Module Control mode
- When checked: Send commands to specific module
- When unchecked: Commands go to Pack Controller

**Module ID Dropdown**
- Select module to control (1-32)
- Only active when DMC checked
- Used for direct module communication

**CAN Base Label**
- Shows calculated CAN ID base address
- Updates based on Pack ID selection
- Format: `0xXXX` (hex)

#### Pack Data Display

Shows real-time pack-level aggregated data:

**Pack State (0x410)**
- `State`: Current pack state (Off/Standby/Precharge/On)
- `Status`: Pack status flags (hex)
- `Total Modules`: Total registered modules
- `Active Modules`: Modules currently active
- `Fault`: Pack-level fault code

**Pack Voltage & Current (0x421)**
- `Voltage`: Total pack voltage (V)
- `Current`: Pack current (A, positive=charge, negative=discharge)

**Pack Limits (0x422)**
- `Charge Limit`: Maximum charge current (A)
- `Discharge Limit`: Maximum discharge current (A)
- `End Voltage Limit`: Maximum pack voltage (V)

**Pack Cell Statistics (0x423)**
- `Hi Cell Volt`: Highest cell voltage in pack (V)
- `Lo Cell Volt`: Lowest cell voltage in pack (V)
- `Avg Cell Volt`: Average cell voltage (V)
- `Hi Cell Temp`: Highest cell temperature (°C)
- `Lo Cell Temp`: Lowest cell temperature (°C)
- `Avg Cell Temp`: Average cell temperature (°C)

**Pack SOC/SOH (0x425)**
- `SOC`: State of Charge (%)
- `SOH`: State of Health (%)

**Pack Isolation (0x430)**
- `Isolation`: Pack isolation resistance (kΩ)
- High value = good isolation from chassis

**Inverter Status (0x408)**
- `Inverter`: Inverter status/state
- VCU/inverter interface data

#### State Control Group

**Send State Checkbox**
- Enable automatic state command transmission
- When checked: Sends state command every 1 second
- When unchecked: Only sends on button click

**State Dropdown**
- Select desired pack/module state
- Options:
  - **Off**: All contactors open, safe state
  - **Standby**: Mechanical relay closed, FET open
  - **Precharge**: Both relay and FET closed
  - **On**: Full operation (same as Precharge)

**Send State Button**
- Manually send state change command
- Useful for one-time state transitions
- Works regardless of "Send State" checkbox

**Timer Indicator**
- Shows when automatic state command sent
- Updates every 1 second when enabled

#### Pack Parameters Group

**Pack Param Dropdown**
- Select EEPROM parameter to read/write
- Parameters include:
  - Platform ID
  - Unique ID
  - Part Number
  - Manufacturer ID
  - Controller ID
  - Debug Level
  - Firmware version
  - Hardware version
  - Configuration values

**Pack Value Edit**
- Enter value to write to EEPROM
- Shows value read from EEPROM

**Pack Param Get Button**
- Read EEPROM parameter from Pack Controller
- Sends `VCU_READ_EEPROM` (0x402) command
- Response displays in Pack Value field

**Pack Param Set Button**
- Write EEPROM parameter to Pack Controller
- Sends `VCU_WRITE_EEPROM` (0x403) command
- Value from Pack Value field written to selected parameter

### Right Panel - Module Data

Shows detailed data for currently selected module:

**Module Selection**
- Use "Module ID" dropdown in Pack Selection to choose module
- Module data updates in real-time when module is active

#### Module Status (0x502)

- `State`: Module state (Off/Standby/Precharge/On)
- `Status`: Module status flags (hex)
- `Cell Count`: Number of cells in module

#### Module Power (0x502)

- `Voltage`: Module voltage (V)
- `Current`: Module current (A)
- `SOC`: Module State of Charge (%)
- `SOH`: Module State of Health (%)

#### Module Cell Statistics (0x503)

- `Hi Cell Volt`: Highest cell voltage in module (V)
- `Lo Cell Volt`: Lowest cell voltage in module (V)
- `Avg Cell Volt`: Average cell voltage (V)

#### Module Temperature (0x504)

- `Hi Cell Temp`: Highest cell temperature (°C)
- `Lo Cell Temp`: Lowest cell temperature (°C)
- `Avg Cell Temp`: Average cell temperature (°C)

#### Module Limits (0x503)

- `Charge Limit`: Maximum module charge current (A)
- `Discharge Limit`: Maximum module discharge current (A)
- `End Voltage`: Maximum module voltage (V)

#### Module Balancing (0x50B)

- `Balance Active`: Balancing status (Active/Inactive)
- `Balance Status`: Bitmask of cells being balanced (hex)

#### Module Isolation (0x504)

- `Isolation`: Module isolation resistance (kΩ)

#### Module Faults (0x505)

- `Fault 0-4`: Five fault registers showing module error codes
- Format: Hexadecimal
- See module documentation for fault code meanings

### Bottom Section - CAN Messages & Info

#### CAN Message List (Left)

**Message List View**
- Shows all CAN messages on bus in real-time
- Columns:
  - **Type**: Message type (STD/EXT, FD, RTR, etc.)
  - **ID**: CAN identifier in hex
  - **Length**: Data length (bytes)
  - **Count**: Number of times message received
  - **Data**: Message payload in hex
  - **Time**: Timestamp or period (ms)

**Message Controls**
- **Read Button**: Start/stop reading CAN messages
- **Clear Button**: Clear message list

#### Info Log (Right)

**Info List Box**
- Diagnostic messages and events
- Version information
- Error messages
- Status updates

**Info Controls**
- **Get Versions Button**: Display PCAN driver version info
- **Info Clear Button**: Clear info log
- **Reset Button**: Reset PCAN adapter hardware
- **Status Button**: Show detailed CAN bus status

## Running the Application

### Initial Setup

1. Install PCAN-Basic drivers from PEAK System
2. Connect PCAN-USB adapter to PC
3. Connect CAN bus to Pack Controller (verify termination)
4. Power on Pack Controller and modules
5. Launch `modbatt.exe`

### Basic Workflow

**1. Connect to CAN Bus**
   - Select PCAN channel (typically "PCAN-USB 1")
   - Select 500 kBit/s baudrate
   - Ensure "CAN FD" is unchecked
   - Click "Connect"
   - Status bar shows "Connected"

**2. Monitor Pack Data**
   - Select Pack ID (usually 0)
   - Ensure "DMC" is unchecked
   - Click "Read" to start monitoring CAN messages
   - Pack data updates automatically in left panel
   - Watch message list for CAN traffic

**3. Monitor Module Data**
   - Select Module ID (1-32) from dropdown
   - Module data displays in right panel
   - Data updates as module sends status messages

**4. Control Pack/Module State**
   - Select desired state from dropdown (Off/Standby/Precharge/On)
   - Option A: Check "Send State" for continuous commands
   - Option B: Click "Send State" button for single command
   - Watch "State" field update when transition completes

**5. Configure EEPROM Parameters**
   - Select parameter from "Pack Param" dropdown
   - Click "Pack Param Get" to read current value
   - Enter new value in "Pack Value" field
   - Click "Pack Param Set" to write to EEPROM
   - **CAUTION**: Incorrect EEPROM values can affect operation!

**6. Direct Module Control (Advanced)**
   - Check "DMC" checkbox
   - Select Module ID
   - State commands now go directly to selected module
   - Useful for testing individual modules
   - **CAUTION**: Bypasses Pack Controller safety logic!

**7. Disconnect**
   - Click "Release" to close CAN connection
   - Safe to disconnect PCAN adapter

## CAN Protocol Details

### Message IDs

The tool monitors and sends messages on the CAN bus using the standard Modbatt protocol:

#### VCU ↔ Pack Controller Messages

| Base ID | Name | Description |
|---------|------|-------------|
| 0x400 | VCU_SEND_STATE | VCU commands pack state |
| 0x401 | VCU_TIME_SYNC | Time synchronization |
| 0x402 | VCU_READ_EEPROM | Read EEPROM parameter |
| 0x403 | VCU_WRITE_EEPROM | Write EEPROM parameter |
| 0x408 | VCU_INVERTER_STATUS | Inverter status |
| 0x410 | BMS_PACK_STATE | Pack state and status |
| 0x421 | BMS_PACK_POWER | Pack voltage/current |
| 0x422 | BMS_PACK_LIMITS | Pack charge/discharge limits |
| 0x423 | BMS_PACK_CELL_STATS | Pack cell statistics |
| 0x425 | BMS_PACK_SOC_SOH | Pack SOC/SOH |
| 0x430 | BMS_PACK_ISOLATION | Pack isolation |

#### Pack Controller ↔ Module Messages

| Base ID | Name | Description |
|---------|------|-------------|
| 0x500 + (module-1) | MODULE_ANNOUNCE | Module discovery |
| 0x502 + (module-1)*0x10 | MODULE_STATUS_1 | Voltage, current, state |
| 0x503 + (module-1)*0x10 | MODULE_STATUS_2 | Cell stats, SOC, SOH |
| 0x504 + (module-1)*0x10 | MODULE_STATUS_3 | Temperature, faults |
| 0x505 + (module-1)*0x10 | MODULE_DETAIL | Cell data |
| 0x50B + (module-1)*0x10 | MODULE_BALANCE_STATUS | Balancing info |

### Pack ID Offset

When Pack ID is changed, all BMS message IDs are offset:
- Pack ID 0: Base IDs (0x410, 0x421, etc.)
- Pack ID 1: Base IDs + 0x100 (0x510, 0x521, etc.)
- Pack ID N: Base IDs + (N * 0x100)

This allows multiple pack controllers on same CAN bus.

### Direct Module Control (DMC)

When DMC checkbox is enabled:
- State commands use Module ID instead of Pack ID
- Command ID: `0x514 + (module_id - 1) * 0x10`
- **WARNING**: Bypasses Pack Controller coordination!
- Use only for isolated module testing

## Features

### Real-Time Monitoring

- Live pack voltage, current, SOC, SOH
- Per-module statistics and status
- Cell-level min/max/average values
- Temperature monitoring
- Fault code display
- Isolation resistance tracking

### Configuration Management

- Read/write EEPROM parameters
- Configure debug levels
- Set pack/module identifiers
- Store calibration values
- Persistent storage on Pack Controller

### State Control

- Manual or automatic state transitions
- Support for all module states (Off/Standby/Precharge/On)
- Direct module control mode
- State command retry with 1 Hz rate

### CAN Bus Analysis

- Real-time message display
- Message counters and periods
- Hex data view
- Timestamp or period mode
- Message filtering
- Traffic analysis

## Troubleshooting

### Connection Issues

**"Cannot initialize PCAN-Basic"**
- Install PCAN-Basic drivers from PEAK System
- Check Windows Device Manager for adapter
- Try different USB port
- Restart PCAN driver service

**"Connection failed"**
- Verify correct CAN channel selected
- Check baudrate set to 500 kBit/s
- Ensure "CAN FD" is unchecked
- Try "HW Refresh" button
- Check adapter LED indicators

**"No messages received"**
- Verify Pack Controller powered on
- Check CAN termination (120Ω at both ends)
- Confirm Pack ID matches (default: 0)
- Verify CAN_H and CAN_L wiring
- Check ground connections
- Click "Read" button to start monitoring

### Pack/Module Communication Issues

**"Pack data not updating"**
- Verify correct Pack ID selected
- Check Pack Controller is running
- Ensure "Read" button pressed
- Look for messages in CAN message list
- Verify Pack Controller firmware sending status messages

**"Module data not showing"**
- Ensure module is registered with Pack Controller
- Select correct Module ID
- Check module is powered and running
- Look for module status messages (0x502-0x505)
- Module must be in active state

**"State command not working"**
- Pack Controller may reject invalid transitions
- Check current state before commanding new state
- Some transitions require intermediate steps
- Module safety limits may prevent transitions
- Check fault codes for blocking conditions

### EEPROM Parameter Issues

**"Get Parameter returns no data"**
- Pack Controller may not support parameter
- EEPROM may not be initialized
- Check CAN message list for response (0x404)
- Some parameters read-only

**"Set Parameter failed"**
- Parameter may be read-only
- Value out of valid range
- EEPROM write protection enabled
- Flash wear leveling may delay write

**"Incorrect value displayed"**
- Check parameter data type and units
- Some values use scaling factors
- Hex vs decimal display
- Multi-byte endianness

### Build Issues

**"Cannot find PCANBasic.h"**
- Install PCAN-Basic API from PEAK System
- Copy headers to Include/ directory
- Update project include paths

**"Linker error: Unresolved external"**
- Ensure PCANBasic.lib in lib/ directory
- Check library search paths in project settings
- Verify correct library for architecture (x86/x64)

## Safety Warnings

⚠️ **DANGER: HIGH VOLTAGE**
- Battery packs contain dangerous voltages and currents
- Always follow proper electrical safety procedures
- Use appropriate PPE (insulated tools, safety glasses)
- Never work on live circuits

⚠️ **FIRE HAZARD**
- Lithium cells can catch fire if mishandled
- Have fire suppression equipment ready
- Monitor temperature continuously
- Never leave batteries unattended while charging

⚠️ **SOFTWARE SAFETY**
- This tool directly controls real hardware
- Incorrect commands can damage batteries
- Test all operations in safe environment first
- Implement emergency stop procedures
- Keep documentation and schematics available

⚠️ **DIRECT MODULE CONTROL**
- DMC mode bypasses Pack Controller safety logic
- Use only for isolated module testing
- Never use DMC with pack assembled
- Improper state transitions can damage modules

## Known Issues

- Web4/blockchain integration not yet implemented
- Some EEPROM parameters undocumented
- CAN message list can slow down with high traffic
- No built-in CAN message filtering

## File Structure

```
Modbatt Configuration Tool/
├── Include/              # Header files
│   ├── can_id_bms_vcu.h      # CAN message IDs
│   ├── can_frm_vcu.h         # CAN frame structures
│   └── bms.h                 # BMS definitions
├── Unit1.h              # Main form header
├── Unit1.cpp            # Main form implementation
├── Unit1.dfm            # Form layout (visual)
├── PCANBasic.h          # PCAN API header
├── PCANBasicClass.h     # PCAN wrapper class
├── PCANBasicClass.cpp   # PCAN wrapper implementation
├── modbatt.cbproj       # C++ Builder project
├── modbatt.cpp          # Application entry point
├── Debug/               # Debug build output
├── Release/             # Release build output
└── README.md            # This file
```

## Development Notes

### Recent Updates

**August 2024**:
- Initial release
- Pack and module monitoring
- EEPROM parameter access
- State control with DMC mode

### Code Organization

- **Unit1.cpp/h**: Main application logic and UI
- **PCANBasicClass**: Wrapper for PCAN-Basic API
- **MessageStatus class**: CAN message tracking and display
- **Module array**: Stores state for up to 32 modules
- **Critical sections**: Thread-safe message handling

### Threading Model

- Main thread: UI and display updates
- CAN read thread: Receives messages via event-driven model
- Critical section protects message list
- Timer-based display refresh (configurable rate)

## License

Copyright (C) 2025 Modular Battery Technologies, Inc.
Protected by US Patents 11,380,942; 11,469,470; 11,575,270; others pending.

## Support

For issues, questions, or to report bugs:
- Create issue at: https://github.com/dp-web4/Pack-Controller-EEPROM/issues
- Contact: Pack Controller development team
- Include: Log files, CAN trace, error messages, hardware configuration
