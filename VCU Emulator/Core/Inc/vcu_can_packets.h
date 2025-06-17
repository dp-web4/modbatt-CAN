/***************************************************************************************************************
 * @file           : can_packets.h
 * @brief          : Modbatt CAN packet structures
 ***************************************************************************************************************
 *
 * Copyright (c) 2023 Modular Battery Technologies, Inc
 *
 **************************************************************************************************************/
#ifndef INC_CAN_PACKETS_H_
#define INC_CAN_PACKETS_H_
/*
                                  =====================
                                  CAN PACKET STRUCTURES
                                  =====================
*/
typedef struct {                  // 0x500 ANNOUNCEMENT - 8 bytes
  uint32_t moduleFw       : 8;    // module firmware version
  uint32_t moduleHw       : 8;    // module hardware capability
  uint32_t moduleMfgId    : 8;    // module hardware manufacturer
  uint32_t modulePartId   : 8;    // module part ID
  uint32_t moduleUniqueId : 32;   // module unique ID
}CANPKT_MODULE_ANNOUNCEMENT;


typedef struct {                  // 0x502 MODULE STATUS - 8 bytes
  uint32_t moduleId     : 8;      // module ID
  uint32_t moduleState  : 4;      // module current state
  uint32_t moduleSoc    : 8;      // module 8 bit state of charge
  uint32_t moduleHiTemp : 12;     // module 12 bit temperature
  uint32_t moduleLoTemp : 12;     // module 12 bit temperature
  uint32_t moduleMmc    : 10;     // module measured voltage
  uint32_t moduleMmv    : 10;     // module measured current
}CANPKT_MODULE_STATUS;


typedef struct {                  // 0x503 MODULE DETAIL - 8 bytes
  uint32_t moduleId     : 8;      // module ID
  uint32_t cellCount    : 10;     // module total number of cells
  uint32_t cellId       : 10;     // cell ID
  // 4 bits ( padding  : 4 )
  uint32_t cellTemp     : 12;     // cell 12 bit temperature
  uint32_t cellVoltage  : 10;     // cell 10 bit voltage
  uint32_t cellSoc      : 8;      // cell 8 bit SOC
  // 4 bits ( padding  : 2 )
}CANPKT_MODULE_DETAIL;


typedef struct {                  // 0x510 MODULE REGISTRATION - 8 bytes
  uint32_t moduleId       : 8;    // modules Id number used for future data exchange rather than unique ID
  uint32_t controllerId   : 8;    // controller Id that the module has been registered with
  uint32_t  moduleMfgId   : 8;    // module hardware manufacturer
  uint32_t  modulePartId  : 8;    // module part ID
  uint32_t moduleUniqueId : 32;   // module unique ID
}CANPKT_MODULE_REGISTRATION;


typedef struct {                  // 0x512 MODULE STATUS REQUEST - 1 byte
  uint8_t moduleId      : 8;      // module ID
}CANPKT_MODULE_STATUS_REQUEST;

typedef struct {                  // 0x513 MODULE DETAIL REQUEST - 3 bytes
  uint8_t moduleId      : 8;      // module ID
  uint16_t cellId       : 10;     // module cell number
  // 3 bits  (padding  : 6 )
}CANPKT_MODULE_DETAIL_REQUEST;


typedef struct {                  // 0x514 MODULE STATE CHANGE - 2 bytes
  uint8_t moduleId      : 8;      // module ID
  uint8_t state         : 4;      // module state
  // 2 bits  ( padding : 4 )
}CANPKT_MODULE_STATE_CHANGE;


typedef struct {                  // 0x51E ALL MODULES DEREGISTER - 1 bytes
  uint8_t controllerId  : 8;      // module ID
}CANPKT_MODULE_ALL_DEREGISTER;


typedef struct {                  // 0x51F ALL MODULES ISOLATE - 1 bytes
  uint8_t controllerId  : 8;      // module ID
}CANPKT_MODULE_ALL_ISOLATE;


#endif /* INC_CAN_PACKETS_H_ */
