# ModBatt CAN System Overview

## Table of Contents

1. [Introduction](#introduction)
2. [System Architecture](#system-architecture)
3. [Technology Stack](#technology-stack)
4. [Component Summary](#component-summary)
5. [Key Features](#key-features)
6. [Communication Protocols](#communication-protocols)
7. [Use Cases](#use-cases)

## Introduction

The ModBatt CAN System is a comprehensive battery management and testing platform that provides real-time monitoring, configuration, and diagnostics for modular battery systems. The system consists of a Windows-based configuration tool, embedded firmware for battery pack controllers, and emulators for testing and development.

This system implements patented modular battery technology (US Patents 11,380,942; 11,469,470; 11,575,270) and provides a complete solution for managing hierarchical battery architectures through CAN bus communication.

## System Architecture

### High-Level Architecture

```
┌─────────────────────────────────────────────────────────────────┐
│                    Windows Configuration Tool                   │
│  (Borland C++ Builder Application with PCAN Interface)         │
└────────────────┬──────────────────────┬─────────────────────────┘
                 │                      │
                 ▼                      ▼
┌────────────────────────┐    ┌────────────────────────┐
│      PCAN Hardware     │    │     CAN Bus Network    │
│  (USB-to-CAN Bridge)   │    │   (500kbps/1Mbps)     │
└───────────┬────────────┘    └───────────┬────────────┘
            │                             │
            └─────────────┬───────────────┘
                          │
         ┌────────────────┴────────────────┐
         │          CAN Bus Network        │
         └────────────────┬────────────────┘
                          │
    ┌─────────────┬───────┴───────┬─────────────┐
    ▼             ▼               ▼             ▼
┌──────────┐ ┌──────────┐ ┌──────────────┐ ┌─────────────┐
│   Pack   │ │  Module  │ │     VCU      │ │   Pack      │
│Controller│ │Controllers│ │   Emulator   │ │  Emulator   │
│(STM32WB) │ │  (AVR)    │ │  (STM32WB)   │ │ (STM32F4)   │
└──────────┘ └──────────┘ └──────────────┘ └─────────────┘
```

### Hierarchical Battery Management

```
┌─────────────────────────────────────┐
│           Vehicle Control Unit       │
│         (VCU/Inverter Interface)    │
└─────────────────┬───────────────────┘
                  │ CAN Bus Commands
                  ▼
┌─────────────────────────────────────┐
│          Pack Controller            │
│       (STM32WB55 with CAN)          │
│  • Pack-level control and monitoring│
│  • Module coordination              │
│  • Safety management                │
└─────────────────┬───────────────────┘
                  │ Module CAN Bus
         ┌────────┴────────┬────────────┐
         ▼                 ▼            ▼
┌──────────────┐  ┌──────────────┐  ┌──────────────┐
│   Module 1   │  │   Module 2   │  │   Module N   │
│   Controller │  │   Controller │  │   Controller │
│   (AVR MCU)  │  │   (AVR MCU)  │  │   (AVR MCU)  │
└──────┬───────┘  └──────┬───────┘  └──────┬───────┘
       │ UART             │ UART             │ UART
   ┌───┴───┐          ┌───┴───┐          ┌───┴───┐
   │Cell 1 │          │Cell 1 │          │Cell 1 │
   │Cell 2 │          │Cell 2 │          │Cell 2 │
   │  ...  │          │  ...  │          │  ...  │
   │Cell 192│         │Cell 192│         │Cell 192│
   └───────┘          └───────┘          └───────┘
```

## Technology Stack

### Windows Configuration Tool
- **Framework**: Borland C++ Builder / Embarcadero RAD Studio
- **Language**: C++ with VCL (Visual Component Library)
- **CAN Interface**: PEAK PCAN-Basic API
- **Supported Hardware**: PCAN-USB, PCAN-PCIe, PCAN-ISA adapters
- **OS Support**: Windows 7/8/10/11 (32-bit and 64-bit)

### Embedded Firmware
- **Pack Controller**: STM32WB55 (ARM Cortex-M4 with wireless capability)
- **Module Controllers**: AVR microcontrollers
- **Development Environment**: STM32CubeIDE, Atmel Studio
- **Real-time OS**: Bare metal with HAL libraries

### Communication Protocols
- **CAN 2.0B**: Standard 11-bit identifier CAN frames
- **CAN-FD**: Flexible Data-rate CAN (when supported)
- **Baudrates**: 125kbps, 250kbps, 500kbps, 1Mbps
- **UART**: Cell-to-module communication

## Component Summary

### 1. Modbatt Configuration Tool
**Windows application for system configuration and monitoring**
- Real-time CAN message display and analysis
- Battery pack parameter configuration
- Module-level diagnostics and control
- Data logging and visualization
- EEPROM parameter management

### 2. Pack Controller Firmware
**STM32WB55-based pack management system**
- Aggregates data from up to 32 modules
- Implements pack-level safety algorithms
- Manages module power sequencing
- Handles VCU communication interface
- EEPROM parameter storage and retrieval

### 3. VCU Emulator
**Test system for vehicle control unit simulation**
- Simulates inverter/VCU commands
- Tests pack controller responses
- Validates communication protocols
- Development and testing platform

### 4. Pack Emulator
**Hardware simulation for development**
- Emulates complete battery pack behavior
- Generates realistic sensor data
- Testing without physical battery hardware
- Protocol validation and debugging

## Key Features

### Real-time Monitoring
- **Live Data Display**: Voltage, current, temperature, SOC, SOH
- **Cell-level Visibility**: Individual cell monitoring (up to 192 cells/module)
- **Fault Detection**: Real-time alarm and fault status
- **Performance Metrics**: Charge/discharge limits, efficiency tracking

### Configuration Management
- **Parameter Programming**: EEPROM-based configuration storage
- **Calibration Support**: Voltage, current, and temperature calibration
- **Safety Limits**: Configurable protection thresholds
- **Module Addressing**: Dynamic module ID assignment

### Diagnostic Capabilities
- **CAN Traffic Analysis**: Message capture and decoding
- **Protocol Validation**: Timing and format verification
- **Error Logging**: Comprehensive fault history
- **Performance Analysis**: Energy efficiency reporting

### Development Tools
- **Emulation Support**: Hardware-in-the-loop testing
- **Protocol Debugging**: Message injection and monitoring
- **Firmware Testing**: Validation of embedded code
- **System Integration**: Multi-component testing

## Communication Protocols

### CAN Message Structure
The system uses a well-defined CAN protocol with specific message IDs:

#### VCU to Pack Controller (0x400-0x40F)
- **0x400**: State commands (Off/Standby/Precharge/On)
- **0x401**: Time synchronization
- **0x402**: EEPROM read requests
- **0x403**: EEPROM write commands
- **0x404**: Module-specific commands
- **0x405**: Keep-alive/heartbeat

#### Pack Controller to VCU (0x410-0x44F)
- **0x410**: Pack state and status
- **0x411-0x417**: Module data (state, power, voltages, temperatures)
- **0x421-0x430**: Pack telemetry data (10 data frames)
- **0x440**: Time requests
- **0x441**: EEPROM data responses

### Data Encoding
- **Voltage**: 0.001V resolution (cells), 0.015V resolution (modules/pack)
- **Current**: ±655.36A range with 0.02A resolution
- **Temperature**: -55.35°C to +655°C with 0.01°C resolution
- **Percentages**: 0-200% range with 0.5% resolution

## Use Cases

### 1. Production Testing
- **Quality Assurance**: Verify module functionality before deployment
- **Calibration**: Set accurate measurement parameters
- **Configuration**: Program operational parameters
- **Validation**: Confirm compliance with specifications

### 2. System Integration
- **Vehicle Integration**: Interface with inverter/VCU systems
- **Protocol Validation**: Verify CAN communication compliance
- **Performance Testing**: Validate under load conditions
- **Safety Testing**: Verify fault detection and response

### 3. Field Service
- **Diagnostics**: Troubleshoot deployed systems
- **Maintenance**: Update firmware and parameters
- **Monitoring**: Real-time system health assessment
- **Repair**: Component-level fault isolation

### 4. Research & Development
- **Algorithm Development**: Test new control strategies
- **Performance Optimization**: Analyze system efficiency
- **Protocol Development**: Validate new communication features
- **Hardware Validation**: Test new component designs

## Next Steps

For detailed information about specific aspects of the system:

- [Core Concepts](../concepts/README.md) - CAN bus and battery management fundamentals
- [Windows Application](../components/windows-app/README.md) - Configuration tool details
- [Pack Controller](../components/pack-controller/README.md) - Embedded firmware specifications
- [Communication Protocols](../protocols/README.md) - CAN message definitions
- [User Interface Guide](../ui/README.md) - Application usage instructions