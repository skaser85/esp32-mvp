#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_wifi.h"
#include "esp_now.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include <string.h>

static const char *TAG = "MONITOR_RX";

// Ensure this structure matches the transmitter's data payload format exactly
typedef struct {
    uint8_t status_id; 
} status_packet_t;

// This callback function triggers automatically whenever a radio packet arrives
static void on_data_recv(const esp_now_recv_info_t *recv_info, const uint8_t *data, int len) {
    status_packet_t packet;
    if (len == sizeof(packet)) {
        memcpy(&packet, data, sizeof(packet));
        
        // Map the binary ID back to readable text logs
        const char* status_strings[] = {"AVAILABLE", "BUSY", "ON A CALL", "IN A MEETING"};
        if (packet.status_id < 4) {
            ESP_LOGI(TAG, "New Status Received: [ %s ]", status_strings[packet.status_id]);
            // (This is exactly where your motor and LED code will plug in later!)
        }
    }
}

void app_main(void) {
    // Initialize NVS storage partition
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        nvs_flash_erase();
        nvs_flash_init();
    }

    // Fire up raw network stack parameters
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_start());

    // Initialize ESP-NOW engine and bind the listener callback
    ESP_ERROR_CHECK(esp_now_init());
    ESP_ERROR_CHECK(esp_now_register_recv_cb(on_data_recv));

    ESP_LOGI(TAG, "Monitor Receiver is live and listening for data packets...");
    
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(1000)); // Keep CPU running silently in background
    }
}

