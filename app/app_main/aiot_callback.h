/*
 * @FileName:: 
 * @Author: 
 * @Date: 2022-10-19 10:35:47
 * @LastEditTime: 2022-12-12 17:52:13
 * @Description: 
 */
#ifndef __CIAS_AIOT_CALLBACK_H__
#define __CIAS_AIOT_CALLBACK_H__

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "FreeRTOS.h"

#include "cias_audio_data_handle.h"

int32_t aiot_asr_callback(uint16_t cmd_id);
int32_t aiot(uint8_t *msg_buf);
void play_send_pcm_finish();

#endif //__CIAS_AIOT_CALLBACK_H__