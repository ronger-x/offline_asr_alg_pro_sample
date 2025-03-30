#ifndef __CIAS_AUDIO_DATA_HANDLE_H__
#define __CIAS_AUDIO_DATA_HANDLE_H__
#include "ci130x_scu.h"
#include "ci130x_uart.h"
#include "user_config.h"
#include "cias_common.h"

typedef enum{
    NET_MSG_HEAD = 1,
    NET_MSG_DATE = 2,
    NET_MSG_ERR  = 0,
    NET_MSG_IDE  = 3,
}cias_communicate_state_t;

int cias_audio_data_transmission(void);

#endif   //__CIAS_AUDIO_DATA_HANDLE_H__
