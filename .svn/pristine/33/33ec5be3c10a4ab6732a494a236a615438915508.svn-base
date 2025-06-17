/***************************************************************************************************************
 * @file           : app.c                                                               V C U   E M U L A T O R
 * @brief          : Implementation of application
 ***************************************************************************************************************
 *
 * Copyright (c) 2023 Modular Battery Technologies, Inc
 *
 **************************************************************************************************************/
// Include files
#include "can_frm_vcu.h"
#include "can_id_bms_vcu.h"


//! Function prototypes

void VCU_TransmitState(packState state);

void VCU_ProcessState(void);
void VCU_ProcessData1(void);
void VCU_ProcessData2(void);
void VCU_ProcessData3(void);
void VCU_ProcessData4(void);
void VCU_ProcessData5(void);
void VCU_ProcessData8(void);
void VCU_ProcessData9(void);
void VCU_ProcessData10(void);
void VCU_ProcessTimeRequest(void);



/***************************************************************************************************************
*     V C U _ P r o c e s s S t a t e                                                    V C U   E M U L A T O R
***************************************************************************************************************/
void VCU_ProcessState(void){

  CANFRM_0x410_BMS_STATE state;

  float soh = 0;


  // copy received data to status structure
  memset(&state,0,sizeof(state));
  memcpy(&state, rxd, sizeof(state));

  soh = VCU_SOH_PERCENTAGE_BASE + (state.bms_soh * VCU_SOH_PERCENTAGE_FACTOR);

  if(debugLevel & (DBG_VCU)){sprintf(tempBuffer,"RX BMS_STATE   %03x : STE=%02x SOH=%.2f%% STS=%02x CBS=%d CBA=%d MOFF=%d, MCNT=%d, MACT=%d"
      ,rxObj.bF.id.SID,state.bms_state,soh, state.bms_status, state.bms_cell_balance_status, state.bms_cell_balance_active, state.bms_module_off,
      state.bms_total_mod_cnt, state.bms_active_mod_cnt)  ; serialOut(tempBuffer);}
}


/***************************************************************************************************************
*     V C U _ P r o c e s s D a t a 1                                                    V C U   E M U L A T O R
***************************************************************************************************************/
void VCU_ProcessData1(void){

  CANFRM_0x421_BMS_DATA_1 data;

  float voltage = 0;
  float current = 0;

  // copy received data to status structure
  memset(&data,0,sizeof(data));
  memcpy(&data, rxd, sizeof(data));

  voltage = data.bms_pack_voltage * VCU_VOLTAGE_FACTOR;
  current = VCU_CURRENT_BASE + (data.bms_pack_current * VCU_CURRENT_FACTOR);

  if(debugLevel & (DBG_VCU)){sprintf(tempBuffer,"RX BMS_DATA_1  %03x : VOLT=%.2fV AMPS=%.2fA",rxObj.bF.id.SID,voltage,current) ; serialOut(tempBuffer);}
}


/***************************************************************************************************************
*     V C U _ P r o c e s s D a t a 2                                                    V C U   E M U L A T O R
***************************************************************************************************************/
void VCU_ProcessData2(void){

  CANFRM_0x422_BMS_DATA_2 data;

  float soc         = 0;
  float hiCellVolt  = 0;
  float loCellVolt  = 0;
  float avgCellVolt = 0;

  // copy received data to status structure
  memset(&data,0,sizeof(data));
  memcpy(&data, rxd, sizeof(data));

  hiCellVolt  = data.bms_high_cell_volt * VCU_CELL_VOLTAGE_FACTOR;
  loCellVolt  = data.bms_low_cell_volt  * VCU_CELL_VOLTAGE_FACTOR;
  avgCellVolt = data.bms_avg_cell_volt  * VCU_CELL_VOLTAGE_FACTOR;
  soc         = data.bms_soc            * VCU_SOC_PERCENTAGE_FACTOR;

  if(debugLevel & (DBG_VCU)){sprintf(tempBuffer,"RX BMS_DATA_2  %03x : HICV=%.2fV LOCV=%.2fV AVCV=%.2fV SOC=%.2f%%",rxObj.bF.id.SID,hiCellVolt,loCellVolt,avgCellVolt,soc) ; serialOut(tempBuffer);}
}

/***************************************************************************************************************
*     V C U _ P r o c e s s D a t a 3                                                    V C U   E M U L A T O R
***************************************************************************************************************/
void VCU_ProcessData3(void){

  CANFRM_0x423_BMS_DATA_3 data;

  float hiCellTemp  = 0;
  float loCellTemp  = 0;
  float avgCellTemp = 0;

  // copy received data to status structure
  memset(&data,0,sizeof(data));
  memcpy(&data, rxd, sizeof(data));

  hiCellTemp  = VCU_TEMPERATURE_BASE + (data.bms_high_cell_temp * VCU_TEMPERATURE_FACTOR);
  loCellTemp  = VCU_TEMPERATURE_BASE + (data.bms_low_cell_temp  * VCU_TEMPERATURE_FACTOR);
  avgCellTemp = VCU_TEMPERATURE_BASE + (data.bms_avg_cell_temp  * VCU_TEMPERATURE_FACTOR);

  if(debugLevel & (DBG_VCU)){sprintf(tempBuffer,"RX BMS_DATA_3  %03x : HICT=%.2fC LOCT=%.2fC AVCT=%.2fC",rxObj.bF.id.SID,hiCellTemp,loCellTemp,avgCellTemp) ; serialOut(tempBuffer);}
}


/***************************************************************************************************************
*     V C U _ P r o c e s s D a t a 5                                                    V C U   E M U L A T O R
***************************************************************************************************************/
void VCU_ProcessData5(void){

  CANFRM_0x425_BMS_DATA_5 data;

  float chgLimit     = 0;
  float dischgLimit  = 0;
  float endVoltage   = 0;

  // copy received data to status structure
  memset(&data,0,sizeof(data));
  memcpy(&data, rxd, sizeof(data));

  chgLimit      = VCU_CURRENT_BASE + (data.bms_charge_limit   * VCU_CURRENT_FACTOR);
  dischgLimit   = VCU_CURRENT_BASE + (data.bms_dischage_limit * VCU_CURRENT_FACTOR);
  endVoltage    = data.bms_charge_end_voltage_limit * VCU_VOLTAGE_FACTOR;

  if(debugLevel & (DBG_VCU)){sprintf(tempBuffer,"RX BMS_DATA_5  %03x : CLIM=%.2fA DLIM=%.2fA VLIM=%.2fV",rxObj.bF.id.SID, chgLimit, dischgLimit, endVoltage) ; serialOut(tempBuffer);}
}


/***************************************************************************************************************
*     V C U _ P r o c e s s D a t a 8                                                    V C U   E M U L A T O R
***************************************************************************************************************/
void VCU_ProcessData8(void){

  CANFRM_0x428_BMS_DATA_8 data;

  // copy received data to status structure
  memset(&data,0,sizeof(data));
  memcpy(&data, rxd, sizeof(data));

  if(debugLevel & (DBG_VCU)){sprintf(tempBuffer,"RX BMS_DATA_8  %03x : HIVM=%d LOVM=%d HIVC=%d LOVC=%d",rxObj.bF.id.SID, data.bms_max_volt_mod, data.bms_min_volt_mod, data.bms_max_volt_cell, data.bms_min_volt_cell) ; serialOut(tempBuffer);}
}


/***************************************************************************************************************
*     V C U _ P r o c e s s D a t a 9                                                    V C U   E M U L A T O R
***************************************************************************************************************/
void VCU_ProcessData9(void){

  CANFRM_0x429_BMS_DATA_9 data;

  // copy received data to status structure
  memset(&data,0,sizeof(data));
  memcpy(&data, rxd, sizeof(data));

  if(debugLevel & (DBG_VCU)){sprintf(tempBuffer,"RX BMS_DATA_9  %03x : HITM=%d LOTM=%d HITC=%d LOTC=%d",rxObj.bF.id.SID,data.bms_max_temp_mod, data.bms_min_temp_mod, data.bms_max_temp_cell, data.bms_min_temp_cell) ; serialOut(tempBuffer);}
}


/***************************************************************************************************************
*     V C U _ P r o c e s s D a t a 1 0                                                  V C U   E M U L A T O R
***************************************************************************************************************/
void VCU_ProcessData10(void){

  CANFRM_0x430_BMS_DATA_10 data;

  float isolation     = 0;

  // copy received data to status structure
  memset(&data,0,sizeof(data));
  memcpy(&data, rxd, sizeof(data));

  isolation = data.bms_hv_bus_actv_iso * VCU_ISOLATION_FACTOR;

  if(debugLevel & (DBG_VCU)){sprintf(tempBuffer,"RX BMS_DATA_10 %03x : ISOL=%.2fOhms/V",rxObj.bF.id.SID,isolation) ; serialOut(tempBuffer);}
}


/***************************************************************************************************************
*     V C U _ T  r a n s m i t S t a t e                                                 V C U   E M U L A T O R
***************************************************************************************************************/
void VCU_TransmitState(packState state){

  CANFRM_0x400_VCU_COMMAND command;
  memset(&command,0,sizeof(command));

  command.vcu_contactor_ctrl = state;

  txObj.word[0] = 0;                              // Configure transmit message
  txObj.word[1] = 0;
  txObj.word[2] = 0;

  memcpy(txd, &command, 12);

  txObj.bF.id.SID = ID_VCU_COMMAND  ;          // Standard ID
  txObj.bF.id.EID = 0;                            // Extended ID

  txObj.bF.ctrl.BRS = 0;                          // Bit Rate Switch - use DBR when set, NBR when cleared
  txObj.bF.ctrl.DLC = CAN_DLC_8;                  // 8 bytes to transmit
  txObj.bF.ctrl.FDF = 0;                          // Frame Data Format - CAN FD when set, CAN 2.0 when cleared
  txObj.bF.ctrl.IDE = 0;                          // ID Extension selection - send base frame when cleared, extended frame when set

  if(debugLevel & (DBG_PCU + DBG_VERBOSE)){sprintf(tempBuffer,"TX 0x400 Command: STATE=%02x",state); serialOut(tempBuffer);}
  VCU_TransmitMessageQueue();                     // Send it
}


/***************************************************************************************************************
*     V C U _ P r o c e s s T i m e R e q u e s t                                        V C U   E M U L A T O R
***************************************************************************************************************/
void VCU_ProcessTimeRequest(void){

  CANFRM_0x401_VCU_TIME vcuTime;

  memset(&vcuTime,0,sizeof(vcuTime));

  vcuTime.vcu_time = readRTC();


  txObj.word[0] = 0;                              // Configure transmit message
  txObj.word[1] = 0;
  txObj.word[2] = 0;

  memcpy(txd, &vcuTime, sizeof(vcuTime));

  txObj.bF.id.SID = ID_VCU_TIME  ;                // Standard ID
  txObj.bF.id.EID = 0;                            // Extended ID

  txObj.bF.ctrl.BRS = 0;                          // Bit Rate Switch - use DBR when set, NBR when cleared
  txObj.bF.ctrl.DLC = CAN_DLC_8;                  // 8 bytes to transmit
  txObj.bF.ctrl.FDF = 0;                          // Frame Data Format - CAN FD when set, CAN 2.0 when cleared
  txObj.bF.ctrl.IDE = 0;                          // ID Extension selection - send base frame when cleared, extended frame when set

  if(debugLevel & (DBG_PCU + DBG_VERBOSE)){sprintf(tempBuffer,"TX 0x401 Set Time"); serialOut(tempBuffer);}
  VCU_TransmitMessageQueue();                     // Send it
}


