/***************************************************************************************************************
 * @file           : can_id_bms_diag.h
 * @brief          : Modbatt CAN packet identifiers for BMS diagnostic messages on vcu to pack interface
 ***************************************************************************************************************
 *
 * Copyright (c) 2023 Modular Battery Technologies, Inc
 *
 **************************************************************************************************************/
#ifndef INC_CAN_ID_BMS_DIAG_H_
#define INC_CAN_ID_BMS_DIAG_H_

// CAN Packet IDs

#define ID_BMS_STATUS               0x220
#define ID_BMS_FAULT                0x221
#define ID_BMS_CELL_DATA            0x222
#define ID_BMS_IO                   0x223
#define ID_BMS_LIMITS               0x224
#define ID_BMS_MOD_DATA_1           0x225
#define ID_BMS_MOD_DATA_2           0x226
#define ID_BMS_MOD_DATA_3           0x227
#define ID_BMS_MOD_DATA_4           0x228


#endif /* INC_CAN_ID_BMS_DIAG_H_ */
