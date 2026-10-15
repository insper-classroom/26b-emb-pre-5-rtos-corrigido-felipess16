/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>
#include <queue.h>


#include <stdio.h>
#include <string.h> 
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/timer.h"
#include "hardware/irq.h"

const int BTN_PIN_R = 28;
const int BTN_PIN_Y = 21;

const int LED_PIN_R = 5;
const int LED_PIN_Y = 10;

QueueHandle_t xQueueBtn;
SemaphoreHandle_t xSemaphore_r;
SemaphoreHandle_t xSemaphore_y;

void btn_callback(uint gpio, uint32_t events) {
    xQueueSendFromISR(xQueueBtn, &gpio, 0);
}

void btn_task(void* p) {
    gpio_init(BTN_PIN_R);
    gpio_set_dir(BTN_PIN_R, GPIO_IN);
    gpio_pull_up(BTN_PIN_R);

    gpio_init(BTN_PIN_Y);
    gpio_set_dir(BTN_PIN_Y, GPIO_IN);
    gpio_pull_up(BTN_PIN_Y);

    gpio_set_irq_enabled_with_callback(BTN_PIN_R, GPIO_IRQ_EDGE_FALL, true,
                                    &btn_callback);
    gpio_set_irq_enabled(BTN_PIN_Y, GPIO_IRQ_EDGE_FALL, true);

    int gpio;

    while (true) {
        if (xQueueReceive(xQueueBtn, &gpio, 0)){
        if (gpio == BTN_PIN_R){
            xSemaphoreGive(xSemaphore_r);
        } else if (gpio == BTN_PIN_Y){
            xSemaphoreGive(xSemaphore_y);
        }
    }
    }
}

void led_r_task(void *p) {
    gpio_init(LED_PIN_R);
    gpio_set_dir(LED_PIN_R, GPIO_OUT);

    int ativo = 0;
    int estado = 0; 

    while (true) {
        if (xSemaphoreTake(xSemaphore_r, pdMS_TO_TICKS(100)) == pdTRUE) {
            ativo = !ativo;
            if (ativo == 0){
                estado = 0;
            }
        } else {
            if (ativo){
                estado = ! estado;
            }
        }
        gpio_put(LED_PIN_R, estado);
    }
}

void led_y_task(void *p) {
    gpio_init(LED_PIN_Y);
    gpio_set_dir(LED_PIN_Y, GPIO_OUT);

    int ativo = 0;
    int estado = 0; 

    while (true) {
        if (xSemaphoreTake(xSemaphore_y, pdMS_TO_TICKS(100)) == pdTRUE) {
            ativo = !ativo;
            if (ativo == 0){
                estado = 0;
            }
        } else {
            if (ativo){
                estado = ! estado;
            }
        }
        gpio_put(LED_PIN_Y, estado);
    }
}

int main() {
    stdio_init_all();

    xQueueBtn = xQueueCreate(32, sizeof(int));
    xSemaphore_r = xSemaphoreCreateBinary();
    xSemaphore_y = xSemaphoreCreateBinary();

    xTaskCreate(btn_task, "BTN_Task 1", 256, NULL, 1, NULL);
    xTaskCreate(led_r_task, "LED_Task 1", 256, NULL, 1, NULL);
    xTaskCreate(led_y_task, "LED_Task 2", 256, NULL, 1, NULL);

    vTaskStartScheduler();

    while(1){}

    return 0;
}