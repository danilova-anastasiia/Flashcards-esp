#ifndef WEB_H
#define WEB_H

#include "esp_check.h"
#include "esp_http_server.h"


static void wifiEventHandler(void *event_handler_arg, esp_event_base_t event_base, int32_t event_id, void *event_data);

void connectWifi();

void severInit();

static esp_err_t getHandler(httpd_req_t *req);

char *parseHTML();

void connectWifi();

#endif