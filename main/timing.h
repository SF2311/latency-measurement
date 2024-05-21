#ifndef TIMING_H
#define TIMING_H

// the output pin defined in menuconfig
#define GPIO_OUTPUT_IO CONFIG_GPIO_OUTPUT
// create bitmask for pinout config
#define GPIO_OUTPUT_PIN_SEL ((1ULL << GPIO_OUTPUT_IO))
// the input pin defined in menuconfig
#define GPIO_INPUT_IO CONFIG_GPIO_INPUT

// create bitmask for input pin configuration
#define GPIO_INPUT_PIN_SEL ((1ULL << GPIO_INPUT_IO))
#define ESP_INTR_FLAG_DEFAULT 0

#define INFLUX_URL CONFIG_INFLUX_URL
#define INFLUX_TOKEN_HEADER CONFIG_INFLUX_TOKEN_HEADER
#define INFLUX_DATA_FORMAT CONFIG_INFLUX_DATA_FORMAT

#define TIME_US(t) ((int64_t)t.tv_sec * 1000000L + (int64_t)t.tv_usec)

void setup_timing(void);

#endif