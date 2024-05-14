/* GPIO Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <inttypes.h>
#include <sys/time.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"

/**
 * Brief:
 * This test code generates a rectangle pattern on the configured output port
 * and measures the time it takes for the signal to trigger an input interrupt.
 *
 */

// the output pin defined in menuconfig
#define GPIO_OUTPUT_IO CONFIG_GPIO_OUTPUT
// create bitmask for pinout config
#define GPIO_OUTPUT_PIN_SEL ((1ULL << GPIO_OUTPUT_IO))
// the input pin defined in menuconfig
#define GPIO_INPUT_IO CONFIG_GPIO_INPUT
// create bitmask for input pin configuration
#define GPIO_INPUT_PIN_SEL ((1ULL << GPIO_INPUT_IO))
#define ESP_INTR_FLAG_DEFAULT 0

#define TIME_US(t) (int64_t) t.tv_sec * 1000000L + (int64_t)t.tv_usec

static QueueHandle_t gpio_evt_queue = NULL;

static void IRAM_ATTR gpio_isr_handler(void *arg)
{
    xQueueSendFromISR(gpio_evt_queue, NULL, NULL);
}

static void gpio_task_example(void *arg)
{
    struct timeval start_time, end_time;
    double duration = 0;
    gettimeofday(&start_time, NULL);

    for (;;)
    {
        if (xQueueReceive(gpio_evt_queue, NULL, portMAX_DELAY))
        {
            gettimeofday(&end_time, NULL);
            duration = TIME_US(end_time) - TIME_US(start_time);
            printf("GPIO intr, val: %d, took: %lf\n", gpio_get_level(GPIO_INPUT_IO), duration);
            gettimeofday(&start_time, NULL);
        }
    }
}

void app_main(void)
{
    // zero-initialize the config structure.
    gpio_config_t io_conf = {};
    // disable interrupt
    io_conf.intr_type = GPIO_INTR_DISABLE;
    // set as output mode
    io_conf.mode = GPIO_MODE_OUTPUT;
    // bit mask of the pins that you want to set,e.g.GPIO18/19
    io_conf.pin_bit_mask = GPIO_OUTPUT_PIN_SEL;
    // disable pull-down mode
    io_conf.pull_down_en = 0;
    // disable pull-up mode
    io_conf.pull_up_en = 0;
    // configure GPIO with the given settings
    gpio_config(&io_conf);

    // interrupt of rising edge
    io_conf.intr_type = GPIO_INTR_ANYEDGE;
    // bit mask of the pins, use GPIO4/5 here
    io_conf.pin_bit_mask = GPIO_INPUT_PIN_SEL;
    // set as input mode
    io_conf.mode = GPIO_MODE_INPUT;
    // enable pull-up mode
    io_conf.pull_up_en = 1;
    gpio_config(&io_conf);

    // create a queue to handle gpio event from isr
    gpio_evt_queue = xQueueCreate(10, sizeof(uint32_t));
    // start gpio task
    xTaskCreate(gpio_task_example, "gpio_task_example", 2048, NULL, 10, NULL);

    // install gpio isr service
    gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT);
    // hook isr handler for specific gpio pin
    gpio_isr_handler_add(GPIO_INPUT_IO, gpio_isr_handler, (void *)GPIO_INPUT_IO);

    printf("Minimum free heap size: %" PRIu32 " bytes\n", esp_get_minimum_free_heap_size());

    int cnt = 0;
    while (1)
    {
        printf("cnt: %d\n", cnt++);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        gpio_set_level(GPIO_OUTPUT_IO, cnt % 2);
    }
}
