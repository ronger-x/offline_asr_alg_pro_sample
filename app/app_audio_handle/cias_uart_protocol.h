/*
 * @FileName:: cias_uart_protocol.h
 * @Author: 
 * @Date: 2022-03-04 11:02:42
 * @LastEditTime: 2022-05-16 15:42:07
 * @Description: uart communication function
 */

#ifndef _CIAS_UART_PROTOCOL_H_
#define _CIAS_UART_PROTOCOL_H_
#include "FreeRTOS.h"
#include "queue.h"

int8_t network_port_init(void);
int32_t network_send(int8_t *str,uint32_t length);
int32_t network_test_send(int8_t *str, uint32_t length);
int32_t get_network_recv_length(void);
void clean_network_recv_length(void);
int8_t * get_network_recv_data(void);
void clean_network_recv_data(void);

extern QueueHandle_t network_msg_recv_queue;

#endif //_CIAS_UART_PROTOCOL_H_