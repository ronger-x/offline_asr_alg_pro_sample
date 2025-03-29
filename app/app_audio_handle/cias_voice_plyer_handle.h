/*
 * @FileName:: 
 * @Author: 
 * @Date: 2022-04-21 13:58:52
 * @LastEditTime: 2023-05-15 13:54:37
 * @Description: 
 */
#ifndef _CIAS_VOICE_PLYER_HANDLE_H_
#define _CIAS_VOICE_PLYER_HANDLE_H_

#include "sdk_default_config.h"
#include <stdbool.h>
#include "cias_network_msg_protocol.h"
#include "audio_play_os_port.h"

typedef enum 
{
    START_PLAY_TTS = 0,
    START_PLAY_M4A,
    START_PLAY_MP3,
}start_play_type_t;

typedef struct {
    start_play_type_t start_play;
    bool recv_media_stream_end;
    bool acc_decode_fail;
    bool mp3_decode_fail;
}media_play_resume_control;

extern audio_play_os_stream_t tts_player ;
extern audio_play_os_sem_t tts_player_end ;

extern audio_play_os_stream_t mp3_player ;
extern audio_play_os_sem_t mp3_player_end ;

extern audio_play_os_stream_t m4a_player ;
extern audio_play_os_sem_t m4a_player_end ;

extern audio_play_os_stream_t wav_player ;
extern audio_play_os_sem_t wav_player_end ;


void audio_player_param_init(void);

void set_repeat_dialogue_flag(bool val);
bool get_repeat_dialogue_flag(void);

void set_voice_invalid(bool val);


void stop_net_player(void);
void set_net_player_status(net_player_type_t net_player ,net_player_status_t val);

net_player_type_t get_net_play_type();

net_player_status_t get_net_player_status(net_player_type_t net_player);



int32_t pause_net_music(bool from_isr_flag);
void clear_net_player_stopsem(void);


void set_music_data_offset(unsigned int offset);
unsigned int get_music_data_offset(void);

bool is_net_player(void);

void tts_player_end_callbk(int32_t play_cb_state);
void wav_player_end_callbk(int32_t play_cb_state);
void mp3_player_end_callbk(int32_t play_cb_state);
void m4a_player_end_callbk(int32_t play_cb_state);

int32_t network_audio_processing(uint8_t *msg_buf);


#endif