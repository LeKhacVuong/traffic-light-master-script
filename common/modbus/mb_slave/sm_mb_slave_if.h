//
// Created by vnbk on 23/11/2023.
//

#ifndef SELEX_BOOT_MASTER_SM_MODBUS_RTU_IF_H
#define SELEX_BOOT_MASTER_SM_MODBUS_RTU_IF_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

#include "sm_modbus_define.h"

typedef struct sm_mb_slave sm_mb_slave_t;

typedef struct sm_mb_slave_cb{
    MODBUS_ERROR_CODE (*read_coils)(uint16_t address, uint16_t quantity, uint8_t* coils_out, uint8_t unit_id, void* arg);

    MODBUS_ERROR_CODE (*read_discrete_inputs)(uint16_t address, uint16_t quantity, uint8_t* inputs_out, uint8_t unit_id,
                                       void* arg);

    MODBUS_ERROR_CODE (*read_holding_registers)(uint16_t address, uint16_t quantity, uint16_t* registers_out, uint8_t unit_id,
                                         void* arg);

    MODBUS_ERROR_CODE (*read_input_registers)(uint16_t address, uint16_t quantity, uint16_t* registers_out, uint8_t unit_id,
                                       void* arg);

    MODBUS_ERROR_CODE (*write_single_coil)(uint16_t address, bool value, uint8_t unit_id, void* arg);

    MODBUS_ERROR_CODE (*write_single_register)(uint16_t address, uint16_t value, uint8_t unit_id, void* arg);

    MODBUS_ERROR_CODE (*write_multiple_coils)(uint16_t address, uint16_t quantity, const uint8_t* coils, uint8_t unit_id, void* arg);

    MODBUS_ERROR_CODE (*write_multiple_registers)(uint16_t address, uint16_t quantity, const uint16_t* registers,
                                           uint8_t unit_id, void* arg);
}sm_mb_slave_cb_t;

typedef struct {
    int32_t (*init)(sm_mb_slave_t*);
    int32_t (*free)(sm_mb_slave_t*);

    int32_t (*set_addr)(sm_mb_slave_t*, uint8_t);
    int32_t (*reg_function_cb)(sm_mb_slave_t*, const sm_mb_slave_cb_t*);

    int32_t (*polling)(sm_mb_slave_t*);
}sm_mb_slave_proc_t;

struct sm_mb_slave{
    const sm_mb_slave_proc_t* m_proc;
};

#ifdef __cplusplus
};
#endif
#endif //SELEX_BOOT_MASTER_SM_MODBUS_RTU_IF_H
