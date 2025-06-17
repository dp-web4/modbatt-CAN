/*
 * battery.h
 *
 *  Created on: May 31, 2023
 *      Author: Dean Claxton
 */

#ifndef INC_BATTERY_H_
#define INC_BATTERY_H_

#include "main.h"

#define MAX_NUMBER_OF_MODULES  2
#define MAX_CELLS_PER_MODULE   256
#define MAX_MODULES_PER_PACK   32
#define CONTROLLER_ID          0x01


typedef enum {
  moduleOff,            // both relays off
  moduleStandby,        // mechanical on, FET off on all modules
  modulePrecharge,      // mechanical on for all modules, FET on for one module (you select the highest voltage one, we can discuss further refinements)
  moduleOn              // both relays on for all modules.
}moduleState;

typedef enum {
  packOff,            // both relays off
  packStandby,        // mechanical on, FET off on all modules
  packPrecharge,      // mechanical on for all modules, FET on for one module (you select the highest voltage one, we can discuss further refinements)
  packOn              // both relays on for all modules.
}packState;


typedef enum {
  packStatusOff,            // charge prohibited/discharge prohibited - pack not in a condition to source or receive current, for any reason, in any state, but always the case in any state except ON
  packStatusEmpty,          // charge allowed/discharge prohibited - pack is empty, state is ON
  packStatusNormal,         // charge allowed/discharge allowed - pack in a normal state, state is ON
  packStatusFull            // charge prohibited/discharge allowed - pack is full, state is ON
}bmsStatus;



enum  ledMode{
  ledOn,
  ledOff,
  ledBlinkX,
  ledBlink1,
  ledBlink2
};


typedef struct {
  uint16_t    ticks;
  uint32_t    overflows;
}timer;


typedef struct {
  uint16_t    voltage;
  uint16_t    temp;       // cell 13 bit temperature
  uint8_t     soc;
}batteryCell;



typedef struct {
  uint8_t     mfgId;
  uint8_t     partId;  // module part ID
  uint32_t    uniqueId;
  uint8_t     moduleId;
  uint8_t     hwVersion;
  uint8_t     fwVersion;
  uint16_t    mmv;
  uint16_t    mmc;
  uint16_t    cellHiTemp;     // highest temperature
  uint16_t    cellLoTemp;     // lowest temperature
  uint16_t    cellAvgTemp;    // average temperature
  uint16_t    cellHiVolt;     // highest voltage
  uint16_t    cellLoVolt;     // lowest voltage
  uint16_t    cellAvgVolt;    // average voltage
  uint8_t     status;
  uint8_t     state;
  uint8_t     soc;
  uint8_t     soh;
  uint8_t     cellCount;
  batteryCell cell[MAX_CELLS_PER_MODULE];
  timer       lastFrame;
  bool        statusRequested;
  bool        fault;
}batteryModule;


typedef struct {
  uint8_t     mfgId;
  uint8_t     partId;  // module part ID
  uint32_t    uniqueId;
  uint8_t     moduleId;
  uint8_t     hwVersion;
  uint8_t     fwVersion;
  uint16_t    mmv;
  uint16_t    mmc;
  uint16_t    cellHiTemp;     // highest temperature
  uint16_t    cellLoTemp;     // lowest temperature
  uint16_t    cellAvgTemp;    // average temperature
  uint16_t    cellHiVolt;     // highest voltage
  uint16_t    cellLoVolt;     // lowest voltage
  uint16_t    cellAvgVolt;    // average voltage
  uint8_t     status;
  uint8_t     state;
  uint8_t     soc;
  uint8_t     soh;
  uint8_t     moduleCount;
  batteryModule module[MAX_NUMBER_OF_MODULES];
  timer       lastFrame;
  bool        statusRequested;
  bool        fault;
}batteryPack;




#endif /* INC_BATTERY_H_ */
