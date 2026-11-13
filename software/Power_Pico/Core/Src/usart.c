#include "usart.h"

/* USER CODE BEGIN 0 */

/* USART2 用作蓝牙串口链路（PP_LINK_USE_UART=1 时由 user_Link.c 启动收发） */
UART_HandleTypeDef huart2;
DMA_HandleTypeDef  hdma_usart2_tx;
DMA_HandleTypeDef  hdma_usart2_rx;

/* DMA + IDLE 接收缓冲，接收回调在 user_Link.c 中处理 */
uint8_t uart_rx_dma_buf[PP_UART_RX_BUF_SIZE];

/* USER CODE END 0 */

void MX_USART2_UART_Init(void)
{
  huart2.Instance = USART2;
  huart2.Init.BaudRate = PP_UART_BAUDRATE;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
#if (PP_UART_HW_FLOWCTRL == 1U)
  huart2.Init.HwFlowCtl = UART_HWCONTROL_RTS_CTS;
#else
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
#endif
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
}

void HAL_UART_MspInit(UART_HandleTypeDef *uartHandle)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if (uartHandle->Instance == USART2)
  {
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_USART2_CLK_ENABLE();

    /* PA2: USART2_TX, PA3: USART2_RX */
    GPIO_InitStruct.Pin = GPIO_PIN_2 | GPIO_PIN_3;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF7_USART2;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

#if (PP_UART_HW_FLOWCTRL == 1U)
    /* PA0: USART2_CTS, PA1: USART2_RTS —— 需蓝牙模块引出 RTS/CTS 才有意义 */
    GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_1;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF7_USART2;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
#endif

    /* USART2_TX -> DMA1_Stream6 (Channel 4) */
    hdma_usart2_tx.Instance = DMA1_Stream6;
    hdma_usart2_tx.Init.Channel = DMA_CHANNEL_4;
    hdma_usart2_tx.Init.Direction = DMA_MEMORY_TO_PERIPH;
    hdma_usart2_tx.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_usart2_tx.Init.MemInc = DMA_MINC_ENABLE;
    hdma_usart2_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_usart2_tx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    hdma_usart2_tx.Init.Mode = DMA_NORMAL;
    hdma_usart2_tx.Init.Priority = DMA_PRIORITY_HIGH;
    hdma_usart2_tx.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
    if (HAL_DMA_Init(&hdma_usart2_tx) != HAL_OK)
    {
      Error_Handler();
    }
    __HAL_LINKDMA(uartHandle, hdmatx, hdma_usart2_tx);

    /* USART2_RX -> DMA1_Stream5 (Channel 4)，配合 IDLE 中断做不定长收包 */
    hdma_usart2_rx.Instance = DMA1_Stream5;
    hdma_usart2_rx.Init.Channel = DMA_CHANNEL_4;
    hdma_usart2_rx.Init.Direction = DMA_PERIPH_TO_MEMORY;
    hdma_usart2_rx.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_usart2_rx.Init.MemInc = DMA_MINC_ENABLE;
    hdma_usart2_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_usart2_rx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    hdma_usart2_rx.Init.Mode = DMA_NORMAL;
    hdma_usart2_rx.Init.Priority = DMA_PRIORITY_HIGH;
    hdma_usart2_rx.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
    if (HAL_DMA_Init(&hdma_usart2_rx) != HAL_OK)
    {
      Error_Handler();
    }
    __HAL_LINKDMA(uartHandle, hdmarx, hdma_usart2_rx);

    /* USART2 中断（IDLE 检测必需） */
    HAL_NVIC_SetPriority(USART2_IRQn, 6, 0);
    HAL_NVIC_EnableIRQ(USART2_IRQn);
    /* 注意：DMA1_Stream5/6 的 NVIC 在 MX_DMA_Init 中使能 */
  }
}

void HAL_UART_MspDeInit(UART_HandleTypeDef *uartHandle)
{
  if (uartHandle->Instance == USART2)
  {
    __HAL_RCC_USART2_CLK_DISABLE();
    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_2 | GPIO_PIN_3);
#if (PP_UART_HW_FLOWCTRL == 1U)
    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_0 | GPIO_PIN_1);
#endif
    HAL_DMA_DeInit(uartHandle->hdmatx);
    HAL_DMA_DeInit(uartHandle->hdmarx);
    HAL_NVIC_DisableIRQ(USART2_IRQn);
  }
}

/* USER CODE BEGIN 1 */
/* USER CODE END 1 */
