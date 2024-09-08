//
// Created by lekhacvuong on 8/25/24.
//

#ifndef LIGHT_SLAVE_LIGH_DATA_H
#define LIGHT_SLAVE_LIGH_DATA_H

typedef enum LIGHT_VALUE{
    LIGHT_GREEN,
    LIGHT_RED,
    LIGHT_YELLOW,
    LIGHT_NONE,
    LIGHT_NUMBER
}LIGHT_VALUE;

typedef enum LIGHT_STT{
    LIGHT_ACTIVE,
    LIGHT_INACTIVE,
    LIGHT_ERROR
}LIGHT_STT;

typedef struct{
    uint8_t m_id;
    uint8_t m_stt;  //enum

    uint8_t m_current_light; //enum

    uint32_t m_latitude;
    uint32_t m_longitude;
}light_slv_info_t;

static inline const char* getLightColorString(uint8_t _color){
    switch (_color) {
        case LIGHT_GREEN:
            return "GREEN";
        case LIGHT_RED:
            return "RED";
        case LIGHT_YELLOW:
            return "YELLOW";
        case LIGHT_NONE:
            return "NONE";;
        default:
            return "INVALID";
    }
}

///Modbus define

#define MODBUS_INPUT_REG_SYNC_DATA_INDEX                    0

#define MODBUS_INPUT_REG_SYNC_DATA_REG_NUM                  20

#define MODBUS_HOLDING_REG_CONTROL_LIGHT_INDEX              0
#define MODBUS_HOLDING_REG_SET_LIGHT_DURATION_INDEX         1

#endif //LIGHT_SLAVE_LIGH_DATA_H
