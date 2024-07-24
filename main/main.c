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

extern struct timeval start_time[3];

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

    ESP_LOGI(TAG, "Minimum free heap size: %" PRIu32 " bytes\n", esp_get_minimum_free_heap_size());
    TickType_t delay = configTICK_RATE_HZ; // should be 1 second
    char cnt = 0;
    TickType_t lastWakeTime = xTaskGetTickCount();
    while (1)
    {
        // vTaskDelayUntil also updates the lastWakeTime accordingly
        vTaskDelayUntil(&lastWakeTime, delay);
        ESP_LOGI(TAG, "cnt: %d\n", cnt);
        cnt = (cnt + 1) % 2;
        gettimeofday(start_time, NULL);
        // We trigger the edge with a sligt delay, to (hopefully) achive some spacing between the interrupts
        // This will not affect the overall period of this loop, since this is achived through an absolute delay interval
        gpio_set_level(GPIO_OUTPUT_IO_0, cnt);
        vTaskDelay(100 / portTICK_PERIOD_MS);
        gettimeofday(start_time + 1, NULL);
        gpio_set_level(GPIO_OUTPUT_IO_1, cnt);
        vTaskDelay(100 / portTICK_PERIOD_MS);
        gettimeofday(start_time + 2, NULL);
        gpio_set_level(GPIO_OUTPUT_IO_2, cnt);
        vTaskDelay(100 / portTICK_PERIOD_MS);
        gettimeofday(start_time + 3, NULL);
        gpio_set_level(GPIO_OUTPUT_IO_3, cnt);
    }
}
