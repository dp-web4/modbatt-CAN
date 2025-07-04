# Complete Data Flow: From Cell to User Interface

## Overview

This document traces the complete data path through the ModBatt battery management system hierarchy, from individual battery cells to the Windows user interface.

## System Architecture

```
┌─────────────────────────────────────────────────────────────────┐
│                     Windows Configuration Tool                  │
│                    (Windows PC Application)                     │
└──────────────────────────┬──────────────────────────────────────┘
                           │ USB (PCAN-USB Adapter)
                           │ CAN Bus @ 500kbps
┌──────────────────────────┼──────────────────────────────────────┐
│                    Pack Controller                              │
│                  (STM32WB55 @ 64MHz)                           │
│              Manages up to 32 modules                          │
└──────────────────────────┬──────────────────────────────────────┘
                           │ CAN Bus @ 250kbps
        ┌──────────────────┼──────────────────┬─────────────────┐
        │                  │                  │                 │
┌───────▼────────┐ ┌───────▼────────┐ ┌──────▼────────┐       │
│  ModuleCPU #1  │ │  ModuleCPU #2  │ │ ModuleCPU #N  │    ...│
│ (ATmega64M1)   │ │ (ATmega64M1)   │ │(up to 32 total)│       │
└───────┬────────┘ └───────┬────────┘ └──────┬────────┘       │
        │                  │                  │                 │
        │ Virtual UART @ 20kbps (Daisy Chain)                  │
┌───────▼────────────────────────────────────────────────────┐ │
│    CellCPU #1 → CellCPU #2 → ... → CellCPU #94            │ │
│        (ATtiny45 microcontrollers)                         │ │
│     (Up to 94 cells per module)                           │ │
└───────┬────────────────────────────────────────────────────┘ │
        │ I2C (Bit-banged)                                      │
┌───────▼────────────────────────────────────────────────────┐ │
│           Temperature Sensors (MCP9843)                     │ │
│         Battery Cell Voltage Monitoring                     │ │
└─────────────────────────────────────────────────────────────┘ │
```

## Layer 1: Cell-Level Data Collection (CellCPU)

### Hardware Platform
- **Microcontroller**: ATtiny45 (8-bit AVR @ 8MHz)
- **Memory**: 4KB Flash, 256B SRAM
- **Key Features**: Pin multiplexing for I2C and ADC on same pin

### Data Collection
```c
// Cell voltage measurement
uint16_t cellVoltage = ADCRead();  // 10-bit ADC
// Conversion: voltage = (ADC * VREF * CAL) / 1024

// Temperature measurement via I2C
uint16_t temperature = MCP9843ReadTemperature();
// Format: 8.4 fixed-point (8 bits integer, 4 bits fractional)
```

### Communication Protocol
- **Virtual UART**: 20kbps bit-banged serial
- **Frame Format**: Start bit + 8 data bits + stop bit + guard bit
- **Special Stop Bit**: 1 = more data coming, 0 = end of transmission
- **Daisy Chain**: Each CellCPU forwards upstream data before adding its own

### Data Transmitted
```c
// Virtual UART data packet (per cell)
struct CellData {
    uint16_t voltage;      // Cell voltage (mV)
    uint8_t temperature;   // Temperature (°C)
    uint8_t status;        // I2C errors, discharge state
};
```

## Layer 2: Module-Level Aggregation (ModuleCPU)

### Hardware Platform
- **Microcontroller**: ATmega64M1 (8-bit AVR @ 8MHz)
- **Memory**: 64KB Flash, 4KB SRAM
- **Peripherals**: Hardware CAN controller, ADC, SPI, I2C

### Data Processing
```c
// Collect data from all cells (300ms cycle)
for (int i = 0; i < TOTAL_CELL_COUNT_MAX; i++) {
    cellVoltages[i] = vUARTReadCellVoltage(i);
    cellTemperatures[i] = vUARTReadCellTemp(i);
}

// Calculate statistics
moduleData.avgVoltage = calculateAverage(cellVoltages);
moduleData.minVoltage = findMinimum(cellVoltages);
moduleData.maxVoltage = findMaximum(cellVoltages);
moduleData.totalVoltage = calculateSum(cellVoltages);

// Current measurement (local ADC)
moduleData.current = readCurrentSensor();  // ACS37002
```

### CAN Communication
- **Protocol**: ModBatt CAN protocol @ 250kbps
- **Message IDs**: 0x200-0x3FF range
- **Update Rate**: Status every 2 seconds

### Messages Transmitted
```c
// Module Registration (on startup)
CAN_ID: 0x300
Data: [Module_Type][Hardware_Rev][Firmware_Build][Cell_Count]

// Module Status (periodic)
CAN_ID: 0x301 + module_id
Data: [State][SOC][SOH][Fault0][Fault1][Status][Balance_Active][Balance_Status]

// Module Power Data
CAN_ID: 0x302 + module_id  
Data: [Voltage_L][Voltage_H][Current_L][Current_H][Reserved...]

// Cell Voltage Data
CAN_ID: 0x303 + module_id
Data: [Hi_Cell_L][Hi_Cell_H][Lo_Cell_L][Lo_Cell_H][Avg_L][Avg_H]

// Cell Temperature Data
CAN_ID: 0x304 + module_id
Data: [Hi_Temp_L][Hi_Temp_H][Lo_Temp_L][Lo_Temp_H][Avg_L][Avg_H]
```

## Layer 3: Pack-Level Coordination (Pack Controller)

### Hardware Platform
- **Microcontroller**: STM32WB55 (ARM Cortex-M4 @ 64MHz)
- **Memory**: 1MB Flash, 256KB SRAM
- **Communication**: Triple CAN architecture via external MCP2518FD

### Data Aggregation
```c
// Collect data from all modules
for (int i = 0; i < activeModules; i++) {
    packData.moduleVoltage[i] = modules[i].voltage;
    packData.moduleCurrent[i] = modules[i].current;
    packData.cellCount += modules[i].cellCount;
}

// Calculate pack-level statistics
packData.totalVoltage = sum(packData.moduleVoltage);
packData.totalCurrent = average(packData.moduleCurrent);
packData.soc = calculateSOC();
packData.soh = calculateSOH();
```

### State Management
```c
// Pack state machine
switch (packState) {
    case PACK_OFF:
        // All outputs disabled
        break;
    case PACK_STANDBY:
        // Mechanical relay on, FET off
        break;
    case PACK_PRECHARGE:
        // Select highest voltage module for soft start
        break;
    case PACK_ON:
        // Full power delivery
        break;
}
```

### CAN Communication to Windows App
- **Protocol**: ModBatt VCU protocol @ 500kbps
- **Message IDs**: 0x400-0x4FF range
- **Update Rate**: 100ms for critical data

### Messages Transmitted
```c
// Pack State (0x410)
Data: [State][Status][Inverter_State][Reserved][Fault][Active_Modules][Total_Modules]

// Pack Power Data (0x421)
Data: [Voltage_L][Voltage_H][Current_L][Current_H][SOC][SOH][Reserved...]

// Cell Voltage Statistics (0x422)
Data: [Hi_Cell_L][Hi_Cell_H][Lo_Cell_L][Lo_Cell_H][Avg_L][Avg_H]

// Cell Temperature Statistics (0x423)
Data: [Hi_Temp_L][Hi_Temp_H][Lo_Temp_L][Lo_Temp_H][Avg_L][Avg_H]

// Charge/Discharge Limits (0x425)
Data: [Chg_Limit_L][Chg_Limit_H][Dis_Limit_L][Dis_Limit_H][End_V_L][End_V_H]

// Module-specific data (0x411-0x417)
// Forwarded from selected module when requested
```

## Layer 4: User Interface (Windows Application)

### Hardware Interface
- **PCAN-USB**: USB to CAN adapter from PEAK System
- **Driver**: PCAN-Basic DLL API
- **Framework**: Borland C++ Builder (VCL)

### Message Reception
```cpp
// CAN read thread
DWORD CANReadThreadFunc() {
    while (active) {
        TPCANMsgFD msg;
        TPCANTimestampFD timestamp;
        
        // Read message
        TPCANStatus status = m_objPCANBasic->ReadFD(m_PcanHandle, &msg, &timestamp);
        
        if (status == PCAN_ERROR_OK) {
            // Process based on message ID
            ProcessMessage(msg, timestamp);
        }
    }
}
```

### Data Display Updates
```cpp
// Process pack voltage/current data (0x421)
void ProcessData1(TPCANMsgFD msg) {
    // Extract voltage (bytes 0-1)
    uint16_t voltage_raw = (msg.Data[1] << 8) | msg.Data[0];
    float voltage = voltage_raw * 0.1f;  // Scale factor
    editVoltage->Text = FloatToStr(voltage) + " V";
    
    // Extract current (bytes 2-3)
    int16_t current_raw = (msg.Data[3] << 8) | msg.Data[2];
    float current = (current_raw * 0.01f) - 327.68f;  // Scale + offset
    editCurrent->Text = FloatToStr(current) + " A";
    
    // Extract SOC/SOH
    float soc = msg.Data[4] * 0.5f;
    float soh = msg.Data[5] * 0.5f;
    editSoc->Text = FloatToStr(soc) + " %";
    editSoh->Text = FloatToStr(soh) + " %";
}
```

### User Control Commands
```cpp
// Send state change command
void SendStateCommand(int state) {
    TPCANMsgFD msg;
    msg.ID = 0x400;  // VCU_STATE command
    msg.DLC = 3;
    
    msg.Data[0] = state;  // 0=Off, 1=Standby, 2=Precharge, 3=On
    msg.Data[1] = inverterVoltage & 0xFF;
    msg.Data[2] = (inverterVoltage >> 8) & 0xFF;
    
    m_objPCANBasic->WriteFD(m_PcanHandle, &msg);
}
```

## Data Transformation Summary

### Voltage Data Path
1. **Cell Level**: 10-bit ADC reading (0-1023)
2. **CellCPU**: Converts to millivolts using calibration factor
3. **ModuleCPU**: Aggregates cell voltages, calculates min/max/avg
4. **Pack Controller**: Sums module voltages for pack total
5. **Windows App**: Displays with 0.1V resolution

### Temperature Data Path
1. **MCP9843 Sensor**: 13-bit temperature with 0.0625°C resolution
2. **CellCPU**: Reads via I2C, converts to 8-bit value
3. **ModuleCPU**: Finds min/max/avg across all cells
4. **Pack Controller**: Reports pack-wide temperature extremes
5. **Windows App**: Displays with 0.1°C resolution

### Current Data Path
1. **ACS37002 Sensor**: Differential voltage proportional to current
2. **ModuleCPU**: 12-bit ADC with 8-sample averaging
3. **Pack Controller**: Averages module currents
4. **Windows App**: Displays with 0.01A resolution

### State/Control Path (Reverse Direction)
1. **Windows App**: User selects desired state
2. **Pack Controller**: Validates state transition
3. **ModuleCPU**: Receives state command via CAN
4. **Hardware**: Activates relays/FETs accordingly

## Timing Characteristics

- **Cell Data Collection**: 300ms per complete scan
- **Module Status Update**: 2 seconds
- **Pack Data Broadcast**: 100ms
- **Windows Display Refresh**: 100ms
- **Total Latency**: ~500ms from cell measurement to display

## Error Handling

Each layer implements error detection and recovery:

1. **CellCPU**: I2C retry on communication failure
2. **ModuleCPU**: Cell timeout detection, invalid voltage rejection
3. **Pack Controller**: Module timeout (4s), heartbeat monitoring
4. **Windows App**: CAN error display, connection loss indication

## Conclusion

The ModBatt system demonstrates a well-architected hierarchical design where each layer:
- Handles its specific responsibilities
- Adds value through data aggregation and processing
- Maintains robust error handling
- Provides appropriate abstraction for the layer above

This architecture enables scalable battery management from individual cells up to complete vehicle integration, with comprehensive monitoring and control at every level.