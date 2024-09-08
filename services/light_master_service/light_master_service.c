//
// Created by lekhacvuong on 8/25/24.
//
#include <malloc.h>
#include "light_master_service.h"
#include "sm_logger.h"

#define MODBUS_ID_OFFSET 1

#define TAG "light_sv"

static LIGHT_VALUE get_opposite_color(LIGHT_VALUE _color){
    switch (_color) {
        case LIGHT_GREEN:
            return LIGHT_RED;
        case LIGHT_RED:
            return LIGHT_GREEN;
        default:
            return _color;
    }
}


static int light_master_control_slave_light(light_master_t * _this, uint8_t _slave_id, LIGHT_VALUE _color){
    if(!_this)
        return -1;

    uint8_t testId = 0;
    uint16_t testVal = _slave_id;
    testVal = (testVal<<8) | (_color&0xFF);

    int ret = sm_sv_mb_master_write_single_reg(_this->m_mb_master, testId + MODBUS_ID_OFFSET,
                                               MODBUS_HOLDING_REG_CONTROL_LIGHT_INDEX, testVal);
    if(ret != MODBUS_ERROR_NONE){
        LOG_ERR(TAG, "control slave %d to color to %s FAILED", _slave_id, getLightColorString(_color));
        return -1;
    }

    LOG_INF(TAG, "control slave %d to color to %s SUCCEED", _slave_id, getLightColorString(_color));
    return 0;
}

static int light_master_get_slave_data(light_master_t * _this, uint8_t _slave_id){
    if(!_this)
        return -1;

    if(_slave_id >= _this->m_slave_number){
        LOG_WRN(TAG, "Slave id %d is invalid", _slave_id);
        return -1;
    }

    uint16_t testIndex = _slave_id;
    uint16_t testId = 0;

    uint16_t data[MODBUS_INPUT_REG_SYNC_DATA_REG_NUM] = {0,};

    int ret = sm_sv_mb_master_read_input_regs(_this->m_mb_master, testId + MODBUS_ID_OFFSET, testIndex,
                                              MODBUS_INPUT_REG_SYNC_DATA_REG_NUM, data);

    if(ret != MODBUS_ERROR_NONE){
        return -1;
    }

    memcpy(&_this->m_slave_data[_slave_id].m_info, data, sizeof(light_slv_info_t));

    LOG_DBG(TAG, "Slave %d now light color %s", _slave_id,
            getLightColorString(_this->m_slave_data[_slave_id].m_info.m_current_light));

    return 0;
}

static bool light_master_check_same_cycle_slave(light_master_t* _this, uint8_t _slave_id){
    if(!_this)
        return false;

    switch (_this->m_slave_number) {
        case 1:
            return false;
        case 2:
            if(_slave_id == 1){
                return true;
            }
            break;
        case 3:
            if (_slave_id == 1){
                return true;
            }
            break;
        default:
            return false;
    }
    return false;
}


light_master_t *light_master_create(sm_mb_master_send_if _sendIf, sm_mb_master_recv_if _receiveIf, void *_arg) {
    light_master_t* this = malloc(sizeof(light_master_t));
    if(!this){
        LOG_ERR(TAG, "Create light master FAILED");
        return NULL;
    }

    this->m_mb_master = sm_mb_master_create(_sendIf, _receiveIf, _arg);
    if(!this->m_mb_master){
        free(this);
        return NULL;
    }

    elapsed_timer_resetz(&this->m_sync_slave_period, DEFAULT_SYNC_PERIOD);
    this->m_slave_number = DEFAULT_SLAVE_NUMBER;
    this->m_light_duration[LIGHT_GREEN] = DEFAULT_GREEN_DURATION*1000;
    this->m_light_duration[LIGHT_RED] = DEFAULT_RED_DURATION*1000;
    this->m_light_duration[LIGHT_YELLOW] = DEFAULT_YELLOW_DURATION*1000;
    this->m_light_duration[LIGHT_NONE] = DEFAULT_NONE_COLOR_DURATION*1000;

    this->m_current_sync_id = 0;
    this->m_current_light = LIGHT_NONE;

    return this;
}

int8_t light_master_set_ctrl_light_if(light_master_t* _this, control_light_if _if){
    if(!_this)
        return -1;

    _this->m_light_ctrl_if = _if;
    return 0;
}

int8_t light_master_set_slave_number(light_master_t* _this, uint8_t _slave_num){
    if(!_this)
        return -1;

    _this->m_slave_number = _slave_num;
    return 0;
}

int8_t light_master_set_sync_period(light_master_t* _this, uint8_t _period){
    if(!_this)
        return -1;

    elapsed_timer_resetz(&_this->m_sync_slave_period, _period);
    return 0;
}

int8_t light_master_set_light_color(light_master_t* _this, LIGHT_VALUE _color){
    if(!_this)
        return -1;

    if(_color >= LIGHT_NUMBER){
        LOG_ERR(TAG, "Color %d is invalid", _color);
        return -1;
    }

    LOG_INF(TAG, "Master change light color to %s", getLightColorString(_color));
    if(_this->m_light_ctrl_if){
        _this->m_light_ctrl_if(_color);
    }
    elapsed_timer_resetz(&_this->m_light_timer, _this->m_light_duration[_color]);

    for(int id = 0; id < _this->m_slave_number; id++){
        if(light_master_check_same_cycle_slave(_this, id)){
            light_master_control_slave_light(_this, id, _color);
        }else{
            light_master_control_slave_light(_this, id, get_opposite_color(_color));
        }
    }
    return 0;
}

int8_t light_master_set_light_duration(light_master_t* _this, LIGHT_VALUE _color, uint16_t _duration){
    if(!_this)
        return -1;

    if(_color >= LIGHT_NUMBER){
        LOG_ERR(TAG, "Color %d is invalid", _color);
        return -1;
    }

    _this->m_light_duration[_color] = _duration;
    return 0;
}



slave_data_t* light_get_slave_data(light_master_t* _this, uint8_t _slave_id){
    if(!_this || _slave_id > _this->m_slave_number)
        return NULL;
    return &_this->m_slave_data[_slave_id];
}

void light_master_process(light_master_t* _this){
    if(!elapsed_timer_get_remain(&_this->m_light_timer)){
        _this->m_current_light++;
        if(_this->m_current_light >= LIGHT_NONE)
            _this->m_current_light = 0;
        light_master_set_light_color(_this, _this->m_current_light);
        return;
    }

    if(!elapsed_timer_get_remain(&_this->m_sync_slave_period)){
        elapsed_timer_reset(&_this->m_sync_slave_period);
        _this->m_current_sync_id++;
        if(_this->m_current_sync_id >= _this->m_slave_number){
            _this->m_current_sync_id = 0;
        }


        if(light_master_get_slave_data(_this, _this->m_current_sync_id) < 0){
            LOG_WRN(TAG, "Read slave %d data FAILED", _this->m_current_sync_id);
            _this->m_slave_data[_this->m_current_sync_id].m_retry_connected++;
            return;
        }

//        LIGHT_VALUE correctColor;
//        if(light_master_check_same_cycle_slave(_this, _this->m_current_sync_id)){
//            correctColor = _this->m_current_light;
//        }else{
//            correctColor = get_opposite_color(_this->m_current_light);
//        }
//
//        if (_this->m_slave_data[_this->m_current_sync_id].m_info.m_current_light != correctColor){
//            LOG_INF(TAG, "Slave %d now incorrect light, control it", _this->m_current_sync_id);
//            light_master_control_slave_light(_this, _this->m_current_sync_id, correctColor);
//        }
    }
}

