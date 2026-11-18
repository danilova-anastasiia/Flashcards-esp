#include <stdio.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#include "freertos/event_groups.h"
#include "esp_wifi.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include "esp_check.h"
#include "esp_http_server.h"

#include "web/credentials.h"
#include "web/web.h"

extern const char index_html_start[] asm("_binary_index_html_start");
extern const char index_html_end[]   asm("_binary_index_html_end");

static httpd_handle_t server_handle = NULL;
static void wifiEventHandler(void *event_handler_arg, esp_event_base_t event_base, int32_t event_id, void *event_data){
    if (event_base == WIFI_EVENT){
        switch (event_id){
        case WIFI_EVENT_STA_START:
            printf("WiFi connecting ... \n");
            break;

        case WIFI_EVENT_STA_CONNECTED:
            printf("WiFi connected ... \n");
            break;

        case WIFI_EVENT_STA_DISCONNECTED:
            printf("WiFi lost connection ... \n");
            break;

        case IP_EVENT_STA_GOT_IP:
            printf("WiFi got IP ... \n\n");
            break;

        default:
            break;
        }
    }else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t *e = (ip_event_got_ip_t *)event_data;
        printf("WiFi got IP: " IPSTR "\n", IP2STR(&e->ip_info.ip));
        if (server_handle == NULL) {
            serverInit();
        }
    }
}

void connectWifi(){
    esp_netif_init();
	esp_event_loop_create_default();
	esp_netif_create_default_wifi_sta();
    wifi_init_config_t wifi_initiation = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&wifi_initiation);

	esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, wifiEventHandler, NULL);
    esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, wifiEventHandler, NULL);
    wifi_config_t wifi_configuration = {
        .sta = {
            .ssid = SSID,
            .password = PASS}};

    esp_wifi_set_mode(WIFI_MODE_STA);
    
    esp_wifi_set_config(WIFI_IF_STA, &wifi_configuration);

    esp_wifi_start();

	esp_wifi_connect();
}



static esp_err_t getHandler(httpd_req_t *req){
    httpd_resp_set_type(req, "text/html");
    return httpd_resp_send(req, index_html_start,index_html_end - index_html_start - 1);
}

void serverInit(){
    httpd_config_t server_config = HTTPD_DEFAULT_CONFIG();

    if (httpd_start(&server_handle, &server_config) != ESP_OK) {
        printf("HTTP server failed to start\n");
        server_handle = NULL;
        return;
    }

    httpd_uri_t uri_get = {
        .uri = "/",
        .method = HTTP_GET,
        .handler = getHandler,
        .user_ctx = NULL,
    };
    httpd_register_uri_handler(server_handle, &uri_get);
    printf("Server started\n");
}