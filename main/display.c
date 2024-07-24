#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_ops.h"
#include "esp_err.h"
#include "esp_log.h"
#include "driver/i2c_master.h"
#include "esp_lvgl_port.h"
#include "lvgl.h"
#include "esp_lcd_panel_vendor.h"

static const char *TAG = "Display";

#define I2C_BUS_PORT 0

#define DISPLAY_LCD_PIXEL_CLOCK_HZ (400 * 1000)
#define DISPLAY_PIN_NUM_SDA 16
#define DISPLAY_PIN_NUM_SCL 17
#define DISPLAY_PIN_NUM_RST -1
#define DISPLAY_I2C_HW_ADDR 0x3C
#define DISPLAY_LCD_H_RES 128
#define DISPLAY_LCD_V_RES 64
#define DISPLAY_LCD_CMD_BITS 8
#define DISPLAY_LCD_PARAM_BITS 8

void setup_display()
{
    ESP_LOGI(TAG, "Initialize I2C bus");
    i2c_master_bus_handle_t i2c_bus = NULL;
    i2c_master_bus_config_t bus_config = {
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .glitch_ignore_cnt = 7,
        .i2c_port = I2C_BUS_PORT,
        .sda_io_num = DISPLAY_PIN_NUM_SDA,
        .scl_io_num = DISPLAY_PIN_NUM_SCL,
        .flags.enable_internal_pullup = true,
    };
    ESP_ERROR_CHECK(i2c_new_master_bus(&bus_config, &i2c_bus));

    ESP_LOGI(TAG, "Install panel IO");
    esp_lcd_panel_io_handle_t io_handle = NULL;
    esp_lcd_panel_io_i2c_config_t io_config = {
        .dev_addr = DISPLAY_I2C_HW_ADDR,
        .scl_speed_hz = DISPLAY_LCD_PIXEL_CLOCK_HZ,
        .control_phase_bytes = 1,               // According to SSD1306 datasheet
        .lcd_cmd_bits = DISPLAY_LCD_CMD_BITS,   // According to SSD1306 datasheet
        .lcd_param_bits = DISPLAY_LCD_CMD_BITS, // According to SSD1306 datasheet
        .dc_bit_offset = 6,                     // According to SSD1306 datasheet
    };
    ESP_ERROR_CHECK(esp_lcd_new_panel_io_i2c(i2c_bus, &io_config, &io_handle));

    ESP_LOGI(TAG, "Install SSD1306 panel driver");
    esp_lcd_panel_handle_t panel_handle = NULL;
    esp_lcd_panel_dev_config_t panel_config = {
        .bits_per_pixel = 1,
        .reset_gpio_num = DISPLAY_PIN_NUM_RST,
    };
    ESP_ERROR_CHECK(esp_lcd_new_panel_ssd1306(io_handle, &panel_config, &panel_handle));

    ESP_ERROR_CHECK(esp_lcd_panel_reset(panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_init(panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_disp_on_off(panel_handle, true));

    ESP_LOGI(TAG, "Initialize LVGL");
    const lvgl_port_cfg_t lvgl_cfg = ESP_LVGL_PORT_INIT_CONFIG();
    ESP_ERROR_CHECK(lvgl_port_init(&lvgl_cfg));

    const lvgl_port_display_cfg_t disp_cfg = {
        .io_handle = io_handle,
        .panel_handle = panel_handle,
        .buffer_size = DISPLAY_LCD_H_RES * DISPLAY_LCD_V_RES,
        .double_buffer = true,
        .hres = DISPLAY_LCD_H_RES,
        .vres = DISPLAY_LCD_V_RES,
        .monochrome = true,
        .rotation = {
            .swap_xy = false,
            .mirror_x = false,
            .mirror_y = false,
        }};

    lv_disp_t *disp = lvgl_port_add_disp(&disp_cfg);

    /* Rotation of the screen */
    lv_disp_set_rotation(disp, LV_DISP_ROT_180);
    if (lvgl_port_lock(0))
    {
        ESP_LOGI(TAG, "Start displaying text.");
        lv_obj_t *scr = lv_disp_get_scr_act(disp);
        lv_obj_clean(scr); // clear the screen

        lv_obj_t *label = lv_label_create(scr);
        lv_label_set_long_mode(label, LV_LABEL_LONG_SCROLL_CIRCULAR); // Circular scroll
        lv_label_set_text(label, "Messung starten\n\n\n -->");
        // Size of the screen (if you use rotation 90 or 270, please set disp->driver->ver_res)
        lv_obj_set_width(label, DISPLAY_LCD_H_RES);
        lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);

        lvgl_port_unlock();
    }
}