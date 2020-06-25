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
#include <lwip/api.h>


#define SSID      "networkESP32"
#define PASSWORD      "azerty1234"

const static char http_html_hdr[] = "HTTP/1.1 200 OK\nContent-type: text/html\n\n";
const static char http_css_hdr[] = "HTTP/1.1 200 OK\nContent-type: text/css\n\n";
const static char http_index_hml[] = "<!DOCTYPE html><html lang=\"fr\"><head>    <title> Serveur Web ESP32 </title>    <meta name=\"Web Server ESP32 for benchmark\" charset=\"UTF-8\">    <link rel=\"stylesheet\" href=\"style.css\"></head><body>    <h1>ESP32</h1></body></html>";
const static char http_style_css[] = "body{    background-color:lightblue;}h1{    color: black;    text-align: center;}";


const gpio_num_t led = GPIO_NUM_2;
static EventGroupHandle_t wifi_event_group;
const int CONNECTED_BIT = BIT0;

static void http_server_netconn_serve(void *param) {

    struct netconn *conn = (struct netconn *)param;

	struct netbuf *inbuf;
	char *buf;
	u16_t buflen;
	err_t err;

	err = netconn_recv(conn, &inbuf);

	if (err == ERR_OK) {
	  
		netbuf_data(inbuf, (void**)&buf, &buflen);
		
		// extract the first line, with the request
		char *first_line = strtok(buf, "\n");
		
		if(first_line) {
			
			// default page
			if(strstr(first_line, "GET / ")) {
                printf("Sending HTML page...\n");
				netconn_write(conn, http_html_hdr, sizeof(http_html_hdr) - 1, NETCONN_NOCOPY);
				netconn_write(conn, http_index_hml, sizeof(http_index_hml) - 1, NETCONN_NOCOPY);
			}
			
			// ON page
			else if(strstr(first_line, "GET /style.css ")) {
				printf("Sending CSS page...\n");
				netconn_write(conn, http_css_hdr, sizeof(http_css_hdr) - 1, NETCONN_NOCOPY);
				netconn_write(conn, http_style_css, sizeof(http_style_css) - 1, NETCONN_NOCOPY);
			}
			
			else printf("Unkown request: %s\n", first_line);
		}
		else printf("Unkown request\n");
	}
	
	// close the connection and free the buffer
	netconn_close(conn);
	netbuf_delete(inbuf);

    netconn_delete(conn);

    gpio_set_level(led, 1);
    vTaskDelay(100 / portTICK_PERIOD_MS);
    gpio_set_level(led, 0);

    vTaskDelete(NULL);
}

static void http_server(void *pvParameters) {
	
	struct netconn *conn;
	err_t err;
	conn = netconn_new(NETCONN_TCP);
	netconn_bind(conn, NULL, 80);
	netconn_listen(conn);
	printf("HTTP Server listening...\n");
	do {
        static struct netconn *newconn;

		err = netconn_accept(conn, &newconn);
		printf("New client connected\n");
		if (err == ERR_OK)
			xTaskCreate(&http_server_netconn_serve, "http_server_netconn_serve", 2048, (void *)newconn, 2, NULL);
		vTaskDelay(1); //allows task to be pre-empted
	} while(err == ERR_OK);
    printf("SERVOR ERROR !!\n");
	netconn_close(conn);
	netconn_delete(conn);
	printf("\n");
}



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

void app_main() {

    esp_vfs_spiffs_conf_t conf = {
      .base_path = "/spiffs",
      .partition_label = NULL,
      .max_files = 5,
      .format_if_mount_failed = false
    };

    esp_err_t err = esp_vfs_spiffs_register(&conf);
    if(err != ESP_OK) {
        printf("Erreur montage SPIFFS ! %d\n", err);
    } else {
        size_t totalSize;
        size_t used;
        if(esp_spiffs_info(NULL, &totalSize, &used) == ESP_OK) {
            printf("SPIFFS total size : %d\n", totalSize);
            printf("SPIFFS used : %d\n", used);
        }
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
    xTaskCreate(&http_server, "http_server", 2048, NULL, 5, NULL);
}