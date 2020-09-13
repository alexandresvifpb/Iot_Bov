#ifndef GT_WiFi_Lib_V2_h
#define GT_WiFi_Lib_V2_h

#include "Arduino.h"
#include <WiFi.h>           // Wifi configuration library
#include <WiFiUdp.h>        // Library required for network communication (OTA)
#include <ArduinoOTA.h>     // ArduinoOTA Library (OTA)
#include <PubSubClient.h>   
#include <ArduinoJson.h>
#include "LinkedList.h"

// WiFi
#define WIFI_WEB_SERVER_PORT    23                    // ServerPort
#define WIFI_STA_SSID           "brisa-594111"        //  "AMD"           "brisa-594111"
#define WIFI_STA_PASS           "gbalklxz"            //  "amd12345678"   "gbalklxz"
#define WIFI_HOSTNAME           "Node-"               //  "Node-25AA24"

#define WIFI_TASK_DELAY_MS      100

// MQTT
#define MQTT_TOPIC_SUBSCRIBE   "/GATEWAY/"
#define MQTT_TOPIC_PUBLISH     "/NODES/"
#define MQTT_SERVER            "test.mosquitto.org"       // "192.168.1.15"      //  "192.168.10.50" "10.0.5.50" "192.168.1.2" "ec2-52-14-53-218.us-east-2.compute.amazonaws.com"  "192.168.1.7" "150.165.82.50"
// #define MQTT_SERVER             "things.ubidots.com"
#define MQTT_PORT               1883                //  1883 995
#define MQTT_TOKEN              ""

// Implemntar se for utilizar a plataforma Ubidots
// #define MQTT_UBIDOTS_API_LABEL  "node-b94288"
// #define MQTT_UBIDOTS_ID         "5f45180f1d84724b89b56319"
// #define MQTT_UBIDOTS_TOKEN      "BBFF-jmgM7LpxCufFGbQseCSDSkD78yL4TL"

#define MQTT_TASK_DELAY_MS      200

typedef struct {
    String id;
    uint8_t type;
    String payload;
} mqtt_t;

class WiFi_GateWay {

  public:
    WiFi_GateWay();

    // WiFi
    bool begin(String device_id);
    bool connect_wifi_STD_mode(void);
    bool active_webserver(void);
    bool disable_wifi(void);
    bool is_wifi_connected(void);

    // OTA
    bool active_OTA(void);

    // WEBSERVER
    void listen_for_clients_WS(void);
    uint8_t number_connected_clients_WS(void);
    int available_recv_data_client_WS(void);
    String recv_message_client_WS(void);
    void send_message_client_WS(String message);

  private:

};

class MQTT_Client {

  public:
    MQTT_Client();

    bool begin(void);
    void run(void);
    bool active_MQTT(void);
    boolean reconnect_MQTT(void);
    void add_send_message(mqtt_t message);
    String get_next_send_message(void);
    mqtt_t get_next_recv_message(void);
    bool is_send_list_empty(void);
    bool is_recv_list_empty(void);
    void send_message(String message);

  private:

};

#endif  // GT_WiFi_Lib_V2_h