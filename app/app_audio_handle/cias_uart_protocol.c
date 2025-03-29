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
    network_port_recv_queue_init();
    return 0;
}
