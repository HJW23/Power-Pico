#include "beep.h"
#include "tim.h"

void beep_open(void) {
    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_2); // 启动PWM通道2用于蜂鸣器
}

void beep_close(void) {
    HAL_TIM_PWM_Stop(&htim3, TIM_CHANNEL_2); // 停止PWM通道2
}
