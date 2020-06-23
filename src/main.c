#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_log.h>
#include <driver/gpio.h>

const gpio_num_t led = GPIO_NUM_2;

void app_main() {



    /** LED Clignotement **/
    gpio_pad_select_gpio(led);

    gpio_set_direction(led, GPIO_MODE_OUTPUT);

    bool on = false;
    while (1) {
        vTaskDelay(500 / portTICK_PERIOD_MS);
        printf("Changement !\n");
        gpio_set_level(led, !on);
        on = !on;
    }
}