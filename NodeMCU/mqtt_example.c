/*
 * Copyright (C) 2015-2018 Alibaba Group Holding Limited
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "iot_import.h"
#include "iot_export.h"
#include "app_entry.h"
#include "esp_system.h"
#include "esp_log.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

#include "cJSON.h"
#include <sys/socket.h>
#include <netdb.h>

// #define PRODUCT_KEY             NULL
#define PRODUCT_KEY             "a1gXXXXXqNV"
#define PRODUCT_SECRET          "a4dtXXXXXXXXdlwP"
#define DEVICE_NAME             "NodeMCU"
#define DEVICE_SECRET           "9L4CSU2XXXXXXXXXXnMeYo7bZWIApqu0"
#define PC_MAC_ADDR             "\x30\x9c\x23\xe3\xf2\x5f"
      
/* These are pre-defined topics */
#define TOPIC_UPDATE            "/"PRODUCT_KEY"/"DEVICE_NAME"/update"
#define TOPIC_ERROR             "/"PRODUCT_KEY"/"DEVICE_NAME"/update/error"
#define TOPIC_GET               "/"PRODUCT_KEY"/"DEVICE_NAME"/get"
#define TOPIC_DATA               "/"PRODUCT_KEY"/"DEVICE_NAME"/data"

#define MQTT_MSGLEN             (1024)

#define EXAMPLE_TRACE(fmt, ...)  \
    do { \
        HAL_Printf("%s|%03d :: ", __func__, __LINE__); \
        HAL_Printf(fmt, ##__VA_ARGS__); \
        HAL_Printf("%s", "\r\n"); \
    } while(0)

static int      user_argc;
static char   **user_argv;
static int  doing_power_on = 0;

static const char *TAG = "MQTT";

void event_handle(void *pcontext, void *pclient, iotx_mqtt_event_msg_pt msg)
{
    uintptr_t packet_id = (uintptr_t)msg->msg;
    iotx_mqtt_topic_info_pt topic_info = (iotx_mqtt_topic_info_pt)msg->msg;

    switch (msg->event_type) {
        case IOTX_MQTT_EVENT_UNDEF:
            EXAMPLE_TRACE("undefined event occur.");
            break;

        case IOTX_MQTT_EVENT_DISCONNECT:
            EXAMPLE_TRACE("MQTT disconnect.");
            break;

        case IOTX_MQTT_EVENT_RECONNECT:
            EXAMPLE_TRACE("MQTT reconnect.");
            break;

        case IOTX_MQTT_EVENT_SUBCRIBE_SUCCESS:
            EXAMPLE_TRACE("subscribe success, packet-id=%u", (unsigned int)packet_id);
            break;

        case IOTX_MQTT_EVENT_SUBCRIBE_TIMEOUT:
            EXAMPLE_TRACE("subscribe wait ack timeout, packet-id=%u", (unsigned int)packet_id);
            break;

        case IOTX_MQTT_EVENT_SUBCRIBE_NACK:
            EXAMPLE_TRACE("subscribe nack, packet-id=%u", (unsigned int)packet_id);
            break;

        case IOTX_MQTT_EVENT_UNSUBCRIBE_SUCCESS:
            EXAMPLE_TRACE("unsubscribe success, packet-id=%u", (unsigned int)packet_id);
            break;

        case IOTX_MQTT_EVENT_UNSUBCRIBE_TIMEOUT:
            EXAMPLE_TRACE("unsubscribe timeout, packet-id=%u", (unsigned int)packet_id);
            break;

        case IOTX_MQTT_EVENT_UNSUBCRIBE_NACK:
            EXAMPLE_TRACE("unsubscribe nack, packet-id=%u", (unsigned int)packet_id);
            break;

        case IOTX_MQTT_EVENT_PUBLISH_SUCCESS:
            EXAMPLE_TRACE("publish success, packet-id=%u", (unsigned int)packet_id);
            break;

        case IOTX_MQTT_EVENT_PUBLISH_TIMEOUT:
            EXAMPLE_TRACE("publish timeout, packet-id=%u", (unsigned int)packet_id);
            break;

        case IOTX_MQTT_EVENT_PUBLISH_NACK:
            EXAMPLE_TRACE("publish nack, packet-id=%u", (unsigned int)packet_id);
            break;

        case IOTX_MQTT_EVENT_PUBLISH_RECEIVED:
            EXAMPLE_TRACE("topic message arrived but without any related handle: topic=%.*s, topic_msg=%.*s",
                          topic_info->topic_len,
                          topic_info->ptopic,
                          topic_info->payload_len,
                          topic_info->payload);
            break;

        case IOTX_MQTT_EVENT_BUFFER_OVERFLOW:
            EXAMPLE_TRACE("buffer overflow, %s", msg->msg);
            break;

        default:
            EXAMPLE_TRACE("Should NOT arrive here.");
            break;
    }
}

void send_magic_package(void *paras){

    doing_power_on = 1;
    struct sockaddr_in server_addr;
    socklen_t sin_size = sizeof(server_addr);
    int send_sock = -1;
    int optval = 1;
    int sendlen;
    uint8_t packet_count = 1;
    int err;
    int ret;
    int idx = 0;
    char send_buf[102+1];

    printf("init send buf\n");
    memset(send_buf,0,sizeof(send_buf));
    memset(send_buf,0xff,6);
    for(idx = 0; idx < 16; idx++){
        memcpy(send_buf+6*(idx+1),PC_MAC_ADDR,6);
    }
    printf("init send buf Donw\n");

    bzero(&server_addr, sizeof(struct sockaddr_in));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = 0;
    server_addr.sin_port = htons(10);
    
    /* Create UDP socket. */
    send_sock = socket(AF_INET, SOCK_DGRAM, 0);
    if ((send_sock < LWIP_SOCKET_OFFSET) || (send_sock > (FD_SETSIZE - 1))) {
        EXAMPLE_TRACE(TAG,  "Creat udp socket failed");
        // goto _end;	
        return;
    }

    setsockopt(send_sock, SOL_SOCKET, SO_BROADCAST | SO_REUSEADDR, &optval, sizeof(int));
    char data = 0;
    struct sockaddr_in local_addr, from;
    socklen_t sockadd_len = sizeof(struct sockaddr);

    bzero(&local_addr, sizeof(struct sockaddr_in));
    bzero(&from, sizeof(struct sockaddr_in));
    local_addr.sin_family = AF_INET;
    local_addr.sin_addr.s_addr = INADDR_ANY;
    local_addr.sin_port = htons(9);

    bind(send_sock, (struct sockaddr *)&local_addr, sockadd_len);

    for(idx = 0; idx < 2; idx++){
        printf("send magice package  --- %d\n",idx);
        sendlen = sendto(send_sock, send_buf, sizeof(send_buf), 0, (struct sockaddr*) &server_addr, sin_size);
        HAL_SleepMs(300);
    }
    close(send_sock);
    printf("send magice package  Done\n");
     doing_power_on = 0;
    // vTaskDelete(NULL);   
   
}

static void _demo_message_arrive(void *pcontext, void *pclient, iotx_mqtt_event_msg_pt msg)
{
    char payload[128];
    iotx_mqtt_topic_info_pt     ptopic_info = (iotx_mqtt_topic_info_pt) msg->msg;

    switch (msg->event_type) {
        case IOTX_MQTT_EVENT_PUBLISH_RECEIVED:
            /* print topic name and topic message */
            EXAMPLE_TRACE("----");
            EXAMPLE_TRACE("PacketId: %d", ptopic_info->packet_id);
            EXAMPLE_TRACE("Topic: '%.*s' (Length: %d)",
                          ptopic_info->topic_len,
                          ptopic_info->ptopic,
                          ptopic_info->topic_len);
            EXAMPLE_TRACE("Payload: '%.*s' (Length: %d)",
                          ptopic_info->payload_len,
                          ptopic_info->payload,
                          ptopic_info->payload_len);
            EXAMPLE_TRACE("----");

            strncpy(payload,ptopic_info->payload,ptopic_info->payload_len);
            cJSON *action = cJSON_Parse(payload);
            if(action == NULL){
                printf("cJSON_Parse error --return \n");
                break;
            }
            cJSON *tmpObject = cJSON_GetObjectItem(action, "power_on");
            if(tmpObject == NULL){
                printf("get power_on state error --return \n");
                cJSON_Delete(action);
                break;
            }
            printf("is dong send package:%d\n",doing_power_on);
            if(tmpObject->valueint == 1 && doing_power_on == 0){
                EXAMPLE_TRACE("power on message !!!!!!!!!!!!!!!!!!!!!!!!!!!\n");  
                // xTaskCreate(send_magic_package, "send_magic_package", 10240, NULL, 5, NULL);
                send_magic_package(NULL);
            }

            cJSON_Delete(tmpObject);
            cJSON_Delete(action);

            // char amount[13] = "";
            // memcpy(amount, tmpObject->valuestring, strlen(tmpObject->valuestring));
            
            break;
        default:
            EXAMPLE_TRACE("Should NOT arrive here.");
            break;
    }
}

int mqtt_client(void)
{
    int rc, msg_len, cnt = 0;
    void *pclient;
    iotx_conn_info_pt pconn_info;
    iotx_mqtt_param_t mqtt_params;
    iotx_mqtt_topic_info_t topic_msg;
    char msg_pub[128];

    /* Device AUTH */
    if (0 != IOT_SetupConnInfo(PRODUCT_KEY, DEVICE_NAME, DEVICE_SECRET, (void **)&pconn_info)) {
        EXAMPLE_TRACE("AUTH request failed!");
        return -1;
    }

    /* Initialize MQTT parameter */
    memset(&mqtt_params, 0x0, sizeof(mqtt_params));

    mqtt_params.port = pconn_info->port;
    mqtt_params.host = pconn_info->host_name;
    mqtt_params.client_id = pconn_info->client_id;
    mqtt_params.username = pconn_info->username;
    mqtt_params.password = pconn_info->password;
    mqtt_params.pub_key = pconn_info->pub_key;

    mqtt_params.request_timeout_ms = 2000;
    mqtt_params.clean_session = 0;
    mqtt_params.keepalive_interval_ms = 20000;
    mqtt_params.read_buf_size = MQTT_MSGLEN;
    mqtt_params.write_buf_size = MQTT_MSGLEN;

    mqtt_params.handle_event.h_fp = event_handle;
    mqtt_params.handle_event.pcontext = NULL;


    /* Construct a MQTT client with specify parameter */
    pclient = IOT_MQTT_Construct(&mqtt_params);
    if (NULL == pclient) {
        EXAMPLE_TRACE("MQTT construct failed");
        return -1;
    }

    /* Subscribe the specific topic */
    rc = IOT_MQTT_Subscribe(pclient, TOPIC_UPDATE, IOTX_MQTT_QOS0, _demo_message_arrive, NULL);
    if (rc < 0) {
        IOT_MQTT_Destroy(&pclient);
        EXAMPLE_TRACE("IOT_MQTT_Subscribe() failed, rc = %d", rc);
        return -1;
    }

   

    do {
        /* Generate topic message */
        cnt = 1;
       
        // /* handle the MQTT packet received from TCP or SSL connection */
        IOT_MQTT_Yield(pclient, 2000);

        /* infinite loop if running with 'loop' argument */
        if (user_argc >= 2 && !strcmp("loop", user_argv[1])) {
            // HAL_SleepMs(2000);
            // cnt = 0;
        }

        if(IOT_MQTT_CheckStateNormal(pclient) != 1){
            break;
        }

        ESP_LOGI(TAG, "min:%u heap:%u", esp_get_minimum_free_heap_size(), esp_get_free_heap_size());
    } while (cnt);

    // IOT_MQTT_Yield(pclient, 200);

    IOT_MQTT_Unsubscribe(pclient, TOPIC_UPDATE);

    IOT_MQTT_Yield(pclient, 200);

    IOT_MQTT_Destroy(&pclient);

    return 0;
}

int linkkit_main(void *paras)
{
    IOT_SetLogLevel(IOT_LOG_DEBUG);

    user_argc = 0;
    user_argv = NULL;

    if (paras != NULL) {
        app_main_paras_t *p = (app_main_paras_t *)paras;
        user_argc = p->argc;
        user_argv = p->argv;
    }

    HAL_SetProductKey(PRODUCT_KEY);
    HAL_SetDeviceName(DEVICE_NAME);
    HAL_SetDeviceSecret(DEVICE_SECRET);
    HAL_SetProductSecret(PRODUCT_SECRET);
    /* Choose Login Server */
    int domain_type = IOTX_CLOUD_REGION_SHANGHAI;
    IOT_Ioctl(IOTX_IOCTL_SET_DOMAIN, (void *)&domain_type);

    /* Choose Login  Method */
    int dynamic_register = 0;
    IOT_Ioctl(IOTX_IOCTL_SET_DYNAMIC_REGISTER, (void *)&dynamic_register);

    mqtt_client();
    IOT_DumpMemoryStats(IOT_LOG_DEBUG);
    IOT_SetLogLevel(IOT_LOG_NONE);

    EXAMPLE_TRACE("out of sample!");

    return 0;
}
