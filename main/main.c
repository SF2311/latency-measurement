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

/**
 * Brief:
 * This test code generates a rectangle pattern on the configured output port
 * and measures the time it takes for the signal to trigger an input interrupt.
 *
 */

#define TAG "main"

extern struct timeval start_time;
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

    ESP_LOGI(TAG, "Minimum free heap size: %" PRIu32 " bytes\n", esp_get_minimum_free_heap_size());

    char cnt = 0;
    gettimeofday(&start_time, NULL);
    while (1)
    {
        ESP_LOGI(TAG, "cnt: %d\n", cnt);
        cnt = (cnt + 1) % 2;
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        gettimeofday(&start_time, NULL);
        gpio_set_level(GPIO_OUTPUT_IO, cnt);
    }
}
