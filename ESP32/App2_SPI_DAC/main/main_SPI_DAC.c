/*

Código de interação com SPI adaptados dos trabalhos do colega Bernardo Barreto e Paulo Pinho. 

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "driver/touch_pad.h"
#include "sdkconfig.h"
#include "esp_log.h"
#include "eeprom.h"
#include "esp_random.h"
#include "driver/dac.h"
#include "driver/dac_common.h"
#include "esp_sleep.h"


#define TOUCH_THRESH_NO_USE   (0)
#define TOUCHPAD_FILTER_TOUCH_PERIOD (10)


static const char *TAG = "MAIN";

#define	EEPROM_MODEL "24LC040A"

void dump(uint8_t *dt, int n)
{
	uint16_t clm = 0;
	uint8_t data;
	uint32_t saddr =0;
	uint32_t eaddr =n-1;

	printf("--------------------------------------------------------\n");
	uint32_t addr;
	for (addr = saddr; addr <= eaddr; addr++) {
		data = dt[addr];
		if (clm == 0) {
			printf("%05x: ",addr);
		}

		printf("%02x ",data);
		clm++;
		if (clm == 16) {
			printf("| \n");
			clm = 0;
		}
	}
	printf("--------------------------------------------------------\n");
}

static void calibrate_touch_pad(touch_pad_t pad)
{
    int avg = 0;
    const size_t calibration_count = 128;
    for (int i = 0; i < calibration_count; ++i) {
        uint16_t val;
        touch_pad_read(pad, &val);
        avg += val;
    }
    avg /= calibration_count;
    const int min_reading = 300;
    if (avg < min_reading) {
        printf("Touch pad #%d average reading is too low: %d (expecting at least %d). "
               "Not using for deep sleep wakeup.\n", pad, avg, min_reading);
        touch_pad_config(pad, 0);
    } else {
        int threshold = avg - 100;
        printf("Touch pad #%d average: %d, wakeup threshold set to %d.\n", pad, avg, threshold);
        touch_pad_config(pad, threshold);
    }
}

static void configure_touch_pad(int index){
    //init touch pad
    ESP_ERROR_CHECK(touch_pad_init());
	touch_pad_set_fsm_mode(TOUCH_FSM_MODE_TIMER);

    // Set reference voltage for charging/discharging
    // In this case, the high reference valtage will be 2.7V - 1V = 1.7V
    // The low reference voltage will be 0.5
    // The larger the range, the larger the pulse count value.
    touch_pad_set_voltage(TOUCH_HVOLT_2V7, TOUCH_LVOLT_0V5, TOUCH_HVOLT_ATTEN_1V);
    touch_pad_config(index, TOUCH_THRESH_NO_USE); // 0 to not generate interrupts
	calibrate_touch_pad(index);
}


void app_main(void)
{
    ESP_LOGI(TAG, "EEPROM_MODEL=%s", EEPROM_MODEL);
	EEPROM_t dev;
	spi_master_init(&dev);
	int32_t totalBytes = eeprom_TotalBytes(&dev);
	ESP_LOGI(TAG, "totalBytes=%d Bytes",totalBytes);

	esp_err_t re = dac_output_enable(DAC_CHANNEL_1); //DAC on GPIO25
	ESP_ERROR_CHECK(re);
	configure_touch_pad(0);
	ESP_ERROR_CHECK(esp_sleep_enable_touchpad_wakeup());
	esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH, ESP_PD_OPTION_ON);

	// Get Status Register
	uint8_t reg;
	esp_err_t ret;
	ret = eeprom_ReadStatusReg(&dev, &reg);
	if (ret != ESP_OK) {
		ESP_LOGI(TAG, "ReadStatusReg Fail %d",ret);
		while(1) { vTaskDelay(1); }
	} 
	ESP_LOGI(TAG, "readStatusReg : 0x%02x", reg);

	uint8_t wdata[256];
	int len;

	esp_fill_random(wdata, sizeof(wdata)); //generate random array

	for (int i = 0; i < sizeof(wdata); i++)
	{
		wdata[i] = wdata[i] % 255; // Cap at 255 for voltage level in DAC output
	}
	
	
   

	for (int addr=0; addr<sizeof(wdata);addr++) {
		len =  eeprom_WriteByte(&dev, addr, wdata[addr]);
		ESP_LOGI(TAG, "WriteByte(addr=%d) len=%d: data=%2x", addr, len, wdata[addr]);
		if (len != 1) {
			ESP_LOGI(TAG, "WriteByte Fail addr=%d", addr);
			while(1) { vTaskDelay(1); }
		}
	}

	// Read random data and apply data to voltage output
	for (int addr=0; addr<sizeof(wdata);addr++) {
		uint8_t data;
		len =  eeprom_Read(&dev, addr, &data, 1);
		ESP_LOGI(TAG, "ReadByte(addr=%d) len=%d: data=%u", addr, len, data);
		if (len != 1) {
			ESP_LOGI(TAG, "ReadByte Fail addr=%d", addr);
			while(1) { vTaskDelay(1); }
		}
		esp_err_t abc = dac_output_voltage(DAC_CHANNEL_1, data);
		ESP_ERROR_CHECK(abc);
		vTaskDelay(50 / portTICK_PERIOD_MS);
	}

	 esp_deep_sleep_start();

}
