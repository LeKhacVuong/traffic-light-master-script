//
// Created by lekhacvuong on 8/25/24.
//

#ifndef LIGHT_SLAVE_LIGHT_MASTER_SERVICE_H
#define LIGHT_SLAVE_LIGHT_MASTER_SERVICE_H

#include "sm_mb_master_impl.h"
#include "light_data.h"
#include "sm_elapsed_timer.h"

#define MAXIMUM_SLAVE_NUMBER            3

#define DEFAULT_SYNC_PERIOD             500
#define DEFAULT_SLAVE_NUMBER            3
#define DEFAULT_GREEN_DURATION          20
#define DEFAULT_RED_DURATION            15
#define DEFAULT_YELLOW_DURATION         5
#define DEFAULT_NONE_COLOR_DURATION     10

#define DEFAULT_SLAVE_DISCONNECTED_TIMEOUT 180 //sec ?

typedef void (*control_light_if)(LIGHT_VALUE color);

typedef struct {
    light_slv_info_t m_info;

    uint8_t m_is_connected;
    uint8_t m_retry_connected;
    elapsed_timer_t m_disconnect_timeout;
}slave_data_t;

typedef struct{
    uint8_t m_stt;
    uint8_t m_current_light;
    uint16_t m_light_duration[LIGHT_NUMBER];
    elapsed_timer_t m_light_timer;

    uint8_t m_slave_number;
    slave_data_t m_slave_data[MAXIMUM_SLAVE_NUMBER];
    elapsed_timer_t m_sync_slave_period;
    uint8_t m_current_sync_id;

    sm_mb_master_t* m_mb_master;
    control_light_if m_light_ctrl_if;
}light_master_t;


light_master_t* light_master_create(sm_mb_master_send_if _sendIf, sm_mb_master_recv_if _receiveIf, void* _arg);

int8_t light_master_set_ctrl_light_if(light_master_t* _this, control_light_if _if);

int8_t light_master_set_slave_number(light_master_t* _this, uint8_t _slave_num);

int8_t light_master_set_sync_period(light_master_t* _this, uint8_t _period);

int8_t light_master_set_light_color(light_master_t* _this, LIGHT_VALUE _color);

int8_t light_master_set_light_duration(light_master_t* _this, LIGHT_VALUE _color, uint16_t _duration);

slave_data_t* light_get_slave_data(light_master_t* _this, uint8_t _slave_id);

void light_master_process(light_master_t* _this);

#endif //LIGHT_SLAVE_LIGHT_MASTER_SERVICE_H
