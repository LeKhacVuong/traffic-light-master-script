//
// Created by vnbk on 01/12/2023.
//

#ifndef SM_MB_RTU_IMPL_H
#define SM_MB_RTU_IMPL_H

#ifdef __cplusplus
extern "C" {
#endif

#include "sm_mb_slave_if.h"

typedef int32_t (*sm_mb_send_if)(uint8_t, const uint8_t*, uint16_t, int32_t, void*);
typedef int32_t (*sm_mb_recv_if)(uint8_t, uint8_t*, uint16_t, int32_t, void*);

sm_mb_slave_t* sm_mb_slave_create_default(uint8_t _addr, sm_mb_send_if _send_fn, sm_mb_recv_if _recv_fn, void* _arg);

sm_mb_slave_t* sm_mb_slave_create(uint8_t _addr, sm_mb_send_if _send_fn, sm_mb_recv_if _recv_fn, void* _arg);

int32_t sm_mb_slave_destroy(sm_mb_slave_t* _this);

#ifdef __cplusplus
};
#endif

#endif //SM_MB_RTU_IMPL_H
