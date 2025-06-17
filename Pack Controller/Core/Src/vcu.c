/***************************************************************************************************************
 * @file           : vcu.c                                                         P A C K   C O N T R O L L E R
 * @brief          : Functions for communication with VCU
 ***************************************************************************************************************
 * Copyright (C) 2023-2024 Modular Battery Technologies, Inc.
 * US Patents 11,380,942; 11,469,470; 11,575,270; others. All rights reserved
 **************************************************************************************************************/
// Include files
#include "vcu.h"
#include "main.h"
#include "bms.h"
#include <can_id_bms_vcu.h>
#include "mcu.h"
#include "canfdspi_api.h"
#include "string.h"
#include "stdio.h"
#include "can_frm_vcu.h"


/***************************************************************************************************************
*
*                               Section: Global Data Definitions                   P A C K   C O N T R O L L E R
*
***************************************************************************************************************/

// Transmit objects
CAN_TX_FIFO_CONFIG vcu_txConfig;
CAN_TX_FIFO_EVENT vcu_txFlags;
CAN_TX_MSGOBJ vcu_txObj;
uint8_t vcu_txd[MAX_DATA_BYTES];

// Receive objects
CAN_RX_FIFO_CONFIG vcu_rxConfig;
REG_CiFLTOBJ vcu_fObj;
REG_CiMASK vcu_mObj;
CAN_RX_FIFO_EVENT vcu_rxFlags;
CAN_RX_MSGOBJ vcu_rxObj;
uint8_t vcu_rxd[MAX_DATA_BYTES];


//CAN_BUS_DIAGNOSTIC busDiagnostics;
uint8_t vcu_tec;
uint8_t vcu_rec;
CAN_ERROR_STATE vcu_errorFlags;


void VCU_ProcessVcuCommand(void);
void VCU_ProcessVcuTime(void);
void VCU_ReceiveMessages(void);
void VCU_TransmitBmsData1(void);
void VCU_TransmitBmsData2(void);
void VCU_TransmitBmsData3(void);
void VCU_TransmitBmsData5(void);
void VCU_TransmitBmsData8(void);
void VCU_TransmitBmsData9(void);
void VCU_TransmitBmsData10(void);
void VCU_RequestTime(void);


extern batteryPack pack;




/***************************************************************************************************************
*
*                              Section: Application Local Functions
*
***************************************************************************************************************/



/***************************************************************************************************************
*     V C U _ R e c e i v e M e s s a g e s                                        P A C K   C O N T R O L L E R
***************************************************************************************************************/

void VCU_ReceiveMessages(void)
{
  // Check if FIFO is not empty
  DRV_CANFDSPI_ReceiveChannelEventGet(CAN1, VCU_RX_FIFO, &vcu_rxFlags);

  while ( vcu_rxFlags & CAN_RX_FIFO_NOT_EMPTY_EVENT){
    // Get message
    DRV_CANFDSPI_ReceiveMessageGet(CAN1, VCU_RX_FIFO, &vcu_rxObj, vcu_rxd, MAX_DATA_BYTES);

    if((debugLevel & (DBG_VCU + DBG_VERBOSE)) == (DBG_VCU + DBG_VERBOSE)){ sprintf(tempBuffer,"VCU RX SID=0x%03x : Byte[0..7]=0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x",vcu_rxObj.bF.id.SID,vcu_rxd[0],vcu_rxd[1],vcu_rxd[2],vcu_rxd[3],vcu_rxd[4],vcu_rxd[5],vcu_rxd[6],vcu_rxd[7]); serialOut(tempBuffer);}

    switch (vcu_rxObj.bF.id.SID + pack.vcuCanOffset) {
      case ID_VCU_COMMAND:
        /// Process the command
        VCU_ProcessVcuCommand();
        break;
      case ID_VCU_TIME:
        /// Process the command
        VCU_ProcessVcuTime();
        break;
      default:
       // Unknown Message
        if((debugLevel & ( DBG_VCU + DBG_ERRORS))==( DBG_VCU + DBG_ERRORS)){ sprintf(tempBuffer,"VCU RX UNKNOWN SID=0x%03x : EID=0x%08x : Byte[0..7]=0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x",vcu_rxObj.bF.id.SID,vcu_rxObj.bF.id.EID,vcu_rxd[0],vcu_rxd[1],vcu_rxd[2],vcu_rxd[3],vcu_rxd[4],vcu_rxd[5],vcu_rxd[6],vcu_rxd[7]); serialOut(tempBuffer);}
        break;
    }
    // check for any more messages
    DRV_CANFDSPI_ReceiveChannelEventGet(CAN1, VCU_RX_FIFO, &vcu_rxFlags);
  }
}


/***************************************************************************************************************
*     V C U _ T r a n s m i t M e s s a g e Q u e u e                              P A C K   C O N T R O L L E R
***************************************************************************************************************/
void VCU_TransmitMessageQueue(CANFDSPI_MODULE_ID index)
{
  uint8_t attempts = MAX_TXQUEUE_ATTEMPTS;

  // Check if FIFO is not full
  do {
    DRV_CANFDSPI_TransmitChannelEventGet(index, VCU_TX_FIFO, &vcu_txFlags);
    if (attempts == 0) {
      Nop();
      Nop();
      DRV_CANFDSPI_ErrorCountStateGet(index, &vcu_tec, &vcu_rec, &vcu_errorFlags);
      if((debugLevel & ( DBG_VCU + DBG_ERRORS))==( DBG_VCU + DBG_ERRORS)){ sprintf(tempBuffer,"VCU TX ERROR - FIFO Full! Check CAN Connection."); serialOut(tempBuffer);}

      //Flush channel
      DRV_CANFDSPI_TransmitChannelFlush(index, VCU_TX_FIFO);
      return;
    }
    attempts--;
  }
  while (!(vcu_txFlags & CAN_TX_FIFO_NOT_FULL_EVENT));

  // Load message and transmit
  uint8_t n = DRV_CANFDSPI_DlcToDataBytes(vcu_txObj.bF.ctrl.DLC);

  DRV_CANFDSPI_TransmitChannelLoad(index, VCU_TX_FIFO, &vcu_txObj, vcu_txd, n, true);
}


/***************************************************************************************************************
*     V C U _ P r o c e s s V c u C o m m a n d                                    P A C K   C O N T R O L L E R
***************************************************************************************************************/
void VCU_ProcessVcuCommand(void){

  CANFRM_0x400_VCU_COMMAND command;


  // Heartbeat - update last contact
  pack.vcuLastContact.overflows = etTimerOverflows ;
  pack.vcuLastContact.ticks =  htim1.Instance->CNT;

  // copy received data to status structure
  memset(&command,0,sizeof(command));
  memcpy(&command, vcu_rxd, sizeof(command));

/*
  float   floatValue  = 0;
  float   packValue    = 0;

  //vcu_hv_bus_voltage - convert it and store as module would want it
  floatValue = VCU_VOLTAGE_BASE + (VCU_VOLTAGE_FACTOR * command.vcu_hv_bus_voltage);
  packValue = (floatValue/MODULE_VOLTAGE_FACTOR) - (MODULE_VOLTAGE_BASE/MODULE_VOLTAGE_FACTOR);
  pack.vcuHvBusVoltage = packValue;
*/

  // pack hv bus voltage is encoder the same as vcu so no need to convert it
  pack.vcuHvBusVoltage = command.vcu_hv_bus_voltage;



  if(pack.vcuRequestedState != command.vcu_contactor_ctrl){

    // State Change! Set requested state
    pack.vcuRequestedState = command.vcu_contactor_ctrl;

    switch (pack.vcuRequestedState) {
      case packOn:
        pack.powerStatus.powerStage = stageSelectModule;
        pack.errorCounts.firstModule = 0;
        break;
      case packPrecharge:
        pack.powerStatus.powerStage = stageSelectModule;
        pack.errorCounts.firstModule = 0;
        break;
      case packOff:
        // Mechanical off, FET off for all modules (handled in app.c)
        break;
      case packStandby:
        // Mechanical on, FET off for all modules (handled in app.c)
        break;
      default:
        // Unknown Message
        if((debugLevel & ( DBG_VCU + DBG_ERRORS))==( DBG_VCU + DBG_ERRORS)){ sprintf(tempBuffer,"VCU RX UNKNOWN REQUESTED STATE : 0x%02x",pack.vcuRequestedState); serialOut(tempBuffer);}
        break;
    }
  }
  if(debugLevel & DBG_VCU){ sprintf(tempBuffer,"VCU RX 0x%03x VCU Command : STATE=%02x HV=%.2fV", ID_VCU_COMMAND + PACK_ID, pack.vcuRequestedState, pack.vcuHvBusVoltage * MODULE_VOLTAGE_FACTOR); serialOut(tempBuffer);}
}

/***************************************************************************************************************
*     V C U _ P r o c e s s V c u T i m e                                          P A C K   C O N T R O L L E R
***************************************************************************************************************/
void VCU_ProcessVcuTime(void){

  // 0x401 VCU_TIME - 8 bytes         8 bytes : Bits          Factor     Offset   Min     Max           Unit
  //  uint64_t time                           : 64;           0          0        0       2^64          time_t    // 64 bit time_t

  CANFRM_0x401_VCU_TIME vcuTime;

  if(debugLevel &  DBG_VCU) {sprintf(tempBuffer,"VCU RX 0x%03x VCU_TIME",vcu_txObj.bF.id.SID); serialOut(tempBuffer);}

  // Heartbeat - update last contact
  pack.vcuLastContact.overflows = etTimerOverflows ;
  pack.vcuLastContact.ticks =  htim1.Instance->CNT;

  // copy received data to status structure
  memset(&vcuTime,0,sizeof(vcuTime));
  memcpy(&vcuTime, vcu_rxd, sizeof(vcuTime));

  time_t rtcTime = vcuTime.vcu_time;

  //set the STM32 RTC based on the time received from the VCU
  writeRTC(rtcTime);

  // flag time as being valid as its come from the VCU
  pack.rtcValid = true;

}

/***************************************************************************************************************
*    V C U _ T i c k s S i n c e L a s t M e s s a g e                             P A C K   C O N T R O L L E R
***************************************************************************************************************/

uint32_t VCU_TicksSinceLastMessage(void){

  uint32_t elapsedTicks;
  uint32_t timerCNT;
  uint32_t overFlows;

  //take a snapshot of the timer counter and overflows
  timerCNT = htim1.Instance->CNT;
  overFlows = etTimerOverflows;

  if ((overFlows - pack.vcuLastContact.overflows) == 0){
    elapsedTicks = timerCNT - pack.vcuLastContact.ticks;
  }else{
    //             (ticks last contact to overflow point)          + (              ticks in completed overflows                                  ) + ( ticks in current timer period)
    elapsedTicks = ((htim1.Init.Period +1) - pack.vcuLastContact.ticks) + ( (htim1.Init.Period +1) * (overFlows - (pack.vcuLastContact.overflows +1))) + (timerCNT);
   }
  return elapsedTicks;
}


/***************************************************************************************************************
*     V C U _ T r a n s m i t B m s S t a t e                                      P A C K   C O N T R O L L E R
***************************************************************************************************************/
void VCU_TransmitBmsState(void){


  CANFRM_0x410_BMS_STATE bmsState;

  float   floatValue  = 0;
  float   vcuValue    = 0;

  //SOH
  floatValue = PERCENTAGE_BASE + (PERCENTAGE_FACTOR * pack.soh);
  vcuValue = (floatValue/VCU_SOH_PERCENTAGE_FACTOR) - (VCU_SOH_PERCENTAGE_BASE/VCU_SOH_PERCENTAGE_FACTOR);
  bmsState.bms_soh = vcuValue;

  bmsState.bms_state                = pack.state;
  bmsState.bms_status               = pack.status;
  bmsState.bms_cell_balance_status  = pack.cellBalanceStatus;
  bmsState.bms_cell_balance_active  = pack.cellBalanceActive;
  bmsState.bms_active_mod_cnt       = pack.activeModules;
  if (pack.faultedModules > 0){
    bmsState.bms_module_off = 1;
  }
  else bmsState.bms_module_off = 0;
  bmsState.bms_total_mod_cnt        = pack.moduleCount;
  bmsState.UNUSED_16                = 0;
  bmsState.UNUSED_31_63             = 0;

  // clear bit fields
  vcu_txObj.word[0] = 0;                              // Configure transmit message
  vcu_txObj.word[1] = 0;
  vcu_txObj.word[2] = 0;

  memcpy(vcu_txd, &bmsState,sizeof(bmsState));

  vcu_txObj.bF.id.SID = ID_BMS_STATE + pack.vcuCanOffset;    // Standard ID + 0x000 for pack 0, +0x100 for pack 1
  vcu_txObj.bF.id.EID = 0   ;                         // Extended ID

  vcu_txObj.bF.ctrl.BRS = 0;                          // Bit Rate Switch - use DBR when set, NBR when cleared
  vcu_txObj.bF.ctrl.DLC = CAN_DLC_8;                  // 8 bytes to transmit
  vcu_txObj.bF.ctrl.FDF = 0;                          // Frame Data Format - CAN FD when set, CAN 2.0 when cleared
  vcu_txObj.bF.ctrl.IDE = 0;                          // ID Extension selection - send base frame when cleared, extended frame when set

  if(debugLevel &  DBG_VCU) {sprintf(tempBuffer,"VCU TX 0x%03x BMS_STATE",vcu_txObj.bF.id.SID); serialOut(tempBuffer);}

  VCU_TransmitMessageQueue(VCU_CAN);                     // Send it
}


/***************************************************************************************************************
*     V C U _ T r a n s m i t B m s D a t a 1                                      P A C K   C O N T R O L L E R
***************************************************************************************************************/
void VCU_TransmitBmsData1(void){

  // 0x421 BMS_DATA_1                     8 bytes  // Bits   Factor     Offset   Min     Max           Unit
  // uint32_t UNUSED_00_31                   : 32; // 00-31
  // uint32_t bms_pack_voltage               : 16; // 32-47  0.05       0        0       3276.75       Volts   The voltage level of the pack
  // uint32_t bms_pack_current               : 16; // 48-63  0.05       -1600    -1600   1676.75       Amps    The current in or out of the pack. A positive value represents current into (charging) the energy storage system.  A negative value represents current out of (discharging) the energy storage system.

  CANFRM_0x421_BMS_DATA_1 bmsData1;

  float   floatValue  = 0;
  float   vcuValue    = 0;

  // Current
  // To convert from 16-bit module value to current (Amps), current = base + (16-bit value * factor). Remember offset is -ve
  floatValue = PACK_CURRENT_BASE + (PACK_CURRENT_FACTOR * pack.current);
  // To convert a current(Amps) to a 16-bit VCU value, VCU value  = (current/factor) - (base/factor). Remember offset is -ve
  vcuValue = (floatValue/VCU_CURRENT_FACTOR)-(VCU_CURRENT_BASE/VCU_CURRENT_FACTOR);
  bmsData1.bms_pack_current = vcuValue;

  //Voltage
  // To convert from 16-bit module value to voltage (Volts), voltage = base + (16-bit value * factor). Remember offset is -ve
  floatValue = MODULE_VOLTAGE_BASE + (MODULE_VOLTAGE_FACTOR * pack.voltage);
  // To convert a voltage (Volts) to a 16-bit VCU value, VCU value  = (voltage/factor) - (base/factor). Remember offset is -ve
  vcuValue = floatValue/VCU_VOLTAGE_FACTOR; // VCU_VOLTAGE_BASE is zero
  bmsData1.bms_pack_voltage = vcuValue;

  bmsData1.UNUSED_00_31 = 0;

  // clear bit fields
  vcu_txObj.word[0] = 0;                              // Configure transmit message
  vcu_txObj.word[1] = 0;
  vcu_txObj.word[2] = 0;

  memcpy(vcu_txd, &bmsData1, sizeof(bmsData1));

  vcu_txObj.bF.id.SID = ID_BMS_DATA_1 +  pack.vcuCanOffset;    // Standard ID + 0x000 for pack 0, +0x100 for pack 1
  vcu_txObj.bF.id.EID = 0   ;                         // Extended ID

  vcu_txObj.bF.ctrl.BRS = 0;                          // Bit Rate Switch - use DBR when set, NBR when cleared
  vcu_txObj.bF.ctrl.DLC = CAN_DLC_8;                  // 8 bytes to transmit
  vcu_txObj.bF.ctrl.FDF = 0;                          // Frame Data Format - CAN FD when set, CAN 2.0 when cleared
  vcu_txObj.bF.ctrl.IDE = 0;                          // ID Extension selection - send base frame when cleared, extended frame when set

  if(debugLevel &  DBG_VCU) {sprintf(tempBuffer,"VCU TX 0x%03x BMS_DATA_1",vcu_txObj.bF.id.SID); serialOut(tempBuffer);}

  VCU_TransmitMessageQueue(VCU_CAN);                     // Send it
}

/***************************************************************************************************************
*     V C U _ T r a n s m i t B m s D a t a 2                                      P A C K   C O N T R O L L E R
***************************************************************************************************************/
void VCU_TransmitBmsData2(void){

 // 0x422 BMS_DATA_2                      8 bytes // Bits   Factor     Offset   Min     Max           Unit
 // uint32_t bms_soc                        : 16; // 00-15  0.0015625  0        0       102.3984375   %        State of charge
 // uint32_t bms_high_cell_volt             : 16; // 16-31  0.001      0        0       65.535        Volts    Highest cell voltage reported by any cell
 // uint32_t bms_low_cell_volt              : 16; // 32-47  0.001      0        0       65.535        Volts    Lowest cell voltage reported by any cell
 // uint32_t bms_avg_cell_volt              : 16; // 48-63  0.001      0        0       65.535        Volts    Average cell voltage

  CANFRM_0x422_BMS_DATA_2 bmsData2;

  float   floatValue  = 0;
  float   vcuValue    = 0;

  //SOC
  floatValue = PERCENTAGE_BASE + (PERCENTAGE_FACTOR * pack.soc);
  vcuValue = (floatValue/VCU_SOC_PERCENTAGE_FACTOR) - (VCU_SOC_PERCENTAGE_BASE/VCU_SOC_PERCENTAGE_FACTOR);
  bmsData2.bms_soc = vcuValue;

  //Avg Cell Volt
  floatValue = CELL_VOLTAGE_BASE + (CELL_VOLTAGE_FACTOR * pack.cellAvgVolt);
  vcuValue = floatValue/VCU_CELL_VOLTAGE_FACTOR- (VCU_CELL_VOLTAGE_BASE/VCU_CELL_VOLTAGE_FACTOR);
  bmsData2.bms_avg_cell_volt = vcuValue;

  //High Cell Volt
  floatValue = CELL_VOLTAGE_BASE + (CELL_VOLTAGE_FACTOR * pack.cellHiVolt);
  vcuValue = (floatValue/VCU_CELL_VOLTAGE_FACTOR) - (VCU_CELL_VOLTAGE_BASE/VCU_CELL_VOLTAGE_FACTOR);
  bmsData2.bms_high_cell_volt = vcuValue;

  //Low Cell Volt
  floatValue = CELL_VOLTAGE_BASE + (CELL_VOLTAGE_FACTOR * pack.cellLoVolt);
  vcuValue = (floatValue/VCU_CELL_VOLTAGE_FACTOR) - (VCU_CELL_VOLTAGE_BASE/VCU_CELL_VOLTAGE_FACTOR);
  bmsData2.bms_low_cell_volt = vcuValue;


  // clear bit fields
  vcu_txObj.word[0] = 0;                              // Configure transmit message
  vcu_txObj.word[1] = 0;
  vcu_txObj.word[2] = 0;

  memcpy(vcu_txd, &bmsData2, sizeof(bmsData2));

  vcu_txObj.bF.id.SID = ID_BMS_DATA_2 +  pack.vcuCanOffset;   // Standard ID + 0x000 for pack 0, +0x100 for pack 1
  vcu_txObj.bF.id.EID = 0   ;                         // Extended ID

  vcu_txObj.bF.ctrl.BRS = 0;                          // Bit Rate Switch - use DBR when set, NBR when cleared
  vcu_txObj.bF.ctrl.DLC = CAN_DLC_8;                  // 8 bytes to transmit
  vcu_txObj.bF.ctrl.FDF = 0;                          // Frame Data Format - CAN FD when set, CAN 2.0 when cleared
  vcu_txObj.bF.ctrl.IDE = 0;                          // ID Extension selection - send base frame when cleared, extended frame when set

  if(debugLevel &  DBG_VCU) {sprintf(tempBuffer,"VCU TX 0x%03x BMS_DATA_2",vcu_txObj.bF.id.SID); serialOut(tempBuffer);}

  VCU_TransmitMessageQueue(VCU_CAN);                     // Send it
}

/***************************************************************************************************************
*     V C U _ T r a n s m i t B m s D a t a 3                                      P A C K   C O N T R O L L E R
***************************************************************************************************************/
void VCU_TransmitBmsData3(void){

  // 0x423 BMS_DATA_3                      8 bytes // Bits   Factor     Offset   Min     Max           Unit
  // uint32_t bms_high_cell_temp             : 16; // 00-15  0.03125    -273     0       1774.96875    Degrees Celcius   Highest cell temperature reported by any cell
  // uint32_t bms_low_cell_temp              : 16; // 16-31  0.03125    -273     0       1774.96875    Degrees Celcius   Lowest cell temperature reported by any cell
  // uint32_t bms_avg_cell_temp              : 16; // 32-47  0.03125    -273     0       1774.96875    Degrees Celcius   The average temperature level of all cells
  // uint32_t UNUSED_48_63                   : 16; // 48-63

  CANFRM_0x423_BMS_DATA_3 bmsData3;

  float   floatValue  = 0;
  float   vcuValue    = 0;

  //Average Cell Temperature
  floatValue = TEMPERATURE_BASE + (TEMPERATURE_FACTOR * pack.cellAvgTemp);
  vcuValue = floatValue/VCU_TEMPERATURE_FACTOR - (VCU_TEMPERATURE_BASE/VCU_TEMPERATURE_FACTOR);
  bmsData3.bms_avg_cell_temp = vcuValue;

  //High Cell Temperature
  floatValue = TEMPERATURE_BASE + (TEMPERATURE_FACTOR * pack.cellHiTemp);
  vcuValue = floatValue/VCU_TEMPERATURE_FACTOR - (VCU_TEMPERATURE_BASE/VCU_TEMPERATURE_FACTOR);
  bmsData3.bms_high_cell_temp = vcuValue;

  //Low Cell Temperature
  floatValue = TEMPERATURE_BASE + (TEMPERATURE_FACTOR * pack.cellLoTemp);
  vcuValue = floatValue/VCU_TEMPERATURE_FACTOR - (VCU_TEMPERATURE_BASE/VCU_TEMPERATURE_FACTOR);
  bmsData3.bms_low_cell_temp = vcuValue;

  bmsData3.UNUSED_48_63 = 0;

  // clear bit fields
  vcu_txObj.word[0] = 0;                              // Configure transmit message
  vcu_txObj.word[1] = 0;
  vcu_txObj.word[2] = 0;

  memcpy(vcu_txd, &bmsData3, sizeof(bmsData3));

  vcu_txObj.bF.id.SID = ID_BMS_DATA_3 +  pack.vcuCanOffset;  // Standard ID + 0x000 for pack 0, +0x100 for pack 1
  vcu_txObj.bF.id.EID = 0   ;                               // Extended ID

  vcu_txObj.bF.ctrl.BRS = 0;                          // Bit Rate Switch - use DBR when set, NBR when cleared
  vcu_txObj.bF.ctrl.DLC = CAN_DLC_8;                  // 8 bytes to transmit
  vcu_txObj.bF.ctrl.FDF = 0;                          // Frame Data Format - CAN FD when set, CAN 2.0 when cleared
  vcu_txObj.bF.ctrl.IDE = 0;                          // ID Extension selection - send base frame when cleared, extended frame when set

  if(debugLevel &  DBG_VCU) {sprintf(tempBuffer,"VCU TX 0x%03x BMS_DATA_3",vcu_txObj.bF.id.SID); serialOut(tempBuffer);}

  VCU_TransmitMessageQueue(VCU_CAN);                     // Send it
}

/***************************************************************************************************************
*    V C U _ T r a n s m i t B m s D a t a 5                                      P A C K   C O N T R O L L E R
***************************************************************************************************************/
void VCU_TransmitBmsData5(void){


 // 0x425 BMS_DATA_5                8 bytes : Bits          Factor     Offset   Min     Max           Unit
 // uint32_t bms_dischage_limit             : 16; // 00-15  0.05       -1600    -1600   1676.75       Amps     The maximum permissible current flow out of the High Voltage Energy Storage System
 // uint32_t bms_charge_limit               : 16; // 16-31  0.05       -1600    -1600   1676.75       Amps     The maximum permissible current flow in to the High Voltage Energy Storage System
 // uint32_t bms_charge_end_voltage_limit   : 16; // 32-47  0.05       0        0       3276.75       Volts    The maximum permissable voltage at end of charge
 // uint32_t UNUSED_48_63                   : 16; // 48-63

  CANFRM_0x425_BMS_DATA_5 bmsData5;

  float   floatValue  = 0;
  float   vcuValue    = 0;

  //bms_charge_limit
  floatValue = PACK_CURRENT_BASE + (PACK_CURRENT_FACTOR * pack.maxChargeA);
  vcuValue = (floatValue/VCU_CURRENT_FACTOR) - (VCU_CURRENT_BASE/VCU_CURRENT_FACTOR);
  bmsData5.bms_charge_limit = vcuValue;

  //bms_discharge_limit
  floatValue = PACK_CURRENT_BASE + (PACK_CURRENT_FACTOR * pack.maxDischargeA);
  vcuValue = (floatValue/VCU_CURRENT_FACTOR) - (VCU_CURRENT_BASE/VCU_CURRENT_FACTOR);
  bmsData5.bms_dischage_limit = vcuValue;

  //bms_charge_end_voltage_limit
  floatValue = MODULE_VOLTAGE_BASE + (MODULE_VOLTAGE_FACTOR * pack.maxChargeEndV);
  vcuValue = (floatValue/VCU_VOLTAGE_FACTOR) - (VCU_VOLTAGE_BASE/VCU_VOLTAGE_FACTOR);
  bmsData5.bms_charge_end_voltage_limit = vcuValue;

  bmsData5.UNUSED_48_63 = 0;

  // clear bit fields
  vcu_txObj.word[0] = 0;                              // Configure transmit message
  vcu_txObj.word[1] = 0;
  vcu_txObj.word[2] = 0;

  memcpy(vcu_txd, &bmsData5, sizeof(bmsData5));

  vcu_txObj.bF.id.SID = ID_BMS_DATA_5 +  pack.vcuCanOffset;   // Standard ID + 0x000 for pack 0, +0x100 for pack 1
  vcu_txObj.bF.id.EID = 0   ;                         // Extended ID

  vcu_txObj.bF.ctrl.BRS = 0;                          // Bit Rate Switch - use DBR when set, NBR when cleared
  vcu_txObj.bF.ctrl.DLC = CAN_DLC_8;                  // 8 bytes to transmit
  vcu_txObj.bF.ctrl.FDF = 0;                          // Frame Data Format - CAN FD when set, CAN 2.0 when cleared
  vcu_txObj.bF.ctrl.IDE = 0;                          // ID Extension selection - send base frame when cleared, extended frame when set

  if(debugLevel &  DBG_VCU) {sprintf(tempBuffer,"VCU TX 0x%03x BMS_DATA_5",vcu_txObj.bF.id.SID); serialOut(tempBuffer);}

  VCU_TransmitMessageQueue(VCU_CAN);                     // Send it
}

/***************************************************************************************************************
*     V C U _ T r a n s m i t B m s D a t a 8                                      P A C K   C O N T R O L L E R
***************************************************************************************************************/
void VCU_TransmitBmsData8(void){

  // 0x428 BMS_DATA_8                8 bytes : Bits          Factor     Offset   Min     Max           Unit
  // uint32_t bms_max_volt_mod               : 8;  // 00-07  1          0        0       255                   Module number with highest cell voltage
  // uint32_t bms_max_volt_cell              : 8;  // 08-15  1          0        0       255                   The number of the cell with highest voltage, within the module
  // uint32_t bms_min_volt_mod               : 8;  // 16-23  1          0        0       255                   Module number with lowest cell voltage
  // uint32_t bms_min_volt_cell              : 8;  // 24-31  1          0        0       255                   The number of the cell with lowest voltage,  within the module
  // uint32_t bms_avg_cell_volt              : 16; // 32-39  0.001      0        0       65.535        Volts   The average cell voltage
  // uint32_t UNUSED_48_63                   : 16; // 48-63

  CANFRM_0x428_BMS_DATA_8 bmsData8;

  bmsData8.UNUSED_32_63       = 0;
  bmsData8.bms_max_volt_cell  = 0;                  // TODO - implement this
  bmsData8.bms_max_volt_mod   = pack.modCellHiVolt; // Module with highest cell voltage
  bmsData8.bms_min_volt_cell  = 0;                  // TODO - implement this
  bmsData8.bms_min_volt_mod   = pack.modCellLoVolt; // Module with lowest cell voltage


  // clear bit fields
  vcu_txObj.word[0] = 0;                              // Configure transmit message
  vcu_txObj.word[1] = 0;
  vcu_txObj.word[2] = 0;

  memcpy(vcu_txd, &bmsData8, sizeof(bmsData8));

  vcu_txObj.bF.id.SID = ID_BMS_DATA_8 +  pack.vcuCanOffset;   // Standard ID + 0x000 for pack 0, +0x100 for pack 1
  vcu_txObj.bF.id.EID = 0   ;                         // Extended ID

  vcu_txObj.bF.ctrl.BRS = 0;                          // Bit Rate Switch - use DBR when set, NBR when cleared
  vcu_txObj.bF.ctrl.DLC = CAN_DLC_8;                  // 8 bytes to transmit
  vcu_txObj.bF.ctrl.FDF = 0;                          // Frame Data Format - CAN FD when set, CAN 2.0 when cleared
  vcu_txObj.bF.ctrl.IDE = 0;                          // ID Extension selection - send base frame when cleared, extended frame when set

  if(debugLevel &  DBG_VCU) {sprintf(tempBuffer,"VCU TX 0x%03x BMS_DATA_8",vcu_txObj.bF.id.SID); serialOut(tempBuffer);}

  VCU_TransmitMessageQueue(VCU_CAN);                     // Send it
}

/***************************************************************************************************************
*     V C U _ T r a n s m i t B m s D a t a 9                                      P A C K   C O N T R O L L E R
***************************************************************************************************************/
void VCU_TransmitBmsData9(void){

  // 0x429 BMS_DATA_9                 8 bytes : Bits          Factor     Offset   Min     Max           Unit
  //  uint32_t bms_max_temp_mod               : 8;  // 00-07  1          0        0       255                       Module number with highest cell temperature
  //  uint32_t bms_max_temp_cell              : 8;  // 08-15  1          0        0       255                       The number of the cell with highest temperature, within the module
  //  uint32_t bms_min_temp_mod               : 8;  // 16-23  1          0        0       255                       Module number with lowest cell temperature
  //  uint32_t bms_min_temp_cell              : 8;  // 24-31  1          0        0       255                       The number of the cell with lowest temperature, within the module
  //  uint32_t UNUSED_32_63                   : 32; // 32-63

  CANFRM_0x429_BMS_DATA_9 bmsData9;

  bmsData9.UNUSED_32_63       = 0;
  bmsData9.bms_max_temp_cell  = 0;                   // TODO - implement this
  bmsData9.bms_max_temp_mod   = pack.modCellHiTemp;  // Module with highest cell temperature
  bmsData9.bms_min_temp_cell  = 0;                   // TODO - implement this
  bmsData9.bms_min_temp_mod   = pack.modCellLoTemp;  // Module with lowest cell temperature

  // clear bit fields
  vcu_txObj.word[0] = 0;                              // Configure transmit message
  vcu_txObj.word[1] = 0;
  vcu_txObj.word[2] = 0;

  memcpy(vcu_txd, &bmsData9, sizeof(bmsData9));

  vcu_txObj.bF.id.SID = ID_BMS_DATA_9 +  pack.vcuCanOffset;   // Standard ID + 0x000 for pack 0, +0x100 for pack 1
  vcu_txObj.bF.id.EID = 0   ;                         // Extended ID

  vcu_txObj.bF.ctrl.BRS = 0;                          // Bit Rate Switch - use DBR when set, NBR when cleared
  vcu_txObj.bF.ctrl.DLC = CAN_DLC_8;                  // 8 bytes to transmit
  vcu_txObj.bF.ctrl.FDF = 0;                          // Frame Data Format - CAN FD when set, CAN 2.0 when cleared
  vcu_txObj.bF.ctrl.IDE = 0;                          // ID Extension selection - send base frame when cleared, extended frame when set

  if(debugLevel &  DBG_VCU) {sprintf(tempBuffer,"VCU TX 0x%03x BMS_DATA_9",vcu_txObj.bF.id.SID); serialOut(tempBuffer);}

  VCU_TransmitMessageQueue(VCU_CAN);                     // Send it

}

/***************************************************************************************************************
*     V C U _ T r a n s m i t B m s D a t a 1 0                                    P A C K   C O N T R O L L E R
***************************************************************************************************************/
void VCU_TransmitBmsData10(void){

  // 0x430 BMS_DATA_10               8 bytes : Bits          Factor     Offset   Min     Max           Unit
  // uint32_t bms_hv_bus_actv_iso            : 16; // 00-15  0.01       0        0       6553.5        Ohm/V        High-Voltage Bus Active Isolation Test Results
  // uint32_t UNUSED_16_31                   : 16; // 16-31
  // uint32_t UNUSED_32_63                   : 32; // 32-63

  CANFRM_0x430_BMS_DATA_10 bmsData10;

  bmsData10.UNUSED_16_31        = 0;
  bmsData10.UNUSED_32_63        = 0;
  bmsData10.bms_hv_bus_actv_iso = 0;  // TODO - implement this

  // clear bit fields
  vcu_txObj.word[0] = 0;                              // Configure transmit message
  vcu_txObj.word[1] = 0;
  vcu_txObj.word[2] = 0;

  memcpy(vcu_txd, &bmsData10, sizeof(bmsData10));

  vcu_txObj.bF.id.SID = ID_BMS_DATA_10 +  pack.vcuCanOffset;   // Standard ID + 0x000 for pack 0, +0x100 for pack 1
  vcu_txObj.bF.id.EID = 0   ;                         // Extended ID

  vcu_txObj.bF.ctrl.BRS = 0;                          // Bit Rate Switch - use DBR when set, NBR when cleared
  vcu_txObj.bF.ctrl.DLC = CAN_DLC_8;                  // 8 bytes to transmit
  vcu_txObj.bF.ctrl.FDF = 0;                          // Frame Data Format - CAN FD when set, CAN 2.0 when cleared
  vcu_txObj.bF.ctrl.IDE = 0;                          // ID Extension selection - send base frame when cleared, extended frame when set

  if(debugLevel &  DBG_VCU) {sprintf(tempBuffer,"VCU TX 0x%03x BMS_DATA_10",vcu_txObj.bF.id.SID); serialOut(tempBuffer);}

  VCU_TransmitMessageQueue(VCU_CAN);                     // Send it

}


/***************************************************************************************************************
*     V C U _ R e q u e s t T i m e                                                P A C K   C O N T R O L L E R
***************************************************************************************************************/
void VCU_RequestTime(void){

  // 0x440 BMS_REQUEST_TIME          8 bytes : Bits          Factor     Offset   Min     Max           Unit
  // uint32_t UNUSED_00_31                   : 32; // 00-31
  // uint32_t UNUSED_32_63                   : 32; // 32-63

  // clear bit fields
  vcu_txObj.word[0] = 0;                              // Configure transmit message
  vcu_txObj.word[1] = 0;
  vcu_txObj.word[2] = 0;

  vcu_txObj.bF.id.SID = ID_BMS_TIME_REQUEST +  pack.vcuCanOffset;   // Standard ID + 0x000 for pack 0, +0x100 for pack 1
  vcu_txObj.bF.id.EID = 0   ;                         // Extended ID

  vcu_txObj.bF.ctrl.BRS = 0;                          // Bit Rate Switch - use DBR when set, NBR when cleared
  vcu_txObj.bF.ctrl.DLC = CAN_DLC_8;                  // 8 bytes to transmit
  vcu_txObj.bF.ctrl.FDF = 0;                          // Frame Data Format - CAN FD when set, CAN 2.0 when cleared
  vcu_txObj.bF.ctrl.IDE = 0;                          // ID Extension selection - send base frame when cleared, extended frame when set

  if(debugLevel &  DBG_VCU) {sprintf(tempBuffer,"VCU TX 0x%03x BMS_REQUEST_TIME",vcu_txObj.bF.id.SID); serialOut(tempBuffer);}

  VCU_TransmitMessageQueue(VCU_CAN);                     // Send it

}







