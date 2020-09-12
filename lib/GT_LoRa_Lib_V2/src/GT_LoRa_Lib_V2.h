#ifndef GT_LoRa_Lib_V2_h 
#define GT_LoRa_Lib_V2_h

#include <ArduinoJson.h>
#include "LoRa.h"
#include "LinkedList.h"
#include "TimeLib.h"

#define LORA_PIN_SCK                5                 // GPIO5  -- SX1278's SCK
#define LORA_PIN_MISO               19                // GPIO19 -- SX1278's MISO
#define LORA_PIN_MOSI               27                // GPIO27 -- SX1278's MOSI
#define LORA_PIN_SS                 18                // GPIO18 -- SX1278's CS
#define LORA_PIN_RST                14                // GPIO14 -- SX1278's RESET
#define LORA_PIN_DIO0               26                // GPIO26 -- SX1278's IRQ(Interrupt Request)
#define LORA_BAND                   915E6             //433E6  //you can set band here directly,e.g. 868E6,915E6
#define LORA_SYN_WORD               0x00

#define TIMEOUT                     10000
#define BROADCAST                   "FFFFFF"

#define GATEWAY                     0
#define NODE                        1

#define BEACON                      0
#define JOIN                        1
#define JOIN_ACK                    2
#define REQUEST                     3
#define REPLAY                      4

#define LORA_TASK_DELAY_MS          10
#define REQUEST_INTERVAL            30000
#define MAX_NUMBER_NO_ACK           10

typedef struct {
  String sourcer_device_id;
  String target_device_id;
  String message_id;
  uint8_t message_type;
  String neighbor_id_list;
  uint8_t hops;
  String payload;
} lora_send_t;

typedef struct {
  String sourcer_device_id;
  String target_device_id;
  String message_id;
  uint8_t message_type;
  String neighbor_id_list;
  uint8_t hops;
  float SNR;
  int RSSI;
  String payload;
} lora_recv_t;

typedef struct {
  float SNR;
  int16_t RSSI;
  String payload;
} lora_temp_t;

typedef struct {
  String device_id;
  // String ubidots_id;         // implementar para utilizar a plataforma Ubidots
  // String ubidots_token;
  float SNR;
  int16_t RSSI;
  long timeout;
  uint8_t counter_no_ack;
} device_node_t;

class LoRaGatewayLib
{
  public:
    LoRaGatewayLib();

    boolean begin(String id);
    void run(void);                                   // network
    void add_send_message(lora_send_t msg);
    void add_recv_message(lora_recv_t msg);
    String get_next_send_message(void);
    lora_recv_t get_next_recv_message(void);
    boolean is_send_list_empty(void);
    boolean is_recv_list_empty(void);
    boolean is_node_list_empty(void);
    boolean send_message(String strSend);

    boolean add_node(lora_recv_t msg_recv);           // network
    // void remove_node(uint8_t index);
    uint8_t get_size_list_node(void);                 // Network
    uint8_t get_number_no_ack(uint8_t index);
    void reset_number_no_ack(String device_id);
    boolean request_new_record(uint8_t index);        // Network

  private:
    static void onReceive(int packetSize);
    void SetRxMode(void);
    void SetTxMode(void);
    String encodeJSON_recv(lora_recv_t msg);
    String encodeJSON_send(lora_send_t msg);
    static lora_recv_t decodeJSON_recv(String strJSON);
    static lora_send_t decodeJSON_send(String strJSON);

    uint8_t synWord = LORA_SYN_WORD;

};

#endif  // GT_LoRa_Lib_V2_h