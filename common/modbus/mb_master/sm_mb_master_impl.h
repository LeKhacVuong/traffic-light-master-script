//
// Created by vuonglk on 08/05/2024.
//

#ifndef SM_MB_MASTER_IMPL_H
#define SM_MB_MASTER_IMPL_H

#ifdef __cplusplus
extern "C"{
#endif

#include "stdint.h"
#include "nanomodbus.h"
#include "sm_modbus_define.h"

typedef void sm_mb_master_t;

typedef int32_t (*sm_mb_master_send_if)(const uint8_t* _data, int32_t _len, int32_t _timeout, void* _arg);
typedef int32_t (*sm_mb_master_recv_if)(uint8_t* _buf, int32_t _len, int32_t _timeout, void* _arg);

sm_mb_master_t* sm_mb_master_create(sm_mb_master_send_if _sendIf, sm_mb_master_recv_if _receiveIf, void* _arg);

MODBUS_ERROR_CODE sm_sv_mb_master_read_coils(sm_mb_master_t* _this, uint16_t _slaveAddr, uint16_t _startAddr, uint16_t _quantity, uint8_t* _coilsOut);
MODBUS_ERROR_CODE sm_sv_mb_master_read_hold_regs(sm_mb_master_t* _this, uint16_t _slaveAddr, uint16_t _startAddr, uint16_t _quantity,
                                       uint16_t* _registersOut);
MODBUS_ERROR_CODE sm_sv_mb_master_read_input_regs(sm_mb_master_t* _this, uint16_t _slaveAddr, uint16_t _startAddr, uint16_t _quantity,
                                     uint16_t* _registersOut);
MODBUS_ERROR_CODE sm_sv_mb_master_write_signle_coil(sm_mb_master_t* _this, uint16_t _slaveAddr, uint16_t _startAddr, bool _value);
MODBUS_ERROR_CODE sm_sv_mb_master_write_single_reg(sm_mb_master_t* _this, uint16_t _slaveAddr, uint16_t _startAddr, uint16_t _value);
MODBUS_ERROR_CODE sm_sv_mb_master_write_multi_coils(sm_mb_master_t* _this, uint16_t _slaveAddr, uint16_t _startAddr, uint16_t _quantity,
                                     const uint8_t* _coils);
MODBUS_ERROR_CODE sm_sv_mb_master_write_multi_regs(sm_mb_master_t* _this, uint16_t _slaveAddr, uint16_t _startAddr, uint16_t _quantity,
                                         const uint16_t* _registers);

int32_t sm_mb_master_destroy(sm_mb_master_t* _this);

#ifdef __cplusplus
};
#endif

#endif //SM_MB_MASTER_IMPL_H
