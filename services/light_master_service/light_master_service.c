//
// Created by lekhacvuong on 8/25/24.
//
#include <malloc.h>
#include "light_master_service.h"
#include "sm_logger.h"
#include "sm_time_utils.h"

#define MODBUS_ID_OFFSET 1

#define SYNC_ENABLE 0

#define FLASH_DURATION_ON           4000
#define FLASH_DURATION_OFF          2000

#define YELLOW_DURATION_FIXED       4000

#define TAG "light_sv"

static light_master_t g_light_master;

static LIGHT_COLOR get_opposite_color(LIGHT_COLOR _color){
    switch (_color) {
        case LIGHT_GREEN:
            return LIGHT_RED;
        case LIGHT_RED:
            return LIGHT_GREEN;
        default:
            return _color;
    }
}

static LIGHT_COLOR get_next_color(LIGHT_COLOR _color){
    switch (_color) {
        case LIGHT_GREEN:
            return LIGHT_RED;
        case LIGHT_RED:
            return LIGHT_YELLOW;
        case LIGHT_YELLOW:
            return LIGHT_GREEN;
        case LIGHT_NONE_COLOR:
            return LIGHT_GREEN;
        default:
            return LIGHT_NONE_COLOR;
    }
}

static int8_t light_master_set_light_for_group_not_flex(light_master_t* _this, LIGHT_COLOR _color);

static int control_slave_light(light_master_t * _this, uint8_t _slave_id, LIGHT_COLOR _color){
    if(_this->m_slave[_slave_id].m_current_light == _color){
        LOG_WRN(TAG, "Slave %d is already in color %s", _slave_id, getLightColorString(_color));
        return 0;
    }
//    delayMs(50); // delay for nano modbus need

    uint8_t testId = 0;
    uint16_t testVal = _slave_id;
    testVal = (testVal<<8) | (_color&0xFF);

    int ret = sm_sv_mb_master_write_single_reg(_this->m_mb_master, testId + MODBUS_ID_OFFSET,
                                               MODBUS_HOLDING_REG_CONTROL_LIGHT_INDEX, testVal);
    if(ret != MODBUS_ERROR_NONE){
        LOG_ERR(TAG, "control slave %d to color to %s FAILED", _slave_id, getLightColorString(_color));
        return -1;
    }

    _this->m_slave[_slave_id].m_current_light = _color;
    LOG_INF(TAG, "control slave %d to color to %s SUCCEED", _slave_id, getLightColorString(_color));
    return 0;
}

static int read_slave_data(light_master_t * _this, uint8_t _slave_id){
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

    memcpy(&_this->m_slave[_slave_id].m_info, data, sizeof(light_slv_info_t));

    LOG_DBG(TAG, "Slave %d now light color %s", _slave_id,
            getLightColorString(_this->m_slave[_slave_id].m_info.m_current_light));

    return 0;
}

static bool check_if_slave_mimic(light_master_t* _this, uint8_t _slave_id){
    if(_slave_id >= _this->m_slave_number){
        return -1;
    }
    return _this->m_slave[_slave_id].m_is_mimic;
}


light_master_t *light_master_create(sm_mb_master_send_if _sendIf,
                                    sm_mb_master_recv_if _receiveIf,
                                    control_light_if _if,
                                    void *_arg) {

    light_master_t* this = &g_light_master;

    this->m_mb_master = sm_mb_master_create(_sendIf, _receiveIf, _arg);
    if(!this->m_mb_master){
        return NULL;
    }

    elapsed_timer_resetz(&this->m_sync_slave_period, DEFAULT_SYNC_PERIOD);
    this->m_slave_number = DEFAULT_SLAVE_NUMBER;
    this->m_light_duration[LIGHT_GREEN] = DEFAULT_GREEN_DURATION*1000;
    this->m_light_duration[LIGHT_RED] = DEFAULT_RED_DURATION_MIN * 1000;
    this->m_light_duration[LIGHT_YELLOW] = DEFAULT_YELLOW_DURATION*1000;
    this->m_light_duration[LIGHT_NONE_COLOR] = DEFAULT_NONE_COLOR_DURATION * 1000;
    this->m_light_ctrl_if = _if;

    this->m_current_sync_id = 0;
    elapsed_timer_resetz(&this->m_light_timer, this->m_light_duration[LIGHT_NONE_COLOR]);
    this->m_current_light = LIGHT_NONE_COLOR;
    this->m_light_ctrl_if(LIGHT_NONE_COLOR);

    for(int i = 0; i < this->m_slave_number; i++){
        this->m_slave[i].m_current_light = LIGHT_COLOR_NUMBER;
        this->m_slave[i].m_is_mimic = false;
        this->m_slave[i].m_is_connected = true;
        this->m_slave[i].m_retry_connected = 0;
        this->m_slave[i].m_light_duration[LIGHT_NONE_COLOR]     = DEFAULT_NONE_COLOR_DURATION * 1000;
        this->m_slave[i].m_light_duration[LIGHT_GREEN]    = DEFAULT_GREEN_DURATION * 1000;
        this->m_slave[i].m_light_duration[LIGHT_RED]      = DEFAULT_RED_DURATION_MIN * 1000;
        this->m_slave[i].m_light_duration[LIGHT_YELLOW]   = DEFAULT_YELLOW_DURATION * 1000;
    }

    /// TODO: load config data, if not, keep default value

    return this;
}

int8_t light_master_set_slave_flex_duration(light_master_t* _this, uint8_t _slave_id,
                                            uint16_t _green_sec, uint16_t _red_sec){
    if(!_this || _slave_id >= _this->m_slave_number)
        return -1;

    _this->m_slave[_slave_id].m_light_duration[LIGHT_NONE_COLOR]     = DEFAULT_NONE_COLOR_DURATION * 1000;
    _this->m_slave[_slave_id].m_light_duration[LIGHT_GREEN]    = _green_sec * 1000;
    _this->m_slave[_slave_id].m_light_duration[LIGHT_RED]      = _red_sec * 1000;
    _this->m_slave[_slave_id].m_light_duration[LIGHT_YELLOW]   = DEFAULT_YELLOW_DURATION * 1000;
    return 0;
}

int8_t light_master_set_group_mode(light_master_t* _this, LIGHT_GROUP_MODE _mode, uint8_t _mimic_slave_id){
    if(!_this || _mode >= GROUP_MODE_NUMBER)
        return -1;
    _this->m_is_flex_process = false;

    switch (_mode) {
        case GROUP_MODE_OPPOSITE:
            _this->m_group_mode = _mode;
            for(uint8_t id = 0; id < _this->m_slave_number; id++){
                _this->m_slave[id].m_is_mimic = false;
            }
            light_master_set_light_for_group_not_flex(_this, LIGHT_NONE_COLOR);
            elapsed_timer_resetz(&_this->m_light_timer, _this->m_light_duration[LIGHT_NONE_COLOR]);
            break;
        case GROUP_MODE_MIMIC:
            _this->m_group_mode = _mode;
            for(uint8_t id = 0; id < _this->m_slave_number; id++){
                _this->m_slave[id].m_is_mimic = true;
            }
            light_master_set_light_for_group_not_flex(_this, LIGHT_NONE_COLOR);
            elapsed_timer_resetz(&_this->m_light_timer, _this->m_light_duration[LIGHT_NONE_COLOR]);
            break;
        case GROUP_MODE_DOUBLE_ONE:
            _this->m_group_mode = _mode;
            for(uint8_t id = 0; id < _this->m_slave_number; id++){
                _this->m_slave[id].m_is_mimic = false;
            }
            _this->m_slave[_mimic_slave_id].m_is_mimic = true;
            light_master_set_light_for_group_not_flex(_this, LIGHT_NONE_COLOR);
            elapsed_timer_resetz(&_this->m_light_timer, _this->m_light_duration[LIGHT_NONE_COLOR]);
            break;
        case GROUP_MODE_FLEX:
            _this->m_group_mode = _mode;
            _this->m_is_flex_process = true;
            _this->m_light_ctrl_if(LIGHT_NONE_COLOR);
            elapsed_timer_resetz(&_this->m_light_timer, _this->m_light_duration[LIGHT_NONE_COLOR]);
            for(uint8_t id = 0; id < _this->m_slave_number; id++){
                control_slave_light(_this, id, LIGHT_NONE_COLOR);
                slave_data_t* slave = &_this->m_slave[id];
                elapsed_timer_resetz(&slave->m_light_timer, slave->m_light_duration[LIGHT_NONE_COLOR]);
            }
            break;
        default:
            break;
    }
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

int8_t light_master_set_light_for_group_not_flex(light_master_t* _this, LIGHT_COLOR _color){
    if(!_this)
        return -1;

    LOG_INF(TAG, "Master turn  light color to %s", getLightColorString(_color));
    _this->m_light_ctrl_if(_color);
    _this->m_current_light = _color;

    for(int id = 0; id < _this->m_slave_number; id++){
        if(check_if_slave_mimic(_this, id)){
            control_slave_light(_this, id, _color);
        }else{
            control_slave_light(_this, id, get_opposite_color(_color));
        }
    }
    printf("\n");
    return 0;
}

int8_t light_master_set_master_light_duration(light_master_t* _this, uint16_t _green_sec, uint16_t _red_sec){
    if(!_this)
        return -1;

    _this->m_light_duration[LIGHT_GREEN]        = DEFAULT_GREEN_DURATION*1000;
    _this->m_light_duration[LIGHT_RED]          = DEFAULT_RED_DURATION_MIN * 1000;
    _this->m_light_duration[LIGHT_YELLOW]       = DEFAULT_YELLOW_DURATION*1000;
    _this->m_light_duration[LIGHT_NONE_COLOR]   = DEFAULT_NONE_COLOR_DURATION * 1000;
    return 0;
}

int8_t light_master_reset_group(light_master_t* _this){
    _this->m_current_sync_id = 0;
    _this->m_current_light = LIGHT_NONE_COLOR;
    _this->m_current_sync_id = 0;
    for(int i = 0; i < _this->m_slave_number; i++){
        _this->m_slave[i].m_is_connected = true;
        _this->m_slave[i].m_retry_connected = 0;
    }
    elapsed_timer_reset(&_this->m_sync_slave_period);
    elapsed_timer_resetz(&_this->m_light_timer, _this->m_light_duration[LIGHT_NONE_COLOR]);
    return 0;
}

slave_data_t* light_get_slave_data(light_master_t* _this, uint8_t _slave_id){
    if(!_this || _slave_id > _this->m_slave_number)
        return NULL;
    return &_this->m_slave[_slave_id];
}

int8_t light_master_change_light_color(light_master_t* _this, LIGHT_COLOR _master_color){
    if(!_this || _master_color >= LIGHT_COLOR_NUMBER)
        return -1;

}

static void light_master_stt_auto_process(light_master_t* _this){
    if(_this->m_is_flex_process){
        if(!elapsed_timer_get_remain(&_this->m_light_timer)){
            LIGHT_COLOR color = get_next_color(_this->m_current_light);
            if(_this->m_light_ctrl_if){
                _this->m_light_ctrl_if(color);
                LOG_INF(TAG, "Master turn  light color to %s", getLightColorString(color));
                _this->m_current_light = color;
            }
            elapsed_timer_resetz(&_this->m_light_timer, _this->m_light_duration[_this->m_current_light]);
        }

        for(int slave_id = 0; slave_id < _this->m_slave_number; slave_id++){
            slave_data_t* slave = &_this->m_slave[slave_id];

            if(!elapsed_timer_get_remain(&slave->m_light_timer)){
                LIGHT_COLOR color = get_next_color(slave->m_current_light);
                if(control_slave_light(_this, slave_id, color) < 0){
                    continue;
                }
                elapsed_timer_resetz(&slave->m_light_timer, slave->m_light_duration[slave->m_current_light]);
            }
        }


    }else{
        if(!elapsed_timer_get_remain(&_this->m_light_timer)){
            LIGHT_COLOR color = get_next_color(_this->m_current_light);
            light_master_set_light_for_group_not_flex(_this, color);
            elapsed_timer_resetz(&_this->m_light_timer, _this->m_light_duration[_this->m_current_light]);
            return;
        }
    }





#if SYNC_ENABLE
    if(!elapsed_timer_get_remain(&_this->m_sync_slave_period)){
        elapsed_timer_reset(&_this->m_sync_slave_period);
        _this->m_current_sync_id++;
        if(_this->m_current_sync_id >= _this->m_slave_number){
            _this->m_current_sync_id = 0;
        }

        slave_data_t* slave = &_this->m_slave[_this->m_current_sync_id];

        if(!slave->m_is_connected){
            if(!elapsed_timer_get_remain(&slave->m_disconnect_timeout)){
                slave->m_is_connected = false;
            }else{
                return;
            }
        }

        if(read_slave_data(_this, _this->m_current_sync_id) < 0){
            LOG_WRN(TAG, "Read slave %d data FAILED", _this->m_current_sync_id);
            slave->m_retry_connected++;
            if(slave->m_retry_connected >= MAXIMUM_SLAVE_NUMBER){
                slave->m_is_connected = false;
                elapsed_timer_resetz(&slave->m_disconnect_timeout, DEFAULT_SLAVE_DISCONNECTED_TIMEOUT);
                if(_this->m_cb.slave_change_connected_stt){
                    _this->m_cb.slave_change_connected_stt(_this->m_current_sync_id, false, _this->m_cb_arg);
                }
            }
            return;
        }

        if(!slave->m_is_connected){
            slave->m_is_connected = true;
            if(_this->m_cb.slave_change_connected_stt){
                _this->m_cb.slave_change_connected_stt(_this->m_current_sync_id, true, _this->m_cb_arg);
            }
        }

        LIGHT_COLOR correctColor;
        if(check_same_cycle_slave(_this, _this->m_current_sync_id)){
            correctColor = _this->m_current_light;
        }else{
            correctColor = get_opposite_color(_this->m_current_light);
        }

        if (slave->m_info.m_current_light != correctColor){
            LOG_INF(TAG, "Slave %d now incorrect light, control it", _this->m_current_sync_id);
            control_slave_light(_this, _this->m_current_sync_id, correctColor);
        }
    }
#endif
}

static void flash_mode_process(light_master_t* _this){
    if(!elapsed_timer_get_remain(&_this->m_light_timer)){
        if(_this->m_current_light == LIGHT_NONE_COLOR){
            light_master_set_light_for_group_not_flex(_this, LIGHT_YELLOW);
            elapsed_timer_resetz(&_this->m_light_timer, FLASH_DURATION_ON);
        }else{
            light_master_set_light_for_group_not_flex(_this, LIGHT_NONE_COLOR);
            elapsed_timer_resetz(&_this->m_light_timer, FLASH_DURATION_OFF);
        }
    }
}


void light_master_process(light_master_t* _this){
    switch (_this->m_process_mode) {
        case PROCESS_MODE_AUTO:
            light_master_stt_auto_process(_this);
            break;
        case PROCESS_MODE_MANUAL:
            break;
        case PROCESS_MODE_FLASH:
            flash_mode_process(_this);
            break;
        case PROCESS_MODE_NUMBER:
            break;
    }
}

int8_t light_master_set_process_mode(light_master_t* _this, LIGHT_PROCESS_MODE _mode){
    switch (_mode) {
        case PROCESS_MODE_AUTO:
            _this->m_process_mode = PROCESS_MODE_AUTO;
            light_master_set_light_for_group_not_flex(_this, LIGHT_NONE_COLOR);
            elapsed_timer_resetz(&_this->m_light_timer, _this->m_light_duration[LIGHT_NONE_COLOR]);
            break;
        case PROCESS_MODE_MANUAL:
            _this->m_process_mode = PROCESS_MODE_MANUAL;
            break;
        case PROCESS_MODE_FLASH:
            _this->m_process_mode = PROCESS_MODE_FLASH;
            light_master_set_light_for_group_not_flex(_this, LIGHT_NONE_COLOR);
            elapsed_timer_resetz(&_this->m_light_timer, FLASH_DURATION_OFF);
            break;
        case PROCESS_MODE_NUMBER:
            break;
    }
}


