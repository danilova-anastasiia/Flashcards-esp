#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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
#include "cJSON.h"
#include "esp_event.h"
#include "esp_lvgl_port.h"
#include "ui/src/ui/ui.h"

#include "web/credentials.h"
#include "web/web.h"
#include "cards/cards.h"

#define WIFI_MAX_RETRY 10
#define MAX_BODY  8192
#define MAX_TEXT  200

static bool webStarted = false;
static bool webOn = false;

extern const char index_html_start[] asm("_binary_index_html_start");
extern const char index_html_end[]   asm("_binary_index_html_end");

static esp_err_t getIndexHandler(httpd_req_t *req);
static esp_err_t getListDecksHandler(httpd_req_t *req);
static esp_err_t getDeckOpenHandler(httpd_req_t *req);
static esp_err_t postDeckHandler(httpd_req_t *req);
static esp_err_t deleteDeckHandler(httpd_req_t *req);
static esp_err_t postCardsHandler(httpd_req_t *req);
static esp_err_t deleteCardsHandler(httpd_req_t *req);

static const httpd_uri_t uris[] = {
    {.uri = "/",.method = HTTP_GET,.handler = getIndexHandler},
    {.uri = "/decks",.method = HTTP_GET,.handler = getListDecksHandler},
    {.uri = "/deck",.method = HTTP_GET,.handler = getDeckOpenHandler},
    {.uri = "/deck",.method = HTTP_POST,.handler = postDeckHandler},
    {.uri = "/deck",.method = HTTP_DELETE,.handler = deleteDeckHandler},
    {.uri = "/card",.method = HTTP_POST,.handler = postCardsHandler},
    {.uri = "/card",.method = HTTP_DELETE,.handler = deleteCardsHandler}
};

static int wifi_retry_count = 0;

static httpd_handle_t server_handle = NULL;

static esp_err_t sendJson(httpd_req_t *req, const char *status, cJSON *obj) {
    char *text = cJSON_PrintUnformatted(obj);
    cJSON_Delete(obj);
    httpd_resp_set_status(req, status);
    httpd_resp_set_type(req, "application/json");
    esp_err_t r = httpd_resp_sendstr(req, text);
    free(text);
    return r;
}

static char *readBody(httpd_req_t *req) {
    size_t len = req->content_len;
    if(len == 0 || len > MAX_BODY){
        return NULL;
    }

    char *buf = malloc(len + 1);
    
    if(!buf){
        return NULL;
    }

    size_t got = 0;
    int timeouts = 0;
    while(got<len){
        int r = httpd_req_recv(req, buf + got, len - got);
        if(r == HTTPD_SOCK_ERR_TIMEOUT && ++timeouts < 5) continue;
        if(r<= 0){ 
            free(buf); return NULL;
        }
        got += (size_t)r;
    }
    buf[len] = '\0';
    return buf;
}

static esp_err_t fail(httpd_req_t *req, const char *status, const char *message){
    cJSON *o = cJSON_CreateObject();
    cJSON_AddBoolToObject(o, "ok", false);
    cJSON_AddStringToObject(o, "error", message);
    return sendJson(req, status, o);
}

static esp_err_t success(httpd_req_t *req, int added){
    cJSON *o = cJSON_CreateObject();
    cJSON_AddBoolToObject(o, "ok", true);
    cJSON_AddNumberToObject(o, "added", added);
    return sendJson(req, "200 OK", o);
}

static void wifiEventHandler(void *event_handler_arg, esp_event_base_t event_base, int32_t event_id, void *event_data){
    if (event_base == WIFI_EVENT){
        switch (event_id){
        case WIFI_EVENT_STA_START:
            wifi_retry_count = 0;
            printf("WiFi connecting ... \n");
            esp_wifi_connect();
            break;

        case WIFI_EVENT_STA_CONNECTED:
            wifi_retry_count = 0;
            printf("WiFi connected ... \n");
            break;

        case WIFI_EVENT_STA_DISCONNECTED:
            printf("WiFi disconnected / attempt failed ... \n");
            if (wifi_retry_count < WIFI_MAX_RETRY) {
                wifi_retry_count++;
                vTaskDelay(pdMS_TO_TICKS(1000));
                printf("Retrying WiFi (%d/%d)...\n", wifi_retry_count, WIFI_MAX_RETRY);
                esp_wifi_connect();
            } else {
                webOn = false;
                webStarted = false;
                printf("WiFi retry limit reached\n");
            }
            break;

        default:
            break;
        }
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t *e = (ip_event_got_ip_t *)event_data;
        wifi_retry_count = 0;
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

static void netEventHandler(void *arg, esp_event_base_t base, int32_t id, void *data){
    bool on = false;
    if(base == IP_EVENT && id == IP_EVENT_STA_GOT_IP){
        on=true;
    }
    char ip[16]="";
    if(on){
        ip_event_got_ip_t *e=(ip_event_got_ip_t *)data;
        snprintf(ip, sizeof(ip), IPSTR, IP2STR(&e->ip_info.ip));
    }

    if(lvgl_port_lock(0)){
        ui_set_web(on);
        ui_set_ip(ip);
        lvgl_port_unlock();
    }
}
void toggleWeb(void) {
    if(!webStarted){
        connectWifi();
        esp_event_handler_register(IP_EVENT,IP_EVENT_STA_GOT_IP, netEventHandler, NULL);
        esp_event_handler_register(IP_EVENT,IP_EVENT_STA_LOST_IP,netEventHandler, NULL);
        esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED,netEventHandler, NULL);
        webStarted = true;
        webOn=true;
        printf("web: connecting");
        return;
    }

    if(webOn){
        if(lvgl_port_lock(0)){
            ui_set_web(false);
            ui_set_ip("");
            lvgl_port_unlock();
        }
        esp_wifi_stop();
        webOn = false;
        if(lvgl_port_lock(0)){
            ui_set_web(false);
            lvgl_port_unlock();
        }
        printf("web: off");

    }else{
        esp_wifi_start();
        webOn = true;
        printf("web: connecting");
    }
}

static esp_err_t getIndexHandler(httpd_req_t *req){
    httpd_resp_set_type(req, "text/html");
    return httpd_resp_send(req, index_html_start, index_html_end - index_html_start - 1);
}

static esp_err_t getListDecksHandler(httpd_req_t *req){
    static char names[MAX_DECKS][67];
    int n = deckList(DECK_DIR, names, MAX_DECKS);

    cJSON *arr = cJSON_CreateArray();
    if (!arr) {
        return httpd_resp_send_500(req);
    }
    for (int i = 0; i < n; i++) {
        char path[128];
        snprintf(path, sizeof(path), "%s/%s", DECK_DIR, names[i]);

        char *dot=strrchr(names[i], '.');
        if(dot){
            *dot='\0';
        }
        cJSON *deck = cJSON_CreateObject();
        cJSON_AddStringToObject(deck, "name", names[i]);
        cJSON_AddNumberToObject(deck, "cards", getCardCount(path));
        cJSON_AddItemToArray(arr, deck);
    }
    return sendJson(req, "200 OK", arr);
}

static esp_err_t postDeckHandler(httpd_req_t *req){
    char *body = readBody(req);
    cJSON *in = cJSON_Parse(body);
    free(body);
    if(!in){
        cJSON *err = cJSON_CreateObject();
        cJSON_AddBoolToObject(err, "ok", false);
        cJSON_AddStringToObject(err, "error", "invalid json");
        return sendJson(req, "400 Bad Request", err);
    }

    cJSON *name = cJSON_GetObjectItemCaseSensitive(in, "name");
    cJSON *title = cJSON_GetObjectItemCaseSensitive(in, "title");

    if(!cJSON_IsString(name) || name->valuestring[0] == '\0'){
        cJSON_Delete(in);
        cJSON *err = cJSON_CreateObject();
        cJSON_AddBoolToObject(err, "ok", false);
        cJSON_AddStringToObject(err, "error", "missing deck name");
        return sendJson(req, "400 Bad Request", err);
    }

    char path[128];
    snprintf(path, sizeof(path), "%s/%s.json", DECK_DIR, name->valuestring);

    const char *t=(cJSON_IsString(title)&&title->valuestring[0] != '\0') ? title->valuestring: name->valuestring;

    esp_err_t r = createDeck(path, t);
    cJSON_Delete(in);

    if(r == ESP_OK){
        return success(req, 0);
    }

    return fail(req, "400 Bad Request","failed to create deck");
}

static esp_err_t deleteDeckHandler(httpd_req_t *req){
    char name[48], path[128], query[96];

    httpd_req_get_url_query_str(req, query, sizeof(query));
    httpd_query_key_value(query, "name", name, sizeof(name));

    if(httpd_req_get_url_query_str(req, query, sizeof(query))!= ESP_OK ||
        httpd_query_key_value(query, "name", name, sizeof(name))!=ESP_OK || name[0] == '\0'){
        return fail(req, "400 Bad Request", "missing deck name");
    }

    snprintf(path, sizeof(path), "%s/%s.json", DECK_DIR, name);
    
    if(deleteDeckFile(path)){
        return success(req, 0);
    }else{
        return fail(req, "404 Not Found","deck not found");
    }
}

static esp_err_t getDeckOpenHandler(httpd_req_t *req){
    char name[48], path[128], query[96];

    httpd_req_get_url_query_str(req, query, sizeof(query));
    httpd_query_key_value(query, "name", name, sizeof(name));

    snprintf(path, sizeof(path), "%s/%s.json", DECK_DIR, name);
    FILE *f = fopen(path, "rb");
    if(!f){
        return fail(req, "404 Not Found","deck not found");
    }
    httpd_resp_set_type(req, "application/json");
    char chunk[512];
    size_t n;
    while((n = fread(chunk, 1, sizeof(chunk), f)) > 0){
        if(httpd_resp_send_chunk(req, chunk, (ssize_t)n)!=ESP_OK){
            fclose(f);
            return ESP_FAIL;
        }
    }
    fclose(f);
    return httpd_resp_send_chunk(req, NULL, 0);
}

static esp_err_t postCardsHandler(httpd_req_t *req){
    char *body = readBody(req);

    cJSON *in = cJSON_Parse(body);
    free(body);
    if(!in){
       return fail(req, "400 Bad Request","invalid json");;
    }

    cJSON *deck= cJSON_GetObjectItemCaseSensitive(in, "deck");
    cJSON *term = cJSON_GetObjectItemCaseSensitive(in, "term");
    cJSON *definition = cJSON_GetObjectItemCaseSensitive(in, "definition");

    if(!cJSON_IsString(deck) || deck->valuestring[0]=='\0' || !cJSON_IsString(term) || !cJSON_IsString(definition)){
        cJSON_Delete(in);
        return fail(req, "400 Bad Request", "missing or invalid fields");
    }

    char path[128];
    snprintf(path, sizeof(path), "%s/%s.json", DECK_DIR, deck->valuestring);

    int r = addCard(path, term->valuestring, definition->valuestring);
    cJSON_Delete(in);

    if(r){
        return success(req, 0);

    }else{
        return fail(req, "400 Bad Request","failed to add card");;
    }
}

static esp_err_t deleteCardsHandler(httpd_req_t *req){
    char deck[67], index_str[16], query[128], path[128];

    httpd_req_get_url_query_str(req, query, sizeof(query));
    httpd_query_key_value(query, "deck", deck, sizeof(deck));
    httpd_query_key_value(query, "index", index_str, sizeof(index_str));

    int idx=atoi(index_str);
    snprintf(path, sizeof(path), "%s/%s.json", DECK_DIR, deck);

    if(deleteCardByIndex(path, idx)){
        return success(req, 0);
    }else{
        return fail(req, "404 Not Found","card or deck not found");;
    }
}

void serverInit(void){
    httpd_config_t server_config = HTTPD_DEFAULT_CONFIG();

    if (httpd_start(&server_handle, &server_config) != ESP_OK) {
        printf("HTTP server failed to start\n");
        server_handle = NULL;
        return;
    }
    for (size_t i = 0; i < sizeof(uris) / sizeof(uris[0]); i++) {
        httpd_register_uri_handler(server_handle, &uris[i]);
    }

    printf("Server started\n");
}