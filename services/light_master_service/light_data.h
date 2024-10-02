//
// Created by lekhacvuong on 8/25/24.
//

#ifndef LIGHT_SLAVE_LIGH_DATA_H
#define LIGHT_SLAVE_LIGH_DATA_H

typedef enum LIGHT_VALUE{
    LIGHT_GREEN,
    LIGHT_RED,
    LIGHT_YELLOW,
    LIGHT_NONE_COLOR,
    LIGHT_COLOR_NUMBER
}LIGHT_COLOR;

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

#define RED   "\x1B[31m"
#define GRN   "\x1B[32m"
#define YEL   "\033[93m"
#define BLU   "\x1B[34m"
#define MAG   "\x1B[35m"
#define CYN   "\x1B[36m"
#define WHT   "\x1B[37m"
#define RESET "\x1B[0m"

static inline const char* getLightColorString(uint8_t _color) {
    switch (_color) {
        case LIGHT_GREEN:
            return GRN "GREEN" WHT;
        case LIGHT_RED:
            return RED "RED" WHT;
        case LIGHT_YELLOW:
            return YEL "YELLOW" WHT;
        case LIGHT_NONE_COLOR:
            return WHT "NONE" WHT;
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
