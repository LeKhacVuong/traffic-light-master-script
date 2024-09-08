//
// Created by vnbk on 01/12/2023.
//
#include "sm_mb_slave_impl.h"
//#include "sm_logger.h"

#include <stdlib.h>

#include "nanomodbus.h"

#define TAG "sm_mb_impl"

#define NMBS_SERVER_READ_COILS_DISABLED
#define NMBS_SERVER_READ_DISCRETE_INPUTS_DISABLED
#define NMBS_SERVER_WRITE_SINGLE_COIL_DISABLED
#define NMBS_SERVER_WRITE_MULTIPLE_COILS_DISABLED
#define NMBS_SERVER_READ_FILE_RECORD_DISABLED
#define NMBS_SERVER_WRITE_FILE_RECORD_DISABLED
#define NMBS_CLIENT_DISABLED

#define _impl(p) ((sm_mb_slave_impl_t*)(p))

typedef struct sm_mb_slave_impl{
    sm_mb_slave_t m_base;

    const sm_mb_slave_cb_t* m_cb;
    void* m_arg;

    uint8_t m_addr;
    sm_mb_send_if m_send_fn_t;
    sm_mb_recv_if m_recv_fn_t;

    nmbs_t m_nmbs_instance;
    nmbs_platform_conf m_nmbs_if;
    nmbs_callbacks m_nmbs_cb;
}sm_mb_slave_impl_t;

int32_t nmbs_read_fn(uint8_t* buf, uint16_t count, int32_t byte_timeout_ms, void* arg);
int32_t nmbs_write_fn(const uint8_t* buf, uint16_t count, int32_t byte_timeout_ms, void* arg);

int32_t sm_mb_slave_init(sm_mb_slave_t* _this){
    if(!_this){
        return -1;
    }
    _impl(_this)->m_nmbs_if.read = nmbs_read_fn;
    _impl(_this)->m_nmbs_if.write = nmbs_write_fn;
    _impl(_this)->m_nmbs_if.arg = _this;
    _impl(_this)->m_nmbs_if.transport = NMBS_TRANSPORT_RTU;

    nmbs_error err = nmbs_server_create(&_impl(_this)->m_nmbs_instance,
                                        _impl(_this)->m_addr,
                                        &_impl(_this)->m_nmbs_if,
                                        &_impl(_this)->m_nmbs_cb);

    if(err != NMBS_ERROR_NONE){
        return err;
    }

    nmbs_set_byte_timeout(&_impl(_this)->m_nmbs_instance, 50);
    nmbs_set_read_timeout(&_impl(_this)->m_nmbs_instance, 50);

    return 0;
}

int32_t sm_mb_slave_free(sm_mb_slave_t* _this){
    if(!_this){
        return -1;
    }
    return 0;
}

int32_t sm_mb_slave_set_addr(sm_mb_slave_t* _this, uint8_t _addr){
    if(!_this){
        return -1;
    }
    _impl(_this)->m_addr = _addr;
    return 0;
}

int32_t sm_mb_slave_reg_function_cb(sm_mb_slave_t* _this, const sm_mb_slave_cb_t* _cb){
    if(!_this){
        return -1;
    }
    _impl(_this)->m_cb = _cb;

    _impl(_this)->m_nmbs_cb.write_single_register = (nmbs_error (*)(uint16_t, uint16_t, uint8_t, void *))_cb->write_single_register;
    _impl(_this)->m_nmbs_cb.write_multiple_registers = (nmbs_error (*)(uint16_t, uint16_t, const uint16_t*, uint8_t, void*))_cb->write_multiple_registers;

#ifndef NMBS_SERVER_WRITE_MULTIPLE_COILS_DISABLED
    _impl(_this)->m_nmbs_cb.write_multiple_coils = (nmbs_error (*)(uint16_t, uint16_t, const uint8_t*, uint8_t, void*))_cb->write_multiple_coils;
#endif

#ifndef NMBS_SERVER_WRITE_SINGLE_COIL_DISABLED
    _impl(_this)->m_nmbs_cb.write_single_coil = (nmbs_error (*)(uint16_t address, bool value, uint8_t unit_id, void* arg)) _cb->write_single_coil;
#endif

    _impl(_this)->m_nmbs_cb.read_input_registers = (nmbs_error (*)(uint16_t, uint16_t, uint16_t*, uint8_t, void* )) _cb->read_input_registers;
    _impl(_this)->m_nmbs_cb.read_holding_registers = (nmbs_error (*)(uint16_t, uint16_t, uint16_t*, uint8_t,void*)) _cb->read_holding_registers;


#ifndef NMBS_SERVER_READ_DISCRETE_INPUTS_DISABLED
    _impl(_this)->m_nmbs_cb.read_discrete_inputs = (nmbs_error (*)(uint16_t, uint16_t, uint8_t*, uint8_t,void*)) _cb->read_discrete_inputs;
#endif

#ifndef NMBS_SERVER_READ_COILS_DISABLED
    _impl(_this)->m_nmbs_cb.read_coils = (nmbs_error (*)(uint16_t, uint16_t, uint8_t*, uint8_t,void*)) _cb->read_coils;
#endif

    _impl(_this)->m_nmbs_instance.callbacks = _impl(_this)->m_nmbs_cb;

    return 0;
}

int32_t sm_mb_slave_polling(sm_mb_slave_t* _this){
    if(!_this){
        return -1;
    }
    return nmbs_server_poll(&_impl(_this)->m_nmbs_instance);
}

static sm_mb_slave_proc_t g_mb_proc_default = {
        .init = sm_mb_slave_init,
        .free = sm_mb_slave_free,
        .reg_function_cb = sm_mb_slave_reg_function_cb,
        .set_addr = sm_mb_slave_set_addr,
        .polling = sm_mb_slave_polling
};

static sm_mb_slave_impl_t g_mb_slave = {
        .m_base.m_proc = &g_mb_proc_default,
        .m_addr = 0,
};

int32_t nmbs_read_fn(uint8_t* buf, uint16_t count, int32_t byte_timeout_ms, void* arg){
    sm_mb_slave_impl_t* this = _impl(arg);
    if(this == NULL) return -1;
    return this->m_recv_fn_t(_impl(arg)->m_addr, buf, count, byte_timeout_ms, arg);
}

int32_t nmbs_write_fn(const uint8_t* buf, uint16_t count, int32_t byte_timeout_ms, void* arg){
    sm_mb_slave_impl_t* this = _impl(arg);
    if(this == NULL) return -1;
    return this->m_send_fn_t(_impl(arg)->m_addr, buf, count, byte_timeout_ms, arg);
}


sm_mb_slave_t* sm_mb_slave_create_default(uint8_t _addr,
                                          sm_mb_send_if _send_fn,
                                          sm_mb_recv_if _recv_fn,
                                          void* _arg){
    g_mb_slave.m_addr = _addr;
    g_mb_slave.m_arg = _arg;
    g_mb_slave.m_send_fn_t = _send_fn;
    g_mb_slave.m_recv_fn_t = _recv_fn;
    
    return &g_mb_slave.m_base;
}

sm_mb_slave_t* sm_mb_slave_create(uint8_t _addr, sm_mb_send_if _send_fn, sm_mb_recv_if _recv_fn, void* _arg) {
    sm_mb_slave_impl_t* this = malloc(sizeof(sm_mb_slave_impl_t));
    if(this == NULL) return NULL;

    this->m_addr = _addr;
    this->m_arg = _arg;
    this->m_send_fn_t = _send_fn;
    this->m_recv_fn_t = _recv_fn;

    return &this->m_base;
}

int32_t sm_mb_slave_destroy(sm_mb_slave_t* _this) {
    if(_this == NULL) return -1;
    if(_this == &g_mb_slave.m_base) return -1;
    free(_this);
    return 0;
}

