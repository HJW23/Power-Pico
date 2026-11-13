/**
  ******************************************************************************
  * @file    user_Link.c
  * @brief   上位机通信链路抽象层实现
  *
  *          PP_LINK_USE_UART == 0 → 走 USB CDC（默认，与改动前行为一致）
  *          PP_LINK_USE_UART == 1 → 走 USART2（蓝牙串口模块）
  ******************************************************************************
  */

#include "user_Link.h"

#include "user_TasksInit.h"

#include <string.h>

#if (PP_LINK_USE_UART == 1U)
#include "usart.h"
#else
#include "usbd_cdc_if.h"
#endif

/**
  * @brief  收到的字节统一投递到命令队列（USB / UART 共用同一队列）
  * @note   命令帧最大 CMD_RX_CHUNK_SIZE(64) 字节，超长块直接计为溢出丢弃
  */
void Link_PushRx(const uint8_t *data, uint16_t len)
{
    CmdRxChunk_t chunk;

    if (CmdRxQueue != NULL && len <= CMD_RX_CHUNK_SIZE)
    {
        chunk.length = len;
        if (len != 0U)
        {
            memcpy(chunk.data, data, len);
        }
        if (osMessageQueuePut(CmdRxQueue, &chunk, 0U, 0U) != osOK)
        {
            CmdRxOverflowCount++;
        }
    }
    else
    {
        CmdRxOverflowCount++;
    }
}

/**
  * @brief  通过当前链路发送一帧数据
  * @retval 0 = 已交给链路（非阻塞），非 0 = 忙/失败，需重试
  */
uint8_t Link_Send(uint8_t *buf, uint16_t len)
{
#if (PP_LINK_USE_UART == 1U)
    if (huart2.gState != HAL_UART_STATE_READY)
    {
        return 1U;
    }
    return (HAL_UART_Transmit_DMA(&huart2, buf, len) == HAL_OK) ? 0U : 1U;
#else
    return CDC_Transmit_FS(buf, len);
#endif
}

/**
  * @brief  启动链路接收
  * @note   USB 由 CDC_Receive_FS 回调驱动，无需额外启动
  */
void Link_StartRx(void)
{
#if (PP_LINK_USE_UART == 1U)
    if (huart2.RxState == HAL_UART_STATE_READY)
    {
        /* DMA + IDLE：不定长命令帧到达后由 HAL_UARTEx_RxEventCallback 通知 */
        HAL_UARTEx_ReceiveToIdle_DMA(&huart2, uart_rx_dma_buf, PP_UART_RX_BUF_SIZE);
        __HAL_DMA_DISABLE_IT(&hdma_usart2_rx, DMA_IT_HT);   /* 只要整帧/空闲事件 */
    }
#endif
}

#if (PP_LINK_USE_UART == 1U)

/**
  * @brief  UART 收包事件（IDLE 或 DMA 半满/全满）
  */
void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
{
    if (huart->Instance != USART2)
    {
        return;
    }

    if (Size != 0U)
    {
        Link_PushRx(uart_rx_dma_buf, Size);
    }

    /* 重新挂起接收，等待下一帧 */
    HAL_UARTEx_ReceiveToIdle_DMA(&huart2, uart_rx_dma_buf, PP_UART_RX_BUF_SIZE);
    __HAL_DMA_DISABLE_IT(&hdma_usart2_rx, DMA_IT_HT);
}

/**
  * @brief  UART 发送完成：唤醒 MessageSendTask 继续下一帧
  */
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART2)
    {
        osThreadFlagsSet(MessageSendTaskHandle, FLAG_USB_TX_COMPLETE);
    }
}

/**
  * @brief  UART 出错（噪声/溢出/帧错误）：清标志并重启接收，避免链路卡死
  */
void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART2)
    {
        __HAL_UART_CLEAR_OREFLAG(huart);
        HAL_UARTEx_ReceiveToIdle_DMA(&huart2, uart_rx_dma_buf, PP_UART_RX_BUF_SIZE);
        __HAL_DMA_DISABLE_IT(&hdma_usart2_rx, DMA_IT_HT);
    }
}

#endif /* PP_LINK_USE_UART */
