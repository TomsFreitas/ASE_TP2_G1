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
#include "tc74.h"
#include "esp_log.h"
#include "sdkconfig.h"


static const char *TAG = "I2C_PWM application example";

static void i2c_temperature_task(void *arg){

  // setup the sensor
  ESP_ERROR_CHECK(i2c_master_init());

  // signed integer value of 8 bits
  uint8_t temperature_value;

  // signed integer value of 8 bits
  uint8_t operation_mode;

  // set standby mode for testing (5uA consuption)
  // i2c_master_set_tc74_mode(I2C_MASTER_NUM, SET_STANBY_VALUE);

  // set normal mode for testing (200uA consuption)
  i2c_master_set_tc74_mode(I2C_MASTER_NUM, SET_NORM_OP_VALUE);

  // periodically read temp values from sensor and set the sensor to power saving mode
  while(1){

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

    vTaskDelay(8000 / portTICK_RATE_MS);
  }
}

void app_main(void)
{
    // sensor handling task
    xTaskCreate(i2c_temperature_task, "i2c_temperature_task", 1024 * 2, (void *)0, 10, NULL);
}
