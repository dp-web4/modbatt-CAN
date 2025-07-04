# Data Management and Persistence

## Table of Contents

1. [Overview](#overview)
2. [Data Architecture](#data-architecture)
3. [EEPROM Emulation](#eeprom-emulation)
4. [Parameter Management](#parameter-management)
5. [Data Validation](#data-validation)
6. [Backup and Recovery](#backup-and-recovery)
7. [Data Synchronization](#data-synchronization)
8. [Development Patterns](#development-patterns)

## Overview

The ModBatt CAN system implements a comprehensive data management strategy that ensures reliable storage, validation, and synchronization of configuration parameters and operational data across the distributed battery management architecture.

### Key Components

**Pack Controller EEPROM Emulation**
- STM32 flash-based parameter storage
- Wear leveling and corruption protection
- Factory default fallback mechanisms

**Windows Application Data Management**
- Real-time data processing and display
- Parameter configuration interface
- Historical data logging capabilities

**Module-Level Data Storage**
- Distributed parameter storage
- Local configuration management
- Fault-tolerant operation

## Data Architecture

### Hierarchical Data Model

```
Data Management Hierarchy:
┌─────────────────────────────────────────────────────────────────┐
│                       System Level                             │
│  ┌─────────────────────────────────────────────────────────┐   │
│  │                 Pack Controller                         │   │
│  │  • Global system parameters                            │   │
│  │  • Pack-level operational limits                       │   │
│  │  • Inter-module coordination settings                  │   │
│  └─────────────────┬───────────────────────────────────────┘   │
└────────────────────┼───────────────────────────────────────────┘
                     │
    ┌────────────────┼────────────────┬───────────────────────────┐
    │                │                │                           │
┌───▼────────┐ ┌──────▼──────┐ ┌──────▼──────┐ ┌─────────────────▼──┐
│  Module 1  │ │  Module 2   │ │  Module 3   │ │    Module N        │
│  • Cell    │ │  • Cell     │ │  • Cell     │ │    • Cell          │
│    config  │ │    config   │ │    config   │ │      config        │
│  • Local   │ │  • Local    │ │  • Local    │ │    • Local         │
│    limits  │ │    limits   │ │    limits   │ │      limits        │
│  • Cal     │ │  • Cal      │ │  • Cal      │ │    • Cal           │
│    data    │ │    data     │ │    data     │ │      data          │
└────────────┘ └─────────────┘ └─────────────┘ └────────────────────┘
```

### Data Categories

#### System Configuration Parameters
```c
// Global system settings
struct SystemConfig {
    uint16_t maxPackVoltage;        // Maximum allowed pack voltage
    uint16_t minPackVoltage;        // Minimum allowed pack voltage
    uint16_t maxChargeCurrent;      // Maximum charge current limit
    uint16_t maxDischargeCurrent;   // Maximum discharge current limit
    uint16_t maxTemperature;        // Maximum operating temperature
    uint16_t minTemperature;        // Minimum operating temperature
    uint16_t balanceThreshold;      // Cell voltage balance threshold
    uint16_t isolationThreshold;    // Minimum isolation resistance
    uint32_t serialNumber;          // System serial number
    uint32_t firmwareVersion;       // Firmware version identifier
};
```

#### Pack-Level Operational Data
```c
// Real-time pack data
struct PackData {
    float voltage;                  // Total pack voltage (V)
    float current;                  // Pack current (A, + = charge)
    uint8_t stateOfCharge;         // Pack SOC (0-100%)
    uint8_t stateOfHealth;         // Pack SOH (0-100%)
    packState currentState;         // Pack operational state
    bmsStatus status;              // Pack status flags
    uint16_t faultCodes;           // Active fault conditions
    uint8_t activeModules;         // Number of active modules
    uint32_t timestamp;            // Data timestamp
};
```

#### Module-Level Configuration
```c
// Module-specific parameters
struct ModuleConfig {
    uint8_t moduleId;              // Module identifier (0-31)
    uint8_t cellCount;             // Number of cells in module
    uint16_t nominalCapacity;      // Module capacity (mAh)
    uint16_t maxCellVoltage;       // Cell overvoltage protection
    uint16_t minCellVoltage;       // Cell undervoltage protection
    uint16_t balanceCurrent;       // Cell balancing current
    uint16_t thermalLimit;         // Thermal shutdown threshold
    uint32_t calibrationData[16];  // ADC calibration coefficients
    uint32_t manufacturingDate;    // Module manufacture date
    uint32_t checksum;             // Configuration integrity check
};
```

## EEPROM Emulation

### STM32 Flash-Based Storage

The Pack Controller implements EEPROM emulation using STM32 flash memory to provide non-volatile parameter storage:

#### Memory Organization
```
STM32WB55 Flash Layout (1MB total):
├── Application Code (768KB)
│   ├── Main application firmware
│   ├── HAL and middleware libraries
│   └── Protocol stacks
├── EEPROM Emulation Area (32KB)
│   ├── Page 0: Active parameter storage
│   ├── Page 1: Backup parameter storage
│   └── Reserved for wear leveling
├── Bootloader (64KB)
│   ├── Firmware update mechanism
│   └── Recovery procedures
└── Reserved (160KB)
    ├── Future expansion
    └── System use
```

#### Implementation Details
```c
// EEPROM emulation constants
#define FLASH_PAGE_SIZE         0x1000      // 4KB per page
#define EEPROM_START_ADDRESS    0x08070000  // Start of EEPROM area
#define EEPROM_PAGE_SIZE        FLASH_PAGE_SIZE
#define EEPROM_PAGE_COUNT       8           // 32KB total
#define MAX_EEPROM_VARIABLES    64          // Maximum parameters

// Parameter storage structure
typedef struct {
    uint16_t virtualAddress;     // Parameter identifier
    uint16_t data;              // Parameter value
    uint32_t crc;               // Data integrity check
} EEPROMVariable;

// EEPROM status enumeration
typedef enum {
    EE_OK = 0,
    EE_ERROR_NOACTIVE_PAGE,
    EE_ERROR_NOVALID_PAGE,
    EE_ERROR_WRITE,
    EE_ERROR_ERASE,
    EE_ERROR_READ,
    EE_ERROR_CORRUPT
} EEPROMStatus;
```

#### Core EEPROM Functions
```c
EEPROMStatus EE_Init(void) {
    // Initialize EEPROM emulation
    // Check page validity
    // Perform recovery if needed
    // Set up active page
}

EEPROMStatus EE_WriteVariable(uint16_t virtualAddress, uint16_t data) {
    // Find parameter location
    // Write new value with CRC
    // Handle page full condition
    // Trigger garbage collection if needed
}

EEPROMStatus EE_ReadVariable(uint16_t virtualAddress, uint16_t* data) {
    // Search for latest parameter value
    // Verify CRC integrity
    // Return current value or error
}

EEPROMStatus EE_Format(void) {
    // Erase all EEPROM pages
    // Initialize first page as active
    // Set page header information
}
```

### Wear Leveling Strategy

```c
void EE_PageTransfer(uint16_t virtualAddress, uint16_t data) {
    // Current page is full, transfer to new page
    uint32_t newPageAddress = GetNextAvailablePage();
    
    // Copy all valid variables to new page
    for (uint16_t varIndex = 0; varIndex < MAX_EEPROM_VARIABLES; varIndex++) {
        uint16_t currentValue;
        if (EE_ReadVariable(varIndex, &currentValue) == EE_OK) {
            WriteVariableToPage(newPageAddress, varIndex, currentValue);
        }
    }
    
    // Add new variable
    WriteVariableToPage(newPageAddress, virtualAddress, data);
    
    // Mark new page as active
    SetPageStatus(newPageAddress, PAGE_STATUS_ACTIVE);
    
    // Erase old page
    EraseOldPage();
}
```

## Parameter Management

### Parameter Definition System

```c
// Parameter ID enumeration
typedef enum {
    // Current limits
    PARAM_MAX_CHARGE_CURRENT     = 0x0001,
    PARAM_MAX_DISCHARGE_CURRENT  = 0x0002,
    
    // Voltage limits  
    PARAM_MAX_CELL_VOLTAGE       = 0x0003,
    PARAM_MIN_CELL_VOLTAGE       = 0x0004,
    PARAM_PACK_OVERVOLTAGE       = 0x0005,
    PARAM_PACK_UNDERVOLTAGE      = 0x0006,
    
    // Temperature limits
    PARAM_MAX_CELL_TEMP          = 0x0007,
    PARAM_MIN_CELL_TEMP          = 0x0008,
    PARAM_THERMAL_DERATING       = 0x0009,
    
    // Balancing parameters
    PARAM_BALANCE_THRESHOLD      = 0x000A,
    PARAM_BALANCE_CURRENT        = 0x000B,
    PARAM_BALANCE_TIMEOUT        = 0x000C,
    
    // System configuration
    PARAM_MODULE_COUNT           = 0x000D,
    PARAM_CELLS_PER_MODULE       = 0x000E,
    PARAM_NOMINAL_CAPACITY       = 0x000F,
    
    // Safety parameters
    PARAM_ISOLATION_THRESHOLD    = 0x0010,
    PARAM_CONTACTOR_DELAY        = 0x0011,
    PARAM_PRECHARGE_TIMEOUT      = 0x0012,
    
    // End marker
    PARAM_COUNT
} ParameterID;

// Parameter metadata structure
typedef struct {
    ParameterID id;              // Parameter identifier
    uint32_t defaultValue;       // Factory default value
    uint32_t minValue;           // Minimum allowed value
    uint32_t maxValue;           // Maximum allowed value
    uint16_t units;              // Units (mV, mA, degC, etc.)
    uint16_t resolution;         // Value resolution/scaling
    bool writeProtected;         // Write protection flag
    const char* description;     // Human-readable description
} ParameterDefinition;
```

### Parameter Validation
```c
bool ValidateParameter(ParameterID paramId, uint32_t value) {
    const ParameterDefinition* param = GetParameterDefinition(paramId);
    
    if (param == NULL) {
        return false;  // Unknown parameter
    }
    
    // Range check
    if (value < param->minValue || value > param->maxValue) {
        return false;  // Out of range
    }
    
    // Cross-parameter validation
    switch (paramId) {
        case PARAM_MAX_CELL_VOLTAGE:
            // Must be greater than minimum cell voltage
            uint32_t minVoltage;
            if (GetParameter(PARAM_MIN_CELL_VOLTAGE, &minVoltage)) {
                if (value <= minVoltage) {
                    return false;
                }
            }
            break;
            
        case PARAM_MAX_CHARGE_CURRENT:
            // Must be reasonable compared to pack capacity
            uint32_t capacity;
            if (GetParameter(PARAM_NOMINAL_CAPACITY, &capacity)) {
                uint32_t maxReasonableCurrent = capacity * 3; // 3C max
                if (value > maxReasonableCurrent) {
                    return false;
                }
            }
            break;
    }
    
    return true;
}
```

### Factory Defaults Management
```c
// Factory default parameter table
const ParameterDefinition factoryDefaults[] = {
    {PARAM_MAX_CHARGE_CURRENT,    50000,    0,   200000, UNITS_MA,   100, false, "Maximum Charge Current"},
    {PARAM_MAX_DISCHARGE_CURRENT, 100000,   0,   300000, UNITS_MA,   100, false, "Maximum Discharge Current"},
    {PARAM_MAX_CELL_VOLTAGE,      4200,     3000, 4500,  UNITS_MV,   1,   false, "Maximum Cell Voltage"},
    {PARAM_MIN_CELL_VOLTAGE,      2500,     2000, 3500,  UNITS_MV,   1,   false, "Minimum Cell Voltage"},
    {PARAM_MAX_CELL_TEMP,         6000,     4000, 8000,  UNITS_DC,   10,  false, "Maximum Cell Temperature"},
    {PARAM_MIN_CELL_TEMP,         -2000,    -4000, 0,    UNITS_DC,   10,  false, "Minimum Cell Temperature"},
    {PARAM_BALANCE_THRESHOLD,     50,       10,   200,   UNITS_MV,   1,   false, "Balance Threshold"},
    {PARAM_ISOLATION_THRESHOLD,   500,      100,  2000,  UNITS_KOHM, 1,   true,  "Isolation Threshold"},
    // ... additional parameters
};

void LoadFactoryDefaults(void) {
    // Write all factory defaults to EEPROM
    for (int i = 0; i < PARAM_COUNT; i++) {
        EE_WriteVariable(factoryDefaults[i].id, factoryDefaults[i].defaultValue);
    }
    
    // Set factory reset flag
    EE_WriteVariable(PARAM_FACTORY_RESET_FLAG, 0xA5A5);
}
```

## Data Validation

### Integrity Checking

```c
// CRC calculation for data integrity
uint32_t CalculateParameterCRC(ParameterID paramId, uint32_t value) {
    uint32_t crc = 0xFFFFFFFF;
    
    // Include parameter ID in CRC
    crc = CRC32_Update(crc, (uint8_t*)&paramId, sizeof(paramId));
    
    // Include parameter value in CRC
    crc = CRC32_Update(crc, (uint8_t*)&value, sizeof(value));
    
    // Include magic constant
    uint32_t magic = 0x12345678;
    crc = CRC32_Update(crc, (uint8_t*)&magic, sizeof(magic));
    
    return crc;
}

bool VerifyParameterIntegrity(ParameterID paramId, uint32_t value, uint32_t storedCRC) {
    uint32_t calculatedCRC = CalculateParameterCRC(paramId, value);
    return (calculatedCRC == storedCRC);
}
```

### Cross-Parameter Validation
```c
bool ValidateSystemConfiguration(void) {
    uint32_t maxCellV, minCellV, maxPackV, minPackV;
    uint32_t moduleCount, cellsPerModule;
    
    // Read critical parameters
    if (!GetParameter(PARAM_MAX_CELL_VOLTAGE, &maxCellV) ||
        !GetParameter(PARAM_MIN_CELL_VOLTAGE, &minCellV) ||
        !GetParameter(PARAM_PACK_OVERVOLTAGE, &maxPackV) ||
        !GetParameter(PARAM_PACK_UNDERVOLTAGE, &minPackV) ||
        !GetParameter(PARAM_MODULE_COUNT, &moduleCount) ||
        !GetParameter(PARAM_CELLS_PER_MODULE, &cellsPerModule)) {
        return false;  // Cannot read required parameters
    }
    
    // Validate voltage relationships
    if (maxCellV <= minCellV) {
        return false;  // Invalid cell voltage range
    }
    
    uint32_t expectedMaxPackV = maxCellV * cellsPerModule * moduleCount;
    uint32_t expectedMinPackV = minCellV * cellsPerModule * moduleCount;
    
    if (maxPackV > expectedMaxPackV * 1.1 || maxPackV < expectedMaxPackV * 0.9) {
        return false;  // Pack voltage limits inconsistent with cell limits
    }
    
    if (minPackV > expectedMinPackV * 1.1 || minPackV < expectedMinPackV * 0.9) {
        return false;  // Pack voltage limits inconsistent with cell limits
    }
    
    return true;
}
```

## Backup and Recovery

### Automatic Backup Strategy

```c
typedef enum {
    BACKUP_TRIGGER_PARAMETER_CHANGE,
    BACKUP_TRIGGER_FIRMWARE_UPDATE,
    BACKUP_TRIGGER_PERIODIC,
    BACKUP_TRIGGER_MANUAL
} BackupTrigger;

void CreateParameterBackup(BackupTrigger trigger) {
    BackupHeader backup;
    
    // Fill backup header
    backup.magic = BACKUP_MAGIC_NUMBER;
    backup.version = BACKUP_FORMAT_VERSION;
    backup.timestamp = HAL_GetTick();
    backup.trigger = trigger;
    backup.parameterCount = PARAM_COUNT;
    
    // Calculate backup address
    uint32_t backupAddress = GetNextBackupSlot();
    
    // Write backup header
    WriteFlash(backupAddress, &backup, sizeof(backup));
    backupAddress += sizeof(backup);
    
    // Write all parameters
    for (ParameterID paramId = 0; paramId < PARAM_COUNT; paramId++) {
        uint32_t value;
        if (GetParameter(paramId, &value)) {
            ParameterBackup paramBackup;
            paramBackup.id = paramId;
            paramBackup.value = value;
            paramBackup.crc = CalculateParameterCRC(paramId, value);
            
            WriteFlash(backupAddress, &paramBackup, sizeof(paramBackup));
            backupAddress += sizeof(paramBackup);
        }
    }
    
    // Finalize backup
    FinalizeBackup();
}
```

### Recovery Procedures

```c
bool RestoreFromBackup(uint32_t backupTimestamp) {
    // Find backup by timestamp
    BackupHeader* backup = FindBackup(backupTimestamp);
    if (backup == NULL) {
        return false;
    }
    
    // Validate backup integrity
    if (!ValidateBackupIntegrity(backup)) {
        return false;
    }
    
    // Restore parameters
    uint32_t paramAddress = (uint32_t)backup + sizeof(BackupHeader);
    
    for (int i = 0; i < backup->parameterCount; i++) {
        ParameterBackup* paramBackup = (ParameterBackup*)paramAddress;
        
        // Verify parameter integrity
        if (VerifyParameterIntegrity(paramBackup->id, paramBackup->value, paramBackup->crc)) {
            // Write parameter to EEPROM
            EE_WriteVariable(paramBackup->id, paramBackup->value);
        }
        
        paramAddress += sizeof(ParameterBackup);
    }
    
    // Validate restored configuration
    if (!ValidateSystemConfiguration()) {
        // Restore failed, load factory defaults
        LoadFactoryDefaults();
        return false;
    }
    
    return true;
}
```

## Data Synchronization

### Pack-to-Module Synchronization

```c
void SynchronizeModuleParameters(uint8_t moduleId) {
    ModuleConfig config;
    
    // Build module configuration from pack parameters
    config.moduleId = moduleId;
    GetParameter(PARAM_CELLS_PER_MODULE, &config.cellCount);
    GetParameter(PARAM_MAX_CELL_VOLTAGE, &config.maxCellVoltage);
    GetParameter(PARAM_MIN_CELL_VOLTAGE, &config.minCellVoltage);
    GetParameter(PARAM_BALANCE_CURRENT, &config.balanceCurrent);
    GetParameter(PARAM_MAX_CELL_TEMP, &config.thermalLimit);
    
    // Calculate configuration checksum
    config.checksum = CalculateConfigChecksum(&config);
    
    // Send configuration to module
    SendModuleConfiguration(moduleId, &config);
    
    // Wait for acknowledgment
    if (WaitForModuleAck(moduleId, CONFIG_ACK_TIMEOUT)) {
        MarkModuleConfigured(moduleId);
    } else {
        RetryModuleConfiguration(moduleId);
    }
}
```

### Version Management

```c
typedef struct {
    uint16_t major;
    uint16_t minor;
    uint16_t patch;
    uint32_t build;
} Version;

bool CheckParameterCompatibility(Version firmwareVersion, Version parameterVersion) {
    // Major version must match
    if (firmwareVersion.major != parameterVersion.major) {
        return false;
    }
    
    // Minor version backward compatibility
    if (firmwareVersion.minor < parameterVersion.minor) {
        return false;
    }
    
    return true;
}
```

## Development Patterns

### Thread-Safe Parameter Access

```c
// Critical section for parameter operations
static volatile bool parameterLock = false;

bool AcquireParameterLock(uint32_t timeout) {
    uint32_t startTime = HAL_GetTick();
    
    while (parameterLock) {
        if ((HAL_GetTick() - startTime) > timeout) {
            return false;  // Timeout
        }
        HAL_Delay(1);
    }
    
    __disable_irq();
    if (!parameterLock) {
        parameterLock = true;
        __enable_irq();
        return true;
    }
    __enable_irq();
    
    return false;
}

void ReleaseParameterLock(void) {
    __disable_irq();
    parameterLock = false;
    __enable_irq();
}

// Thread-safe parameter read
bool GetParameterSafe(ParameterID paramId, uint32_t* value) {
    if (!AcquireParameterLock(PARAMETER_LOCK_TIMEOUT)) {
        return false;
    }
    
    bool result = GetParameter(paramId, value);
    
    ReleaseParameterLock();
    return result;
}
```

### Error Handling Patterns

```c
typedef enum {
    PARAM_ERROR_OK = 0,
    PARAM_ERROR_INVALID_ID,
    PARAM_ERROR_OUT_OF_RANGE,
    PARAM_ERROR_WRITE_PROTECTED,
    PARAM_ERROR_EEPROM_FAILURE,
    PARAM_ERROR_VALIDATION_FAILED,
    PARAM_ERROR_TIMEOUT
} ParameterError;

ParameterError SetParameterWithValidation(ParameterID paramId, uint32_t value) {
    // Input validation
    if (paramId >= PARAM_COUNT) {
        return PARAM_ERROR_INVALID_ID;
    }
    
    // Range validation
    if (!ValidateParameter(paramId, value)) {
        return PARAM_ERROR_OUT_OF_RANGE;
    }
    
    // Write protection check
    const ParameterDefinition* paramDef = GetParameterDefinition(paramId);
    if (paramDef->writeProtected) {
        return PARAM_ERROR_WRITE_PROTECTED;
    }
    
    // Acquire lock
    if (!AcquireParameterLock(PARAMETER_LOCK_TIMEOUT)) {
        return PARAM_ERROR_TIMEOUT;
    }
    
    // Write to EEPROM
    EEPROMStatus status = EE_WriteVariable(paramId, value);
    if (status != EE_OK) {
        ReleaseParameterLock();
        return PARAM_ERROR_EEPROM_FAILURE;
    }
    
    // Cross-parameter validation
    if (!ValidateSystemConfiguration()) {
        // Rollback change
        uint32_t previousValue;
        if (GetParameterFromBackup(paramId, &previousValue)) {
            EE_WriteVariable(paramId, previousValue);
        }
        
        ReleaseParameterLock();
        return PARAM_ERROR_VALIDATION_FAILED;
    }
    
    ReleaseParameterLock();
    
    // Create backup after successful change
    CreateParameterBackup(BACKUP_TRIGGER_PARAMETER_CHANGE);
    
    return PARAM_ERROR_OK;
}
```

This comprehensive data management system ensures reliable, validated, and synchronized parameter storage across the distributed ModBatt architecture while maintaining data integrity and providing robust recovery mechanisms.