#include "string.h"
#include "ssd1306.h"
#include "driver/i2c.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_err.h"
#include "esp_timer.h"
#include "esp_system.h"

// I2C
#define I2C_PORT I2C_NUM_0
#define I2C_SDA_PIN 21
#define I2C_SCL_PIN 22

// LED
#define LED_GPIO_PIN 2

//HC-SR04
#define TRIGGER_PIN 4
#define ECHO_PIN 5

#define WHITE 1
#define BLACK 0
#define LED_ON 1
#define LED_OFF 0

//Dist
#define DISTANCE_THRESHOLD 10

void i2c_init()
{
    i2c_config_t conf = {
        .sda_io_num = I2C_SDA_PIN,
        .scl_io_num = I2C_SCL_PIN,
        .master.clk_speed = 100000,
        .mode = I2C_MODE_MASTER
    };
    ESP_ERROR_CHECK(i2c_param_config(I2C_PORT, &conf));
    ESP_ERROR_CHECK(i2c_driver_install(I2C_PORT, I2C_MODE_MASTER, 0, 0, 0));
}

void gpio_init()
{
    gpio_set_direction(LED_GPIO_PIN, GPIO_MODE_OUTPUT);

    gpio_set_direction(TRIGGER_PIN, GPIO_MODE_OUTPUT);
    gpio_set_direction(ECHO_PIN, GPIO_MODE_INPUT);
}

float get_distance()
{
    gpio_set_level(TRIGGER_PIN, 1);
    esp_rom_delay_us(10);
    gpio_set_level(TRIGGER_PIN, 0);

    while (gpio_get_level(ECHO_PIN) == 0);
    uint32_t start_time = esp_timer_get_time();

    while (gpio_get_level(ECHO_PIN) == 1);
    uint32_t end_time = esp_timer_get_time();  
    uint32_t duration = end_time - start_time;
    float distance = (duration / 2.0) * 0.0343; //calculating cm from voce
    return distance;
}

void app_main(void)
{
    i2c_init();
    gpio_init();

    ssd1306_handle_t ssd1306 = ssd1306_create(I2C_PORT, SSD1306_I2C_ADDRESS);
    //error handling
    if (ssd1306 == NULL) {
        ESP_LOGE("SSD1306", "Ekran başlatılamadı");
        return;
    }

    ssd1306_clear_screen(ssd1306, BLACK);

    while (1) {
        float distance = get_distance();

        char distance_str[16];
        snprintf(distance_str, sizeof(distance_str), "Dist: %.2f cm", distance);
        ssd1306_clear_screen(ssd1306, BLACK);
        ssd1306_draw_string(ssd1306, 0, 0, (const uint8_t *)distance_str, 12, WHITE);

        if (distance > DISTANCE_THRESHOLD) {
            gpio_set_level(LED_GPIO_PIN, LED_OFF);  // 10dan buyukse kapali
        } 
        else if (distance>7){
            gpio_set_level(LED_GPIO_PIN, LED_ON); // 10 ile 7 arasi yak sondur
            vTaskDelay(20);
            gpio_set_level(LED_GPIO_PIN, LED_OFF);
            vTaskDelay(20);
        }
        else if (distance>4){
			gpio_set_level(LED_GPIO_PIN, LED_ON); // 7 ile 4 arasi hizli yak sondur
            vTaskDelay(5);
            gpio_set_level(LED_GPIO_PIN, LED_OFF);
            vTaskDelay(5);
		}
        else {
			gpio_set_level(LED_GPIO_PIN, LED_ON); //4cmden kucukse full yak
		}

        // Ekranı yenile
        ssd1306_refresh_gram(ssd1306);

        // 50ms
        vTaskDelay(50 / portTICK_PERIOD_MS);
    }
}
