/***************************************************************************************************************
 * @file           : app.h                                                             P A C K   E M U L A T O R
 * @brief          : Application header file
 ***************************************************************************************************************
 *
 * Copyright (c) 2023 Modular Battery Technologies, Inc
 *
 **************************************************************************************************************/
#ifndef APP_H_
#define APP_H_

// Include files
//#include <stdint.h>
//#include <stdbool.h>
//#include <stddef.h>
//#include <stdlib.h>
#include "canfdspi_api.h"



/***************************************************************************************************************
*
*                               Section: Definitions                                   P A C K   E M U L A T O R
*
***************************************************************************************************************/

//#define TEST_SPI

//! Use RX and TX Interrupt pins to check FIFO status
#define APP_USE_RX_INT

#define MAX_TXQUEUE_ATTEMPTS 50


// Switches
#define APP_SWITCH_RELEASED true  //Switch has an internal pullup when not pressed - input = 1
#define APP_SWITCH_PRESSED  false  //Switch connects to ground when pressed - input  = 0

#define APP_DEBOUNCE_TIME  100

#define APP_S1_READ() (HAL_GPIO_ReadPin(BUTTON1_GPIO_Port, BUTTON1_Pin)) //(ioport_get_pin_level(GPIO_PUSH_BUTTON_1))


// LEDs
#define LED_COUNT		2
#define APP_N_LED LED_COUNT

// Special LEDs
#define APP_INIT_LED    0
#define APP_LED_D1		1
#define APP_TX_LED      2 //0

#define APP_LED_TIME    500 //50000

// Interrupts
/*
#define INT_IN			EXT1_PIN_9
#define INT_TX_IN		EXT1_PIN_13
#define INT_RX_IN		EXT1_PIN_14
*/
#define APP_INT()		(HAL_GPIO_ReadPin(CAN_INT_GPIO_Port, CAN_INT_Pin))   //(!ioport_get_pin_level(INT_IN))
#define APP_TX_INT()	(HAL_GPIO_ReadPin(CAN_INT0_GPIO_Port, CAN_INT0_Pin)) //(!ioport_get_pin_level(INT_TX_IN))
#define APP_RX_INT()	(HAL_GPIO_ReadPin(CAN_INT1_GPIO_Port, CAN_INT1_Pin)) //(!ioport_get_pin_level(INT_RX_IN))


#define MODULE_VOLTAGE_BASE       0         // Volts
#define MODULE_VOLTAGE_FACTOR     0.015      // Volts
#define CELL_VOLTAGE_BASE         0         // Volts
#define CELL_VOLTAGE_FACTOR       0.001     // Volts
#define MODULE_CURRENT_BASE       -655.36   // Amps
#define MODULE_CURRENT_FACTOR     0.02      // Amps
#define TEMPERATURE_BASE          -55.35    // Degrees C
#define TEMPERATURE_FACTOR        0.01      // Degrees C
#define PERCENTAGE_BASE           0         // Percent
#define PERCENTAGE_FACTOR         0.5       // Percent



// Test output
//#define TST1_OUT	EXT1_PIN_5


// Transmit Channels
#define APP_TX_FIFO CAN_FIFO_CH2

// Receive Channels
#define APP_RX_FIFO CAN_FIFO_CH1

// Switch states

typedef struct {
	bool S1;
} APP_SwitchState;

// Payload

typedef struct {
	bool On;
	uint8_t Dlc;
	bool Mode;
	uint8_t Counter;
	uint8_t Delay;
	bool BRS;
} APP_Payload;

/***************************************************************************************************************
* Application states                                                                   P A C K   E M U L A T O R

  Summary:
    Application states enumeration

  Description:
    This enumeration defines the valid application states.  These states
    determine the behavior of the application at various times.
***************************************************************************************************************/

typedef enum {
    // Initialization
    APP_STATE_INIT = 0,

    // Normal run state
    APP_STATE_IDLE,

    APP_STATE_ANNOUNCE,

    // POR signaling
    APP_STATE_FLASH_LEDS,

    // Transmit and Receive
    APP_STATE_TRANSMIT,
    APP_STATE_RECEIVE,

    // Test SPI access
    APP_STATE_TEST_RAM_ACCESS,
    APP_STATE_TEST_REGISTER_ACCESS,


} APP_STATES;


/***************************************************************************************************************
* Application Data                                                                     P A C K   E M U L A T O R

  Summary:
    Holds application data

  Description:
    This structure holds the application's data.

  Remarks:
    Application strings and buffers are be defined outside this structure.
***************************************************************************************************************/
typedef struct {
    /* The application's current state */
    APP_STATES state;

    /* TODO: Define any additional data used by the application. */

} APP_DATA;


/***************************************************************************************************************
*
*       Section: Application Initialization and State Machine Functions                P A C K   E M U L A T O R
*
***************************************************************************************************************/
//! Application Initialization

void APP_Initialize(void);

//! Application Tasks
// This routine must be called from SYS_Tasks() routine.

void APP_Tasks(void);

//! Write LEDs based on input byte

void APP_LED_Write(uint8_t led);

//! Clear One LED

void APP_LED_Clear(uint8_t led);

//! Set One LED

void APP_LED_Set(uint8_t led);

//! Initialize CANFDSPI
void APP_CANFDSPI_Init(void);

//! Add message to transmit FIFO
void APP_TransmitMessageQueue(void);

//! Decode received messages
APP_STATES APP_ReceiveMessage_Tasks(void);

void APP_AnnounceUnregisteredModules(void);
void APP_RegisterModule(void);
void APP_DeRegisterAllModules(void);
void APP_IsolateAllModules(void);
void APP_TransmitStatus(uint8_t index);
void APP_ReplyToCellDetailRequest(void);
void APP_TransmitCellZeroDetails(uint8_t index);
void APP_ReplyToStatusRequest(void);
void APP_StateChange(void);

//! Test SPI access
bool APP_TestRegisterAccess(void);

//! Test RAM access
bool APP_TestRamAccess(void);

#endif /* APP_H_ */
