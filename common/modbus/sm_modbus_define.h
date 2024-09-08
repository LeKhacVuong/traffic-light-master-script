//
// Created by mrgundam on 06/05/2024.
//

#ifndef SM_MODBUS_DEFINE_H
#define SM_MODBUS_DEFINE_H

#define MODBUS_MAX_COILS_SUPPORT    250

#define MODBUS_RESPONSE_TIMEOUT_DEFAULT 200

#define BSS_MODBUS_COM_BAUD_RATE_DEFAULT    115200

enum MODBUS_FUNCTION_CODE{
    READ_COILS                          = 0x01,
    READ_DISCRETE_INPUT                 = 0x02,
    READ_HOLDING_REGISTERS              = 0x03,
    READ_INPUT_REGISTERS                = 0x04,
    WRITE_SINGLE_COIL                   = 0x05,
    WRITE_SINGLE_HOLDING_REGISTER       = 0x06,
    WRITE_MULTIPLE_COILS                = 0x15,
    WRITE_MULTIPLE_HOLDING_REGISTERS    = 0x16,
    READ_FILE_RECORD                    = 0x14,
    WRITE_FILE_RECORD                   = 0x15
};

typedef enum {
    // Library errors
    MODBUS_ERROR_INVALID_UNIT_ID = -7,  /**< Received invalid unit ID in response from server */
    MODBUS_ERROR_INVALID_TCP_MBAP = -6, /**< Received invalid TCP MBAP */
    MODBUS_ERROR_CRC = -5,              /**< Received invalid CRC */
    MODBUS_ERROR_TRANSPORT = -4,        /**< Transport error */
    MODBUS_ERROR_TIMEOUT = -3,          /**< Read/write timeout occurred */
    MODBUS_ERROR_INVALID_RESPONSE = -2, /**< Received invalid response from server */
    MODBUS_ERROR_INVALID_ARGUMENT = -1, /**< Invalid argument provided */
    MODBUS_ERROR_NONE = 0,              /**< No error */

    // Modbus exceptions
    MODBUS_EXCEPTION_ILLEGAL_FUNCTION = 1,      /**< Modbus exception 1 */
    MODBUS_EXCEPTION_ILLEGAL_DATA_ADDRESS = 2,  /**< Modbus exception 2 */
    MODBUS_EXCEPTION_ILLEGAL_DATA_VALUE = 3,    /**< Modbus exception 3 */
    MODBUS_EXCEPTION_SERVER_DEVICE_FAILURE = 4, /**< Modbus exception 4 */
}MODBUS_ERROR_CODE;

#define MODBUS_RTU_ENCODE_ID(code,reg)               ((((uint32_t)code) << 16) | (reg+1))
#define MODBUS_RTU_DECODE_FUNCTION_CODE(value)       ((uint8_t)((value)>>16 & 0xFF))
#define MODBUS_RTU_DECODE_REGISTER(value)            ((uint16_t)(((value) & 0xFFFF) - 1))

/// DECODE register Value;
#define MODBUS_RTU_REG_GET_HIGH_BYTE(value)         (((value) >> 8) & 0xFF)
#define MODBUS_RTU_REG_GET_LOW_BYTE(value)          ((value) & 0xFF)


#endif //SM_MODBUS_DEFINE_H
