# Pack Controller Firmware

## Table of Contents

1. [Overview](#overview)
2. [Hardware Platform](#hardware-platform)
3. [Software Architecture](#software-architecture)
4. [Core Modules](#core-modules)
5. [Communication Systems](#communication-systems)
6. [EEPROM Management](#eeprom-management)
7. [Safety Systems](#safety-systems)
8. [Development Environment](#development-environment)

## Overview

The Pack Controller firmware runs on STM32WB55 microcontrollers and serves as the central management unit for modular battery packs. It coordinates communication between the Vehicle Control Unit (VCU) and individual battery modules while implementing comprehensive safety and monitoring functions.

### Key Responsibilities
- **Pack-Level Control**: Manages overall pack state and operations
- **Module Coordination**: Interfaces with up to 32 battery modules
- **VCU Communication**: Provides standardized interface to vehicle systems
- **Safety Management**: Implements multi-layer protection systems
- **Configuration Storage**: Manages parameters in emulated EEPROM

### Firmware Variants
- **Pack Controller**: Standard pack management firmware
- **Pack Controller EEPROM**: Enhanced version with advanced parameter storage

## Hardware Platform

### STM32WB55 Microcontroller

```
STM32WB55RG Specifications:
├── ARM Cortex-M4 @ 64MHz (Application Processor)
├── ARM Cortex-M0+ @ 32MHz (Network Processor)
├── Memory
│   ├── 1MB Flash Memory
│   ├── 256KB SRAM
│   └── 128KB SRAM2 (secure/non-secure)
├── Connectivity
│   ├── CAN-FD Controller (integrated)
│   ├── 2x SPI (for external CAN controllers)
│   ├── 2x UART/LPUART
│   ├── USB 2.0 FS
│   └── Bluetooth Low Energy 5.0
├── Peripherals
│   ├── 16-channel 12-bit ADC
│   ├── Real-Time Clock (RTC)
│   ├── Hardware Security Module
│   ├── True Random Number Generator
│   └── Public Key Accelerator (PKA)
└── Power Management
    ├── Multiple low-power modes
    ├── Battery backup for RTC
    └── Brownout protection
```

### External Components

#### CAN Interface
```
Primary CAN Interface:
├── Integrated CAN-FD controller
├── External transceiver (TJA1043/TJA1463)
├── 120Ω termination resistor
└── ESD protection

Secondary CAN Interface:
├── MCP2517FD (SPI-to-CAN controller)
├── MCP2562FD (CAN-FD transceiver)
├── Configurable termination
└── Galvanic isolation (optional)
```

#### Power System
```
Power Architecture:
├── 12V Vehicle Power Input
│   ├── Reverse polarity protection
│   ├── Overvoltage protection (>16V)
│   └── Undervoltage protection (<8V)
├── 3.3V Regulated Supply
│   ├── MCU power supply
│   ├── CAN transceiver power
│   └── Sensor power
├── 5V Supply (if needed)
│   ├── External sensor power
│   └── Module communication power
└── Battery Backup
    ├── Supercapacitor or coin cell
    ├── RTC power maintenance
    └── Critical data preservation
```

## Software Architecture

### Layered Architecture

```
┌─────────────────────────────────────────────────────────────────┐
│                     Application Layer                           │
│  ┌─────────────┬─────────────┬─────────────┬─────────────────┐  │
│  │    VCU      │   Module    │   Safety    │   Diagnostic    │  │
│  │ Interface   │  Manager    │   System    │   Services      │  │
│  └─────────────┴─────────────┴─────────────┴─────────────────┘  │
└─────────────────────────┬───────────────────────────────────────┘
                          │ Application APIs
┌─────────────────────────┼───────────────────────────────────────┐
│                     Middleware Layer                            │
│  ┌─────────────┬─────────────┬─────────────┬─────────────────┐  │
│  │    CAN      │   EEPROM    │   Timer     │    State        │  │
│  │  Protocol   │  Manager    │  Services   │   Machine       │  │
│  └─────────────┴─────────────┴─────────────┴─────────────────┘  │
└─────────────────────────┬───────────────────────────────────────┘
                          │ HAL APIs
┌─────────────────────────┼───────────────────────────────────────┐
│                Hardware Abstraction Layer                       │
│  ┌─────────────┬─────────────┬─────────────┬─────────────────┐  │
│  │     CAN     │    SPI      │    UART     │   GPIO/Timer    │  │
│  │   Driver    │   Driver    │   Driver    │    Drivers      │  │
│  └─────────────┴─────────────┴─────────────┴─────────────────┘  │
└─────────────────────────┬───────────────────────────────────────┘
                          │ Register Access
┌─────────────────────────┼───────────────────────────────────────┐
│                     Hardware Layer                              │
│  STM32WB55 Peripherals + External ICs (MCP2517FD, etc.)        │
└─────────────────────────────────────────────────────────────────┘
```

### File Organization

```
Pack Controller EEPROM/
├── Core/
│   ├── Inc/                     # Header files
│   │   ├── main.h              # Main application header
│   │   ├── bms.h               # BMS data structures
│   │   ├── vcu.h               # VCU interface definitions
│   │   ├── mcu.h               # MCU hardware abstraction
│   │   ├── eeprom_data.h       # EEPROM data structures
│   │   ├── eeprom_emul.h       # EEPROM emulation
│   │   ├── canfdspi_api.h      # External CAN controller API
│   │   └── can_*.h             # CAN protocol definitions
│   ├── Src/                     # Source files
│   │   ├── main.c              # Main application loop
│   │   ├── vcu.c               # VCU communication handler
│   │   ├── mcu.c               # Hardware initialization
│   │   ├── eeprom_emul.c       # EEPROM emulation implementation
│   │   ├── canfdspi_api.c      # External CAN controller driver
│   │   └── stm32wbxx_*.c       # STM32 HAL implementations
│   └── Startup/
│       └── startup_stm32wb55rgvx.s # Boot code
├── Drivers/                     # STM32 HAL drivers
│   ├── STM32WBxx_HAL_Driver/   # Hardware abstraction layer
│   └── CMSIS/                  # ARM CMSIS libraries
├── Debug/                       # Debug build output
├── Release/                     # Release build output
└── Project Files
    ├── Pack Controller EEPROM.ioc # STM32CubeMX configuration
    ├── *.ld                     # Linker scripts
    └── *.launch                 # Debug configurations
```

## Core Modules

### 1. Main Application (main.c)

**Central control loop and system initialization**

#### Key Functions:
```c
int main(void) {
    // Hardware initialization
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();
    MX_LPUART1_UART_Init();
    MX_SPI1_Init();
    MX_TIM1_Init();
    MX_RTC_Init();
    
    // EEPROM emulation initialization
    EE_Init();
    
    // Application initialization
    mcuInit();
    vcuInit();
    
    // Main control loop
    while (1) {
        // Process VCU communication
        vcuMain();
        
        // Process module communication
        moduleMain();
        
        // Safety monitoring
        safetyCheck();
        
        // Background tasks
        backgroundTasks();
        
        // Power management
        if (lowPowerCondition()) {
            enterLowPowerMode();
        }
    }
}
```

#### System Initialization:
```c
void SystemClock_Config(void) {
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
    
    // Configure MSI as system clock source
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_MSI;
    RCC_OscInitStruct.MSIState = RCC_MSI_ON;
    RCC_OscInitStruct.MSICalibrationValue = RCC_MSICALIBRATION_DEFAULT;
    RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_6; // 4MHz
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_MSI;
    RCC_OscInitStruct.PLL.PLLM = 1;
    RCC_OscInitStruct.PLL.PLLN = 32;  // 64MHz system clock
    RCC_OscInitStruct.PLL.PLLR = 2;
    
    HAL_RCC_OscConfig(&RCC_OscInitStruct);
    
    // Configure system clock
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK |
                                  RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;   // 64MHz
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;    // 64MHz
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;    // 64MHz
    
    HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_3);
}
```

### 2. VCU Interface Module (vcu.c)

**Handles communication with Vehicle Control Unit**

#### Core Responsibilities:
- Process VCU commands (state changes, parameter requests)
- Send pack status and telemetry data
- Handle keep-alive and timeout monitoring
- Manage time synchronization

#### Key Functions:
```c
void vcuInit(void) {
    // Initialize CAN interface
    canInit();
    
    // Set initial pack state
    pack.state = packOff;
    pack.vcuRequestedState = packOff;
    pack.status = packStatusOff;
    
    // Initialize communication timeouts
    pack.vcuLastContact.ticks = HAL_GetTick();
    pack.vcuStateChange = false;
}

void vcuMain(void) {
    // Process received CAN messages
    vcuProcessMessages();
    
    // Send periodic status updates
    if (timeForStatusUpdate()) {
        vcuSendPackStatus();
        vcuSendTelemetryData();
    }
    
    // Check VCU communication timeout
    vcuCheckTimeout();
    
    // Process state changes
    vcuProcessStateChanges();
}

void vcuProcessCommand(CAN_Message* msg) {
    switch (msg->id) {
        case ID_VCU_COMMAND:
            // Extract requested state
            packState requestedState = msg->data[0];
            if (validateStateTransition(pack.state, requestedState)) {
                pack.vcuRequestedState = requestedState;
                pack.vcuStateChange = true;
            }
            break;
            
        case ID_VCU_READ_EEPROM:
            // Handle parameter read request
            uint16_t paramId = (msg->data[1] << 8) | msg->data[0];
            vcuSendParameterValue(paramId);
            break;
            
        case ID_VCU_WRITE_EEPROM:
            // Handle parameter write request
            uint16_t paramId = (msg->data[1] << 8) | msg->data[0];
            uint16_t value = (msg->data[3] << 8) | msg->data[2];
            vcuWriteParameter(paramId, value);
            break;
            
        case ID_VCU_KEEP_ALIVE:
            // Update last contact time
            pack.vcuLastContact.ticks = HAL_GetTick();
            break;
    }
}
```

### 3. Module Manager

**Coordinates communication with battery modules**

#### Module Communication:
```c
void moduleMain(void) {
    // Poll all active modules
    for (int i = 0; i < pack.moduleCount; i++) {
        if (modules[i].active) {
            // Request module status
            moduleRequestStatus(i);
            
            // Process received data
            moduleProcessData(i);
            
            // Update module state if needed
            if (modules[i].commandPending) {
                moduleSendCommand(i);
            }
        }
    }
    
    // Update pack aggregated data
    updatePackData();
}

void updatePackData(void) {
    // Reset aggregated values
    pack.cellHiVolt = 0;
    pack.cellLoVolt = 65535;
    pack.cellHiTemp = 0;
    pack.cellLoTemp = 65535;
    pack.cellAvgVolt = 0;
    pack.cellAvgTemp = 0;
    
    uint16_t totalCells = 0;
    uint32_t voltageSum = 0;
    uint32_t temperatureSum = 0;
    
    // Aggregate data from all modules
    for (int i = 0; i < pack.moduleCount; i++) {
        if (modules[i].active) {
            // Update voltage extremes
            if (modules[i].cellHiVolt > pack.cellHiVolt) {
                pack.cellHiVolt = modules[i].cellHiVolt;
                pack.modCellHiVolt = i;
            }
            if (modules[i].cellLoVolt < pack.cellLoVolt) {
                pack.cellLoVolt = modules[i].cellLoVolt;
                pack.modCellLoVolt = i;
            }
            
            // Update temperature extremes
            if (modules[i].cellHiTemp > pack.cellHiTemp) {
                pack.cellHiTemp = modules[i].cellHiTemp;
                pack.modCellHiTemp = i;
            }
            if (modules[i].cellLoTemp < pack.cellLoTemp) {
                pack.cellLoTemp = modules[i].cellLoTemp;
                pack.modCellLoTemp = i;
            }
            
            // Accumulate for averages
            totalCells += modules[i].cellCount;
            voltageSum += modules[i].cellAvgVolt * modules[i].cellCount;
            temperatureSum += modules[i].cellAvgTemp * modules[i].cellCount;
        }
    }
    
    // Calculate averages
    if (totalCells > 0) {
        pack.cellAvgVolt = voltageSum / totalCells;
        pack.cellAvgTemp = temperatureSum / totalCells;
    }
    
    pack.totalCells = totalCells;
}
```

### 4. Safety System

**Implements comprehensive protection mechanisms**

#### Safety Checks:
```c
void safetyCheck(void) {
    // Check voltage limits
    if (pack.cellHiVolt > getParameter(PARAM_MAX_CELL_VOLTAGE)) {
        triggerFault(FAULT_OVERVOLTAGE);
    }
    if (pack.cellLoVolt < getParameter(PARAM_MIN_CELL_VOLTAGE)) {
        triggerFault(FAULT_UNDERVOLTAGE);
    }
    
    // Check temperature limits
    if (pack.cellHiTemp > getParameter(PARAM_MAX_CELL_TEMP)) {
        triggerFault(FAULT_OVERTEMPERATURE);
    }
    if (pack.cellLoTemp < getParameter(PARAM_MIN_CELL_TEMP)) {
        triggerFault(FAULT_UNDERTEMPERATURE);
    }
    
    // Check current limits
    if (pack.current > getParameter(PARAM_MAX_CHARGE_CURRENT)) {
        triggerFault(FAULT_OVERCURRENT_CHARGE);
    }
    if (pack.current < -getParameter(PARAM_MAX_DISCHARGE_CURRENT)) {
        triggerFault(FAULT_OVERCURRENT_DISCHARGE);
    }
    
    // Check communication timeouts
    checkCommunicationTimeouts();
    
    // Check isolation
    if (pack.isolation < getParameter(PARAM_MIN_ISOLATION)) {
        triggerFault(FAULT_ISOLATION);
    }
}

void triggerFault(FaultType fault) {
    // Set fault flag
    pack.faults |= (1 << fault);
    
    // Take protective action
    switch (fault) {
        case FAULT_OVERVOLTAGE:
        case FAULT_OVERTEMPERATURE:
        case FAULT_OVERCURRENT_CHARGE:
            // Stop charging immediately
            pack.maxChargeA = 0;
            if (pack.state == packOn && pack.current > 0) {
                // Emergency transition to standby
                pack.vcuRequestedState = packStandby;
                pack.vcuStateChange = true;
            }
            break;
            
        case FAULT_UNDERVOLTAGE:
        case FAULT_OVERCURRENT_DISCHARGE:
            // Stop discharging immediately
            pack.maxDischargeA = 0;
            if (pack.state == packOn && pack.current < 0) {
                // Emergency transition to standby
                pack.vcuRequestedState = packStandby;
                pack.vcuStateChange = true;
            }
            break;
            
        case FAULT_ISOLATION:
        case FAULT_COMMUNICATION:
            // Emergency shutdown
            pack.vcuRequestedState = packOff;
            pack.vcuStateChange = true;
            break;
    }
    
    // Log fault
    logFault(fault, HAL_GetTick());
    
    // Notify VCU
    vcuSendFaultNotification(fault);
}
```

## Communication Systems

### CAN Protocol Implementation

#### Message Transmission:
```c
HAL_StatusTypeDef canSendMessage(uint32_t id, uint8_t* data, uint8_t length) {
    CAN_TxHeaderTypeDef txHeader;
    uint32_t txMailbox;
    
    // Configure message header
    txHeader.StdId = id;
    txHeader.ExtId = 0;
    txHeader.RTR = CAN_RTR_DATA;
    txHeader.IDE = CAN_ID_STD;
    txHeader.DLC = length;
    txHeader.TransmitGlobalTime = DISABLE;
    
    // Send message
    return HAL_CAN_AddTxMessage(&hcan1, &txHeader, data, &txMailbox);
}

void vcuSendPackStatus(void) {
    uint8_t data[8];
    
    // Pack status message (ID 0x410)
    data[0] = pack.state;
    data[1] = pack.status;
    data[2] = pack.activeModules;
    data[3] = pack.faults & 0xFF;
    data[4] = (pack.faults >> 8) & 0xFF;
    data[5] = pack.soc;
    data[6] = pack.soh;
    data[7] = 0; // Reserved
    
    canSendMessage(ID_BMS_STATE, data, 8);
}

void vcuSendTelemetryData(void) {
    uint8_t data[8];
    
    // Data frame 1: Voltage and current
    uint16_t voltage = (uint16_t)(pack.voltage / MODULE_VOLTAGE_FACTOR);
    uint16_t current = (uint16_t)((pack.current - MODULE_CURRENT_BASE) / MODULE_CURRENT_FACTOR);
    
    data[0] = voltage & 0xFF;
    data[1] = (voltage >> 8) & 0xFF;
    data[2] = current & 0xFF;
    data[3] = (current >> 8) & 0xFF;
    data[4] = pack.soc * 2;  // 0.5% resolution
    data[5] = pack.soh * 2;  // 0.5% resolution
    data[6] = 0;
    data[7] = 0;
    
    canSendMessage(ID_BMS_DATA_1, data, 8);
    
    // Additional data frames...
    // (Temperature data, limits, etc.)
}
```

### Module Communication Protocol

#### UART-based Module Interface:
```c
void moduleRequestStatus(uint8_t moduleId) {
    uint8_t command[4];
    
    // Module status request protocol
    command[0] = 0xAA;              // Sync byte
    command[1] = moduleId;          // Module address
    command[2] = CMD_STATUS_REQUEST; // Command
    command[3] = calculateChecksum(command, 3);
    
    HAL_UART_Transmit(&huart1, command, 4, 100);
}

void moduleProcessResponse(uint8_t* data, uint16_t length) {
    if (length < 8 || data[0] != 0xAA) {
        return; // Invalid frame
    }
    
    uint8_t moduleId = data[1];
    uint8_t command = data[2];
    
    if (validateChecksum(data, length)) {
        switch (command) {
            case CMD_STATUS_RESPONSE:
                // Extract module data
                modules[moduleId].voltage = (data[4] << 8) | data[3];
                modules[moduleId].current = (data[6] << 8) | data[5];
                modules[moduleId].soc = data[7];
                modules[moduleId].soh = data[8];
                modules[moduleId].status = data[9];
                modules[moduleId].lastContact = HAL_GetTick();
                break;
                
            case CMD_CELL_DATA:
                // Process individual cell data
                processCellData(moduleId, &data[3], length - 4);
                break;
        }
    }
}
```

## EEPROM Management

### Parameter Storage System

The Pack Controller uses STM32's flash memory to emulate EEPROM for configuration parameter storage:

#### EEPROM Emulation:
```c
// EEPROM data structure
typedef struct {
    uint16_t parameterId;
    uint32_t value;
    uint32_t checksum;
} EEPROMParameter;

// Parameter definitions
enum ParameterID {
    PARAM_MAX_CHARGE_CURRENT = 0x0001,
    PARAM_MAX_DISCHARGE_CURRENT = 0x0002,
    PARAM_MAX_CELL_VOLTAGE = 0x0003,
    PARAM_MIN_CELL_VOLTAGE = 0x0004,
    PARAM_MAX_CELL_TEMP = 0x0005,
    PARAM_MIN_CELL_TEMP = 0x0006,
    PARAM_BALANCE_THRESHOLD = 0x0007,
    PARAM_ISOLATION_THRESHOLD = 0x0008,
    // ... additional parameters
};

HAL_StatusTypeDef writeParameter(uint16_t paramId, uint32_t value) {
    // Validate parameter
    if (!validateParameter(paramId, value)) {
        return HAL_ERROR;
    }
    
    // Write to EEPROM emulation
    return EE_WriteVariable(paramId, value);
}

uint32_t readParameter(uint16_t paramId) {
    uint32_t value;
    
    if (EE_ReadVariable(paramId, &value) == EE_OK) {
        return value;
    } else {
        // Return default value
        return getDefaultParameter(paramId);
    }
}

uint32_t getDefaultParameter(uint16_t paramId) {
    switch (paramId) {
        case PARAM_MAX_CHARGE_CURRENT:   return 50000;  // 50A
        case PARAM_MAX_DISCHARGE_CURRENT: return 100000; // 100A
        case PARAM_MAX_CELL_VOLTAGE:     return 4200;   // 4.2V
        case PARAM_MIN_CELL_VOLTAGE:     return 2500;   // 2.5V
        case PARAM_MAX_CELL_TEMP:        return 6000;   // 60°C
        case PARAM_MIN_CELL_TEMP:        return -2000;  // -20°C
        default: return 0;
    }
}
```

### Configuration Management:
```c
void loadConfiguration(void) {
    // Initialize EEPROM emulation
    if (EE_Init() != EE_OK) {
        // Format if necessary
        EE_Format();
        EE_Init();
        
        // Load default parameters
        loadDefaultParameters();
    }
    
    // Read all parameters
    for (int i = 0; i < PARAM_COUNT; i++) {
        uint16_t paramId = parameterList[i];
        uint32_t value = readParameter(paramId);
        setSystemParameter(paramId, value);
    }
}

void loadDefaultParameters(void) {
    // Write default values to EEPROM
    for (int i = 0; i < PARAM_COUNT; i++) {
        uint16_t paramId = parameterList[i];
        uint32_t defaultValue = getDefaultParameter(paramId);
        writeParameter(paramId, defaultValue);
    }
}
```

## Safety Systems

### State Machine Implementation

#### Pack State Management:
```c
typedef enum {
    STATE_OFF = 0,
    STATE_STANDBY = 1,
    STATE_PRECHARGE = 2,
    STATE_ON = 3
} PackState;

void processStateChange(void) {
    if (pack.vcuStateChange) {
        PackState currentState = pack.state;
        PackState requestedState = pack.vcuRequestedState;
        
        // Validate state transition
        if (validateStateTransition(currentState, requestedState)) {
            switch (requestedState) {
                case STATE_OFF:
                    transitionToOff();
                    break;
                case STATE_STANDBY:
                    transitionToStandby();
                    break;
                case STATE_PRECHARGE:
                    transitionToPrecharge();
                    break;
                case STATE_ON:
                    transitionToOn();
                    break;
            }
        }
        
        pack.vcuStateChange = false;
    }
}

bool validateStateTransition(PackState from, PackState to) {
    // Define allowed transitions
    switch (from) {
        case STATE_OFF:
            return (to == STATE_STANDBY);
        case STATE_STANDBY:
            return (to == STATE_OFF || to == STATE_PRECHARGE);
        case STATE_PRECHARGE:
            return (to == STATE_STANDBY || to == STATE_ON);
        case STATE_ON:
            return (to == STATE_STANDBY || to == STATE_OFF);
        default:
            return false;
    }
}

void transitionToOn(void) {
    // Safety checks before turning on
    if (pack.faults != 0) {
        // Cannot turn on with active faults
        return;
    }
    
    if (pack.cellHiVolt > getParameter(PARAM_MAX_CELL_VOLTAGE) ||
        pack.cellLoVolt < getParameter(PARAM_MIN_CELL_VOLTAGE)) {
        // Voltage out of range
        triggerFault(FAULT_VOLTAGE_RANGE);
        return;
    }
    
    // Enable all modules
    for (int i = 0; i < pack.moduleCount; i++) {
        moduleSetState(i, moduleOn);
    }
    
    // Update pack state
    pack.state = STATE_ON;
    pack.status = packStatusNormal;
    
    // Calculate operating limits
    calculateOperatingLimits();
}
```

### Watchdog and Monitoring:
```c
void backgroundTasks(void) {
    static uint32_t lastWatchdogRefresh = 0;
    static uint32_t lastSelfTest = 0;
    
    uint32_t currentTime = HAL_GetTick();
    
    // Refresh watchdog timer
    if (currentTime - lastWatchdogRefresh > 500) {
        HAL_IWDG_Refresh(&hiwdg);
        lastWatchdogRefresh = currentTime;
    }
    
    // Periodic self-test
    if (currentTime - lastSelfTest > 10000) {
        performSelfTest();
        lastSelfTest = currentTime;
    }
    
    // Monitor stack usage
    checkStackUsage();
    
    // Monitor memory corruption
    checkMemoryIntegrity();
}

void performSelfTest(void) {
    // Test CAN communication
    if (!testCANCommunication()) {
        triggerFault(FAULT_SELF_TEST_CAN);
    }
    
    // Test EEPROM
    if (!testEEPROM()) {
        triggerFault(FAULT_SELF_TEST_EEPROM);
    }
    
    // Test ADC
    if (!testADC()) {
        triggerFault(FAULT_SELF_TEST_ADC);
    }
    
    // Test timers
    if (!testTimers()) {
        triggerFault(FAULT_SELF_TEST_TIMER);
    }
}
```

## Development Environment

### STM32CubeIDE Setup

#### Project Configuration:
- **Toolchain**: GCC for ARM Embedded
- **Debug Interface**: ST-Link SWD
- **Optimization**: -O2 for release, -Og for debug
- **C Standard**: C99
- **Floating Point**: Hardware FPU

#### Build Configuration:
```makefile
# Compiler flags
CFLAGS = -mcpu=cortex-m4 -mthumb -mfpu=fpv4-sp-d16 -mfloat-abi=hard
CFLAGS += -DUSE_HAL_DRIVER -DSTM32WB55xx
CFLAGS += -Wall -Wextra -Werror
CFLAGS += -fdata-sections -ffunction-sections

# Linker flags
LDFLAGS = -mcpu=cortex-m4 -mthumb -mfpu=fpv4-sp-d16 -mfloat-abi=hard
LDFLAGS += -Wl,--gc-sections -static
LDFLAGS += -Wl,--start-group -lc -lm -Wl,--end-group

# Debug configuration
DEBUG_FLAGS = -g3 -DDEBUG -O0

# Release configuration
RELEASE_FLAGS = -O2 -DNDEBUG
```

#### Memory Layout:
```
STM32WB55RG Memory Map:
├── Flash Memory (1MB)
│   ├── Application Code (800KB)
│   ├── EEPROM Emulation (32KB)
│   ├── Bootloader Space (64KB)
│   └── Reserved (128KB)
├── SRAM1 (192KB)
│   ├── Application Data
│   ├── Stack (8KB)
│   └── Heap (16KB)
├── SRAM2a (32KB)
│   └── Shared with RF stack
└── SRAM2b (32KB)
    └── Secure area
```

The Pack Controller firmware provides the central intelligence for modular battery pack management, implementing robust safety systems, efficient communication protocols, and comprehensive monitoring capabilities.