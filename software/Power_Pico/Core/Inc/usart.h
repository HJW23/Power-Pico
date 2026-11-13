#ifndef __USART_H__
#define __USART_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"

/* USER CODE BEGIN Includes */
/* USER CODE END Includes */

extern UART_HandleTypeDef huart2;

void MX_USART2_UART_Init(void);

/* USER CODE BEGIN Prototypes */

/* --------------------------------------------------------------------------
 * 蓝牙串口链路参数（USART2）
 *   PA2 -> USART2_TX   (接蓝牙模块 RX)
 *   PA3 -> USART2_RX   (接蓝牙模块 TX)
 *   PA0 -> USART2_CTS  (仅 PP_UART_HW_FLOWCTRL=1 时启用)
 *   PA1 -> USART2_RTS  (仅 PP_UART_HW_FLOWCTRL=1 时启用)
 *
 * 速率依据：应用层数据流 81 B/ms ≈ 81 KB/s，8N1 下 921600 bps ≈ 92 KB/s。
 * APB1 = 48 MHz，921600 的实际分频误差 +0.16%（48e6/16 = 3 MHz 基准），
 * 若要更高余量可取 1500000/2000000（48 MHz 下均为整数分频，误差 0）。
 * -------------------------------------------------------------------------- */
#define PP_UART_BAUDRATE        921600U
#define PP_UART_HW_FLOWCTRL     0U          /* 1 = 启用 PA0/PA1 硬件流控 */
#define PP_UART_RX_BUF_SIZE     256U

extern uint8_t uart_rx_dma_buf[PP_UART_RX_BUF_SIZE];
extern DMA_HandleTypeDef hdma_usart2_tx;
extern DMA_HandleTypeDef hdma_usart2_rx;

/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif

#endif /* __USART_H__ */
