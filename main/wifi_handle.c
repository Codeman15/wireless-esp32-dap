#include "sdkconfig.h"

#include <string.h>
#include <stdint.h>
#include <sys/param.h>

#include "main/wifi_configuration.h"
#include "main/uart_bridge.h"

#include "components/DAP/include/gpio_op.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event_loop.h"
#include "esp_log.h"

#ifdef CONFIG_IDF_TARGET_ESP8266
    #define PIN_LED_WIFI_STATUS 15
#elif defined CONFIG_IDF_TARGET_ESP32
    #define PIN_LED_WIFI_STATUS 27
#elif defined CONFIG_IDF_TARGET_ESP32C3
    #define PIN_LED_WIFI_STATUS 10
#else
    #error unknown hardware
#endif

static EventGroupHandle_t wifi_event_group;

const int IPV4_GOTIP_BIT = BIT0;

#ifdef CONFIG_EXAMPLE_IPV6
const int IPV6_GOTIP_BIT = BIT1;
#endif

static esp_err_t event_handler(void *ctx, system_event_t *event)
{
    switch (event->event_id)
    {
        case SYSTEM_EVENT_AP_START:
            GPIO_SET_LEVEL_HIGH(PIN_LED_WIFI_STATUS);
            os_printf("DAP ACCESS POINT STARTED\r\n");
            os_printf("SSID: DAP\r\n");
            os_printf("PASSWORD: 12345678\r\n");
            break;

        case SYSTEM_EVENT_AP_STACONNECTED:
            os_printf("DAP CLIENT CONNECTED\r\n");
            break;

        case SYSTEM_EVENT_AP_STADISCONNECTED:
            os_printf("DAP CLIENT DISCONNECTED\r\n");
            break;

        default:
            break;
    }

    return ESP_OK;
}

void wifi_init(void)
{
    GPIO_FUNCTION_SET(PIN_LED_WIFI_STATUS);
    GPIO_SET_DIRECTION_NORMAL_OUT(PIN_LED_WIFI_STATUS);
    GPIO_SET_LEVEL_LOW(PIN_LED_WIFI_STATUS);

    tcpip_adapter_init();

    wifi_event_group = xEventGroupCreate();

    ESP_ERROR_CHECK(esp_event_loop_init(event_handler, NULL));

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();

    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));

    /*
     * Use Access Point mode.
     * This makes ESP32 #2 create its own Wi-Fi network
     * instead of trying to connect to another router.
     */
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));

    wifi_config_t ap_config = {
        .ap = {
            .ssid = "DAP",
            .password = "12345678",
            .ssid_len = 3,
            .channel = 1,
            .authmode = WIFI_AUTH_WPA2_PSK,
            .ssid_hidden = 0,
            .max_connection = 4,
            .beacon_interval = 100,
        },
    };

    ESP_ERROR_CHECK(
        esp_wifi_set_config(ESP_IF_WIFI_AP, &ap_config)
    );

    ESP_ERROR_CHECK(esp_wifi_set_ps(WIFI_PS_NONE));

    ESP_ERROR_CHECK(esp_wifi_start());

    os_printf("\r\n");
    os_printf("================================\r\n");
    os_printf("WIRELESS ESP-DAP\r\n");
    os_printf("================================\r\n");
    os_printf("WiFi Mode: ACCESS POINT\r\n");
    os_printf("SSID: DAP\r\n");
    os_printf("Password: 12345678\r\n");
    os_printf("================================\r\n");
}
