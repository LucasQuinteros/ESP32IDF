/* WiFi softAP Example

  This example code is in the Public Domain (or CC0 licensed, at your option.)

  Unless required by applicable law or agreed to in writing, this
  software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
  CONDITIONS OF ANY KIND, either express or implied.
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_err.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "driver/gpio.h"
#include "driver/ledc.h"

#include "lwip/err.h"
#include "lwip/sys.h"

#define EXAMPLE_ESP_WIFI_SSID "Lucas"
#define EXAMPLE_ESP_WIFI_PASS "12345678"
#define EXAMPLE_MAX_STA_CONN (3)
#define GPIO_OUTPUT_PIN_SEL ((1ULL<<GPIO_NUM_2))

#ifdef CONFIG_IDF_TARGET_ESP32
#define LEDC_HS_TIMER       LEDC_TIMER_0
#define LEDC_HS_MODE        LEDC_HIGH_SPEED_MODE
#define LEDC_HS_CH0_GPIO    (2)
#define LEDC_HS_CH0_CHANNEL LEDC_CHANNEL_0
#endif

#define LEDC_TEST_CH_NUM      (4)
#define LEDC_TEST_DUTY        (1000)
#define LEDC_TEST_FADE_TIME   (3000)


//1ULL unsigned long long 64bits

static const char *TAG = "wifi softAP";

static void wifi_event_handler(void *arg, esp_event_base_t event_base,
                              int32_t event_id, void *event_data)
{
  if (event_id == WIFI_EVENT_AP_STACONNECTED)
  {
    wifi_event_ap_staconnected_t *event = (wifi_event_ap_staconnected_t *)event_data;
    ESP_LOGI(TAG, "station " MACSTR " join, AID=%d",MAC2STR(event->mac), event->aid);

    //gpio_set_level(GPIO_NUM_2,1);
    //ledc_set_duty(LEDC_HS_MODE, LEDC_HS_CH0_CHANNEL,100);
    //ledc_update_duty(LEDC_HS_MODE, LEDC_HS_CH0_CHANNEL);
    ledc_set_fade_with_time(LEDC_HS_MODE,LEDC_HS_CH0_CHANNEL,LEDC_TEST_DUTY,1000);
    ledc_fade_start(LEDC_HS_MODE,LEDC_HS_CH0_CHANNEL,LEDC_FADE_NO_WAIT);
  }
  else if (event_id == WIFI_EVENT_AP_STADISCONNECTED)
  {
    wifi_event_ap_stadisconnected_t *event = (wifi_event_ap_stadisconnected_t *)event_data;
    ESP_LOGI(TAG, "station " MACSTR " leave, AID=%d",
            MAC2STR(event->mac), event->aid);
    
    //gpio_set_level(GPIO_NUM_2,0);
    //ledc_set_duty(LEDC_HS_MODE, LEDC_HS_CH0_CHANNEL,0);
    //ledc_update_duty(LEDC_HS_MODE, LEDC_HS_CH0_CHANNEL);
    ledc_set_fade_with_time(LEDC_HS_MODE,LEDC_HS_CH0_CHANNEL,0,1000);
    ledc_fade_start(LEDC_HS_MODE,LEDC_HS_CH0_CHANNEL,LEDC_FADE_NO_WAIT);
  }
}

void wifi_init_softap()
{
  tcpip_adapter_init();
  ESP_ERROR_CHECK(esp_event_loop_create_default());

  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  ESP_ERROR_CHECK(esp_wifi_init(&cfg));

  ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL));

  wifi_config_t wifi_config = {
      .ap = {
          .ssid = EXAMPLE_ESP_WIFI_SSID,
          .ssid_len = strlen(EXAMPLE_ESP_WIFI_SSID),
          .password = EXAMPLE_ESP_WIFI_PASS,
          .max_connection = EXAMPLE_MAX_STA_CONN,
          .authmode = WIFI_AUTH_WPA_WPA2_PSK},
  };
  if (strlen(EXAMPLE_ESP_WIFI_PASS) == 0)
  {
    wifi_config.ap.authmode = WIFI_AUTH_OPEN;
  }

  ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
  ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_AP, &wifi_config));
  ESP_ERROR_CHECK(esp_wifi_start());

  ESP_LOGI(TAG, "wifi_init_softap finished. SSID:%s password:%s",
          EXAMPLE_ESP_WIFI_SSID, EXAMPLE_ESP_WIFI_PASS);
}

void init_led(){
  gpio_config_t io_config = {};
  io_config.intr_type = GPIO_INTR_DISABLE;
  io_config.mode = GPIO_MODE_OUTPUT;
  io_config.pin_bit_mask = GPIO_OUTPUT_PIN_SEL;
  io_config.pull_down_en = GPIO_PULLDOWN_DISABLE;
  io_config.pull_up_en = GPIO_PULLUP_DISABLE;
  gpio_config(&io_config);
  
}

void init_pwm(){
  
  //configuracion de timer
  ledc_timer_config_t ledc_timer = {
    .duty_resolution = LEDC_TIMER_13_BIT, //resolucion en bits
    .freq_hz = 5000,                      //frecuencia
    .speed_mode = LEDC_HS_MODE,           //modo sin glitch
    .clk_cfg = LEDC_AUTO_CLK,             //seleccion automatica del reloj
  };
  ledc_timer_config(&ledc_timer);

  ledc_channel_config_t ledc_channel = {
    
      .channel = LEDC_HS_CH0_CHANNEL,
      .duty = 0,
      .gpio_num = LEDC_HS_CH0_GPIO,
      .speed_mode = LEDC_HS_MODE,
      .hpoint = 0,
      .timer_sel = LEDC_HS_TIMER
    
  };
  ledc_channel_config(&ledc_channel);
  ledc_fade_func_install(0);
}

void app_main()
{
  //init_led();
  init_pwm();
  // Initialize NVS
  esp_err_t ret = nvs_flash_init();
  if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
  {
    ESP_ERROR_CHECK(nvs_flash_erase());
    ret = nvs_flash_init();
  }
  ESP_ERROR_CHECK(ret);

  ESP_LOGI(TAG, "ESP_WIFI_MODE_AP");

  wifi_init_softap();
}