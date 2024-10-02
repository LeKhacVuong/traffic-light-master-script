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
#define DEFAULT_GREEN_DURATION          5
#define DEFAULT_RED_DURATION_MIN        5
#define DEFAULT_YELLOW_DURATION         4
#define DEFAULT_NONE_COLOR_DURATION     3

#define DEFAULT_SLAVE_DISCONNECTED_TIMEOUT 60 //sec ?

#define MAX_TIME_RETRY_CTRL_SLAVE       20

typedef void (*control_light_if)(LIGHT_COLOR color);

typedef struct{
    void (*slave_change_connected_stt)(uint8_t id, uint8_t stt, void* arg);
}light_master_call_back_t;

typedef enum {
    PROCESS_MODE_AUTO,
    PROCESS_MODE_MANUAL,
    PROCESS_MODE_FLASH,
    PROCESS_MODE_NUMBER
}LIGHT_PROCESS_MODE;

typedef enum{
    GROUP_MODE_OPPOSITE,
    GROUP_MODE_MIMIC,
    GROUP_MODE_DOUBLE_ONE,
    GROUP_MODE_FLEX,
    GROUP_MODE_NUMBER
}LIGHT_GROUP_MODE;

typedef struct {
    light_slv_info_t m_info;

    //check connect data
    uint8_t m_is_connected;
    uint8_t m_retry_connected;
    elapsed_timer_t m_disconnect_timeout;

    bool m_is_mimic;
    LIGHT_COLOR m_current_light;

    // for flex config
    uint16_t m_light_duration[LIGHT_COLOR_NUMBER];
    elapsed_timer_t m_light_timer;
}slave_data_t;

typedef struct{
    LIGHT_PROCESS_MODE m_process_mode;
    LIGHT_GROUP_MODE m_group_mode;

    bool m_is_flex_process;

    LIGHT_COLOR m_current_light;
    uint16_t m_light_duration[LIGHT_COLOR_NUMBER];
    elapsed_timer_t m_light_timer;

    uint8_t m_slave_number;
    slave_data_t m_slave[MAXIMUM_SLAVE_NUMBER];
    elapsed_timer_t m_sync_slave_period;
    uint8_t m_current_sync_id;

    sm_mb_master_t* m_mb_master;
    control_light_if m_light_ctrl_if;

    light_master_call_back_t m_cb;
    void* m_cb_arg;
}light_master_t;


light_master_t* light_master_create(sm_mb_master_send_if _sendIf,
                                    sm_mb_master_recv_if _receiveIf,
                                    control_light_if _if,
                                    void* _arg);

int8_t light_master_set_process_mode(light_master_t* _this, LIGHT_PROCESS_MODE _mode);

int8_t light_master_set_slave_flex_duration(light_master_t* _this, uint8_t _slave_id,
                                            uint16_t _green_sec, uint16_t _red_sec);

int8_t light_master_set_group_mode(light_master_t* _this, LIGHT_GROUP_MODE _mode, uint8_t _mimic_slave_id);

int8_t light_master_change_light_color(light_master_t* _this, LIGHT_COLOR _master_color);

int8_t light_master_set_ctrl_light_if(light_master_t* _this, control_light_if _if);

int8_t light_master_set_slave_number(light_master_t* _this, uint8_t _slave_num);

int8_t light_master_set_sync_period(light_master_t* _this, uint8_t _period);

int8_t light_master_set_master_light_duration(light_master_t* _this, uint16_t _green_sec, uint16_t _red_sec);

int8_t light_master_reset_group(light_master_t* _this);

slave_data_t* light_get_slave_data(light_master_t* _this, uint8_t _slave_id);

void light_master_process(light_master_t* _this);

#endif //LIGHT_SLAVE_LIGHT_MASTER_SERVICE_H
