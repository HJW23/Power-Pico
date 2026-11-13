/**
  ******************************************************************************
  * @file    user_Link.h
  * @brief   上位机通信链路抽象层（USB CDC / USART2 二选一）
  *
  *          协议层（CmdStrategy / USB_ADC_Packet_t）与物理链路解耦：
  *            - Link_Send()   发送一帧（非阻塞，busy 时返回非 0，由调用方重试）
  *            - Link_PushRx() 把收到的字节投递到 CmdRxQueue
  *            - Link_StartRx() 启动链路接收（UART 下启动 DMA+IDLE 接收）
  ******************************************************************************
  */

#ifndef __USER_LINK_H__
#define __USER_LINK_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

/* --------------------------------------------------------------------------
 * 链路选择：0 = USB CDC 虚拟串口（当前默认，行为与改动前完全一致）
 *           1 = USART2 + 蓝牙串口模块（PA2=TX, PA3=RX, 可选 PA0=CTS/PA1=RTS）
 *
 * USART2 侧的波特率 / 流控开关在 Core/Inc/usart.h 中配置：
 *   PP_UART_BAUDRATE    (默认 921600)
 *   PP_UART_HW_FLOWCTRL (默认 0，为 1 时启用 PA0/PA1 硬件流控)
 * -------------------------------------------------------------------------- */
#ifndef PP_LINK_USE_UART
#define PP_LINK_USE_UART        0
#endif

/* 返回 0 表示已交由链路发送（或成功入队），非 0 表示忙/失败，调用方需重试 */
uint8_t Link_Send(uint8_t *buf, uint16_t len);

/* 链路收字节的统一入口，内部投递到 CmdRxQueue（USB 与 UART 共用） */
void    Link_PushRx(const uint8_t *data, uint16_t len);

/* 启动链路接收。USB 无需此步（CDC_Receive_FS 由 USB 栈回调驱动） */
void    Link_StartRx(void);

#ifdef __cplusplus
}
#endif

#endif /* __USER_LINK_H__ */
