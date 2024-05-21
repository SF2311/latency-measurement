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
#include "esp_event.h"
#include "esp_log.h"
#include "lwip/err.h"
#include "lwip/sys.h"
#include <esp_http_client.h>
#include <esp_tls.h>
#include <esp_crt_bundle.h>

#include "timing.h"

#define TAG "TIMING"

static QueueHandle_t gpio_evt_queue = NULL;
struct timeval start_time;
static void IRAM_ATTR gpio_isr_handler(void *arg)
{
    struct timeval current_time;
    gettimeofday(&current_time, NULL);
    double duration = TIME_US(current_time) - TIME_US(start_time);
    xQueueSendFromISR(gpio_evt_queue, &duration, NULL);
}

static esp_err_t send_data(esp_http_client_handle_t *client, double value)
{
    esp_http_client_set_method(*client, HTTP_METHOD_POST);
    esp_http_client_set_header(*client, "Authorization", INFLUX_TOKEN_HEADER);
    esp_http_client_set_header(*client, "Content-Type", "text/plain; charset=utf-8");

    const size_t buffer_size = snprintf(NULL, 0, INFLUX_DATA_FORMAT, value);
    char *data = (char *)malloc(buffer_size + 1);
    if (data != NULL)
    {
        snprintf(data, buffer_size + 1, INFLUX_DATA_FORMAT, value);
        ESP_LOGI(TAG, "Sending InfluxDB Data: %s", data);
    }
    esp_http_client_set_post_field(*client, data, buffer_size);

    return esp_http_client_perform(*client);
}

static void timing_task(void *arg)
{
    double duration;
    esp_http_client_config_t http_config = {
        .url = INFLUX_URL,
        .crt_bundle_attach = esp_crt_bundle_attach};
    esp_http_client_handle_t client = esp_http_client_init(&http_config);
    for (;;)
    {
        if (xQueueReceive(gpio_evt_queue, &duration, portMAX_DELAY))
        {
            ESP_LOGI(TAG, "GPIO intr, val: %d, took: %lf\n", gpio_get_level(GPIO_INPUT_IO), duration);
        try_send_data:
            if (send_data(&client, duration) == ESP_OK)
            {
                ESP_LOGI(TAG, "Successfully sent data to database. Status: %d", esp_http_client_get_status_code(client));
            }
            else
            {
                ESP_LOGI(TAG, "Something went wrong when sending the data. Recreating the client...");
                esp_http_client_cleanup(client);
                client = esp_http_client_init(&http_config);
                ESP_LOGI(TAG, "Trying again...");
                goto try_send_data;
            }
        }
    }
}

void setup_timing()
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
    gpio_evt_queue = xQueueCreate(5, sizeof(double));
    // start gpio task
    xTaskCreate(timing_task, "timing_task", 2048, NULL, 10, NULL);

    // install gpio isr service
    gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT);
    // hook isr handler for specific gpio pin
    gpio_isr_handler_add(GPIO_INPUT_IO, gpio_isr_handler, (void *)GPIO_INPUT_IO);
}