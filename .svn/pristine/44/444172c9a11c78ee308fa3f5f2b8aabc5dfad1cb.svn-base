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
#include "bms.h"



/***************************************************************************************************************
*
*                               Section: Definitions                                   P A C K   E M U L A T O R
*
***************************************************************************************************************/

#define VCU_CURRENT_BASE            -1600       // amps
#define VCU_CURRENT_FACTOR          0.05        // amps
#define VCU_VOLTAGE_BASE            0           // volts
#define VCU_VOLTAGE_FACTOR          0.05        // volts
#define VCU_TEMPERATURE_BASE        -273        // degrees C
#define VCU_TEMPERATURE_FACTOR      0.03125     // degrees C
#define VCU_CELL_VOLTAGE_BASE       0           // volts
#define VCU_CELL_VOLTAGE_FACTOR     0.001       // volts
#define VCU_SOC_PERCENTAGE_BASE     0           // %
#define VCU_SOC_PERCENTAGE_FACTOR   0.0015625   // %
#define VCU_SOH_PERCENTAGE_BASE     0           // %
#define VCU_SOH_PERCENTAGE_FACTOR   0.4         // %
#define VCU_ISOLATION_FACTOR        0.001       // Ohms/Volt

#define MODULE_VOLTAGE_FACTOR       0.015       // Volts

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

#define PCU_ET_TIMEOUT  5000 //5 seconds


// Interrupts
/*
#define INT_IN			EXT1_PIN_9
#define INT_TX_IN		EXT1_PIN_13
#define INT_RX_IN		EXT1_PIN_14
*/
#define APP_INT()		(HAL_GPIO_ReadPin(CAN_INT_GPIO_Port, CAN_INT_Pin))   //(!ioport_get_pin_level(INT_IN))
#define APP_TX_INT()	(HAL_GPIO_ReadPin(CAN_INT0_GPIO_Port, CAN_INT0_Pin)) //(!ioport_get_pin_level(INT_TX_IN))
#define APP_RX_INT()	(HAL_GPIO_ReadPin(CAN_INT1_GPIO_Port, CAN_INT1_Pin)) //(!ioport_get_pin_level(INT_RX_IN))


// Test output
//#define TST1_OUT	EXT1_PIN_5


// Transmit Channels
#define APP_TX_FIFO CAN_FIFO_CH2

// Receive Channels
#define APP_RX_FIFO CAN_FIFO_CH1




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
    VCU_STATE_INIT = 0,

    // Normal run state
    VCU_STATE_RUN,

    // Test SPI access
    VCU_STATE_TEST_RAM_ACCESS,
    VCU_STATE_TEST_REGISTER_ACCESS,


} VCU_STATES;


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
   VCU_STATES state;

    /* TODO: Define any additional data used by the application. */

} VCU_DATA;


/***************************************************************************************************************
*
*       Section: Application Initialization and State Machine Functions                P A C K   E M U L A T O R
*
***************************************************************************************************************/
extern void VCU_Initialize(void);
extern void VCU_Tasks(void);

extern batteryModule module[MAX_MODULES_PER_PACK];
extern batteryPack pack;
extern packState vcuState;\
extern uint8_t activeConnection;

#endif /* APP_H_ */
