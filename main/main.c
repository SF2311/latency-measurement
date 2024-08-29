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
#include "freertos/event_groups.h"
#include "freertos/queue.h"
#include "driver/gpio.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "lwip/err.h"
#include "lwip/sys.h"

#include "timing.h"
#include "wifi.h"
#include "display.h"
#include "buttons.h"

/**
 * Brief:
 * This test code generates a rectangle pattern on the configured output port
 * and measures the time it takes for the signal to trigger an input interrupt.
 *
 */

/**
 * PIN Mapping NORVI IIOT-AE01-T
 *
 * Digital Input 0 – GPIO18
 * Digital Input 1 – GPIO39
 * Digital Input 2 – GPIO34
 * Digital Input 3 – GPIO35
 * Digital Input 4 – GPIO19
 * Digital Input 5 – GPIO21
 * Digital Input 6 – GPIO22
 * Digital Input 7 – GPIO23
 *
 *
 * T0.1 – GPIO26
 * T0.0 – GPI027
 * T0 – GPI014
 * T1 – GPI012
 * T2 – GPI013
 * T3 – GPI015
 * T4 – GPI02
 * T5 – GPI033
 */

#define TAG "main"

extern struct timeval start_time[NUM_IO_CHANNELS];
extern channel_gate channel_gates[NUM_IO_CHANNELS];

struct timeval current_time;

int pin_mapping[NUM_IO_CHANNELS] = {GPIO_OUTPUT_IO_0, GPIO_OUTPUT_IO_1, GPIO_OUTPUT_IO_2, GPIO_OUTPUT_IO_3};

void trigger_channel(int channel_nr)
{
    gettimeofday(&current_time, NULL);
    // "unlock" the channel after timeout
    if (TIME_US(current_time) - TIME_US(channel_gates[channel_nr].time) > 5000000)
    {
        ESP_LOGI(TAG, "Unlocking channel %d", channel_nr);
        channel_gates[channel_nr].locked = false;
    }

    // "lock" the channel to prevent a new edge from being generated before the current one is received back
    if (!channel_gates[channel_nr].locked)
    {
        uint32_t level = (gpio_get_level(pin_mapping[channel_nr]) + 1) % 2;
        gettimeofday(&(start_time[channel_nr]), NULL);
        channel_gates[channel_nr].locked = true;
        gettimeofday(&(channel_gates[channel_nr].time), NULL);
        gpio_set_level(pin_mapping[channel_nr], level);
        ESP_LOGI(TAG, "Set channel %d to %d", channel_nr, channel_nr);
    }
}

void app_main(void)
{
    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    ESP_LOGI(TAG, "ESP_WIFI_MODE_STA");
    wifi_init_sta();

    setup_timing();

    setup_display();

    start_button_handler();

    ESP_LOGI(TAG, "Minimum free heap size: %" PRIu32 " bytes\n", esp_get_minimum_free_heap_size());
    TickType_t delay = configTICK_RATE_HZ; // should be 1 second
    char cnt = 0;
    TickType_t lastWakeTime = xTaskGetTickCount();

    while (1)
    {
        // vTaskDelayUntil also updates the lastWakeTime accordingly
        vTaskDelayUntil(&lastWakeTime, delay);
        ESP_LOGI(TAG, "cnt: %d\n", cnt);
        for (int i = 0; i < NUM_IO_CHANNELS; ++i)
        {
            trigger_channel(i);
            // We trigger the next edge with a sligt delay, to (hopefully) achive some spacing between the interrupts
            // This will not affect the overall period of this loop, since this is achived through an absolute delay interval
            vTaskDelay(pdMS_TO_TICKS(100));
        }
    }
}
