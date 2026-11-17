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
#include "include/web/credentials.h"
#include "include/web/web.h"

static void wifiEventHandler(void *event_handler_arg, esp_event_base_t event_base, int32_t event_id, void *event_data){
    switch (event_id)
    {
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
    esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_configuration);

    esp_wifi_start();

	esp_wifi_connect();
}

char *parseHTML(){
    FILE *html = fopen("include/web/index.html", "r");

    if(html==NULL){
        return NULL;
    }

    fseek(html,0,SEEK_END);
    int size=ftell(html);
    fseek(html,0,SEEK_SET);

    char *response_message = malloc((size+1)*sizeof(response_message));
    char c;    
    for(int i=0; c=fgetc(html)!=EOF; i++){
        response_message[i]=c;
    }
    response_message[size+1]='\0';
    fclose(html);

    return response_message;

}

static esp_err_t getHandler(httpd_req_t *req){
    
    char *response_message = parseHTML();
    httpd_resp_send(req, response_message, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

void severInit(){
    httpd_config_t server_config = HTTPD_DEFAULT_CONFIG();
    httpd_handle_t server_handle = NULL;
    httpd_start(&server_handle, &server_config);

    httpd_uri_t uri_get = {
        .uri = "/",
        .method = HTTP_GET,
        .handler = getHandler,
        .user_ctx = NULL};
    httpd_register_uri_handler(server_handle, &uri_get);
}