#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_wifi.h"
#include "esp_now.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include <string.h>

static const char *TAG = "DESK_TX";

// Your verified monitor unit hardware address signature
static const uint8_t receiver_mac[6] = {0x20, 0x50, 0x0D, 0xDA, 0xE1, 0xB0};

// Structure representing your availability status payload matching the receiver
typedef struct {
    uint8_t status_id;  // 0=Available, 1=Busy, 2=On a Call, 3=In a Meeting
} status_packet_t;

// Verification callback indicating if the data frame cleared the antenna line
static void on_data_sent(const uint8_t *mac_addr, esp_now_send_status_t status) {
    if (status == ESP_NOW_SEND_SUCCESS) {
        ESP_LOGI(TAG, "Packet successfully delivered over the air!");
    } else {
        ESP_LOGW(TAG, "Delivery failed! Receiver out of range or powered off.");
    }
}

void app_main(void) {
    // 1. Initialize NVS (Non-Volatile Storage)
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        nvs_flash_erase();
        nvs_flash_init();
    }

    // 2. Fire up the base network interface layers
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    // Put radio into Station Mode without accessing a router access point
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_start());

    // 3. Initialize the ESP-NOW communication protocol engine
    // (Note: explicitly casting the callback prevents type warnings on newer versions)
    ESP_ERROR_CHECK(esp_now_init());
    ESP_ERROR_CHECK(esp_now_register_send_cb((esp_now_send_cb_t)on_data_sent));

    // 4. Register the target Monitor receiver board as a verified radio peer
    esp_now_peer_info_t peer_info = {};
    memcpy(peer_info.peer_addr, receiver_mac, 6);
    peer_info.channel = 1;
    peer_info.encrypt = false;
    ESP_ERROR_CHECK(esp_now_add_peer(&peer_info));

    status_packet_t text_packet;
    uint8_t test_counter = 0;

    while (1) {
        // Cycle through status values 0 to 3 to simulate a knob twist
        text_packet.status_id = test_counter % 4;

        ESP_LOGI(TAG, "Sending status frame index: %d", text_packet.status_id);

        // Broadcast data packet to the designated hardware target line
        esp_now_send(receiver_mac, (const uint8_t *) &text_packet, sizeof(text_packet));

        test_counter++;
        vTaskDelay(pdMS_TO_TICKS(3000)); // Broadcast data frame changes every 3 seconds
    }
}

