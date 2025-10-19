# modbatt-CAN

**ModBatt Configuration Utility - Windows CAN Interface Tool**

## Overview

The modbatt-CAN utility is a Windows application providing configuration, monitoring, and diagnostic capabilities for the ModBatt battery management system. It demonstrates how the hierarchical battery architecture can be configured and observed through a CAN bus interface.

**Status**: ✅ **Illustrative utility** for demonstration and development purposes

### Quick Facts

- **Platform**: Windows (C++ Builder)
- **Hardware**: Requires PEAK-System CAN interface (PCAN-USB, PCAN-PCIe, etc.)
- **Purpose**: Configuration, monitoring, diagnostics, and testing
- **License**: AGPL-3.0-or-later (software); Borland runtime appropriate to intended use

## Features

### 🔧 Configuration
- **Module Setup**: Register and configure battery modules
- **Cell Mapping**: Define cell-to-module associations
- **Calibration**: Voltage and temperature sensor calibration
- **Parameters**: Set operational thresholds and safety limits

### 📊 Monitoring
- **Real-time Display**: Live voltage, temperature, and status for all cells
- **Pack Visualization**: Hierarchical view of pack → module → cell
- **Trend Analysis**: Historical data logging and graphing
- **Fault Detection**: Real-time error and warning display

### 🔍 Diagnostics
- **CAN Bus Monitor**: Raw message inspection and logging
- **Message Injection**: Send test commands for validation
- **Protocol Analysis**: Decode ModBatt CAN protocol messages
- **Hardware Testing**: Verify communication paths and component health

### 🌐 Web4 Integration (Conceptual)

This utility provides the foundation for future Web4 integration:
- **API-Bridge Interface**: Specification for blockchain LCT integration (see `docs/API_BRIDGE_INTERFACE.md`)
- **Provenance Tracking**: Framework for build manifest verification
- **Identity Binding**: Support for hardware serial → LCT mapping

## Repository Contents

### Application
- **WEB4 Modbatt Configuration Utility/** - Main application source
  - C++ Builder project and source files
  - PCAN-Basic API integration
  - CAN protocol implementation
  - User interface components

### Emulators (Illustrative)
- **Pack Emulator/** - STM32-based pack controller emulator
- **VCU Emulator/** - STM32-based vehicle control unit emulator
- Note: Require STM32 development tools; illustrative utilities for testing

### Documentation
- `docs/` - API-Bridge specification, protocol documentation
- `Modbatt Configuration Tool/` - Comprehensive README with UI guide
- CAN protocol specifications and implementation notes

## Hardware Requirements

### CAN Interface

**Required**: Genuine PEAK-System CAN hardware

The application uses the PCAN-Basic API from PEAK-System Technik GmbH:
- **Supported Hardware**: PCAN-USB, PCAN-PCIe, PCAN-ISA, etc.
- **License**: DLL and headers freely redistributable (see `docs/PCAN_LICENSE_SUMMARY.md`)
- **Restriction**: Must be used only with genuine PEAK-System hardware
- **Download**: https://www.peak-system.com/PCAN-Basic.239.0.html

### ModBatt Hardware

- Pack Controller with CAN interface
- Battery modules with CAN communication
- Appropriate power supply and safety equipment

## Development

### Build Environment

**Platform**: Windows with Embarcadero C++ Builder (or compatible)

**Note**: This is an illustrative utility. The Borland/Embarcadero runtime licensing should be appropriate to your intended use. Migration to open-source development platforms (Qt, wxWidgets, etc.) is encouraged for production deployments.

### Getting Started

1. Open `modbatt.cbproj` in C++ Builder
2. Ensure PCAN-Basic API installed
3. Build and run
4. Connect PCAN hardware and ModBatt system
5. Configure CAN interface settings

See `Modbatt Configuration Tool/README.md` for comprehensive UI guide.

## API-Bridge Integration

The utility includes specifications for integrating with a Web4 API-Bridge service for blockchain functionality:

- **REST API Specification**: `docs/API_BRIDGE_INTERFACE.md`
- **LCT Management**: Entity creation, binding, pairing
- **Witnessing**: Presence and behavior attestation
- **Trust Tensors**: T3/V3 reputation tracking

**Status**: Interface defined; implementation depends on API-Bridge availability.

## License and Patents

### Software License

This software is licensed under the **GNU Affero General Public License v3.0 or later** (AGPL-3.0-or-later).

See [LICENSE](LICENSE) for full license text.
See [LICENSE_SUMMARY.md](LICENSE_SUMMARY.md) for detailed licensing information.

### Third-Party Components

**PCAN-Basic API**:
- Copyright © PEAK-System Technik GmbH
- Freely redistributable with this software
- For use only with genuine PEAK-System hardware
- See `docs/PCAN_LICENSE_SUMMARY.md` for details

**Borland Runtime**:
- Illustrative utility; licensing appropriate to intended use
- Migration to open-source platforms encouraged

### Patents

This software implements patterns covered by 14 US Patents (see [PATENTS.md](PATENTS.md) for complete list).

**Patent Grant**: A limited patent grant for the software is provided under the AGPL-3.0 license scope.

**Hardware Licensing**: Separate from software. Commercial use requires appropriate licensing.

### Copyright

© 2023-2025 Modular Battery Technologies, Inc.

**Open Source Release**: October 2025 - Part of the Web4 trust-native ecosystem demonstration.

## Integration with ModBatt Ecosystem

This utility interfaces with:
- **[Pack-Controller-EEPROM](https://github.com/dp-web4/Pack-Controller-EEPROM)** - Pack controller firmware
- **[ModuleCPU](https://github.com/dp-web4/ModuleCPU)** - Module controller firmware (via CAN)
- **[CellCPU](https://github.com/dp-web4/CellCPU)** - Cell controller firmware (via ModuleCPU)

## Contributing

By contributing, you agree your changes are licensed under AGPL-3.0-or-later.
- Use conventional commits
- Include SPDX headers in new files
- Document UI changes with screenshots
- Test with actual PCAN hardware

See [CONTRIBUTING.md](CONTRIBUTING.md) for details.

---

## Related Projects

This repository can be viewed as demonstrating patterns of distributed trust and verifiable provenance in embedded systems.

**Related**: [Web4](https://github.com/dp-web4/web4) (trust architecture) • [Synchronism](https://github.com/dp-web4/Synchronism) (conceptual framing)

---

**Note**: This is an illustrative utility demonstrating ModBatt configuration and monitoring capabilities. For production deployments, consider migration to open-source development frameworks for broader platform support.
