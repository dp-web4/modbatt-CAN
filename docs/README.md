# ModBatt CAN System Documentation

## Table of Contents

- [Quick Start](#quick-start)
- [Documentation Structure](#documentation-structure)  
- [System Overview](#system-overview)
- [Component Reference](#component-reference)
- [Development Resources](#development-resources)
- [Navigation Guide](#navigation-guide)

## Quick Start

Welcome to the ModBatt CAN System documentation. This system provides a comprehensive battery management and testing platform for modular battery systems using CAN bus communication.

### What is the ModBatt CAN System?

The ModBatt CAN System is a hierarchical battery management platform that includes:
- **Windows Configuration Tool**: Desktop application for monitoring and configuration
- **Pack Controller Firmware**: STM32WB55-based pack management system
- **Module Controllers**: Individual battery module management (conceptual)
- **Testing Emulators**: Hardware simulation for development and testing

### Key Features

- 🔌 **CAN Bus Communication**: Industry-standard automotive communication
- 📊 **Real-time Monitoring**: Live telemetry from pack and module levels
- ⚙️ **Configuration Management**: EEPROM-based parameter storage
- 🛡️ **Multi-layer Safety**: Hardware and software protection systems
- 🔧 **Development Tools**: Emulators and testing frameworks
- 📈 **Scalable Design**: Support for 1-32 modules per pack, up to 192 cells per module

## Documentation Structure

This documentation is organized hierarchically for easy navigation and maintenance:

```
docs/
├── overview/           # High-level system overview and architecture
├── concepts/           # Core concepts and fundamentals
├── components/         # Detailed component documentation
│   ├── windows-app/    # Windows Configuration Tool
│   ├── pack-controller/# STM32 Pack Controller firmware
│   └── emulators/      # Testing and simulation components
├── protocols/          # CAN communication protocols
├── ui/                 # User interface guides
└── integration/        # Development and integration guides
```

## System Overview

### Technology Stack

**Windows Application**
- Framework: Borland C++ Builder / Embarcadero RAD Studio
- CAN Interface: PEAK PCAN-Basic API
- Platform: Windows 7/8/10/11

**Embedded Firmware**
- Microcontroller: STM32WB55 (ARM Cortex-M4)
- Development: STM32CubeIDE
- Communication: CAN-FD, UART, SPI

**Communication**
- Primary: CAN 2.0B / CAN-FD
- Baudrates: 125k/250k/500k/1M bps
- Protocol: Custom ModBatt BMS protocol

### System Architecture

```
┌─────────────────────────────────────────────────────────────────┐
│                Windows Configuration Tool                       │
│           (Monitoring, Configuration, Diagnostics)             │
└────────────────────┬────────────────────────────────────────────┘
                     │ PCAN USB/PCIe Interface
┌────────────────────┼────────────────────────────────────────────┐
│                CAN Bus Network                                  │
│              (500kbps/1Mbps)                                   │
└────────────────────┼────────────────────────────────────────────┘
                     │
    ┌────────────────┼────────────────┬───────────────────────────┐
    │                │                │                           │
┌───────────┐ ┌──────────────┐ ┌──────────────┐ ┌─────────────────┐
│    VCU    │ │     Pack     │ │     Pack     │ │    Testing      │
│ Emulator  │ │ Controller 1 │ │ Controller N │ │   Emulators     │
└───────────┘ └──────┬───────┘ └──────────────┘ └─────────────────┘
                     │
               Module Network
              (UART/Internal CAN)
                     │
        ┌────────────┼────────────┬────────────────┐
        │            │            │                │
   ┌─────────┐  ┌─────────┐  ┌─────────┐     ┌─────────┐
   │Module 1 │  │Module 2 │  │Module 3 │ ... │Module 32│
   └─────────┘  └─────────┘  └─────────┘     └─────────┘
```

## Component Reference

### 🖥️ [Windows Configuration Tool](./components/windows-app/README.md)
**Desktop application for system interaction**
- Real-time CAN message monitoring and analysis
- Battery pack and module configuration
- Parameter programming (EEPROM)
- Data logging and visualization
- Diagnostic tools and fault analysis

**Key Features:**
- Live telemetry display (voltage, current, temperature, SOC, SOH)
- CAN traffic analysis with message decoding
- EEPROM parameter read/write operations
- Multi-pack monitoring support
- Built-in protocol validation tools

### 🔧 [Pack Controller Firmware](./components/pack-controller/README.md)
**STM32WB55-based embedded system**
- Central pack management and coordination
- VCU interface for vehicle integration
- Module communication and control
- Safety system implementation
- Configuration parameter storage

**Key Features:**
- Multi-layer safety protection
- Real-time telemetry processing
- State machine-based control
- EEPROM emulation for configuration
- Comprehensive fault detection and handling

### 🔄 [Testing Emulators](./components/emulators/README.md)
**Hardware simulation for development**
- **VCU Emulator**: Simulates vehicle control unit behavior
- **Pack Emulator**: Emulates complete battery pack responses  
- **Protocol Testing**: Validates communication compliance
- **Development Support**: Hardware-in-the-loop testing

## Development Resources

### 🚀 Getting Started
- [System Overview](./overview/README.md) - Understand the complete system
- [Core Concepts](./concepts/README.md) - Learn CAN bus and battery management fundamentals
- [Communication Protocols](./protocols/README.md) - Master the CAN message specifications

### 📋 Protocol Reference
- [CAN Message Definitions](./protocols/README.md#message-id-allocation) - Complete message catalog
- [Data Encoding Standards](./protocols/README.md#data-encoding-standards) - Value conversion formulas
- [Timing Requirements](./protocols/README.md#timing-and-synchronization) - Message timing specifications

### 🛠️ Development Setup
- [Windows App Development](./components/windows-app/README.md#development-environment) - C++ Builder setup
- [Firmware Development](./components/pack-controller/README.md#development-environment) - STM32CubeIDE configuration
- [Hardware Requirements](./overview/architecture.md#hardware-architecture) - PCAN interface setup

## Navigation Guide

### 📚 Learning Path

**New to the system?** Follow this recommended reading order:

1. **[System Overview](./overview/README.md)** - Get the big picture
2. **[Core Concepts](./concepts/README.md)** - Understand CAN bus and battery management
3. **[System Architecture](./overview/architecture.md)** - Learn how components interact
4. **[Windows Application](./components/windows-app/README.md)** - Master the configuration tool
5. **[Pack Controller](./components/pack-controller/README.md)** - Understand the embedded firmware
6. **[Communication Protocols](./protocols/README.md)** - Learn the CAN message details

### 🔍 Quick Reference

**Looking for specific information?**

| Task | Go To |
|------|-------|
| Connect to CAN hardware | [Windows App → PCAN Integration](./components/windows-app/README.md#pcan-integration) |
| Read battery telemetry | [Windows App → Message Processing](./components/windows-app/README.md#message-processing) |
| Configure pack parameters | [Windows App → Configuration Features](./components/windows-app/README.md#configuration-features) |
| Understand CAN messages | [Protocols → Message Definitions](./protocols/README.md#message-id-allocation) |
| Decode sensor values | [Protocols → Data Encoding](./protocols/README.md#data-encoding-standards) |
| Implement safety systems | [Pack Controller → Safety Systems](./components/pack-controller/README.md#safety-systems) |
| Debug communication issues | [Protocols → Error Handling](./protocols/README.md#error-handling-and-recovery) |

### 🎯 By Use Case

**Battery System Integrators**
- Start with [System Overview](./overview/README.md) for complete system understanding
- Review [Communication Protocols](./protocols/README.md) for integration requirements
- Check [Windows Application](./components/windows-app/README.md) for testing and configuration

**Embedded Developers**
- Focus on [Pack Controller](./components/pack-controller/README.md) for firmware details
- Study [Core Concepts](./concepts/README.md) for battery management fundamentals
- Reference [Protocols](./protocols/README.md) for communication implementation

**Test Engineers**
- Review [Windows Application](./components/windows-app/README.md) for testing tools
- Study [Emulator Documentation](./components/emulators/README.md) for test setup
- Reference [Protocols](./protocols/README.md) for validation procedures

**Field Service Technicians**
- Start with [Windows Application User Guide](./ui/README.md) for operation procedures
- Reference [Troubleshooting Guide](./integration/troubleshooting.md) for common issues
- Use [Protocol Reference](./protocols/README.md) for diagnostic procedures

### 📖 Documentation Features

This documentation is designed to be:
- **Hierarchical**: Start broad, drill down to specifics
- **Navigable**: Clear cross-references between sections
- **Practical**: Includes code examples and real-world usage
- **Maintainable**: Modular structure for easy updates
- **Comprehensive**: Complete coverage from concepts to implementation

## Patent Information

This system implements patented modular battery technology:
- **US Patent 11,380,942**: Modular battery cell monitoring and management
- **US Patent 11,469,470**: Hierarchical battery management system architecture  
- **US Patent 11,575,270**: Advanced communication protocols for battery systems

## Contributing

This documentation is structured for easy maintenance and updates. Each component is documented in separate files to allow independent updates without affecting other sections.

---

**Next Steps:**
- 🏗️ [Explore System Architecture](./overview/architecture.md) to understand the technical design
- 💡 [Learn Core Concepts](./concepts/README.md) to grasp fundamental principles  
- 🔨 [Dive into Components](./components/) to see detailed implementations