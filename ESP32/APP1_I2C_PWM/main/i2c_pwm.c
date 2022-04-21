/* Blink Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2c.h"
#include "driver/timer.h"
#include "driver/ledc.h"
#include "tc74.h"
#include "esp_log.h"
#include "sdkconfig.h"


#define LEDC_TIMER              LEDC_TIMER_0
#define LEDC_MODE               LEDC_LOW_SPEED_MODE
#define LEDC_OUTPUT_IO          (2) // Define the output GPIO
#define LEDC_CHANNEL            LEDC_CHANNEL_0
#define LEDC_DUTY_RES           LEDC_TIMER_13_BIT // Set duty resolution to 13 bits
#define LEDC_DUTY               (4091) // Set duty to 50%. ((2 ** 13) - 1) * 50% = 4095
#define LEDC_FREQUENCY          (5000) // Frequency in Hertz. Set frequency at 5 kHz


static const char *TAG = "I2C_PWM application example";



static void timer_config() {

    // Prepare and then apply the LEDC PWM timer configuration
    ledc_timer_config_t ledc_timer = {
        .speed_mode       = LEDC_MODE,
        .timer_num        = LEDC_TIMER,
        .duty_resolution  = LEDC_DUTY_RES,
        .freq_hz          = LEDC_FREQUENCY,  // Set output frequency at 5 kHz
        .clk_cfg          = LEDC_AUTO_CLK
    };

    ledc_timer_config(&ledc_timer);

    ledc_channel_config_t ledc_channel = {
        // Prepare and then apply the LEDC PWM channel configuration
        .speed_mode     = LEDC_MODE,
        .channel        = LEDC_CHANNEL,
        .timer_sel      = LEDC_TIMER, 
        .intr_type      = LEDC_INTR_DISABLE,
        .gpio_num       = LEDC_OUTPUT_IO,
        .duty           = 0, // Set duty to 0%
        .hpoint         = 0

    };
    ledc_channel_config(&ledc_channel);      
}


void app_main(void)
{   

    // setup the sensor
    ESP_ERROR_CHECK(i2c_master_init());

    uint8_t temperature_value;
    uint8_t operation_mode;

    uint8_t initial_temp;
    i2c_master_read_tc74_config(I2C_MASTER_NUM,&operation_mode);
    // ESP_LOGI(TAG,"Operation mode is : %d",operation_mode);
    // set normal mode for testing (200uA consuption)
    i2c_master_set_tc74_mode(I2C_MASTER_NUM, SET_NORM_OP_VALUE);
    vTaskDelay(250 / portTICK_RATE_MS);
    i2c_master_read_temp(I2C_MASTER_NUM,&initial_temp);
    ESP_LOGI(TAG,"Initial Temperature is : %d",initial_temp);
    i2c_master_read_tc74_config(I2C_MASTER_NUM,&operation_mode);
    // ESP_LOGI(TAG,"Operation mode is : %d",operation_mode);
    // set standby mode for testing (5uA consuption)
    i2c_master_set_tc74_mode(I2C_MASTER_NUM, SET_STANBY_VALUE);
    vTaskDelay(1000 / portTICK_RATE_MS);
    
    uint32_t dt = 4094; 

    timer_config();
    ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, dt);
    ledc_update_duty(LEDC_MODE, LEDC_CHANNEL);

    while(1) {

        i2c_master_read_tc74_config(I2C_MASTER_NUM,&operation_mode);
        // ESP_LOGI(TAG,"Operation mode is : %d",operation_mode);
        // set normal mode for testing (200uA consuption)
        i2c_master_set_tc74_mode(I2C_MASTER_NUM, SET_NORM_OP_VALUE);
        vTaskDelay(250 / portTICK_RATE_MS);
        i2c_master_read_temp(I2C_MASTER_NUM,&temperature_value);
        ESP_LOGI(TAG,"Temperature is : %d",temperature_value);


        i2c_master_read_tc74_config(I2C_MASTER_NUM,&operation_mode);
        // ESP_LOGI(TAG,"Operation mode is : %d",operation_mode);
        // set standby mode for testing (5uA consuption)
        i2c_master_set_tc74_mode(I2C_MASTER_NUM, SET_STANBY_VALUE);

        dt = temperature_value > initial_temp ? 6140 : 2500;

        dt = (temperature_value * LEDC_DUTY) / initial_temp;
        ESP_LOGI(TAG,"Duty Cycle is : %d",dt);

        ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, dt);
        ledc_update_duty(LEDC_MODE, LEDC_CHANNEL);

        vTaskDelay(8000 / portTICK_RATE_MS);
    }
}
