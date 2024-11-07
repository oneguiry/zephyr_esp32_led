#include <zephyr.h>
#include <device.h>
#include <drivers/gpio.h>
#include <net/socket.h>
#include <logging/log.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <zephyr/net/net_ip.h>
#include <zephyr/net/http_server.h>

LOG_MODULE_REGISTER(main, LOG_LEVEL_INF);

#define LED_PIN 13 

const struct device *led_dev;
static struct sockaddr_in server_addr;

const char *http_response_html = 
    "<html>"
    "<head><title>LED Control</title></head>"
    "<body>"
    "<h1>ESP32 LED Control</h1>"
    "<form action='/led/on' method='GET'><button type='submit'>Turn ON</button></form><br>"
    "<form action='/led/off' method='GET'><button type='submit'>Turn OFF</button></form>"
    "</body>"
    "</html>";

// Обработчик HTTP-запросов
void http_server_handler(struct http_request *req, struct http_response *res)
{

    if (strcmp(req->uri, "/") == 0) {
        res->status = HTTP_STATUS_OK;
        res->body = http_response_html;
        return;
    }

    if (strcmp(req->method, "GET") == 0) {
        if (strstr(req->uri, "/led/on") != NULL) {
            gpio_pin_set(led_dev, LED_PIN, 1); // Включаем LED
            res->status = HTTP_STATUS_OK;
            res->body = "LED turned ON. <a href='/'>Back</a>";
        } else if (strstr(req->uri, "/led/off") != NULL) {
            gpio_pin_set(led_dev, LED_PIN, 0); // Выключаем LED
            res->status = HTTP_STATUS_OK;
            res->body = "LED turned OFF. <a href='/'>Back</a>";
        } else {
            res->status = HTTP_STATUS_NOT_FOUND;
            res->body = "Invalid URL";
        }
    }
}

void start_http_server(void)
{
    int ret = http_server_init(http_server_handler);
    if (ret < 0) {
        LOG_ERR("Failed to initialize HTTP server");
    } else {
        LOG_INF("HTTP server started");
    }
}

void main(void)
{
    led_dev = device_get_binding(DT_LABEL(DT_NODELABEL(gpio0)));
    if (led_dev == NULL) {
        LOG_ERR("Cannot find GPIO device");
        return;
    }

    gpio_pin_configure(led_dev, LED_PIN, GPIO_OUTPUT_ACTIVE);

    LOG_INF("Starting Wi-Fi...");
    int wifi_status = wifi_connect();
    if (wifi_status != 0) {
        LOG_ERR("Wi-Fi connection failed: %d", wifi_status);
        return;
    }

    start_http_server();

    LOG_INF("Wi-Fi connected, server running...");
}
