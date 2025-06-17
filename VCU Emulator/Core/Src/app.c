/***************************************************************************************************************
 * @file           : app.c                                                               V C U   E M U L A T O R
 * @brief          : Implementation of application
 ***************************************************************************************************************
 *
 * Copyright (c) 2023 Modular Battery Technologies, Inc
 *
 **************************************************************************************************************/
// Include files
#include "app.h"
#include "main.h"
#include "canfdspi_api.h"
#include "bms.h"
#include "string.h"
#include "stdio.h"
#include "can_frm_vcu.h"
#include "can_frm_bms_diag.h"
#include "can_id_bms_vcu.h"
#include "can_id_bms_diag.h"

//#include "led.h"



//! Function prototypes
void VCU_Initialize(void);
void VCU_TransmitMessageQueue(void);
void VCU_ReceiveMessage_Tasks(void);
void VCU_Tasks(void);
bool VCU_TestRegisterAccess(void);
bool VCU_TestRamAccess(void);
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


void APP_LED_Write(uint8_t led);
void APP_LED_Clear(uint8_t led);
void APP_LED_Set(uint8_t led);
void APP_CANFDSPI_Init(void);
void APP_AnnounceUnregisteredModules(void);
void APP_RegisterModule(void);
void APP_DeRegisterAllModules(void);
void APP_IsolateAllModules(void);
void APP_TransmitStatus(uint8_t index);
void APP_ReplyToCellDetailRequest(void);
void APP_TransmitCellZeroDetails(uint8_t index);
uint32_t VCU_TicksSinceLastMessage(void);
packState VCU_GetState(uint8_t index);


/***************************************************************************************************************
*
*                               Section: Global Data Definitions                         V C U   E M U L A T O R
*
***************************************************************************************************************/


VCU_DATA vcuData;

CAN_CONFIG config;
CAN_OPERATION_MODE opMode;

// Transmit objects
CAN_TX_FIFO_CONFIG txConfig;
CAN_TX_FIFO_EVENT txFlags;
CAN_TX_MSGOBJ txObj;
uint8_t txd[MAX_DATA_BYTES];

// Receive objects
CAN_RX_FIFO_CONFIG rxConfig;
REG_CiFLTOBJ fObj;
REG_CiMASK mObj;
CAN_RX_FIFO_EVENT rxFlags;
CAN_RX_MSGOBJ rxObj;
uint8_t rxd[MAX_DATA_BYTES];

uint32_t delayCount = APP_LED_TIME;

REG_t reg;

//VCU_SwitchState lastSwitchState;

//APP_Payload payload;

uint8_t ledCount = 0, ledState = 0;

uint8_t i;

CAN_BUS_DIAGNOSTIC busDiagnostics;
uint8_t tec;
uint8_t rec;
CAN_ERROR_STATE errorFlags;

timer         status;
batteryModule module[MAX_MODULES_PER_PACK];
batteryPack   pack;
uint8_t       moduleCount           = 0;
uint16_t      busVoltage;
uint8_t       activeConnection      = 0;
packState     vcuState = packOff;

extern char tempBuffer[MAX_BUFFER];
extern uint8_t canRxInterrupt;
extern uint8_t canTxInterrupt;
extern void serialOut(char* message);
extern moduleState vcuStateRequested;

char    stateSequence[MAX_SEQUENCE_LEN];
uint8_t sequenceIndex;
uint8_t sequenceLen;

/***************************************************************************************************************
*
*                   Section: Application Local Functions                                 V C U   E M U L A T O R
*
***************************************************************************************************************/

void APP_LED_Clear(uint8_t led)
{
    switch (led) {
        case 0://LED_Off(LED0);
        	HAL_GPIO_WritePin(LED_RED_GPIO_Port,  LED_RED_Pin , GPIO_PIN_RESET);
            break;
        case 1: //LED_Off(LED1);
        	HAL_GPIO_WritePin(LED_GREEN_GPIO_Port,  LED_GREEN_Pin , GPIO_PIN_RESET);
            break;
        case 2: //LED_On(LED2);
            HAL_GPIO_WritePin(LED_BLUE_GPIO_Port,  LED_BLUE_Pin , GPIO_PIN_RESET);
            break;
        default: break;
    }
}

void APP_LED_Set(uint8_t led)
{
    switch (led) {
        case 0: //LED_On(LED0);
        	HAL_GPIO_WritePin(LED_RED_GPIO_Port,  LED_RED_Pin , GPIO_PIN_SET);
            break;
        case 1: //LED_On(LED1);
        	HAL_GPIO_WritePin(LED_GREEN_GPIO_Port,  LED_GREEN_Pin , GPIO_PIN_SET);
            break;
        case 2: //LED_On(LED2);
            HAL_GPIO_WritePin(LED_BLUE_GPIO_Port,  LED_BLUE_Pin , GPIO_PIN_SET);
            break;
        default: break;
    }
}

void APP_LED_Write(uint8_t led)
{
    uint8_t mask, pin;

    mask = 1;
    pin = 0;
    Nop();

    for (i = 0; i < APP_N_LED; i++, pin++) {
        if (led & mask) {
            // Set LED
            APP_LED_Set(pin);
        } else {
            // Clear LED
            APP_LED_Clear(pin);
        }

        mask = mask << 1;
    }
}

/***************************************************************************************************************
*
*       Section: Application Initialization and State Machine Functions                  V C U   E M U L A T O R
*
***************************************************************************************************************/


/***************************************************************************************************************
*     A P P _ I n i t i a l i z e                                                        V C U   E M U L A T O R
***************************************************************************************************************/
void VCU_Initialize(void)
{



  //clear the batteryModule Array
  memset(module,0,sizeof(module));


  serialOut("");
  serialOut("");
  serialOut("        ██    ██");
  serialOut("     ██ ██ ██ ██ ██");
  serialOut("     ██ ██ ██ ██ ██     VCU Emulator V1.0        (c) 2023");
  serialOut("     ██ ██ ██ ██ ██     Modular Battery Technologies, Inc");
  serialOut("     ██    ██    ██");
  serialOut("     m o d b a t t");
  serialOut("");


	    // Switch state
//    lastSwitchState.S1 = APP_SWITCH_RELEASED;

#ifdef TEST_SPI
    DRV_CANFDSPI_Reset(DRV_CANFDSPI_INDEX_0);

    appData.state = APP_STATE_TEST_RAM_ACCESS;

#else
    /* Place the App state machine in its initial state. */
    vcuData.state = VCU_STATE_TEST_RAM_ACCESS;
#endif

}

/***************************************************************************************************************
*     A P P _ T a s k s                                                                  V C U   E M U L A T O R
***************************************************************************************************************/
void VCU_Tasks(void)
{


    /* Check the application's current state. */
    switch (vcuData.state) {
            /* Application's initial state. */
        case VCU_STATE_INIT:
        {
          if (DEBUG > 1) serialOut("APP_STATE_INIT");
          //Nop();
          APP_LED_Set(APP_INIT_LED);
          APP_CANFDSPI_Init();
          APP_LED_Clear(APP_INIT_LED);


          memset(&stateSequence[0], 0, sizeof(stateSequence));

          if (strlen(STATE_SEQUENCE) == 0){
            if(debugLevel & (DBG_ERRORS)){ sprintf(tempBuffer,"ERROR - STATE_SEQUENCE length = 0! Default sequence 0123 set."); serialOut(tempBuffer);}
            strncpy(stateSequence, "0123", 5);
          }else if (strlen(STATE_SEQUENCE) > MAX_SEQUENCE_LEN){
            if(debugLevel & (DBG_ERRORS)){ sprintf(tempBuffer,"ERROR - STATE_SEQUENCE length > MAX_SEQUENCE_LEN! Default sequence 0123 set."); serialOut(tempBuffer);}
            strncpy(stateSequence, "0123", 5);
          }else{
            strncpy(stateSequence, STATE_SEQUENCE, strlen(STATE_SEQUENCE)+1);
          }
          sequenceLen     = strlen(stateSequence);
          sequenceIndex   = 0;
          // get the initial state
          vcuState = VCU_GetState(sequenceIndex);

          //reset counter since last contact
          pack.lastFrame.ticks = htim1.Instance->CNT;
          pack.lastFrame.overflows = etTimerOverflows;

          vcuData.state = VCU_STATE_RUN;

          break;
        }
        case VCU_STATE_RUN:
        {
          // Check for incoming messages
          VCU_ReceiveMessage_Tasks();

          //Check for expired last contact from Pack Controller
          if(VCU_TicksSinceLastMessage() > PCU_ET_TIMEOUT){
            vcuState = packOff;
            sequenceIndex = 0;
            activeConnection = 0;
            if(debugLevel & (DBG_PCU)){ sprintf(tempBuffer,"PC LOST CONTACT TIMEOUT!"); serialOut(tempBuffer);}
          }
          // only start sending state data once pack controller has contacted us
          if(activeConnection){

            if(changeState){
              // any more states in the sequence?
              if(sequenceIndex < sequenceLen){
                // yes - next state
                sequenceIndex++;
              } else {
                // no - are we repeating the sequence
                if(STATE_REPEAT == true){
                  // yes - reset to beginning, otherwise hold the last state indefinitely
                  sequenceIndex = 0;
                }
              }
              vcuState = VCU_GetState(sequenceIndex);
              changeState = 0;
            }

            // sendState fires at interval defined by STATE_INTERVAL
            if (sendState){
              VCU_TransmitState(vcuState);
              sendState = 0;
            }
          }

          break;
        }

        case VCU_STATE_TEST_RAM_ACCESS:             // RAM access test
        {
          bool passed = VCU_TestRamAccess();
          if (passed) { sprintf(tempBuffer,"     MCP2518FD RAM TEST      : PASSED"); serialOut(tempBuffer);
          }else{        sprintf(tempBuffer,"     MCP2518FD RAM TEST      : FAILED"); serialOut(tempBuffer);
          }
          vcuData.state = VCU_STATE_TEST_REGISTER_ACCESS;
          break;
        }
        case VCU_STATE_TEST_REGISTER_ACCESS: /* Register access test */
        {
          bool passed = VCU_TestRegisterAccess();
          if (passed) { sprintf(tempBuffer,"     MCP2518FD REGISTER TEST : PASSED"); serialOut(tempBuffer);
          }else{        sprintf(tempBuffer,"     MCP2518FD REGISTER TEST : FAILED"); serialOut(tempBuffer);
          }
          sprintf(tempBuffer," "); serialOut(tempBuffer);
          vcuData.state = VCU_STATE_INIT;
          break;
        }
        // The default state should never be executed
        default:
        {
          /* TODO: Handle error in application's state machine. */
          vcuData.state = VCU_STATE_INIT;
          break;
        }
    }
}

/***************************************************************************************************************
*
*                              Section: Application Local Functions
*
***************************************************************************************************************/


/***************************************************************************************************************
*     A P P _ C A N F D S P I _ I n i t                                                  V C U   E M U L A T O R
***************************************************************************************************************/
void APP_CANFDSPI_Init(void)
{
    // Reset device
    DRV_CANFDSPI_Reset(DRV_CANFDSPI_INDEX_0);

    // Enable ECC and initialize RAM
    DRV_CANFDSPI_EccEnable(DRV_CANFDSPI_INDEX_0);

    DRV_CANFDSPI_RamInit(DRV_CANFDSPI_INDEX_0, 0xff);

    // Configure device
    DRV_CANFDSPI_ConfigureObjectReset(&config);
    config.IsoCrcEnable = 1;
    config.StoreInTEF = 0;

    DRV_CANFDSPI_Configure(DRV_CANFDSPI_INDEX_0, &config);

    // Setup TX FIFO
    DRV_CANFDSPI_TransmitChannelConfigureObjectReset(&txConfig);
    txConfig.FifoSize = 7;
    txConfig.PayLoadSize = CAN_PLSIZE_64;
    txConfig.TxPriority = 1;

    DRV_CANFDSPI_TransmitChannelConfigure(DRV_CANFDSPI_INDEX_0, APP_TX_FIFO, &txConfig);

    // Setup RX FIFO
    DRV_CANFDSPI_ReceiveChannelConfigureObjectReset(&rxConfig);
    rxConfig.FifoSize = 15;
    rxConfig.PayLoadSize = CAN_PLSIZE_64;

    DRV_CANFDSPI_ReceiveChannelConfigure(DRV_CANFDSPI_INDEX_0, APP_RX_FIFO, &rxConfig);

    // Setup RX Filter
    fObj.word = 0;
    //fObj.bF.SID = 0xda;
    fObj.bF.SID = 0x00;
    fObj.bF.EXIDE = 0;
    fObj.bF.EID = 0x00;

    DRV_CANFDSPI_FilterObjectConfigure(DRV_CANFDSPI_INDEX_0, CAN_FILTER0, &fObj.bF);

    // Setup RX Mask
    mObj.word = 0;
    mObj.bF.MSID = 0x0;
    mObj.bF.MIDE = 1; // Only allow standard IDs
    mObj.bF.MEID = 0x0;
    DRV_CANFDSPI_FilterMaskConfigure(DRV_CANFDSPI_INDEX_0, CAN_FILTER0, &mObj.bF);

    // Link FIFO and Filter
    DRV_CANFDSPI_FilterToFifoLink(DRV_CANFDSPI_INDEX_0, CAN_FILTER0, APP_RX_FIFO, true);

    // Setup Bit Time
    DRV_CANFDSPI_BitTimeConfigure(DRV_CANFDSPI_INDEX_0, CAN_500K_2M, CAN_SSP_MODE_AUTO, CAN_SYSCLK_40M);

    // Setup Transmit and Receive Interrupts
    DRV_CANFDSPI_GpioModeConfigure(DRV_CANFDSPI_INDEX_0, GPIO_MODE_INT, GPIO_MODE_INT);
	#ifdef APP_USE_TX_INT
    DRV_CANFDSPI_TransmitChannelEventEnable(DRV_CANFDSPI_INDEX_0, APP_TX_FIFO, CAN_TX_FIFO_NOT_FULL_EVENT);
	#endif
    DRV_CANFDSPI_ReceiveChannelEventEnable(DRV_CANFDSPI_INDEX_0, APP_RX_FIFO, CAN_RX_FIFO_NOT_EMPTY_EVENT);
    DRV_CANFDSPI_ModuleEventEnable(DRV_CANFDSPI_INDEX_0, CAN_TX_EVENT | CAN_RX_EVENT);

    // Select Normal Mode
    DRV_CANFDSPI_OperationModeSelect(DRV_CANFDSPI_INDEX_0, CAN_NORMAL_MODE);
}

/***************************************************************************************************************
*     V C U _ G e t S t a t e                                                            V C U   E M U L A T O R
***************************************************************************************************************/
packState VCU_GetState(uint8_t index)
{
  packState state;
  state = stateSequence[sequenceIndex] - '0';

  if (state > packOn || state < packOff){
    //rubbish data in the sequence
    if(debugLevel & DBG_ERRORS){ sprintf(tempBuffer,"ERROR - STATE_SEQUENCE bad data at index %d!",sequenceIndex); serialOut(tempBuffer);}
    state = 0;
  }


  return state;
}


/***************************************************************************************************************
*     V C U _ R e c e i v e M e s s a g e _ T a s k s                                    V C U   E M U L A T O R
***************************************************************************************************************/
void VCU_ReceiveMessage_Tasks(void)
{

    // CANPKT_REGISTER registration;
    //uint8_t index;

    // Check if FIFO is not empty
    DRV_CANFDSPI_ReceiveChannelEventGet(DRV_CANFDSPI_INDEX_0, APP_RX_FIFO, &rxFlags);

    while ( rxFlags & CAN_RX_FIFO_NOT_EMPTY_EVENT){

      // Get message
      DRV_CANFDSPI_ReceiveMessageGet(DRV_CANFDSPI_INDEX_0, APP_RX_FIFO, &rxObj, rxd, MAX_DATA_BYTES);

      activeConnection = 1;
      // reset last contact
      pack.lastFrame.ticks = htim1.Instance->CNT;
      pack.lastFrame.overflows = etTimerOverflows;

      switch (rxObj.bF.id.SID) {
        case ID_BMS_DATA_1:
          if((debugLevel & (DBG_PCU + DBG_VERBOSE))==(DBG_PCU + DBG_VERBOSE)){sprintf(tempBuffer,"RX BMS_DATA_1 SID=0x%03x : Byte[0..7]=0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x",rxObj.bF.id.SID,rxd[0],rxd[1],rxd[2],rxd[3],rxd[4],rxd[5],rxd[6],rxd[7]); serialOut(tempBuffer);}
          VCU_ProcessData1();
          break;
        case ID_BMS_DATA_2:
          if((debugLevel & (DBG_PCU + DBG_VERBOSE))==(DBG_PCU + DBG_VERBOSE)){sprintf(tempBuffer,"RX BMS_DATA_2 SID=0x%03x : Byte[0..7]=0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x",rxObj.bF.id.SID,rxd[0],rxd[1],rxd[2],rxd[3],rxd[4],rxd[5],rxd[6],rxd[7]); serialOut(tempBuffer);}
          VCU_ProcessData2();
          break;
        case ID_BMS_DATA_3:
          if((debugLevel & (DBG_PCU + DBG_VERBOSE))==(DBG_PCU + DBG_VERBOSE)){sprintf(tempBuffer,"RX BMS_DATA_3 SID=0x%03x : Byte[0..7]=0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x",rxObj.bF.id.SID,rxd[0],rxd[1],rxd[2],rxd[3],rxd[4],rxd[5],rxd[6],rxd[7]); serialOut(tempBuffer);}
          VCU_ProcessData3();
          break;
        case ID_BMS_DATA_5:
          if((debugLevel & (DBG_PCU + DBG_VERBOSE))==(DBG_PCU + DBG_VERBOSE)){sprintf(tempBuffer,"RX BMS_DATA_5 SID=0x%03x : Byte[0..7]=0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x",rxObj.bF.id.SID,rxd[0],rxd[1],rxd[2],rxd[3],rxd[4],rxd[5],rxd[6],rxd[7]); serialOut(tempBuffer);}
          VCU_ProcessData5();
          break;
        case ID_BMS_DATA_8:
          if((debugLevel & (DBG_PCU + DBG_VERBOSE))==(DBG_PCU + DBG_VERBOSE)){sprintf(tempBuffer,"RX BMS_DATA_8 SID=0x%03x : Byte[0..7]=0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x",rxObj.bF.id.SID,rxd[0],rxd[1],rxd[2],rxd[3],rxd[4],rxd[5],rxd[6],rxd[7]); serialOut(tempBuffer);}
          VCU_ProcessData8();
          break;
        case ID_BMS_DATA_9:
          if((debugLevel & (DBG_PCU + DBG_VERBOSE))==(DBG_PCU + DBG_VERBOSE)){sprintf(tempBuffer,"RX BMS_DATA_9 SID=0x%03x : Byte[0..7]=0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x",rxObj.bF.id.SID,rxd[0],rxd[1],rxd[2],rxd[3],rxd[4],rxd[5],rxd[6],rxd[7]); serialOut(tempBuffer);}
          VCU_ProcessData9();
          break;
        case ID_BMS_DATA_10:
          if((debugLevel & (DBG_PCU + DBG_VERBOSE))==(DBG_PCU + DBG_VERBOSE)){sprintf(tempBuffer,"RX BMS_DATA_19 SID=0x%03x : Byte[0..7]=0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x",rxObj.bF.id.SID,rxd[0],rxd[1],rxd[2],rxd[3],rxd[4],rxd[5],rxd[6],rxd[7]); serialOut(tempBuffer);}
          VCU_ProcessData10();
          break;
        case ID_BMS_STATE:
          if((debugLevel & (DBG_PCU + DBG_VERBOSE))==(DBG_PCU + DBG_VERBOSE)){sprintf(tempBuffer,"RX BMS_STATE SID=0x%03x : Byte[0..7]=0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x",rxObj.bF.id.SID,rxd[0],rxd[1],rxd[2],rxd[3],rxd[4],rxd[5],rxd[6],rxd[7]); serialOut(tempBuffer);}
          VCU_ProcessState();
          break;
        case ID_BMS_TIME_REQUEST:
          if((debugLevel & (DBG_PCU + DBG_VERBOSE))==(DBG_PCU + DBG_VERBOSE)){sprintf(tempBuffer,"RX BMS_TIME_REQUEST SID=0x%03x : Byte[0..7]=0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x",rxObj.bF.id.SID,rxd[0],rxd[1],rxd[2],rxd[3],rxd[4],rxd[5],rxd[6],rxd[7]); serialOut(tempBuffer);}
          VCU_ProcessTimeRequest();
          break;
        default:
          if((debugLevel & (DBG_PCU + DBG_VERBOSE))==(DBG_PCU + DBG_VERBOSE)){sprintf(tempBuffer,"RX UNKNOWN ID=0x%03x : Byte[0..7]=0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x",rxObj.bF.id.SID,rxd[0],rxd[1],rxd[2],rxd[3],rxd[4],rxd[5],rxd[6],rxd[7]); serialOut(tempBuffer);}
          break;
      }


      // check for any more messages
      DRV_CANFDSPI_ReceiveChannelEventGet(DRV_CANFDSPI_INDEX_0, APP_RX_FIFO, &rxFlags);
    }

    //    APP_LED_Clear(APP_RX_LED);


    canRxInterrupt = 0;

}
/***************************************************************************************************************
*     A P P _ T r a n s m i t M e s s a g e Q u e u e                                    V C U   E M U L A T O R
***************************************************************************************************************/
void VCU_TransmitMessageQueue(void)
{
    APP_LED_Set(APP_TX_LED);

    uint8_t attempts = MAX_TXQUEUE_ATTEMPTS;

    // Check if FIFO is not full
    do {
        DRV_CANFDSPI_TransmitChannelEventGet(DRV_CANFDSPI_INDEX_0, APP_TX_FIFO, &txFlags);
        if (attempts == 0) {
            Nop();
            Nop();
            DRV_CANFDSPI_ErrorCountStateGet(DRV_CANFDSPI_INDEX_0, &tec, &rec, &errorFlags);

            // Error - FIFO full!
            sprintf(tempBuffer,"TX ERROR - FIFO Full! Check CAN Connection."); serialOut(tempBuffer);

            //Flush channel
            DRV_CANFDSPI_TransmitChannelReset(DRV_CANFDSPI_INDEX_0, APP_TX_FIFO);
            return;
        }
        attempts--;
    }
    while (!(txFlags & CAN_TX_FIFO_NOT_FULL_EVENT));

    // Load message and transmit
    uint8_t n = DRV_CANFDSPI_DlcToDataBytes(txObj.bF.ctrl.DLC);

    DRV_CANFDSPI_TransmitChannelLoad(DRV_CANFDSPI_INDEX_0, APP_TX_FIFO, &txObj, txd, n, true);

    APP_LED_Clear(APP_TX_LED);
}

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
  command.vcu_hv_bus_voltage = 26668; //400.02V


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

  if(debugLevel & (DBG_PCU + DBG_VERBOSE)){sprintf(tempBuffer,"TX 0x400 Command: STATE=%02x HV=%.2fV",state, command.vcu_hv_bus_voltage * MODULE_VOLTAGE_FACTOR ); serialOut(tempBuffer);}
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


/***************************************************************************************************************
*     V C U _ T i c k s S i n c e L a s t M e s s a g e                                  V C U   E M U L A T O R
***************************************************************************************************************/
uint32_t VCU_TicksSinceLastMessage(void)
{
  uint32_t elapsedTicks;
  uint32_t timerCNT;
  uint32_t overFlows;

  //take a snapshot of the timer counter and overflows
  timerCNT = htim1.Instance->CNT;
  overFlows = etTimerOverflows;

  if ((overFlows - pack.lastFrame.overflows) == 0){
    elapsedTicks = timerCNT - pack.lastFrame.ticks;
  }else{
    //             (ticks last contact to overflow point)          + (              ticks in completed overflows                           ) + ( ticks in current timer period)
    elapsedTicks = ((htim1.Init.Period +1) - pack.lastFrame.ticks) + ( (htim1.Init.Period +1) * (overFlows - (pack.lastFrame.overflows +1))) + (timerCNT);
  }
  return elapsedTicks;

}


/***************************************************************************************************************
*     V C U _ T i c k s S i n c e L a s t S t a t u s                                    V C U   E M U L A T O R
***************************************************************************************************************/
uint32_t VCU_TicksSinceLastStatus(void)
{
  uint32_t elapsedTicks;

  if ((etTimerOverflows - status.overflows) == 0){
    elapsedTicks = htim1.Instance->CNT - status.ticks;
  }else{
    //             (ticks last contact to overflow point)  + (  ticks in completed overflows   + ( ticks in current timer period)
    elapsedTicks = ((htim1.Init.Period +1) - status.ticks) + ( (htim1.Init.Period +1) * (etTimerOverflows - (status.overflows +1))) + (htim1.Instance->CNT);
  }
  return elapsedTicks;

}


/***************************************************************************************************************
*     A P P _ T e s t R e g i s t e r A c c e s s                                        V C U   E M U L A T O R
***************************************************************************************************************/
bool VCU_TestRegisterAccess(void)
{
  // Variables
  uint8_t length;
  bool good = false;

  Nop();

  // Verify read/write with different access length
  // Note: registers can be accessed in multiples of bytes
  for (length = 1; length <= MAX_DATA_BYTES; length++) {
    for (i = 0; i < length; i++) {
        txd[i] = rand() & 0x7f; // Bit 31 of Filter objects is not implemented
        rxd[i] = 0xff;
    }
    Nop();

    // Write data to registers
    DRV_CANFDSPI_WriteByteArray(DRV_CANFDSPI_INDEX_0, cREGADDR_CiFLTOBJ, txd, length);

    // Read data back from registers
    DRV_CANFDSPI_ReadByteArray(DRV_CANFDSPI_INDEX_0, cREGADDR_CiFLTOBJ, rxd, length);

    // Verify
    good = false;
    for (i = 0; i < length; i++) {
      good = txd[i] == rxd[i];

      if (!good) {
        Nop();
        Nop();
        // Data mismatch
        return false;
      }
    }
  }
  Nop();
  Nop();
  return true;
}


/***************************************************************************************************************
*     A P P _ T e s t R a m A c c e s s                                                  V C U   E M U L A T O R
***************************************************************************************************************/
bool VCU_TestRamAccess(void)
{
  // Variables
  uint8_t length;
  bool good = false;

  Nop();

  // Verify read/write with different access length
  // Note: RAM can only be accessed in multiples of 4 bytes
  for (length = 4; length <= MAX_DATA_BYTES; length += 4) {
    for (i = 0; i < length; i++) {
        txd[i] = rand() & 0xff;
        rxd[i] = 0xff;
    }
    Nop();

    // Write data to RAM
    DRV_CANFDSPI_WriteByteArray(DRV_CANFDSPI_INDEX_0, cRAMADDR_START, txd, length);

    // Read data back from RAM
    DRV_CANFDSPI_ReadByteArray(DRV_CANFDSPI_INDEX_0, cRAMADDR_START, rxd, length);

    // Verify
    good = false;
    for (i = 0; i < length; i++) {
      good = txd[i] == rxd[i];

      if (!good) {
        Nop();
        Nop();

        // Data mismatch
        return false;
      }
    }
  }

  return true;
}
