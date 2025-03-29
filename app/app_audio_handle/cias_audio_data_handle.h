#ifndef __CIAS_AUDIO_DATA_HANDLE_H__
#define __CIAS_AUDIO_DATA_HANDLE_H__
#include "ci130x_scu.h"
#include "ci130x_uart.h"
#include "user_config.h"
#include "cias_common.h"


#define NEWORK_RECV_QUEUE_ENABLE                        1    //网络端串口数据入队列

#if VOICE_PLAY_BY_UART
#define IOT_AUDIO_PLAY_BUF_SIZE            (36 * 1024) // 
#define PLAY_BUF_SIZE_MAX                  (10 * 1024)  //超过此大小开始从网络获取下一帧数据
#if PCM_VOICE_PLAY_BY_UART
#define UART_VOICE_BAUD     UART_BaudRate921600
#define UART_VOICE_LEN      1024    
#elif MP3_VOICE_PLAY_BY_UART
#define UART_VOICE_BAUD     UART_BaudRate921600
#define UART_VOICE_LEN      1
#endif
#endif

#define NETWORK_PORT_NUM                                UART1 //网络端交互的串口
#define NETWORK_PORT_BAUDRATE                           UART_BaudRate921600 //网络端交互的串口波特率
#define NETWORK_RECV_BUFF_MAX_LEN                       (1024*2)//网络端串口交互数据接收缓冲区大小
#define RECV_PCM_DATA_SIZE                              (1152*2)//(320*8)

#define NETWORK_SEND_BUFF_MAX_LEN                       (1024)   //网络端串口交互数据发送缓冲区大小

typedef enum{
    NET_MSG_HEAD = 1,
    NET_MSG_DATE = 2,
    NET_MSG_ERR  = 0,
    NET_MSG_IDE  = 3,
}cias_communicate_state_t;

int cias_audio_data_transmission(void);

#endif   //__CIAS_AUDIO_DATA_HANDLE_H__
