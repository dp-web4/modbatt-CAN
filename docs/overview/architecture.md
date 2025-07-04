# ModBatt CAN System Architecture

## Table of Contents

1. [System Overview](#system-overview)
2. [Hardware Architecture](#hardware-architecture)
3. [Software Architecture](#software-architecture)
4. [Communication Architecture](#communication-architecture)
5. [Data Flow](#data-flow)
6. [Security Architecture](#security-architecture)
7. [Deployment Architecture](#deployment-architecture)

## System Overview

The ModBatt CAN System implements a hierarchical battery management architecture with four distinct layers, each handling specific responsibilities in the overall system operation.

### Architectural Layers

```
┌─────────────────────────────────────────────────────────────────┐
│                     Application Layer                           │
│            (Windows Configuration Tool)                         │
│  • User Interface          • Data Visualization                │
│  • Configuration Management • Diagnostics                       │
└─────────────────────────┬───────────────────────────────────────┘
                          │ PCAN API
┌─────────────────────────┼───────────────────────────────────────┐
│                     Interface Layer                             │
│                  (PCAN Hardware)                                │
│  • USB/PCIe to CAN      • Protocol Translation                  │
│  • Hardware Abstraction • Driver Interface                      │
└─────────────────────────┼───────────────────────────────────────┘
                          │ CAN Bus
┌─────────────────────────┼───────────────────────────────────────┐
│                     Control Layer                               │
│                 (Pack Controllers)                              │
│  • Pack Management      • Module Coordination                   │
│  • Safety Systems       • VCU Interface                         │
└─────────────────────────┼───────────────────────────────────────┘
                          │ Module CAN / UART
┌─────────────────────────┼───────────────────────────────────────┐
│                     Device Layer                                │
│               (Module Controllers)                              │
│  • Cell Monitoring      • Local Control                         │
│  • Data Acquisition     • Protection                            │
└─────────────────────────────────────────────────────────────────┘
```

## Hardware Architecture

### Component Hierarchy

#### Layer 1: Application Interface
**Windows Configuration Tool Host**
- **Platform**: Windows PC (x86/x64)
- **Interface**: USB 2.0/3.0, PCIe slots
- **Requirements**: Windows 7+ with .NET Framework
- **Hardware**: PEAK PCAN adapter required

#### Layer 2: CAN Interface Hardware
**PCAN Hardware Adapters**
- **PCAN-USB**: USB to CAN interface (galvanic isolation)
- **PCAN-PCIe**: PCIe card with multiple CAN channels
- **PCAN-ISA**: Legacy ISA bus interface
- **Specifications**: CAN 2.0A/B compliance, optional CAN-FD support

#### Layer 3: Pack Controllers
**STM32WB55 Microcontroller System**
```
STM32WB55RG (Pack Controller)
├── ARM Cortex-M4 @ 64MHz
├── 1MB Flash / 256KB RAM
├── CAN-FD Controller
├── Multiple UART interfaces
├── SPI for external CAN controllers
├── EEPROM emulation support
└── Real-time clock (RTC)

External Components:
├── MCP2517FD (External CAN-FD controller)
├── Power management circuits
├── Isolation circuits
└── Diagnostic LEDs
```

#### Layer 4: Module Controllers
**AVR Microcontroller System**
```
AVR MCU (Module Controller)
├── 8-bit AVR core
├── UART interface to pack controller
├── ADC for cell voltage measurement
├── Temperature sensors interface
├── Cell balancing control
└── Protection circuits

Cell Interface:
├── Up to 192 cells per module
├── Individual cell voltage monitoring
├── Cell-level temperature monitoring
├── Active/passive cell balancing
└── Cell protection circuits
```

### Physical Topology

```
PC with PCAN ────USB/PCIe───► PCAN Hardware
                                   │
                              CAN Bus Network
                         (500kbps/1Mbps, 120Ω terminated)
                                   │
    ┌──────────────┬──────────────┼──────────────┬──────────────┐
    │              │              │              │              │
   VCU         Pack Ctrl 1    Pack Ctrl 2    Pack Ctrl N    Emulators
 Emulator      (STM32WB55)   (STM32WB55)   (STM32WB55)    (Test Units)
    │              │              │              │              │
    └──────────────┼──────────────┼──────────────┼──────────────┘
                   │              │              │
              Module CAN      Module CAN     Module CAN
             (Internal Bus)  (Internal Bus)  (Internal Bus)
                   │              │              │
        ┌─────┬────┴────┬─────┐   │              │
        │     │         │     │   │              │
    Module  Module   Module  ... │              │
      1       2        3         │              │
    (AVR)   (AVR)    (AVR)       │              │
      │       │        │         │              │
   Cells   Cells    Cells       ...            ...
   1-192   1-192    1-192
```

## Software Architecture

### Windows Application Architecture

```
┌─────────────────────────────────────────────────────────────────┐
│                     Presentation Layer                          │
│  ┌─────────────┬─────────────┬─────────────┬─────────────────┐  │
│  │    Main     │   Config    │  Diagnostic │   Data Logger   │  │
│  │    Form     │   Dialogs   │   Views     │   Interfaces    │  │
│  └─────────────┴─────────────┴─────────────┴─────────────────┘  │
└─────────────────────────┬───────────────────────────────────────┘
                          │ VCL Components
┌─────────────────────────┼───────────────────────────────────────┐
│                     Business Logic Layer                        │
│  ┌─────────────┬─────────────┬─────────────┬─────────────────┐  │
│  │   Message   │   Data      │   Protocol  │   Configuration │  │
│  │  Processing │  Analysis   │  Handling   │   Management    │  │
│  └─────────────┴─────────────┴─────────────┴─────────────────┘  │
└─────────────────────────┼───────────────────────────────────────┘
                          │ Internal APIs
┌─────────────────────────┼───────────────────────────────────────┐
│                     Hardware Abstraction Layer                  │
│  ┌─────────────┬─────────────┬─────────────┬─────────────────┐  │
│  │   PCAN      │   Message   │   Timer     │    Threading    │  │
│  │  Interface  │   Queues    │  Services   │   Management    │  │
│  └─────────────┴─────────────┴─────────────┴─────────────────┘  │
└─────────────────────────┼───────────────────────────────────────┘
                          │ PCAN-Basic API
┌─────────────────────────┼───────────────────────────────────────┐
│                     Driver Layer                                │
│            PCAN Driver (PCANBasic.dll)                         │
└─────────────────────────────────────────────────────────────────┘
```

### Embedded Firmware Architecture

#### Pack Controller Software Stack

```
┌─────────────────────────────────────────────────────────────────┐
│                     Application Layer                           │
│  ┌─────────────┬─────────────┬─────────────┬─────────────────┐  │
│  │    VCU      │   Module    │    Safety   │    Diagnostic   │  │
│  │ Interface   │  Manager    │   System    │    Services     │  │
│  └─────────────┴─────────────┴─────────────┴─────────────────┘  │
└─────────────────────────┬───────────────────────────────────────┘
                          │ Application APIs
┌─────────────────────────┼───────────────────────────────────────┐
│                     Middleware Layer                            │
│  ┌─────────────┬─────────────┬─────────────┬─────────────────┐  │
│  │    CAN      │   EEPROM    │    Timer    │      Data       │  │
│  │  Protocol   │ Management  │   Services  │   Processing    │  │
│  └─────────────┴─────────────┴─────────────┴─────────────────┘  │
└─────────────────────────┼───────────────────────────────────────┘
                          │ HAL APIs
┌─────────────────────────┼───────────────────────────────────────┐
│                Hardware Abstraction Layer (HAL)                 │
│  ┌─────────────┬─────────────┬─────────────┬─────────────────┐  │
│  │     CAN     │    UART     │    SPI      │     GPIO/ADC    │  │
│  │   Driver    │   Driver    │   Driver    │     Drivers     │  │
│  └─────────────┴─────────────┴─────────────┴─────────────────┘  │
└─────────────────────────┼───────────────────────────────────────┘
                          │ Register Access
┌─────────────────────────┼───────────────────────────────────────┐
│                     Hardware Layer                              │
│            STM32WB55 Peripherals & External ICs                │
└─────────────────────────────────────────────────────────────────┘
```

#### Module Controller Software Stack

```
┌─────────────────────────────────────────────────────────────────┐
│                     Application Layer                           │
│  ┌─────────────┬─────────────┬─────────────┬─────────────────┐  │
│  │    Cell     │  Balancing  │  Protection │   Communication │  │
│  │ Monitoring  │   Control   │   System    │     Handler     │  │
│  └─────────────┴─────────────┴─────────────┴─────────────────┘  │
└─────────────────────────┬───────────────────────────────────────┘
                          │ Control APIs
┌─────────────────────────┼───────────────────────────────────────┐
│                     Driver Layer                                │
│  ┌─────────────┬─────────────┬─────────────┬─────────────────┐  │
│  │     ADC     │    UART     │    Timer    │      GPIO       │  │
│  │   Driver    │   Driver    │   Services  │    Control      │  │
│  └─────────────┴─────────────┴─────────────┴─────────────────┘  │
└─────────────────────────┼───────────────────────────────────────┘
                          │ Register Access
┌─────────────────────────┼───────────────────────────────────────┐
│                     Hardware Layer                              │
│                 AVR MCU Peripherals                             │
└─────────────────────────────────────────────────────────────────┘
```

## Communication Architecture

### Protocol Stack

```
┌─────────────────────────────────────────────────────────────────┐
│                     Application Protocol                        │
│  • ModBatt BMS Protocol  • Configuration Commands              │
│  • Diagnostic Messages   • Parameter Management                 │
└─────────────────────────┬───────────────────────────────────────┘
                          │ Message Formatting
┌─────────────────────────┼───────────────────────────────────────┐
│                     Transport Protocol                          │
│  • Message Segmentation • Acknowledgment                        │
│  • Sequence Numbers     • Error Recovery                        │
└─────────────────────────┬───────────────────────────────────────┘
                          │ Frame Assembly
┌─────────────────────────┼───────────────────────────────────────┐
│                     Network Protocol                            │
│  • CAN 2.0B Standard   • Message Arbitration                   │
│  • 11-bit Identifiers  • Priority Management                   │
└─────────────────────────┬───────────────────────────────────────┘
                          │ Bit Stream
┌─────────────────────────┼───────────────────────────────────────┐
│                     Physical Protocol                           │
│  • Differential Signaling • Bus Termination                    │
│  • Bit Timing            • Error Detection                     │
└─────────────────────────────────────────────────────────────────┘
```

### Message Flow Architecture

```
Configuration Tool           Pack Controller              Module Controller
        │                          │                            │
        │─── VCU_COMMAND ─────────►│                            │
        │   (0x400: Set State)      │                            │
        │                           │─── Module Command ───────►│
        │                           │   (Internal Protocol)      │
        │                           │                            │
        │                           │◄── Module Response ───────│
        │                           │   (Status/Data)            │
        │◄── BMS_STATE ─────────────│                            │
        │   (0x410: Pack Status)    │                            │
        │                           │                            │
        │◄── BMS_DATA_1..10 ───────│                            │
        │   (0x421-0x430: Telemetry)│                            │
        │                           │                            │
        │─── VCU_READ_EEPROM ──────►│                            │
        │   (0x402: Parameter Read) │                            │
        │                           │                            │
        │◄── BMS_EEPROM_DATA ──────│                            │
        │   (0x441: Parameter Data) │                            │
```

## Data Flow

### Real-time Data Pipeline

```
Cell Sensors ──► Module MCU ──► Pack Controller ──► CAN Bus ──► PC Application
     │               │                │                │              │
     ▼               ▼                ▼                ▼              ▼
• Voltage        • ADC Conv.     • Aggregation    • Message      • Display
• Temperature    • Filtering     • Processing     • Formatting   • Logging
• Balancing      • Averaging     • Safety Check   • Transmission • Analysis
```

### Configuration Data Flow

```
PC Application ──► CAN Bus ──► Pack Controller ──► EEPROM ──► Module MCU
     │                │               │              │            │
     ▼                ▼               ▼              ▼            ▼
• Parameter      • Parameter     • Validation   • Storage    • Configuration
• Validation     • Transmission  • Processing   • Management • Application
• UI Interface   • Error Check   • Response     • Retrieval  • Operation
```

### Diagnostic Data Flow

```
System Events ──► Event Processing ──► Message Generation ──► Transmission ──► Analysis
     │                   │                      │                    │              │
     ▼                   ▼                      ▼                    ▼              ▼
• Fault Detection   • Error Logging       • Fault Codes        • CAN Frames   • Fault Display
• Status Changes    • Event Timestamps    • Status Messages    • Priority     • Trend Analysis
• Performance       • Data Correlation    • Diagnostic Data    • Routing      • Reporting
```

## Security Architecture

### Access Control

```
┌─────────────────────────────────────────────────────────────────┐
│                     Application Security                        │
│  • User Authentication    • Session Management                 │
│  • Permission Control     • Activity Logging                   │
└─────────────────────────┬───────────────────────────────────────┘
                          │
┌─────────────────────────┼───────────────────────────────────────┐
│                     Communication Security                      │
│  • Message Authentication • Parameter Validation               │
│  • Command Authorization  • Secure Configuration               │
└─────────────────────────┬───────────────────────────────────────┘
                          │
┌─────────────────────────┼───────────────────────────────────────┐
│                     Hardware Security                           │
│  • Physical Access Control • Tamper Detection                  │
│  • Secure Boot            • Hardware Locks                     │
└─────────────────────────────────────────────────────────────────┘
```

### Safety Systems

```
Safety Layer 1: Cell Level
├── Overvoltage Protection
├── Undervoltage Protection
├── Temperature Monitoring
└── Balancing Control

Safety Layer 2: Module Level
├── Module Voltage Limits
├── Current Limiting
├── Temperature Management
└── Isolation Monitoring

Safety Layer 3: Pack Level
├── Pack Voltage Limits
├── Pack Current Limits
├── Thermal Management
└── Emergency Shutdown

Safety Layer 4: System Level
├── Communication Monitoring
├── Heartbeat/Watchdog
├── Redundant Sensing
└── Fail-safe Modes
```

## Deployment Architecture

### Development Environment

```
Development Workstation
├── STM32CubeIDE (Pack Controller)
├── Atmel Studio (Module Controller)
├── Borland C++ Builder (Windows App)
├── PCAN-View (Protocol Analysis)
└── Hardware Debuggers (ST-Link, JTAG)

Testing Environment
├── Pack Emulator (STM32F4)
├── VCU Emulator (STM32WB)
├── CAN Network Analyzer
├── Power Supply Emulation
└── Environmental Test Chamber
```

### Production Environment

```
Manufacturing Setup
├── Automated Test Equipment
├── Calibration Systems
├── Programming Stations
├── Quality Control Systems
└── Configuration Management

Field Deployment
├── Vehicle Integration Systems
├── Service Tool Interfaces
├── Remote Monitoring Capability
├── Firmware Update Systems
└── Diagnostic Access Points
```

### Integration Patterns

#### Hardware-in-the-Loop (HIL)
```
Real Hardware ←→ Emulated Components ←→ Test Software
     │                    │                     │
Pack Controller ←→ Cell Emulator    ←→ Test Automation
Module MCU      ←→ Pack Emulator    ←→ Validation Suite
CAN Network     ←→ Protocol Sim     ←→ Regression Tests
```

#### Software-in-the-Loop (SIL)
```
Simulated Environment ←→ Real Software ←→ Test Framework
        │                      │               │
Virtual Hardware   ←→ Application Code ←→ Test Scripts
Protocol Model     ←→ Control Logic   ←→ Verification
Behavioral Model   ←→ Safety Systems  ←→ Coverage Analysis
```