# Windows Configuration Tool

## Table of Contents

1. [Overview](#overview)
2. [Architecture](#architecture)
3. [Core Components](#core-components)
4. [User Interface](#user-interface)
5. [PCAN Integration](#pcan-integration)
6. [Message Processing](#message-processing)
7. [Data Management](#data-management)
8. [Configuration Features](#configuration-features)

## Overview

The ModBatt Configuration Tool is a Windows desktop application built using Borland C++ Builder (Embarcadero RAD Studio). It provides a comprehensive interface for monitoring, configuring, and diagnosing modular battery systems through CAN bus communication.

### Key Features
- **Real-time Monitoring**: Live battery pack and module telemetry
- **Configuration Management**: EEPROM parameter programming
- **Diagnostic Tools**: CAN message analysis and fault detection
- **Data Logging**: Historical data capture and analysis
- **Multi-pack Support**: Monitor multiple battery packs simultaneously

### Technology Stack
- **Framework**: Borland C++ Builder / Embarcadero RAD Studio
- **Language**: C++ with Visual Component Library (VCL)
- **CAN Interface**: PEAK PCAN-Basic API
- **Platform**: Windows 7/8/10/11 (32-bit and 64-bit)

## Architecture

### Application Structure

```
ModBatt Configuration Tool
├── Presentation Layer (VCL Forms)
│   ├── Main Form (TForm1)
│   ├── Dialog Forms
│   └── Custom Controls
├── Business Logic Layer
│   ├── Message Processing
│   ├── Data Analysis
│   ├── Protocol Handling
│   └── Configuration Management
├── Hardware Abstraction Layer
│   ├── PCAN Interface (PCANBasicClass)
│   ├── Message Queues
│   ├── Threading Services
│   └── Timer Management
└── PCAN Driver Layer
    └── PCANBasic.dll
```

### File Organization

```
Modbatt Configuration Tool/
├── Source Files
│   ├── modbatt.cpp          # Main application entry point
│   ├── Unit1.cpp            # Main form implementation
│   ├── Unit1.h              # Main form header
│   ├── Unit1.dfm            # Form designer file
│   ├── PCANBasicClass.cpp   # PCAN wrapper implementation
│   └── PCANBasicClass.h     # PCAN wrapper header
├── Header Files
│   ├── Include/
│   │   ├── bms.h            # BMS data structures
│   │   ├── can_frm_vcu.h    # VCU frame definitions
│   │   └── can_id_bms_vcu.h # CAN ID definitions
│   └── PCANBasic.h          # PCAN API header
├── Project Files
│   ├── modbatt.cbproj       # C++ Builder project
│   ├── modbatt.ico          # Application icon
│   └── Dependencies.zip     # Required libraries
└── Build Output
    ├── Debug/               # Debug builds
    ├── Release/             # Release builds
    └── modbatt.exe          # Final executable
```

## Core Components

### 1. Main Form (TForm1)

**Primary application window managing all user interactions**

#### Key Responsibilities:
- CAN hardware connection management
- Real-time data display
- User input handling
- Message processing coordination

#### Major UI Groups:
```cpp
class TForm1 : public TForm {
    // CAN Connection Management
    TGroupBox *GroupBox1;        // Hardware selection
    TComboBox *cbbChannel;       // PCAN channel selection
    TComboBox *cbbBaudrates;     // Baudrate configuration
    TButton *btnConnect;         // Connection control
    
    // Pack Data Display
    TGroupBox *grpPackData;      // Pack-level telemetry
    TEdit *editVoltage;          // Pack voltage
    TEdit *editCurrent;          // Pack current
    TEdit *editSoc;              // State of charge
    TEdit *editSoh;              // State of health
    
    // Module Data Display
    TGroupBox *grpModuleData;    // Module-level telemetry
    TEdit *editModuleVoltage;    // Selected module voltage
    TEdit *editModuleCurrent;    // Selected module current
    TEdit *editModuleTemp;       // Module temperature
    
    // Configuration Management
    TGroupBox *grpPackParam;     // Parameter configuration
    TComboBox *cboPackParam;     // Parameter selection
    TEdit *edtPackValue;         // Parameter value
    TButton *btnPackParamSet;    // Write parameter
    TButton *btnPackParamGet;    // Read parameter
    
    // Message Display
    TListView *lstMessages;      // CAN message log
    TListBox *lbxInfo;          // Information messages
};
```

### 2. PCAN Interface (PCANBasicClass)

**Hardware abstraction layer for PCAN communication**

#### Core Functionality:
```cpp
class PCANBasicClass {
public:
    // Connection management
    TPCANStatus Initialize(TPCANHandle channel, TPCANBaudrate baudrate);
    TPCANStatus Uninitialize(TPCANHandle channel);
    
    // Message operations
    TPCANStatus Read(TPCANHandle channel, TPCANMsg* msg, TPCANTimestamp* timestamp);
    TPCANStatus ReadFD(TPCANHandle channel, TPCANMsgFD* msg, TPCANTimestampFD* timestamp);
    TPCANStatus Write(TPCANHandle channel, TPCANMsg* msg);
    TPCANStatus WriteFD(TPCANHandle channel, TPCANMsgFD* msg);
    
    // Configuration
    TPCANStatus SetValue(TPCANHandle channel, TPCANParameter parameter, void* buffer, DWORD length);
    TPCANStatus GetValue(TPCANHandle channel, TPCANParameter parameter, void* buffer, DWORD length);
    
    // Status and information
    TPCANStatus GetStatus(TPCANHandle channel);
    TPCANStatus GetErrorText(TPCANStatus error, WORD language, char* buffer);
};
```

#### Supported PCAN Hardware:
- **PCAN-USB**: USB to CAN interface
- **PCAN-PCIe**: PCIe CAN cards
- **PCAN-ISA**: Legacy ISA cards
- **PCAN-Dongle**: Parallel port interface

### 3. Message Management System

**Handles CAN message processing and display**

#### Message Status Class:
```cpp
class MessageStatus {
private:
    TPCANMsgFD m_Msg;           // CAN message data
    TPCANTimestampFD m_TimeStamp; // Message timestamp
    int m_iIndex;               // List position
    int m_Count;                // Reception count
    bool m_bShowPeriod;         // Show timing info
    bool m_bWasChanged;         // Update indicator

public:
    // Properties for display
    AnsiString TypeString;      // Frame type (STD/EXT/RTR/FD)
    AnsiString IdString;        // Message ID (hex)
    AnsiString DataString;      // Data bytes (hex)
    AnsiString TimeString;      // Timestamp or period
    
    // Methods
    void Update(TPCANMsgFD canMsg, TPCANTimestampFD canTimestamp);
    void SetShowingPeriod(bool value);
};
```

#### Message Processing Flow:
```
Received CAN Message
        ↓
Extract Message Content
        ↓
Determine Message Type
        ↓
┌─────────────┬─────────────┬─────────────┬─────────────┐
│  Pack Data  │Module Data  │ Parameters  │ Diagnostic  │
│  (0x410)    │ (0x411-417) │  (0x441)    │   Messages  │
└─────────────┴─────────────┴─────────────┴─────────────┘
        ↓           ↓           ↓           ↓
Update Pack    Update Module  Update Config  Log Info
Display        Display        Values         Messages
```

### 4. Threading System

**Manages asynchronous CAN message reception**

#### Threading Architecture:
```cpp
// Main application thread
DWORD WINAPI CallCANReadThreadFunc(LPVOID lpParam);

// Member thread function
DWORD CANReadThreadFunc() {
    while (ThreadActive) {
        // Wait for CAN events
        WaitForSingleObject(m_hEvent, INFINITE);
        
        // Process all available messages
        ReadMessages();
        
        // Update display (thread-safe)
        PostMessage(MainFormHandle, WM_UPDATE_DISPLAY, 0, 0);
    }
    return 0;
}

// Critical section protection
class clsCritical {
    CRITICAL_SECTION *m_objpCS;
    bool m_bDoRecursive;
public:
    int Enter();    // Lock critical section
    int Leave();    // Unlock critical section
};
```

## User Interface

### Main Interface Layout

```
┌─────────────────────────────────────────────────────────────────┐
│ ModBatt Configuration Tool                              [_][□][X] │
├─────────────────────────────────────────────────────────────────┤
│ Hardware Setup                                                  │
│ ┌─────────────────┬─────────────────┬─────────────────────────┐ │
│ │ Channel: USB1   │ Baudrate: 500k  │ [Connect] [Disconnect]  │ │
│ └─────────────────┴─────────────────┴─────────────────────────┘ │
├─────────────────────────────────────────────────────────────────┤
│ Pack Data                           │ Module Data               │
│ ┌─────────────────────────────────┐ │ ┌───────────────────────┐ │
│ │ Voltage:      48.2 V            │ │ │ Module: [1] ▼         │ │
│ │ Current:     -12.5 A            │ │ │ Voltage:    3.65 V    │ │
│ │ SOC:           85 %             │ │ │ Current:   -0.78 A    │ │
│ │ SOH:           96 %             │ │ │ Temp:       23.5 °C   │ │
│ │ Status:     Normal              │ │ │ SOC:          84 %    │ │
│ │ State:         On               │ │ │ Balance:    Active    │ │
│ └─────────────────────────────────┘ │ └───────────────────────┘ │
├─────────────────────────────────────┼───────────────────────────┤
│ Configuration                       │ CAN Messages              │
│ ┌─────────────────────────────────┐ │ ┌───────────────────────┐ │
│ │ Parameter: [Max Charge Current] │ │ │ ID   Data    Time     │ │
│ │ Value: 50.0 A                   │ │ │ 410  1A2B... 12:34:56│ │
│ │ [Get] [Set]                     │ │ │ 421  FF00... 12:34:57│ │
│ └─────────────────────────────────┘ │ │ 422  3456... 12:34:58│ │
│                                     │ │ [Clear] [▶] [⏸]      │ │
│                                     │ └───────────────────────┘ │
└─────────────────────────────────────┴───────────────────────────┘
```

### Key UI Components

#### Hardware Configuration Group
- **Channel Selection**: Dropdown with available PCAN channels
- **Baudrate Selection**: 125k/250k/500k/1M options
- **Hardware Type**: USB/PCIe/ISA selection
- **Connection Status**: Connected/Disconnected indicator

#### Pack Data Display Group
- **Real-time Telemetry**: Voltage, current, SOC, SOH
- **Status Information**: Pack state, operational status
- **Limit Values**: Charge/discharge limits, temperature ranges
- **Fault Indicators**: Alarm and warning status

#### Module Data Display Group
- **Module Selection**: Dropdown to select specific module
- **Module Telemetry**: Individual module measurements
- **Cell Information**: High/low/average cell voltages and temperatures
- **Balancing Status**: Active balancing indication

#### Configuration Management Group
- **Parameter Selection**: Dropdown with configurable parameters
- **Value Entry**: Numeric input with validation
- **Read/Write Controls**: Get and set parameter values
- **Status Feedback**: Operation success/failure indication

#### Message Display Area
- **Live Message Log**: Real-time CAN traffic display
- **Message Filtering**: Show/hide specific message types
- **Timing Information**: Message periods and timestamps
- **Export Capability**: Save logs for analysis

## PCAN Integration

### Hardware Detection

```cpp
void TForm1::btnHwRefreshClick(TObject *Sender) {
    // Clear existing entries
    cbbChannel->Items->Clear();
    
    // Scan for available PCAN channels
    for (int i = PCAN_USBBUS1; i <= PCAN_USBBUS16; i++) {
        TPCANStatus status = m_objPCANBasic->GetStatus((TPCANHandle)i);
        if (status == PCAN_ERROR_OK || status == PCAN_ERROR_BUSOFF) {
            // Add available channel to list
            AnsiString channelName = FormatChannelName((TPCANHandle)i);
            cbbChannel->Items->Add(channelName);
        }
    }
    
    // Check PCIe channels
    for (int i = PCAN_PCIBUS1; i <= PCAN_PCIBUS16; i++) {
        // Similar detection logic
    }
}
```

### Connection Management

```cpp
void TForm1::btnConnectClick(TObject *Sender) {
    // Get selected parameters
    TPCANHandle channel = GetSelectedChannel();
    TPCANBaudrate baudrate = GetSelectedBaudrate();
    
    // Initialize PCAN channel
    TPCANStatus status = m_objPCANBasic->Initialize(channel, baudrate);
    
    if (status == PCAN_ERROR_OK) {
        // Configure message filtering
        SetupMessageFilters();
        
        // Start reading thread
        StartCANReadThread();
        
        // Update UI state
        SetConnectionStatus(true);
        IncludeTextMessage("Connected to " + GetChannelName(channel));
    } else {
        // Handle connection error
        AnsiString error = GetFormatedError(status);
        IncludeTextMessage("Connection failed: " + error);
    }
}
```

### Message Filtering

```cpp
void TForm1::SetupMessageFilters() {
    // Accept only ModBatt protocol messages
    DWORD filter_from = 0x400;  // Start of VCU messages
    DWORD filter_to = 0x44F;    // End of BMS messages
    
    // Set acceptance filter
    m_objPCANBasic->SetValue(m_PcanHandle, PCAN_ACCEPTANCE_FILTER11, 
                            &filter_from, sizeof(filter_from));
    
    // Enable timestamp
    DWORD timestamp_on = PCAN_PARAMETER_ON;
    m_objPCANBasic->SetValue(m_PcanHandle, PCAN_MESSAGE_FILTER, 
                            &timestamp_on, sizeof(timestamp_on));
}
```

## Message Processing

### Received Message Handler

```cpp
void TForm1::ProcessMessage(TPCANMsgFD theMsg, TPCANTimestampFD itsTimeStamp) {
    // Determine message type based on ID
    switch (theMsg.ID) {
        case ID_BMS_STATE:          // 0x410
            ProcessState(theMsg);
            break;
            
        case ID_BMS_DATA_1:         // 0x421
            ProcessData1(theMsg);
            break;
            
        case ID_BMS_DATA_2:         // 0x422
            ProcessData2(theMsg);
            break;
            
        // ... additional data frames
            
        case ID_MODULE_STATE:       // 0x411
            ProcessModuleState(theMsg);
            break;
            
        case ID_MODULE_POWER:       // 0x412
            ProcessModulePower(theMsg);
            break;
            
        case ID_BMS_EEPROM_DATA:    // 0x441
            ProcessEEPROMData(theMsg);
            break;
            
        default:
            // Log unknown message
            IncludeTextMessage("Unknown message ID: 0x" + 
                              IntToHex(theMsg.ID, 3));
    }
    
    // Update message list display
    InsertMsgEntry(theMsg, itsTimeStamp);
}
```

### Pack Data Processing

```cpp
void TForm1::ProcessData1(TPCANMsgFD theMsg) {
    // Extract pack voltage (bytes 0-1)
    uint16_t voltage_raw = (theMsg.Data[1] << 8) | theMsg.Data[0];
    float voltage = voltage_raw * MODULE_VOLTAGE_FACTOR;
    editVoltage->Text = FloatToStrF(voltage, ffFixed, 15, 1) + " V";
    
    // Extract pack current (bytes 2-3)
    uint16_t current_raw = (theMsg.Data[3] << 8) | theMsg.Data[2];
    float current = (current_raw * MODULE_CURRENT_FACTOR) + MODULE_CURRENT_BASE;
    editCurrent->Text = FloatToStrF(current, ffFixed, 15, 1) + " A";
    
    // Extract SOC (byte 4)
    uint8_t soc_raw = theMsg.Data[4];
    float soc = soc_raw * MODULE_PERCENTAGE_FACTOR;
    editSoc->Text = FloatToStrF(soc, ffFixed, 15, 1) + " %";
    
    // Extract SOH (byte 5)
    uint8_t soh_raw = theMsg.Data[5];
    float soh = soh_raw * MODULE_PERCENTAGE_FACTOR;
    editSoh->Text = FloatToStrF(soh, ffFixed, 15, 1) + " %";
}
```

### Module Data Processing

```cpp
void TForm1::ProcessModulePower(TPCANMsgFD theMsg) {
    // Get module ID from CAN message context
    uint8_t moduleId = GetCurrentSelectedModule();
    
    // Update only if this is the selected module
    if (IsSelectedModule(moduleId)) {
        // Extract module voltage
        uint16_t voltage_raw = (theMsg.Data[1] << 8) | theMsg.Data[0];
        float voltage = voltage_raw * MODULE_VOLTAGE_FACTOR;
        editModuleVoltage->Text = FloatToStrF(voltage, ffFixed, 15, 2) + " V";
        
        // Extract module current
        uint16_t current_raw = (theMsg.Data[3] << 8) | theMsg.Data[2];
        float current = (current_raw * MODULE_CURRENT_FACTOR) + MODULE_CURRENT_BASE;
        editModuleCurrent->Text = FloatToStrF(current, ffFixed, 15, 2) + " A";
        
        // Extract temperature data
        uint16_t temp_raw = (theMsg.Data[5] << 8) | theMsg.Data[4];
        float temperature = (temp_raw * MODULE_TEMPERATURE_FACTOR) + MODULE_TEMPERATURE_BASE;
        editModuleHiCellTemp->Text = FloatToStrF(temperature, ffFixed, 15, 1) + " °C";
    }
}
```

## Data Management

### Data Structures

The application maintains internal data structures that mirror the embedded system:

```cpp
// Pack-level data
struct PackData {
    float voltage;           // Pack total voltage
    float current;           // Pack current (+ = charge)
    uint8_t soc;            // State of charge (%)
    uint8_t soh;            // State of health (%)
    packState state;        // Current pack state
    bmsStatus status;       // Pack status
    uint8_t activeModules;  // Number of active modules
    uint8_t faultCode;      // Current fault conditions
    
    // Temperature extremes
    float highCellTemp;     // Highest cell temperature
    float lowCellTemp;      // Lowest cell temperature
    float avgCellTemp;      // Average cell temperature
    
    // Voltage extremes
    float highCellVolt;     // Highest cell voltage
    float lowCellVolt;      // Lowest cell voltage
    float avgCellVolt;      // Average cell voltage
};

// Module-level data
struct ModuleData {
    uint8_t moduleId;       // Module identifier
    float voltage;          // Module voltage
    float current;          // Module current
    uint8_t soc;           // Module SOC
    uint8_t soh;           // Module SOH
    moduleState state;      // Module state
    uint8_t status;         // Module status
    uint8_t cellCount;      // Number of cells
    
    // Cell data arrays
    float cellVoltages[MAX_CELLS_PER_MODULE];
    float cellTemperatures[MAX_CELLS_PER_MODULE];
    uint8_t balanceStatus[MAX_CELLS_PER_MODULE];
};
```

### Data Validation

```cpp
bool ValidatePackData(const PackData& data) {
    // Voltage range check
    if (data.voltage < 0 || data.voltage > 1000) {
        return false;
    }
    
    // Current range check  
    if (data.current < -655.36 || data.current > 655.35) {
        return false;
    }
    
    // Percentage range check
    if (data.soc > 200 || data.soh > 200) {
        return false;
    }
    
    // State validation
    if (data.state < packOff || data.state > packOn) {
        return false;
    }
    
    return true;
}
```

## Configuration Features

### EEPROM Parameter Management

The application provides comprehensive EEPROM parameter configuration:

```cpp
// Parameter types
enum ParameterType {
    PARAM_MAX_CHARGE_CURRENT,
    PARAM_MAX_DISCHARGE_CURRENT,
    PARAM_MAX_CHARGE_VOLTAGE,
    PARAM_MIN_DISCHARGE_VOLTAGE,
    PARAM_MAX_CELL_TEMP,
    PARAM_MIN_CELL_TEMP,
    PARAM_BALANCE_THRESHOLD,
    // ... additional parameters
};

// Parameter read operation
void TForm1::btnPackParamGetClick(TObject *Sender) {
    ParameterType param = GetSelectedParameter();
    
    // Send EEPROM read request
    TPCANMsgFD msg;
    msg.ID = ID_VCU_READ_EEPROM;
    msg.MSGTYPE = PCAN_MESSAGE_STANDARD;
    msg.DLC = 2;
    msg.Data[0] = param & 0xFF;
    msg.Data[1] = (param >> 8) & 0xFF;
    
    TPCANStatus status = m_objPCANBasic->WriteFD(m_PcanHandle, &msg);
    
    if (status == PCAN_ERROR_OK) {
        IncludeTextMessage("Parameter read request sent");
        // Response will be processed in ProcessEEPROMData()
    }
}

// Parameter write operation
void TForm1::btnPackParamSetClick(TObject *Sender) {
    ParameterType param = GetSelectedParameter();
    float value = StrToFloat(edtPackValue->Text);
    
    // Validate parameter value
    if (!ValidateParameterValue(param, value)) {
        ShowMessage("Invalid parameter value");
        return;
    }
    
    // Convert to appropriate format
    uint16_t encoded_value = EncodeParameterValue(param, value);
    
    // Send EEPROM write request
    TPCANMsgFD msg;
    msg.ID = ID_VCU_WRITE_EEPROM;
    msg.MSGTYPE = PCAN_MESSAGE_STANDARD;
    msg.DLC = 4;
    msg.Data[0] = param & 0xFF;
    msg.Data[1] = (param >> 8) & 0xFF;
    msg.Data[2] = encoded_value & 0xFF;
    msg.Data[3] = (encoded_value >> 8) & 0xFF;
    
    TPCANStatus status = m_objPCANBasic->WriteFD(m_PcanHandle, &msg);
    
    if (status == PCAN_ERROR_OK) {
        IncludeTextMessage("Parameter write request sent");
    }
}
```

This Windows application serves as the primary interface for interacting with the ModBatt system, providing comprehensive monitoring, configuration, and diagnostic capabilities through a user-friendly graphical interface.