#include "ci_log.h"

#include "user_config.h"

#include "aiot_callback.h"
#include "cias_network_msg_send_task.h"
#include "cias_network_msg_protocol.h"
#include "cias_voice_plyer_handle.h"

#include "system_msg_deal.h"
#include "prompt_player.h"
#include "audio_play_api.h"
#include "status_share.h"

#if CIAS_DOUBLE_MIC_DEMO_ENABLE | CIAS_SIGNAL_MIC_DEMO_ENABLE

bool rev_voice_end_flag = false;
static int8_t network_configing = 0;

extern volatile bool voice_upload_vad_enable;
extern QueueHandle_t pcm_speex_deal_queue;
static void play_done_callback(cmd_handle_t cmd_handle)
{
#if AUDIO_PLAYER_ENABLE
    // resume_voice_in();
    if (get_net_player_status(PLAY_M4A) == NET_PLAYER_PAUSE)
    {
        ci_logdebug(LOG_USER, "resume M4a start!\n");
        cias_send_cmd(NET_PLAY_RECONECT_URL, get_music_data_offset());
    }
/* #if ON_LINE_SUPPORT
    cias_send_cmd(NET_PLAY_END, DEF_FILL);
#endif */

#endif
}

extern media_play_resume_control media_play;
static start_connect_wifi = false;
int32_t aiot_asr_callback(uint16_t cmd_id)
{
    uint8_t vol;
    switch (cmd_id)
    {
        case 38:/*增大音量*/
        {
            vol = vol_get();
            if(vol < VOLUME_MAX)
            {
                vol ++;
                vol_set(vol);
            }
            break;
        }
        case 39:/*减小音量*/
        {
             vol = vol_get();
            if(vol > VOLUME_MIN)
            {
                vol --;
                vol_set(vol);
            }
            break;
        }
        case 40:/*最大音量*/
        {
            vol = VOLUME_MAX;
            vol_set(VOLUME_MAX);
            break;
        }
        case 41:/*最小音量*/
        {
            vol = VOLUME_MIN;
            vol_set(VOLUME_MIN);
            break;
        }
        case 44: /* 开始配网采用AIRKISS配网 */
        {
            if(start_connect_wifi == false)
            {
                ci_loginfo(LOG_USER, "-----start connect wifi------\r\n");
                cias_send_cmd(ENTER_NET_CONFIG, DEF_FILL);
                wifi_current_state_set(WIFI_CONFIGING_STATE);
                start_connect_wifi = true;
            }
            else
            {
                cias_send_cmd(ENTER_NET_CONFIG, DEF_FILL);
                prompt_play_by_cmd_id(1001, -1, play_done_callback, true);
            }

            break;
        }
        // case 2001: /* 退出配网 */
        // {
        //     // network_configing = 0;
        //     //xTimerStop(g_wifi_timer_handle, 0);	//关闭wifi计时定时器
        //     cias_send_cmd(EXIT_NET_CONFIG, DEF_FILL);
        //     prompt_play_by_cmd_id(FUN_EXIT_SMATRCONFIG, -1, play_done_callback, true); //yuanxiaoping 0527
        //     break;
        // }
        // case 2002: /* 默认配网设置 */
        // {

        //     // network_configing = 0;
        //     cias_send_cmd(INIT_SMARTCONFIG, DEF_FILL);
        //     prompt_play_by_cmd_id(2002, -1, play_done_callback, true); //yuanxiaoping 0527
        //     break;
        // }
        // case 2005: /* airkiss 配网 */
        // {
        //     // network_configing = 1;
        //     cias_send_cmd(SEND_ENTER_AIRKISS_CMD, DEF_FILL);
        //     prompt_play_by_cmd_id(FUN_ENTER_SMATRCONFIG, -1, play_done_callback, true); //yuanxiaoping 0527
        //     break;
        // }
        default:
        {
            ci_loginfo(LOG_USER, "\r\n switch=============2\r\n");
            cais_send_cmd_to_network(cmd_id); //iot send
            break;
        }
    }

}


int32_t aiot_callback(uint8_t *msg_buf)
{
    int32_t ret = RETURN_OK;
    static connext_cloud = false;

    /*这里将数据发送到需要的BUF中,例如下面*/
    cias_data_standard_head_t *pheader = (cias_data_standard_head_t *)(msg_buf);
    wifi_communicate_cmd_t wifi_cmd = (wifi_communicate_cmd_t)pheader->type;
    
    if (wifi_cmd != PLAY_DATA_GET)
        ci_logdebug(LOG_MEDIA, "[qcloud aux:]recv type: %x\n", wifi_cmd);

    switch (wifi_cmd)
    {
       
        #if AUDIO_PLAYER_ENABLE
            case PLAY_DATA_GET:
            case NET_PLAY_START:
            case NET_PLAY_STOP:
            case NET_PLAY_PAUSE:
            case NET_PLAY_RESUME :
            case PLAY_DATA_END:
            {
                network_audio_processing(msg_buf);
                break;
            }
        #endif //AUDIO_PLAYER_ENABLE        


        case NET_VOLUME:
        {
            if(!connext_cloud)
                break;
            int vol =  *((unsigned int *)(msg_buf + 16))/14;
            if(vol < VOLUME_MIN)
            {
                vol = VOLUME_MIN;
            }
            vol_set(vol);
            break;
        }
        
        case GET_PROFILE:
        {
            ci_logdebug(LOG_MEDIA, "RECV GET PROFILE CMD\n");
            cias_send_profile();
            while(CI_SS_PLAY_STATE_PLAYING == ciss_get(CI_SS_PLAY_STATE))
            {
                vTaskDelay(1000);
            }
            prompt_play_by_cmd_id(1004, -1, play_done_callback, true);
            break;
         }
        case NEED_PROFILE:
        {
            while(CI_SS_PLAY_STATE_PLAYING == ciss_get(CI_SS_PLAY_STATE))
            {
                vTaskDelay(1000);
            }
            ci_logdebug(LOG_MEDIA, "RECV NEED PROFILE CMD\n");
            prompt_play_by_cmd_id(1005, -1, play_done_callback, true);
            break;
        }

        case QCLOUD_IOT_CMD:
        {
            unsigned short qcloud_cmd;

            qcloud_cmd = *((unsigned short *)(msg_buf + 16));
            ci_logdebug(LOG_MEDIA, "[ qcloud ]recv cmd: %d\n", qcloud_cmd);
            // tanmi_uart_send_msg_to_elc(qcloud_cmd&0xff,0x02);

            pause_voice_in();
            prompt_play_by_cmd_id(qcloud_cmd, -1, play_done_callback, true);
            break;
        }       
        case VAD_END:  //云端VAD结束
        {
            // #if RECEIVE_VAD_FROM_THE_CLOUD_ENABLE
            #if 1
                voice_upload_disable();
                voice_upload_vad_enable = false;
                xQueueReset(pcm_speex_deal_queue);
                ci_loginfo(LOG_VOICE_UPLOAD, "rev cloud vad end flag********************\n");
                break;
            #endif
        }    
        // case RECV_NET_CONNECTED_CMD:
        case CLOUD_CONNECTED:  //云端已连接
        {
            start_connect_wifi = false;
            connext_cloud = true;
            if(NETWORK_CONNECTED_STATE != wifi_current_state_get())
            {
                ci_loginfo(LOG_IR, "RECV_NET_CONNECTED_CMD cmd is 0x%x\n", wifi_cmd);
                wifi_current_state_set(NETWORK_CONNECTED_STATE);
                //pause_voice_in();
                vTaskDelay(pdMS_TO_TICKS(1000));
                prompt_play_by_cmd_id(1002, -1, play_done_callback, true);      
            }
            
            break;
        }        
        case WIFI_DISCONNECTED://配网失败，网络已断开
        {
            start_connect_wifi = false;
            connext_cloud = false;
            if(WIFI_CONFIGING_STATE == wifi_current_state_get())
            {
                wifi_current_state_set(WIFI_DISCONNECT_STATE);
                pause_voice_in();
                prompt_play_by_cmd_id(1003, -1, play_done_callback, true);
            }
                
            ci_loginfo(LOG_IR, "WIFI_DISCONNECTED cmd is 0x%x\n", wifi_cmd);
            break;
        }
        case ENTER_NET_CONFIG://WIFI进入配网模式
        {
            mprintf("---进入配网模式\n");
            prompt_play_by_cmd_id(1001, -1, play_done_callback, true);
            break;
        }
        case NET_CONFIG_SUCCESS://配网成功
        {
            mprintf("---配网成功\n");
            prompt_play_by_cmd_id(1007, -1, play_done_callback, true);	
            break;
        }
        case NET_CONFIG_FAIL://配网失败
        {
            mprintf("---配网失败\n");
            start_connect_wifi = false;
            connext_cloud = false;
            if(WIFI_CONFIGING_STATE == wifi_current_state_get())
            {
                wifi_current_state_set(WIFI_DISCONNECT_STATE);
                pause_voice_in();	
                prompt_play_by_cmd_id(1006, -1, play_done_callback, true);
            }	
            break;
        }
        default:
        {
            ci_loginfo(LOG_IR, "other cmd is 0x%x\n", wifi_cmd);
            if(wifi_cmd == PLAY_EMPTY)
            {
                //ci_logdebug(LOG_MEDIA, "MP3 play_end, change to bf *********************\n");
#if USE_BEAMFORMING_MODULE
                ciss_set(CI_SS_PLAY_TYPE,CI_SS_PLAY_TYPE_CMD_PROMPT);
#endif
                media_play.recv_media_stream_end = true;
            }
                
/*             if((wifi_cmd == 0xeeee)&&(media_play.start_play == START_PLAY_TTS)&&(media_play.mp3_decode_fail))
            {
                media_play.mp3_decode_fail = false;
                ci_loginfo(LOG_USER, "\r\n TTS fail, NET_PLAY_RESTART=============\r\n");
                cias_send_cmd(NET_PLAY_RESTART, DEF_FILL);
            } */
            
            ret = RETURN_ERR;
            break;
        }
    }


}
void play_send_pcm_finish()
{
    prompt_play_by_cmd_id(1008, -1, play_done_callback, true);
}
#endif 