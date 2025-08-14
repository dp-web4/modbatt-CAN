# Claude Context for ModBatt CAN Tool

## Project Context System

**IMPORTANT**: A comprehensive context system exists at `/mnt/c/projects/ai-agents/misc/context-system/`

Quick access:
```bash
# Get overview of this tool's role
cd /mnt/c/projects/ai-agents/misc/context-system
python3 query_context.py project modbatt-can

# See integration plans
python3 query_context.py search "CAN configuration"

# Find related projects
python3 query_context.py relationships modbatt-can
```

## This Project's Role

ModBatt CAN Tool provides Windows GUI configuration for the battery management system:
- PCAN-Basic API integration
- CAN message monitoring and transmission
- Pack Emulator and VCU Emulator tools
- Configuration interface for battery hierarchy

## Key Features
- Real-time CAN bus monitoring
- Message injection for testing
- Emulation modes for development
- C++ Builder for Windows compatibility

## Key Relationships
- **Configures**: Battery management hierarchy (Cell/Module/Pack)
- **Documents**: Blockchain integration with web4-modbatt-demo
- **Enables**: Testing and development of CAN protocols

## Integration Vision
Documentation describes future where CAN tool could trigger blockchain transactions, creating seamless bridge between physical battery operations and decentralized energy markets. This represents distributed intelligence extending from hardware through software to economic systems.