#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_log.h>
#include <driver/gpio.h>
#include <esp_http_server.h>
#include <esp_wifi_types.h>
#include <esp_wifi.h>
#include <esp_spiffs.h>
#include <esp_event.h>
#include <esp_event_base.h>
#include <esp_system.h>
#include <tcpip_adapter.h>
#include <esp_log.h>

const gpio_num_t led = GPIO_NUM_2;


esp_err_t get_index_handler(httpd_req_t *req)
{
    /* Send a simple response */

    char buf[500];
    FILE *f = fopen("/spiffs/index.html", "r");
    fread(buf, 50, 10, f);
    buf[500] = '\0';

    const char *resp = buf;
    httpd_resp_send(req, resp, strlen(resp));
    return ESP_OK;
}

esp_err_t get_style_handler(httpd_req_t *req)
{
    /* Send a simple response */
    char buf[500];
    FILE *f = fopen("/spiffs/style.css", "r");
    fread(buf, 50, 10, f);
    buf[500] = '\0';

    const char *resp = buf;
    httpd_resp_send(req, resp, strlen(resp));
    return ESP_OK;
}

httpd_uri_t index_get = {
    .uri      = "/",
    .method   = HTTP_GET,
    .handler  = get_index_handler,
    .user_ctx = NULL
};

httpd_uri_t style_get = {
    .uri      = "/style.css",
    .method   = HTTP_GET,
    .handler  = get_style_handler,
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
    esp_vfs_spiffs_register(&conf);

    /** LED init **/
    gpio_pad_select_gpio(led);
    gpio_set_direction(led, GPIO_MODE_OUTPUT);

    /** Web server **/
    start_webserver();
}