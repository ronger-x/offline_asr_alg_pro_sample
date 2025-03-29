/*
 * @FileName::
 * @Author: 
 * @Date: 2022-03-04 10:41:12
 * @LastEditTime: 2022-05-17 20:16:17
 * @Description: Network driven implementation 主要用于和wifi芯片通讯
 */
#include <stdint.h>
#include <string.h>
#include "sdk_default_config.h"
#include "ci130x_uart.h"
#include "ci130x_dma.h"
#include "cias_uart_protocol.h"
#include "system_msg_deal.h"
#include "ci_log.h"
#include "cias_audio_data_handle.h"
#include "FreeRTOS.h"
#include "queue.h"
#include "codec_manager.h"
#include "audio_play_process.h"
#include "audio_play_device.h"
#include "audio_play_api.h"

QueueHandle_t network_msg_recv_queue = NULL;
#if (VOICE_PLAY_BY_UART)

static unsigned char buf_recv_hw[UART_VOICE_LEN] = {0};
void uart1_dma_read_irq_callback(void)
{
    xQueueSendFromISR(network_msg_recv_queue,buf_recv_hw,0);
    DMAC_M2P_P2M_advance_config(DMACChannel1,
                            DMAC_Peripherals_UART1_RX,
                            P2M_DMA,
                            UART1_DMA_ADDR,
                            (unsigned int)buf_recv_hw,
                            UART_VOICE_LEN,
                            TRANSFERWIDTH_8b,
                            BURSTSIZE1,
                            DMAC_AHBMaster1);
}

extern audio_play_os_stream_t mp3_player;
extern audio_play_os_sem_t mp3_player_end;
volatile int cias_wakeup_cmd_flag = 1;

int network_recv_data_task(void)
{
//    scu_set_dma_mode(DMAINT_SEL_CHANNELALL);
    #if PCM_VOICE_PLAY_BY_UART
    unsigned char* recv_pcm_message = pvPortMalloc(UART_VOICE_LEN);
    network_msg_recv_queue = xQueueCreate(4, UART_VOICE_LEN);
    cm_pcm_buffer_info_t pcm_buffer_info;
    pcm_buffer_info.play_buffer_info.block_num = 1;
    pcm_buffer_info.play_buffer_info.buffer_num = 16;
    pcm_buffer_info.play_buffer_info.block_size = UART_VOICE_LEN;
    pcm_buffer_info.play_buffer_info.buffer_size = pcm_buffer_info.play_buffer_info.block_size * pcm_buffer_info.play_buffer_info.block_num;
    int pcm_buffer_total_size = pcm_buffer_info.play_buffer_info.buffer_size*pcm_buffer_info.play_buffer_info.buffer_num;
    pcm_buffer_info.play_buffer_info.pcm_buffer = pvPortMalloc(pcm_buffer_total_size);

    audio_format_info_t audio_format_info;
    audio_format_info.samprate = 16000;
    audio_format_info.nChans = 1;
    #elif MP3_VOICE_PLAY_BY_UART
    unsigned char* recv_pcm_message = pvPortMalloc(256);
    network_msg_recv_queue = xQueueCreate(256, UART_VOICE_LEN);
    #endif
    UARTDMAConfig(UART1, UART_VOICE_BAUD);
    DMAC_M2P_P2M_advance_config(DMACChannel1,
                            DMAC_Peripherals_UART1_RX,
                            P2M_DMA,
                            UART1_DMA_ADDR,
                            (unsigned int)buf_recv_hw,
                            UART_VOICE_LEN,
                            TRANSFERWIDTH_8b,
                            BURSTSIZE1,
                            DMAC_AHBMaster1);
    extern void set_dma_int_callback(DMACChannelx dmachannel,dma_callback_func_ptr_t func);
    set_dma_int_callback(DMACChannel1,uart1_dma_read_irq_callback); 

    for(;;)
    {
        #if PCM_VOICE_PLAY_BY_UART
        xQueueReceive(network_msg_recv_queue, recv_pcm_message, portMAX_DELAY);
        // vPortEnterCritical();
        if (cias_wakeup_cmd_flag == 1)
        {
            cm_config_pcm_buffer(PLAY_CODEC_ID, CODEC_OUTPUT, &pcm_buffer_info);
            audio_play_hw_start(ENABLE, &audio_format_info);
            cias_wakeup_cmd_flag = 0;
        }
        uint32_t ret_buf;
        while(!(cm_get_codec_empty_buffer_number(PLAY_CODEC_ID,CODEC_OUTPUT) > 1))
        {
            ;
        }
        cm_get_pcm_buffer(PLAY_CODEC_ID,&ret_buf, portMAX_DELAY);
        memcpy(ret_buf,recv_pcm_message,UART_VOICE_LEN);
        cm_write_codec(PLAY_CODEC_ID,ret_buf,portMAX_DELAY);
        // vPortExitCritical();
        #elif MP3_VOICE_PLAY_BY_UART
        if(xQueueReceive(network_msg_recv_queue, recv_pcm_message, pdMS_TO_TICKS(500)) == pdPASS)
        {
            if(cias_wakeup_cmd_flag == 1)
            {
                cias_wakeup_cmd_flag = 0;
                set_curr_outside_handle(mp3_player, mp3_player_end);
                outside_clear_stream(mp3_player, mp3_player_end);
            }
            outside_write_stream(mp3_player, recv_pcm_message, 1, false);
            play_with_outside(0, "mp3", NULL); 
        }
        else
            outside_send_end_sem(mp3_player_end);
        #endif
    }
}

static int8_t uart1_temp = 0;
void UART1_IRQHandler(void)
{
    mprintf("-\n");
    /*接受数据*/
    if ((NETWORK_PORT_NUM->UARTMIS & (1UL << UART_RXInt)) || (NETWORK_PORT_NUM->UARTMIS & (1UL << UART_RXTimeoutInt)))
    {
        while (!(NETWORK_PORT_NUM->UARTFlag & (0x1 << 6))) // read fifo have data
        {
            uart1_temp= (int8_t)UartPollingReceiveData(NETWORK_PORT_NUM);
            if(xQueueSendFromISR(network_msg_recv_queue, &uart1_temp, pdFALSE) == pdFALSE)
                mprintf("----queue send err------\n");
        }
        UART_IntClear(NETWORK_PORT_NUM, UART_RXInt);
    }
    NETWORK_PORT_NUM->UARTICR = 0xFFF;
}
#endif

void network_port_recv_queue_init(void)
{
    network_msg_recv_queue = xQueueCreate(NETWORK_RECV_BUFF_MAX_LEN, sizeof(int8_t));
    if (network_msg_recv_queue == NULL)
    {
        mprintf("network_msg_recv_queue create fail\r\n");
    }
    mprintf("network_msg_recv_queue create success\r\n");
}

static int8_t network_send_Byte(int8_t ch)
{
    //发送FIFO满标志
    if (UART_FLAGSTAT(NETWORK_PORT_NUM, UART_TXFF))
    {
        return 0;
    }
    UartPollingSenddata(NETWORK_PORT_NUM, ch);

    return 1;
}

int32_t network_send(int8_t *str, uint32_t length)
{
    int32_t ret = 0;

    if (str && length)
    {
        while (length && (ret < NETWORK_SEND_BUFF_MAX_LEN))
        {
            if (network_send_Byte(str[ret]))
            {
                ret++;
                length--;
            }
        }
    }
    return ret;
}

/** 网络通讯初始化
 *  入口参数：void
 *  return： 0
*/
int8_t network_port_init(void)
{
    UARTInterruptConfig(NETWORK_PORT_NUM, NETWORK_PORT_BAUDRATE);
#if NEWORK_RECV_QUEUE_ENABLE
    network_port_recv_queue_init();
#else
    memset(network_recv_buf, 0, NETWORK_RECV_BUFF_MAX_LEN);
#endif
    return 0;
}
