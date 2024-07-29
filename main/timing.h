#ifndef TIMING_H
#define TIMING_H

#include <stddef.h>
#include <time.h>

// the output pin defined in menuconfig
#define GPIO_OUTPUT_IO_0 CONFIG_GPIO_OUTPUT_0
#define GPIO_OUTPUT_IO_1 CONFIG_GPIO_OUTPUT_1
#define GPIO_OUTPUT_IO_2 CONFIG_GPIO_OUTPUT_2
#define GPIO_OUTPUT_IO_3 CONFIG_GPIO_OUTPUT_3
// create bitmask for pinout config
#define GPIO_OUTPUT_PIN_SEL ((1ULL << GPIO_OUTPUT_IO_0) | (1ULL << GPIO_OUTPUT_IO_1) | (1ULL << GPIO_OUTPUT_IO_2) | (1ULL << GPIO_OUTPUT_IO_3))
// the input pin defined in menuconfig
#define GPIO_INPUT_IO_0 CONFIG_GPIO_INPUT_0
#define GPIO_INPUT_IO_1 CONFIG_GPIO_INPUT_1
#define GPIO_INPUT_IO_2 CONFIG_GPIO_INPUT_2
#define GPIO_INPUT_IO_3 CONFIG_GPIO_INPUT_3

// create bitmask for input pin configuration
#define GPIO_INPUT_PIN_SEL ((1ULL << GPIO_INPUT_IO_0) | (1ULL << GPIO_INPUT_IO_1) | (1ULL << GPIO_INPUT_IO_2) | (1ULL << GPIO_INPUT_IO_3))
#define ESP_INTR_FLAG_DEFAULT 0

#define INFLUX_URL CONFIG_INFLUX_URL
#define INFLUX_TOKEN_HEADER CONFIG_INFLUX_TOKEN_HEADER
#define INFLUX_DATA_FORMAT CONFIG_INFLUX_DATA_FORMAT

// Number of IO pin pairs used for latency measurement
#define NUM_IO_CHANNELS 4

#define TIME_US(t) ((int64_t)t.tv_sec * 1000000L + (int64_t)t.tv_usec)

typedef struct _delay_data
{
    uint32_t pin;
    long duration;
} delay_data;

typedef struct _channel_gate
{
    bool locked;
    struct timeval time;
} channel_gate;

void setup_timing(void);

#endif