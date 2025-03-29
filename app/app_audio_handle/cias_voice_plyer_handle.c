/*
 * @FileName:: 
 * @Author: 
 * @Date: 2022-04-21 13:58:23
 * @LastEditTime: 2023-05-15 14:46:08
 * @Description: 
 */
#include "audio_play_api.h"
#include "cias_voice_plyer_handle.h"
#include "audio_play_api.h"
#include "cias_audio_data_handle.h"
#include "status_share.h"

media_play_resume_control media_play;
audio_play_os_stream_t tts_player = NULL;
audio_play_os_sem_t tts_player_end = NULL;

audio_play_os_stream_t mp3_player = NULL;
audio_play_os_sem_t mp3_player_end = NULL;

audio_play_os_stream_t m4a_player = NULL;
audio_play_os_sem_t m4a_player_end = NULL;

audio_play_os_stream_t wav_player = NULL;
audio_play_os_sem_t wav_player_end = NULL;

net_player_status_t tts_player_status = NET_PLAYER_IDLE;
net_player_status_t mp3_player_status = NET_PLAYER_IDLE;
net_player_status_t m4a_player_status = NET_PLAYER_IDLE;
net_player_status_t wav_player_status = NET_PLAYER_IDLE;

net_player_type_t net_play_type = PLAY_NULL; // 1 mp3  ;2 tts


static bool repeat_dialogue_flag = false;
static bool voice_invalid = false;
#if 1



/**
 * @brief Set the repeat dialogue flag object
 * 设置多轮对话标记
 *
 * @param val
 */
void set_repeat_dialogue_flag(bool val)
{
    repeat_dialogue_flag = val;
}

#if 1
/**
 * @brief Get the repeat dialogue flag object
 * 获取多轮对话标记
 *
 * @return true
 * @return false
 */
bool get_repeat_dialogue_flag(void)
{
    return repeat_dialogue_flag;
}

/**
 * @brief 设置语音无效标记
 *
 * @param val
 */
void set_voice_invalid(bool val)
{
    voice_invalid = val;
}

/**
 * @brief 获取语音无效标记
 *
 * @return true 无效
 * @return false 有效
 */
bool get_voice_invalid(void)
{
    return voice_invalid;
}


/**
 * @brief Set the net player status object
 *
 * @param net_player
 * @param val
 */
void set_net_player_status(net_player_type_t net_player, net_player_status_t val)
{

    if (net_player == PLAY_MP3)
    {
        mp3_player_status = val;
    }
    else if (net_player == PLAY_TTS)
    {
        tts_player_status = val;
    }
    else if (net_player == PLAY_M4A)
    {
        m4a_player_status = val;
    }
    else if (net_player == PLAY_WAV)
    {
        wav_player_status = val;
    }
    else
    {
        ci_logdebug(LOG_MEDIA, "err state\n");
    }
}

/**
 * @brief Get the net player status object
 *
 * @param net_player
 * @return net_player_status_t
 */
net_player_status_t get_net_player_status(net_player_type_t net_player)
{
    if (net_player == PLAY_MP3)
    {
        return mp3_player_status;
    }
    else if (net_player == PLAY_TTS)
    {
        return tts_player_status;
    }
    else if (net_player == PLAY_M4A)
    {
        return m4a_player_status;
    }
    else if (net_player == PLAY_WAV)
    {
        return wav_player_status;
    }
    else
    {
        return NET_PLAYER_IDLE;
    }
}


uint8_t next_resume_play_data[8] = {0};
static unsigned int music_data_offset = 0;
static unsigned int last_music_data_offset = 0;


unsigned int get_music_data_offset(void)
{
    return music_data_offset;
}

void set_music_data_offset(unsigned int offset)
{
    music_data_offset = offset;
}

uint8_t *get_next_resume_play_data(void)
{
    return next_resume_play_data;
}

/**
 * @brief Set the next resume play data object
 *
 * @param next_data NULL 清空buf
 */
void set_next_resume_play_data(uint8_t *next_data)
{
    if (next_data != NULL)
    {
        memcpy(next_resume_play_data, next_data, 8);
    }
    else
    {
        memset(next_resume_play_data, 0, 8);
    }
}

void stop_net_player(void)
{
    outside_send_end_sem(tts_player_end);
    outside_send_end_sem(mp3_player_end);
    outside_send_end_sem(m4a_player_end);
    outside_send_end_sem(wav_player_end);
}

void clear_net_player_stopsem(void)
{
    outside_clear_stream(NULL, m4a_player_end);
}


/**
 * @brief Set the net play type object
 *
 * @param val 1 mp3 2 tts
 */
void set_net_play_type(net_player_type_t val)
{
    if (val != PLAY_NULL)
    {
        send_exit_wakeup_msg(1);
    }
    net_play_type = val;
}

net_player_type_t get_net_play_type()
{
    return net_play_type;
}


static bool rev_voice_end_flag = false;
void mp3_player_end_callbk(int32_t play_cb_state)
{
    if (rev_voice_end_flag == false) //先收到0x0818 后收到0x0817
    {
        set_net_player_status(PLAY_MP3, NET_PLAYER_IDLE);
        set_net_play_type(PLAY_NULL);
        set_music_data_offset(get_play_offset());
        uint8_t next_data[8] = {0};
        if (RETURN_OK == get_near_offset_data(next_data))
        {
            set_next_resume_play_data(next_data);
            ci_logdebug(LOG_MEDIA, "next_data = 0x%x 0x%x 0x%x 0x%x ", next_data[0], next_data[1], next_data[2], next_data[3]);
            ci_logdebug(LOG_MEDIA, "0x%x 0x%x 0x%x 0x%x\n", next_data[4], next_data[5], next_data[6], next_data[7]);
        }
        if (AUDIO_PLAY_CB_STATE_INTERNAL_ERR == play_cb_state)
        {
            if ((get_music_data_offset() != 0) || (last_music_data_offset != 0))
            {
                cias_send_cmd(NET_PLAY_NEXT, DEF_FILL);
            }
            last_music_data_offset = get_music_data_offset();
            ci_logdebug(LOG_MEDIA, "INTERNAL_ERR\n");
        }
        else if (AUDIO_PLAY_CB_STATE_DONE == play_cb_state)
        {
            ci_logdebug(LOG_MEDIA, "STATE_DONE\n");
        }
        last_music_data_offset = get_music_data_offset();
        cias_send_cmd(NET_PLAY_RECONECT_URL, get_music_data_offset());
        if (AUDIO_PLAY_CB_STATE_PAUSE == play_cb_state)
        {
            ci_logdebug(LOG_MEDIA, "STATE_PAUSE\n");
            set_net_player_status(PLAY_MP3, NET_PLAYER_PAUSE);
        }
        // if(!get_is_wake_up())
        {
            cias_send_cmd(NET_PLAY_END, DEF_FILL);
        }

        //ci_logdebug(LOG_MEDIA, "MP3 play_end, change to bf *********************\n");
#if CIAS_AIOT_ENABLE
       // ciss_set(CI_SS_PLAY_TYPE,CI_SS_PLAY_TYPE_CMD_PROMPT);
#endif
        start_recv_flag = false;
         if((media_play.mp3_decode_fail)&&(!media_play.recv_media_stream_end))
        {
            media_play.mp3_decode_fail = false;
            ci_loginfo(LOG_USER, "\r\n NET_PLAY_RESTART=============\r\n");
            cias_send_cmd(NET_PLAY_RESTART, DEF_FILL);
        } 
    }
}

void m4a_player_end_callbk(int32_t play_cb_state)
{
#if USE_AAC_DECODER
#if CIAS_AIOT_ENABLE
    //ciss_set(CI_SS_PLAY_TYPE,CI_SS_PLAY_TYPE_CMD_PROMPT);
#endif
    play_cb_state = -1;
    set_net_player_status(PLAY_M4A, NET_PLAYER_IDLE);
    set_net_play_type(PLAY_NULL);
    set_music_data_offset(get_play_offset());
    uint8_t next_data[8] = {0};
    if (RETURN_OK == get_near_offset_data(next_data))
    {
        set_next_resume_play_data(next_data);
    }
    if (AUDIO_PLAY_CB_STATE_PARSE_M4A_MEM_ERR == play_cb_state)
    {
        cias_send_cmd(NET_PLAY_NEXT, DEF_FILL);
        last_music_data_offset = get_music_data_offset();
        ci_logdebug(LOG_MEDIA, "M4A_MEM_ERR\n");
        return;
    }
    else if (AUDIO_PLAY_CB_STATE_INTERNAL_ERR == play_cb_state)
    {
        if ((get_music_data_offset() != 0) || (last_music_data_offset != 0))
        {
            cias_send_cmd(NET_PLAY_NEXT, DEF_FILL);
        }
        last_music_data_offset = get_music_data_offset();
        //ci_logdebug(LOG_MEDIA, "M4A play_end\n");
        //ci_logdebug(LOG_MEDIA, "M4A play_end :....%d,,,,,%d\n", media_play.acc_decode_fail,media_play.recv_media_stream_end);
        if((media_play.acc_decode_fail)&&(!media_play.recv_media_stream_end))
        {
            media_play.acc_decode_fail = false;
            ci_loginfo(LOG_USER, "\r\n NET_PLAY_RESTART=============\r\n");
            cias_send_cmd(NET_PLAY_RESTART, DEF_FILL);
            change_acc_decode_size();
        }
        else if (media_play.recv_media_stream_end)
        {
            ci_loginfo(LOG_USER, "\r\n NET_PLAY_NEXT=============\r\n");
            cias_send_cmd(PLAY_NEXT, DEF_FILL);
        }
        return;
    }
    else if ((AUDIO_PLAY_CB_STATE_DONE == play_cb_state) || (AUDIO_PLAY_CB_STATE_PARSE_FILE_LEASTDATA_ERR == play_cb_state))
    {
        ci_logdebug(LOG_MEDIA, "STATE_DONE %d\n", play_cb_state);
    }
    last_music_data_offset = get_music_data_offset();
    cias_send_cmd(NET_PLAY_RECONECT_URL, get_music_data_offset());
    if (AUDIO_PLAY_CB_STATE_PAUSE == play_cb_state)
    {
        ci_logdebug(LOG_MEDIA, "STATE_PAUSE\n");
        set_net_player_status(PLAY_M4A, NET_PLAYER_PAUSE);
    }
    // if(!get_is_wake_up())
    {
        cias_send_cmd(NET_PLAY_END, DEF_FILL);
    }
#endif
}

void wav_player_end_callbk(int32_t play_cb_state)
{
    set_net_player_status(PLAY_WAV, NET_PLAYER_IDLE);
    set_net_play_type(PLAY_NULL);
    set_music_data_offset(get_play_offset());
    uint8_t next_data[8] = {0};
    if (RETURN_OK == get_near_offset_data(next_data))
    {
        set_next_resume_play_data(next_data);
        ci_logdebug(LOG_MEDIA, "next_data = 0x%x 0x%x 0x%x 0x%x ", next_data[0], next_data[1], next_data[2], next_data[3]);
        ci_logdebug(LOG_MEDIA, "0x%x 0x%x 0x%x 0x%x\n", next_data[4], next_data[5], next_data[6], next_data[7]);
    }
    if (AUDIO_PLAY_CB_STATE_INTERNAL_ERR == play_cb_state)
    {
        if ((get_music_data_offset() != 0) || (last_music_data_offset != 0))
        {
            cias_send_cmd(NET_PLAY_NEXT, DEF_FILL);
        }
        last_music_data_offset = get_music_data_offset();
        ci_logdebug(LOG_MEDIA, "INTERNAL_ERR\n");
        ci_logdebug(LOG_MEDIA, "WAV play_end\n");
        return;
    }
    else if (AUDIO_PLAY_CB_STATE_DONE == play_cb_state)
    {
        ci_logdebug(LOG_MEDIA, "STATE_DONE\n");
    }
    last_music_data_offset = get_music_data_offset();
    cias_send_cmd(NET_PLAY_RECONECT_URL, get_music_data_offset());
    if (AUDIO_PLAY_CB_STATE_PAUSE == play_cb_state)
    {
        ci_logdebug(LOG_MEDIA, "STATE_PAUSE\n");
        set_net_player_status(PLAY_WAV, NET_PLAYER_PAUSE);
    }
    // if(!get_is_wake_up())
    {
        cias_send_cmd(NET_PLAY_END, DEF_FILL);
    }
    ci_logdebug(LOG_MEDIA, "WAV play_end\n");
}

void tts_player_end_callbk(int32_t play_cb_state)
{
    set_net_play_type(PLAY_NULL);
    /*是否进入多轮对话*/
    if (get_repeat_dialogue_flag())
    {
        set_repeat_dialogue_flag(false);
        // enter_next_dialogue(EXIT_WAKEUP_TIME);
    }
    else if (get_net_player_status(PLAY_MP3) == NET_PLAYER_PAUSE)
    {
        // ci_logdebug(LOG_MEDIA,"resume MP3 start!\n");
        cias_send_cmd(NET_PLAY_RECONECT_URL, get_music_data_offset());
    }
    else if (get_net_player_status(PLAY_M4A) == NET_PLAYER_PAUSE)
    {
        // ci_logdebug(LOG_MEDIA,"resume M4a start!\n");
        cias_send_cmd(NET_PLAY_RECONECT_URL, get_music_data_offset());
    }
    else if (get_net_player_status(PLAY_WAV) == NET_PLAYER_PAUSE)
    {
        // ci_logdebug(LOG_MEDIA,"resume WAV start!\n");
        cias_send_cmd(NET_PLAY_RECONECT_URL, get_music_data_offset());
    }
    cias_send_cmd(NET_PLAY_END, DEF_FILL);
    set_net_player_status(PLAY_TTS, NET_PLAYER_IDLE);
    //ci_logdebug(LOG_MEDIA, "TTS play_end, change to bf *********************\n");
#if CIAS_AIOT_ENABLE
   // ciss_set(CI_SS_PLAY_TYPE,CI_SS_PLAY_TYPE_CMD_PROMPT);
#endif
    start_recv_flag = false;
    cias_send_cmd(PLAY_TTS_END, DEF_FILL);
    if((media_play.mp3_decode_fail)&&(!media_play.recv_media_stream_end))
    {
        media_play.mp3_decode_fail = false;
        ci_loginfo(LOG_USER, "\r\n NET_PLAY_RESTART=============\r\n");
        cias_send_cmd(NET_PLAY_RESTART, DEF_FILL);
    }
}

int32_t pause_net_music(bool from_isr_flag)
{
    BaseType_t pxHigherPriorityTaskWoken;
    int32_t ret = RETURN_ERR;
    if (check_current_playing())
    {
        if (get_net_player_status(PLAY_M4A) == NET_PLAYER_START)
        {
            if (from_isr_flag)
            {
                ret = pause_play(NULL, &pxHigherPriorityTaskWoken);
            }
            else
            {
                ret = pause_play(NULL, NULL);
            }
        }
    }
    return ret;
}


bool is_net_player(void)
{
#if AUDIO_PLAYER_ENABLE
    if (check_current_playing())
    {
        if (PLAY_NULL != get_net_play_type())
        {
            return true;
        }
    }
    return false;
#else
    return check_current_playing();
#endif
}
#endif
#endif

#if VOICE_PLAY_BY_UART
/****************
* 播报器参数初始化
* 
* * * **/
void audio_player_param_init()    
{
    outside_init_stream(&mp3_player, &mp3_player_end, IOT_AUDIO_PLAY_BUF_SIZE);
    if (!mp3_player)
        mprintf("mp3_player alloc error %x\n", mp3_player);
    else
        ci_loginfo(LOG_MEDIA, "mp3_player alloc success\n");
    m4a_player = mp3_player;
    m4a_player_end = mp3_player_end;
    wav_player = mp3_player;
    wav_player_end = mp3_player_end;

    tts_player = mp3_player;
    tts_player_end = mp3_player_end;

    if (tts_player == NULL)
    {
        ci_logerr(LOG_MEDIA, "tts_player alloc error\n");
    }
    set_curr_outside_handle(tts_player, tts_player_end);
}

extern bool no_audio_data_flag;
extern bool recv_music_end_sem;

/**
 * Network broadcast processing
 *
 */

int32_t network_audio_processing(uint8_t *msg_buf)
{
    int32_t ret = RETURN_OK;
    static int32_t recv_music_data_len = 0;
    static int32_t recv_net_mp3_num = 0;
    static int16_t recv_resume_cmd_num = 0;

    cias_data_standard_head_t *pheader = (cias_data_standard_head_t *)(msg_buf);
    wifi_communicate_cmd_t wifi_cmd = (wifi_communicate_cmd_t)pheader->type;


    switch (wifi_cmd)
    {

            /*云端播放数据*/
            case PLAY_DATA_GET:
            {
                if (pheader->len == 0)
                {
                    ci_logdebug(LOG_MEDIA, "pheader->len = %d\n", pheader->len);
                    ret = RETURN_ERR;
                    break;
                }

                int ret_tts = outside_write_stream(cur_play_stream, (uint32_t)(msg_buf + 16), pheader->len, false);
                if (RETURN_OK == ret_tts)
                {
                    if (recv_net_mp3_num == 0)
                    {
                        if ((get_net_player_status(get_net_play_type()) == NET_PLAYER_RESUME) && (recv_music_data_len == 0))
                        {
                            if (0 != memcmp(get_next_resume_play_data(), (uint8_t *)(msg_buf + 16), 8))
                            {
                                ci_loginfo(LOG_MEDIA, "error first packet\n");
                                for (int32_t w = 0; w < 8; w++)
                                {
                                    ci_loginfo(LOG_MEDIA, "0x%x ", *(uint8_t *)(msg_buf + 16 + w));
                                    mprintf("0x%x ", *(uint8_t *)(msg_buf + 16 + w));
                                }
                                ci_loginfo(LOG_MEDIA, "\n");
                                mprintf("\n");
                                set_net_player_status(get_net_play_type(), NET_PLAYER_IDLE);
                                cias_send_cmd(NET_PLAY_NEXT, DEF_FILL);
                            }
                            else
                            {
                                ci_loginfo(LOG_MEDIA, "resume ok\n");
                            }
                        }
                    }
                    recv_music_data_len += pheader->len;
                    recv_net_mp3_num += pheader->len;
                    recv_resume_cmd_num = 0;
                    // mprintf("write data ok!  (%d)recv_net_mp3_num = %d size = %d pheader->len = %d\n", recv_music_data_len, recv_net_mp3_num, audio_play_stream_buffer_get_spaces_size(cur_play_stream), pheader->len);
                }
                else
                {
                    ci_logdebug(LOG_MEDIA, "write data fail!\n");
                }

                if(recv_net_mp3_num >= PLAY_BUF_SIZE_MAX) start_recv_flag = true;
                if ((recv_music_data_len >= PLAY_BUF_SIZE_MAX ) && (get_net_player_status(get_net_play_type()) == NET_PLAYER_READY))
                {                           //18
                    ci_logdebug(LOG_MEDIA, "write data get_net_play_type =%d\n", get_net_play_type());
                    if (get_net_play_type() == PLAY_MP3)
                    {
                        play_with_outside(0, "mp3", mp3_player_end_callbk);
                        set_net_player_status(PLAY_MP3, NET_PLAYER_START);
                    }
                    else if (get_net_play_type() == PLAY_TTS)
                    {
                        play_with_outside(0, "mp3", tts_player_end_callbk);
                        set_net_player_status(PLAY_TTS, NET_PLAYER_START);
                        get_response_timer_cnt();
                    }
                    else if (get_net_play_type() == PLAY_M4A)
                    {
                        play_with_outside(0, "m4a", m4a_player_end_callbk);
                        set_net_player_status(PLAY_M4A, NET_PLAYER_START);
                    }
                    else if (get_net_play_type() == PLAY_WAV)
                    {
                        play_with_outside(0, "ms_wav", wav_player_end_callbk);
                        set_net_player_status(PLAY_WAV, NET_PLAYER_START);
                    }
                    // ci_logdebug(LOG_MEDIA, "--start playing-- play_type1 = %d\n", get_net_play_type());
                    no_audio_data_flag = false;
                }
                else if (((recv_music_data_len >= 20 * 1024) && (get_net_player_status(get_net_play_type()) == NET_PLAYER_RESUME) && (no_audio_data_flag)))
                {                                 //22
                    if (get_net_play_type() == PLAY_MP3)
                    {
                        play_with_outside(get_music_data_offset(), "history", mp3_player_end_callbk);
                        set_net_player_status(PLAY_MP3, NET_PLAYER_START);
                    }
                    else if (get_net_play_type() == PLAY_TTS)
                    {
                        play_with_outside(get_music_data_offset(), "history", tts_player_end_callbk);
                        set_net_player_status(PLAY_TTS, NET_PLAYER_START);
                        get_response_timer_cnt();
                    }
                    else if (get_net_play_type() == PLAY_M4A)
                    {
                        play_with_outside(get_music_data_offset(), "history", m4a_player_end_callbk);
                        set_net_player_status(PLAY_M4A, NET_PLAYER_START);
                    }
                    else if (get_net_play_type() == PLAY_WAV)
                    {
                        play_with_outside(get_music_data_offset(), "history", wav_player_end_callbk);
                        set_net_player_status(PLAY_WAV, NET_PLAYER_START);
                    }
                    ci_logdebug(LOG_MEDIA, "--start playing-- play_type2 = %d\n", get_net_play_type());
                    no_audio_data_flag = false;
                }
                else if (((recv_music_data_len >= 20 * 1024) && (get_net_player_status(get_net_play_type()) == NET_PLAYER_RESUME) && (!no_audio_data_flag)))
                {                                //22
                    if (get_net_play_type() == PLAY_MP3)
                    {
                        play_with_outside(get_music_data_offset(), "history", mp3_player_end_callbk);
                        set_net_player_status(PLAY_MP3, NET_PLAYER_START);
                    }
                    else if (get_net_play_type() == PLAY_TTS)
                    {
                        play_with_outside(get_music_data_offset(), "history", tts_player_end_callbk);
                        set_net_player_status(PLAY_TTS, NET_PLAYER_START);
                        get_response_timer_cnt();
                    }
                    else if (get_net_play_type() == PLAY_M4A)
                    {
                        play_with_outside(get_music_data_offset(), "history", m4a_player_end_callbk);
                        set_net_player_status(PLAY_M4A, NET_PLAYER_START);
                    }
                    else if (get_net_play_type() == PLAY_WAV)
                    {
                        play_with_outside(get_music_data_offset(), "history", wav_player_end_callbk);
                        set_net_player_status(PLAY_WAV, NET_PLAYER_START);
                    }
                    ci_logdebug(LOG_MEDIA, "--start playing-- play_type3 = %d\n", get_net_play_type());
                    no_audio_data_flag = false;
                }
                break;
            }
            /*开始播放*/
            case NET_PLAY_START:
            {

                if (check_current_playing())
                {
                    cias_send_cmd(NET_PLAY_LOCAL_TTS, NO_WAKEUP_FILL_DATA);
                    ci_logdebug(LOG_MEDIA, "NET_PLAY_START error under playing\n");
                    // ret = RETURN_ERR;
                    // break;
                }

                // set_machine_status(WIFI_LINK_STATUS,1);
                recv_resume_cmd_num = 0;
                recv_music_end_sem = false;
                recv_music_data_len = 0;
                recv_net_mp3_num = 0;
                start_recv_flag = false;

                // ci_logdebug(LOG_MEDIA,"pheader->fill_data(%d)\n",pheader->fill_data);
                if (pheader->fill_data == INVAILD_SPEAK)
                {
                    ci_logdebug(LOG_MEDIA, "INVAILD_SPEAK\n");
                    // send_cloud_return(CLOUD_RETURN_INVAILD);
                    if (get_voice_invalid())
                    {
                        cias_send_cmd(SKIP_INVAILD_SPEAK, DEF_FILL);
                        // enter_next_dialogue(4000);
                        break;
                    }
                    cur_play_stream = tts_player;
                    set_net_play_type(PLAY_TTS);
                    outside_clear_stream(tts_player, tts_player_end);
                    set_curr_outside_handle(tts_player, tts_player_end);
                    ci_logdebug(LOG_MEDIA, " receive TTS start 1 !\n");
                    cias_send_cmd(PLAY_DATA_GET, DEF_FILL);
                    set_net_player_status(PLAY_TTS, NET_PLAYER_READY);
                    media_play.start_play = START_PLAY_TTS;
                }
                else if (pheader->fill_data == RECV_TTS_PLAY)
                {

                    cur_play_stream = tts_player;
                    set_net_play_type(PLAY_TTS);
                    outside_clear_stream(tts_player, tts_player_end);
                    set_curr_outside_handle(tts_player, tts_player_end);
                    ci_logdebug(LOG_MEDIA, "receive TTS start 2 !\n");
                    cias_send_cmd(PLAY_DATA_GET, DEF_FILL);
                    set_net_player_status(PLAY_TTS, NET_PLAYER_READY);
                    media_play.start_play = START_PLAY_TTS;
                }
                else if (pheader->fill_data == RECV_MP3_PLAY)
                {
                    // send_cloud_return(CLOUD_RETURN_VAILD);
                    cur_play_stream = mp3_player;
                    set_net_play_type(PLAY_MP3);
                    outside_clear_stream(mp3_player, mp3_player_end);
                    set_curr_outside_handle(mp3_player, mp3_player_end);
                    ci_logdebug(LOG_MEDIA, "receive MP3 start!\n");
                    cias_send_cmd(PLAY_DATA_GET, DEF_FILL);
                    set_net_player_status(PLAY_MP3, NET_PLAYER_READY);
                    set_music_data_offset(0);
                    //ci_logdebug(LOG_MEDIA, "receive MP3 start! change to music *************\n");
#if USE_BEAMFORMING_MODULE
                    ciss_set(CI_SS_PLAY_TYPE,CI_SS_PLAY_TYPE_MUSIC);
#endif
                }
                else if (pheader->fill_data == RECV_M4A_PLAY)
                {
                    // send_cloud_return(CLOUD_RETURN_VAILD);
                    cur_play_stream = m4a_player;
                    set_net_play_type(PLAY_M4A);
                    outside_clear_stream(m4a_player, m4a_player_end);
                    set_curr_outside_handle(m4a_player, m4a_player_end);
                    ci_logdebug(LOG_MEDIA, "receive M4a start!\n");
                    cias_send_cmd(PLAY_DATA_GET, DEF_FILL);
                    set_net_player_status(PLAY_M4A, NET_PLAYER_READY);
                    set_music_data_offset(0);
#if USE_BEAMFORMING_MODULE
                    ciss_set(CI_SS_PLAY_TYPE,CI_SS_PLAY_TYPE_MUSIC);
#endif
                    media_play.start_play = START_PLAY_M4A;

                }
                else if (pheader->fill_data == RECV_WAV_PLAY)
                {
                    // send_cloud_return(CLOUD_RETURN_VAILD);
                    cur_play_stream = wav_player;
                    set_net_play_type(PLAY_WAV);
                    outside_clear_stream(wav_player, wav_player_end);
                    set_curr_outside_handle(wav_player, wav_player_end);
                    ci_logdebug(LOG_MEDIA, "receive WAV start!\n");
                    cias_send_cmd(PLAY_DATA_GET, DEF_FILL);
                    set_net_player_status(PLAY_WAV, NET_PLAYER_READY);
                    set_music_data_offset(0);
                }
                break;
            }
            /*结束播放*/
            case NET_PLAY_STOP:
            {
                ci_logdebug(LOG_MEDIA, "receive media stop!\n");
                // send_cloud_return(CLOUD_RETURN_VAILD);
                if (SYS_STATE_WAKEUP != get_wakeup_state())
                {
                    // stop_net_player();
                    pause_play(NULL, NULL);
                }

                break;
            }
            /*暂停播放*/
            case NET_PLAY_PAUSE:
            {
                ci_logdebug(LOG_MEDIA, "receive mp3 pause!\n");
                if (get_net_play_type() == PLAY_M4A)
                {
                    pause_play(NULL, NULL);
                }

                break;
            }
            /*继续播放*/
            case NET_PLAY_RESUME :
            {
                ci_logdebug(LOG_MEDIA, "receive resume! fill_data = 0x%x\n", pheader->fill_data);
                if (recv_resume_cmd_num > 0)
                {
                    recv_resume_cmd_num = 0;
                    cias_send_cmd(NET_PLAY_NEXT, DEF_FILL);
                    ci_logdebug(LOG_MEDIA, "resume recv 2 times\n");
                }
                if (check_current_playing())
                {
                    ci_logdebug(LOG_MEDIA, "resume error under playing\n");
                    ret = RETURN_ERR;
                    break;
                }
                recv_music_data_len = 0;

                recv_music_end_sem = false;
                recv_net_mp3_num = 0;
                start_recv_flag = false;
                if ((get_net_player_status(PLAY_MP3) == NET_PLAYER_PAUSE) && ((pheader->fill_data == RECV_MP3_PLAY)))
                {
                    set_net_play_type(PLAY_MP3);
                    cur_play_stream = mp3_player;
                    set_curr_outside_handle(mp3_player, mp3_player_end);
                    outside_clear_stream(mp3_player, mp3_player_end);
                    set_net_player_status(PLAY_MP3, NET_PLAYER_RESUME);
                    cias_send_cmd(PLAY_DATA_GET, get_music_data_offset());
                    ci_logdebug(LOG_MEDIA, "recv next data 2\n");
                }
                else if ((get_net_player_status(PLAY_M4A) == NET_PLAYER_PAUSE) && ((pheader->fill_data == RECV_M4A_PLAY)) || (pheader->fill_data == IDLE_STATUS_RECV_M4A_PLAY))
                {
                    set_net_play_type(PLAY_M4A);
                    cur_play_stream = m4a_player;
                    set_curr_outside_handle(m4a_player, m4a_player_end);
                    outside_clear_stream(m4a_player, m4a_player_end);
                    set_net_player_status(PLAY_M4A, NET_PLAYER_RESUME);
                    cias_send_cmd(PLAY_DATA_GET, get_music_data_offset());
                    ci_logdebug(LOG_MEDIA, "recv next data 3\n");
                }
                else if ((get_net_player_status(PLAY_WAV) == NET_PLAYER_PAUSE) && ((pheader->fill_data == RECV_WAV_PLAY)))
                {
                    set_net_play_type(PLAY_WAV);
                    cur_play_stream = wav_player;
                    set_curr_outside_handle(wav_player, wav_player_end);
                    outside_clear_stream(wav_player, wav_player_end);
                    set_net_player_status(PLAY_WAV, NET_PLAYER_RESUME);
                    cias_send_cmd(PLAY_DATA_GET, get_music_data_offset());
                    ci_logdebug(LOG_MEDIA, "recv next data 4\n");
                }
                else
                {
                    if ((get_net_player_status(PLAY_M4A) == NET_PLAYER_IDLE) && (get_net_player_status(PLAY_MP3) == NET_PLAYER_IDLE) && (get_net_player_status(PLAY_WAV) == NET_PLAYER_IDLE))
                    {
                        cias_send_cmd(NET_PLAY_NEXT, DEF_FILL);
                        ci_logdebug(LOG_MEDIA, "resume error NET_PLAYER_IDLE\n");
                    }
                    else
                    {
                        ci_logdebug(LOG_MEDIA, "resume not pasue error\n");
                        ++recv_resume_cmd_num;
                    }
                }

                break;
            }

            /*语音数据接收完*/
            case PLAY_DATA_END:
            {
                rev_voice_end_flag = true;
                //ci_logdebug(LOG_MEDIA, "recv end recv_music_data_len = %d!\n", recv_music_data_len);

                recv_music_end_sem = true;
                recv_net_mp3_num = 0;

                if (get_net_player_status(get_net_play_type()) == NET_PLAYER_START)
                {
                    if (get_net_play_type() == PLAY_TTS)
                    {
                        outside_send_end_sem(tts_player_end);
                    }
                    else if (get_net_play_type() == PLAY_MP3)
                    {
                        outside_send_end_sem(mp3_player_end);
                    }
                    else if (get_net_play_type() == PLAY_M4A)
                    {
                        outside_send_end_sem(m4a_player_end);
                    }
                    else if (get_net_play_type() == PLAY_WAV)
                    {
                        outside_send_end_sem(wav_player_end);
                    }
                    ci_logdebug(LOG_MEDIA, "send_end_sem2 play_type = %d\n", get_net_play_type());
                }
                else if (get_net_player_status(get_net_play_type()) == NET_PLAYER_RESUME)
                {
                    if (get_net_play_type() == PLAY_MP3)
                    {
                        play_with_outside(get_music_data_offset(), "history", mp3_player_end_callbk);
                        set_net_player_status(PLAY_MP3, NET_PLAYER_START);
                        outside_send_end_sem(mp3_player_end);
                    }
                    else if (get_net_play_type() == PLAY_TTS)
                    {
                        play_with_outside(get_music_data_offset(), "history", tts_player_end_callbk);
                        set_net_player_status(PLAY_TTS, NET_PLAYER_START);
                        outside_send_end_sem(tts_player_end);
                    }
                    else if (get_net_play_type() == PLAY_M4A)
                    {
                        play_with_outside(get_music_data_offset(), "history", m4a_player_end_callbk);
                        set_net_player_status(PLAY_M4A, NET_PLAYER_START);
                        outside_send_end_sem(m4a_player_end);
                    }
                    else if (get_net_play_type() == PLAY_WAV)
                    {
                        play_with_outside(get_music_data_offset(), "history", wav_player_end_callbk);
                        set_net_player_status(PLAY_WAV, NET_PLAYER_START);
                        outside_send_end_sem(wav_player_end);
                    }
                }
                else if (get_net_player_status(get_net_play_type()) == NET_PLAYER_READY)
                {
                    if (get_net_play_type() == PLAY_MP3)
                    {
                        set_net_player_status(PLAY_MP3, NET_PLAYER_START);
                        play_with_outside(0, "mp3", mp3_player_end_callbk);
                        outside_send_end_sem(mp3_player_end);
                    }
                    else if (get_net_play_type() == PLAY_TTS)
                    {
                        set_net_player_status(PLAY_TTS, NET_PLAYER_START);
                        play_with_outside(0, "mp3", tts_player_end_callbk);
                        outside_send_end_sem(tts_player_end);
                    }
                    else if (get_net_play_type() == PLAY_M4A)
                    {
                        set_net_player_status(PLAY_M4A, NET_PLAYER_START);
                        play_with_outside(0, "m4a", m4a_player_end_callbk);
                        outside_send_end_sem(m4a_player_end);
                    }
                    else if (get_net_play_type() == PLAY_WAV)
                    {
                        set_net_player_status(PLAY_WAV, NET_PLAYER_START);
                        play_with_outside(0, "ms_wav", wav_player_end_callbk);
                        outside_send_end_sem(wav_player_end);
                    }
                    ci_logdebug(LOG_MEDIA, "Start net player\n");
                }
                break;
            }


    }
    return 0;
}
#endif //VOICE_PLAY_BY_UART