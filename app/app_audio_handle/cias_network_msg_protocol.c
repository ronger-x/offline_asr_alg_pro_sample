/**
 ******************************************************************************
 * @文件    cias_network_msg_protocol.c
 * @版本    V1.0.0
 * @日期    2019-5-9
 * @概要    接收网络端的数据，并处理
 ******************************************************************************
 * @注意
 *
 * 版权归chipintelli公司所有，未经允许不得使用或修改
 *
 ******************************************************************************
 */
#include <string.h>
#include "ci_log.h"
#include "cias_network_msg_protocol.h"
#include "cias_network_msg_send_task.h"
#include "prompt_player.h"
#include "cias_uart_protocol.h"
#include "audio_play_api.h"
#include "FreeRTOS.h"
#include "queue.h"
#include "cias_audio_data_handle.h"
#include "codec_manager.h"
// #include "cias_voice_plyer_handle.h"

#define MAX_SEND_NUM 5
#define MAX_ACK_TIME 500 // MS
#define REQ_PLAY_DATA_SIZE (10 * 1024)


pro_wait_acktypedef wait_ackstruct;
audio_play_os_stream_t cur_play_stream = PLAY_NULL;


bool recv_music_end_sem = false;
bool no_audio_data_flag = false;

// int8_t package_data[1050] = {0};

uint8_t gwork_mode = WORKING_APPLICATION_MODE;
int16_t ligth_test_interval_time = 3000; // 3S默认3S



static uint32_t response_timer_cnt = 0;
static uint8_t *login_info = NULL;



int32_t wifi_msg_deal(uint8_t *msg_buf, int32_t msg_len);



uint32_t get_response_timer_cnt(void)
{
    response_timer_cnt = xTaskGetTickCount() - response_timer_cnt;
    ci_loginfo(LOG_MEDIA, "response_timer %d ms\n", response_timer_cnt);
    return response_timer_cnt;
}

uint32_t set_response_timer_cnt_start(void)
{
    return response_timer_cnt = xTaskGetTickCount();
}


void resp_file_config(void)
{
#if 0
    uint16_t login_info_len = 500;
    if(login_info == NULL)
    {
        login_info = pvPortMalloc(login_info_len);
        if(login_info == NULL)
        {
            ci_loginfo(LOG_MEDIA,"login info malloc error\n");
            return;
        }
    }
    requset_flash_ctl();
    flash_read(FLASH_SPI_PORT,(uint32_t)login_info,PRODUCT_INFO_SPIFLASH_START_ADDR,login_info_len);
    release_flash_ctl();
    cias_send_cmd_and_data(CI_PROFILE_MSG, login_info, login_info_len, DEF_FILL);
    ci_loginfo(LOG_MEDIA,"send login info\n");
#endif
}

static int32_t current_network_state_t = WIFI_DISCONNECT_STATE;
int32_t wifi_current_state_set(int32_t arg)
{
    if(arg > NETWORK_DISCONNECT_STATE||arg < WIFI_CONNECTED_STATE)
        return -1;

    current_network_state_t = arg;
    return 0;
}
int32_t wifi_current_state_get(void)
{
    return current_network_state_t;
}

/**
 * @brief 发送退出唤醒消息
 *
 */
void send_exit_wakeup_msg(char debug)
{
    ci_logdebug(LOG_USER, "send_exit_wakeup_msg = %d\n", debug);
    sys_msg_t send_msg;
    send_msg.msg_type = SYS_MSG_TYPE_CMD_INFO;
    send_msg.msg_data.cmd_info_data.cmd_info_status = MSG_CMD_INFO_STATUS_EXIT_WAKEUP;
    if (pdPASS != send_msg_to_sys_task(&send_msg, NULL))
    {
        ci_logdebug(LOG_USER, "send_exit_wakeup_msg err\n");
    }
}


// typedef enum{

//     NET_MSG_HEAD = 1, //
//     NET_MSG_DATE = 2,
//     NET_MSG_ERR  = 0,
//     NET_MSG_IDE  = 3,
// }wifi_communicate_state_t;


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

volatile bool start_recv_flag = false;//向wifi请求数据

/**
 * @brief WIFI 消息处理
 *
 * @param msg_buf 数据信息
 * @param msg_len 数据长度
 * @return int32_t RETURN_OK 处理成功，RETURN_ERR 数据异常
 */
// extern int32_t aiot_callback(uint8_t *msg_buf);
int32_t wifi_msg_deal(uint8_t *msg_buf, int32_t msg_len)
{
    if ((msg_buf == NULL) || (msg_len <= 0))
    {
        return RETURN_ERR;
    }
#if CIAS_DOUBLE_MIC_DEMO_ENABLE | CIAS_SIGNAL_MIC_DEMO_ENABLE
    aiot_callback(msg_buf);
#endif

    return RETURN_OK;
}
