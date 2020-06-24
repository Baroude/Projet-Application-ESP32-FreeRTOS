#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <driver/gpio.h>
#include <esp_http_server.h>
#include <esp_wifi_types.h>
#include <esp_wifi.h>
#include <esp_spiffs.h>
#include <esp_event.h>
#include <esp_event_base.h>
#include <tcpip_adapter.h>
#include <esp_log.h>
#include <esp_event_loop.h>
#include <freertos/event_groups.h>
#include <esp_system.h>
#include <nvs_flash.h>
#include <dirent.h>


#define SSID      "networkESP32"
#define PASSWORD      "azerty1234"

const gpio_num_t led = GPIO_NUM_2;
static EventGroupHandle_t wifi_event_group;
const int CONNECTED_BIT = BIT0;

void wifi_connect(){
    wifi_config_t cfg = {
        .sta = {
            .ssid = SSID,
            .password = PASSWORD,
        },
    };
    ESP_ERROR_CHECK( esp_wifi_disconnect() );
    ESP_ERROR_CHECK( esp_wifi_set_config(ESP_IF_WIFI_STA, &cfg) );
    ESP_ERROR_CHECK( esp_wifi_connect() );
}

static esp_err_t event_handler(void *ctx, system_event_t *event)
{
    switch(event->event_id) {
    case SYSTEM_EVENT_STA_START:
        wifi_connect();
        break;
    case SYSTEM_EVENT_STA_GOT_IP:
        xEventGroupSetBits(wifi_event_group, CONNECTED_BIT);
        break;
    case SYSTEM_EVENT_STA_DISCONNECTED:
        esp_wifi_connect();
        xEventGroupClearBits(wifi_event_group, CONNECTED_BIT);
        break;
    default:
        break;
    }
    return ESP_OK;
}

static void initialise_wifi(void)
{
    esp_log_level_set("wifi", ESP_LOG_NONE); // disable wifi driver logging
    tcpip_adapter_init();
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK( esp_wifi_init(&cfg) );
    ESP_ERROR_CHECK( esp_wifi_set_mode(WIFI_MODE_STA) );
    ESP_ERROR_CHECK( esp_wifi_start() );
    esp_err_t ret = tcpip_adapter_set_hostname(TCPIP_ADAPTER_IF_STA ,"PROJET");
    if(ret != ESP_OK ){
      printf("failed to set hostname:%d\n",ret);  
    }
}

void printWiFiIP(void *pvParam){
    printf("printWiFiIP task started \n");
    while(1){
        xEventGroupWaitBits(wifi_event_group,CONNECTED_BIT,true,true,portMAX_DELAY);
        tcpip_adapter_ip_info_t ip_info;
	    ESP_ERROR_CHECK(tcpip_adapter_get_ip_info(TCPIP_ADAPTER_IF_STA, &ip_info));
	    printf("IP :  %s\n", ip4addr_ntoa(&ip_info.ip));
    }
}

esp_err_t get_index_handler(httpd_req_t *req)
{
    DIR *dir = opendir("/spiffs");
    if(dir == NULL) {
        printf("Erreur d'ouverture de repertoire !\n");
        return;
    }
    
    /**
    struct dirent *dirent = NULL;

    printf("Liste des fichiers\n");
    while((dirent = readdir(dir)) != NULL){
        printf("Fichier : %s\n", dirent->d_name);
    }


    char buf[500];
    FILE *f = fopen("/index.html", "r");
    if(f == NULL) {
        printf("Erreur lors de l'ouverture du fichier index.html !");
        return -1;
    }
    fgets(buf, 500, f); **/

    const char *resp = "Les fichiers ne fonctionnent pas ! Pour le moment...";
    httpd_resp_send(req, resp, strlen(resp));

    gpio_set_level(led, 1);
    vTaskDelay(200);
    gpio_set_level(led, 0);

    return ESP_OK;
}

esp_err_t get_style_handler(httpd_req_t *req)
{
   /* Send a simple response */
    char buf[500];
    FILE *f = fopen("/style.css", "r");
    if(f == NULL) {
        printf("Erreur lors de l'ouverture du fichier style.css !");
        return;
    }
    fgets(buf, 500, f);

    const char *resp = buf;
    httpd_resp_send(req, resp, strlen(resp));
    return ESP_OK;
}

httpd_uri_t index_get = {
    .uri      = "/",
    .method   = HTTP_GET,
    .handler  = &get_index_handler,
    .user_ctx = NULL
};

httpd_uri_t style_get = {
    .uri      = "/style.css",
    .method   = HTTP_GET,
    .handler  = &get_style_handler,
    .user_ctx = NULL
};

httpd_handle_t start_webserver(void)
{
    /* Generate default configuration */
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();

    /* Empty handle to esp_http_server */
    httpd_handle_t server = NULL;

    /* Start the httpd server */
    if (httpd_start(&server, &config) == ESP_OK) {
        /* Register URI handlers */
        httpd_register_uri_handler(server, &index_get);
        httpd_register_uri_handler(server, &style_get);
    }
    /* If server failed to start, handle will be NULL */
    return server;
}

void app_main() {

    esp_vfs_spiffs_conf_t conf = {
      .base_path = "/spiffs",
      .partition_label = NULL,
      .max_files = 5,
      .format_if_mount_failed = true
    };

    esp_err_t err = esp_vfs_spiffs_register(&conf);
    if(err != ESP_OK) {
        printf("Erreur montage SPIFFS ! %d\n", err);
    }

    ESP_ERROR_CHECK( esp_event_loop_init(event_handler, NULL) );
    wifi_event_group = xEventGroupCreate();
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    initialise_wifi();
    xTaskCreate(&printWiFiIP,"printWiFiIP",2048,NULL,5,NULL);

    /** LED init **/
    gpio_pad_select_gpio(led);
    gpio_set_direction(led, GPIO_MODE_OUTPUT);

    /** Web server **/
    if(start_webserver() != NULL)
        printf("Web server started !\n");
    else
        printf("Failed to start web server !\n");
}