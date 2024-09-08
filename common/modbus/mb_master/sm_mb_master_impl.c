//
// Created by vuonglk on 08/05/2024.
//
#include "sm_mb_master_impl.h"

#include <stdlib.h>
#include "sm_logger.h"

#define TAG "sm_mb_master"

typedef struct sm_mb_master_impl
{
    sm_mb_master_send_if m_send_if;
    sm_mb_master_recv_if m_recv_if;
    void* m_arg_if;
    nmbs_t m_nmbs_instance;
    nmbs_platform_conf m_nmbs_if;
} sm_mb_master_impl_t;

#define impl(x) ((sm_mb_master_impl_t*)(x))

static int32_t nmbs_read_impl(uint8_t* buf, uint16_t count, int32_t byte_timeout_ms, void* _arg)
{
    sm_mb_master_impl_t* this = impl(_arg);
    if (this == NULL)
        return -1;
    return this->m_recv_if(buf, count, byte_timeout_ms, this->m_arg_if);
}

static int32_t nmbs_write_impl(const uint8_t* buf, uint16_t count, int32_t byte_timeout_ms, void* _arg)
{
    sm_mb_master_impl_t* this = impl(_arg);
    if (this == NULL)
        return -1;
    return this->m_send_if(buf, count, byte_timeout_ms, this->m_arg_if);
}

sm_mb_master_t* sm_mb_master_create(sm_mb_master_send_if _sendIf, sm_mb_master_recv_if _receiveIf, void* _arg)
{
    sm_mb_master_impl_t* this = malloc(sizeof(sm_mb_master_impl_t));
    if (this == NULL)
        return NULL;

    this->m_recv_if = _receiveIf;
    this->m_send_if = _sendIf;
    this->m_arg_if = _arg;
    this->m_nmbs_if.transport = NMBS_TRANSPORT_RTU;
    this->m_nmbs_if.read = nmbs_read_impl;
    this->m_nmbs_if.write = nmbs_write_impl;
    this->m_nmbs_if.arg = this;

    nmbs_error err = nmbs_client_create(&this->m_nmbs_instance, &this->m_nmbs_if);

    if (err != NMBS_ERROR_NONE) {
        LOG_ERR(TAG, "Error creating modbus master\n");
        return NULL;
    }

    nmbs_set_byte_timeout(&this->m_nmbs_instance, MODBUS_RESPONSE_TIMEOUT_DEFAULT);
    nmbs_set_read_timeout(&this->m_nmbs_instance, MODBUS_RESPONSE_TIMEOUT_DEFAULT);

    return this;
}

MODBUS_ERROR_CODE sm_sv_mb_master_read_coils(sm_mb_master_t* _this, uint16_t _slaveAddr, uint16_t _startAddr, uint16_t _quantity,
                            uint8_t* _coilsOut)
{
    sm_mb_master_impl_t* this = impl(_this);
    if (this == NULL)
        return MODBUS_ERROR_TRANSPORT;

    nmbs_set_destination_rtu_address(&this->m_nmbs_instance, _slaveAddr);
    nmbs_error err = nmbs_read_coils(&this->m_nmbs_instance, _startAddr, _quantity, _coilsOut);
    if (err != NMBS_ERROR_NONE)
    {
        LOG_ERR(TAG, "Read coils ERROR: %d", err);
    }
    return err;
}

MODBUS_ERROR_CODE sm_sv_mb_master_read_hold_regs(sm_mb_master_t* _this, uint16_t _slaveAddr, uint16_t _startAddr,
                                       uint16_t _quantity,
                                       uint16_t* _registersOut)
{
    sm_mb_master_impl_t* this = impl(_this);
    if (this == NULL)
        return MODBUS_ERROR_TRANSPORT;

    nmbs_set_destination_rtu_address(&this->m_nmbs_instance, _slaveAddr);
    nmbs_error err = nmbs_read_holding_registers(&this->m_nmbs_instance, _startAddr, _quantity, _registersOut);

    if (err != NMBS_ERROR_NONE)
    {
        LOG_ERR(TAG, "Read holding registers ERROR: %d", err);
    }
    return err;
}

MODBUS_ERROR_CODE sm_sv_mb_master_read_input_regs(sm_mb_master_t* _this, uint16_t _slaveAddr, uint16_t _startAddr,
                                     uint16_t _quantity,
                                     uint16_t* _registersOut)
{
    sm_mb_master_impl_t* this = impl(_this);
    if (this == NULL)
        return MODBUS_ERROR_TRANSPORT;

    nmbs_set_destination_rtu_address(&this->m_nmbs_instance, _slaveAddr);
    nmbs_error err = nmbs_read_input_registers(&this->m_nmbs_instance, _startAddr, _quantity, _registersOut);

    if (err != NMBS_ERROR_NONE)
    {
        LOG_ERR(TAG, "Read Input registers ERROR: %d", err);
    }
    return err;
}

MODBUS_ERROR_CODE sm_sv_mb_master_write_signle_coil(sm_mb_master_t* _this, uint16_t _slaveAddr, uint16_t _startAddr, bool _value)
{
    sm_mb_master_impl_t* this = impl(_this);
    if (this == NULL)
        return MODBUS_ERROR_TRANSPORT;

    nmbs_set_destination_rtu_address(&this->m_nmbs_instance, _slaveAddr);
    nmbs_error err = nmbs_write_single_coil(&this->m_nmbs_instance, _startAddr, _value);

    if (err != NMBS_ERROR_NONE)
    {
        LOG_ERR(TAG, "Write single coil ERROR: %d", err);
    }
    return err;
}

MODBUS_ERROR_CODE sm_sv_mb_master_write_single_reg(sm_mb_master_t* _this, uint16_t _slaveAddr, uint16_t _startAddr, uint16_t _value)
{
    sm_mb_master_impl_t* this = impl(_this);
    if (this == NULL)
        return MODBUS_ERROR_TRANSPORT;

    nmbs_set_destination_rtu_address(&this->m_nmbs_instance, _slaveAddr);
    nmbs_error err = nmbs_write_single_register(&this->m_nmbs_instance, _startAddr, _value);

    if (err != NMBS_ERROR_NONE)
    {
        LOG_ERR(TAG, "Write single register %d ERROR: %d", _startAddr, err);
    }

    return err;
}

MODBUS_ERROR_CODE sm_sv_mb_master_write_multi_coils(sm_mb_master_t* _this, uint16_t _slaveAddr, uint16_t _startAddr,
                                     uint16_t _quantity,
                                     const uint8_t* _coils)
{
    sm_mb_master_impl_t* this = impl(_this);
    if (this == NULL)
        return MODBUS_ERROR_TRANSPORT;

    nmbs_set_destination_rtu_address(&this->m_nmbs_instance, _slaveAddr);
    nmbs_error err = nmbs_write_multiple_coils(&this->m_nmbs_instance, _startAddr, _quantity, _coils);

    if (err != NMBS_ERROR_NONE)
    {
        LOG_ERR(TAG, "Write write multiple coils ERROR: %d", err);
    }

    return err;
}

MODBUS_ERROR_CODE sm_sv_mb_master_write_multi_regs(sm_mb_master_t* _this, uint16_t _slaveAddr, uint16_t _startAddr,
                                         uint16_t _quantity,
                                         const uint16_t* _registers)
{
    sm_mb_master_impl_t* this = impl(_this);
    if (this == NULL)
        return MODBUS_ERROR_TRANSPORT;

    nmbs_set_destination_rtu_address(&this->m_nmbs_instance, _slaveAddr);
    nmbs_error err = nmbs_write_multiple_registers(&this->m_nmbs_instance, _startAddr, _quantity, _registers);

    if (err != NMBS_ERROR_NONE)
    {
        LOG_ERR(TAG, "Write slave %d multiple registers %d ERROR: %d", _slaveAddr, _startAddr, err);
    }

    return err;
}

int32_t sm_mb_master_destroy(sm_mb_master_t* _this)
{
    sm_mb_master_impl_t* this = impl(_this);
    if (this == NULL)
        return -1;
    free(this);
    return 0;
}
