#include <Arduino.h>
#include "esp_task.h"
#include "esp_task_wdt.h"
                                        // https://github.com/alexandresvifpb/Iot_Bov.git
                                        // D:\Users\alexa\OneDrive\doutorado\2020\prototipos\firmware\tests\Test_Gateway_LoRa_TTGO-LoRA32-V1_V02
#define MAIN_MESSAGE_INITIAL            ("D:\\Users\\alexa\\OneDrive\\doutorado\\2020\\prototipos\\firmware\\tests\\Test_Gateway_LoRa_TTGO-LoRa32-V1_V02.05")          // localizacao do projeto
#define MAIN_DEBUG                      (true)          // Variable that enables (1) or disables (0) the sending of data by serial to debug the program
#define MAIN_CORE_0                     (0)             // Defined the core to be used
#define MAIN_CORE_1                     (1)             
#define MAIN_WATCHDOC_TIME_OUT          (600)           // Watchdog wait time to restart the process in seconds

TaskHandle_t TaskIdWiFiOTA;
TaskHandle_t TaskIdLoRa;
TaskHandle_t TaskIdMQTT;

void TaskWiFiOTA( void * pvParameters );
void TaskLoRa( void * pvParameters );
void TaskMQTT( void * pvParameters );

//===========================================
//  ESP32 Util
#include "ESP32UtilLibV01.h"

ESP32UtilLib module;
String device_id = module.get_MAC();

//===========================================
//  Library to implement functionality using the ESP32 WiFi module
#include "GT_WiFi_Lib_V2.h"

// WiFi
WiFi_GateWay wifi_client;
bool wifi_connected = false;

// MQTT
MQTT_Client mqtt_client;
bool mqtt_connected = false;

//===========================================
//  LoRa SX1276/8
#include "GT_LoRa_Lib_V2.h"

LoRaGatewayLib lora;
boolean lora_enable = false;
boolean device_connected = false;
uint8_t device_type = GATEWAY;
uint8_t number_nodes = 0;        // Network

void setup() {
  // Serial Initialization
  Serial.begin(115200);
  Serial.println();
  Serial.println(MAIN_MESSAGE_INITIAL);
  Serial.println();

  // Serial.println(__LINE__);

    // Creates a Task that will execute the TaskLoRa () function, with priority 1 and running in the nucleus 1
  xTaskCreatePinnedToCore(
                TaskWiFiOTA,    // Function with the code that implements the Task
                "TaskWiFiOTA",   // Task name
                2048,           // Stack size to be allocated when creating the Task
                NULL,           // Task input parameters    4096
                1,              // Task priority
                &TaskIdWiFiOTA, // Reference to accompany the Task
                MAIN_CORE_0);   // Core on which the Task will run (0 or 1 for ESP32)
  delay(100);                     // Delay for next command

  // Serial.println(__LINE__);

  //Cria uma Task que executará a funcao TaskLoRa(), com prioridade 1 e rodando no nucleo 1
  xTaskCreatePinnedToCore(
                TaskMQTT,       // Funcao com o codigo que implenta a Task
                "TaskMQTT",     // Nome da Task
                2048,          // Tamanho da pilha (stack) a serem alocada na criacao da Task
                NULL,           // Parametros de entrada da Task
                1,              // Prioridade da Task
                &TaskIdMQTT,    // Referencia para acompanhar a Task
                MAIN_CORE_0);        // Nucleo no qual a Task rodara (0 ou 1 para o ESP32)
    delay(100);                     // delay para proximo comando

  // Serial.println(__LINE__);

  // Creates a Task that will execute the TaskLoRa () function, with priority 1 and running in the nucleus 0
  xTaskCreatePinnedToCore(
                  TaskLoRa,       // Function with the code that implements the Task
                  "TaskLoRa",     // Task name
                  2048,           // Stack size to be allocated when creating the Task
                  NULL,           // Task input parameters
                  1,              // Task priority
                  &TaskIdLoRa,    // Reference to accompany the Task
                  MAIN_CORE_1);   // Core on which the Task will run (0 or 1 for ESP32)
  delay(100);                     // Delay for next command

  // Serial.println(__LINE__);

  // Enables the watchdog with a 15-second timeout
  esp_task_wdt_init(MAIN_WATCHDOC_TIME_OUT, true);
}

// Task for the WiFi module
void TaskWiFiOTA( void * pvParameters ) {
  esp_task_wdt_add(NULL);

  // Serial.println(__LINE__);

  if ( wifi_client.begin(device_id) ) {
    wifi_connected = wifi_client.connect_wifi_STD_mode();
  }

  bool wifi_OTA_active = false;

  // Mandatory infinite loop to keep the Task running
  while(true) {

    if ( wifi_connected ) {

      // Serial.println(__LINE__);

      // verifica se o servico OTA estar ativo,
      // se retornar: 
      //            false - tenta ativar o servico 
      //            true - checa o OTA
      if ( !wifi_OTA_active ) {
        // Serial.println(__LINE__);
        wifi_OTA_active = wifi_client.active_OTA();
      } else {
        // Serial.println(__LINE__);
        ArduinoOTA.handle();
      }

      // Verifica se existe alguma cliente solicitando conexao ao WebServer
      wifi_client.listen_for_clients_WS();

      // verifica o numero de clientes conectado ao WebServer
      if ( wifi_client.number_connected_clients_WS() > 0 ) {
        // Serial.println(__LINE__);

        // verifica se chegou dados do cliente remoto
        if ( wifi_client.available_recv_data_client_WS() ) {
          // Serial.println(__LINE__);

          Serial.println( wifi_client.recv_message_client_WS() );     // apenas para teste
          
        }
      }

    } else {
      wifi_connected = wifi_client.connect_wifi_STD_mode();
      wifi_OTA_active = false;
    }
    
    esp_task_wdt_reset();                                           // Reset watchdog counter
    vTaskDelay(pdMS_TO_TICKS(WIFI_TASK_DELAY_MS));       // Pause Tesk and release the nucleus for the next Tesk in the priority queue
  }
}

// Funcao que implementa a Task para o modulo do cliente MQTT
void TaskMQTT( void * pvParameters ) {
  esp_task_wdt_add(NULL);

  Serial.println(__LINE__);

  long active_client_delay = millis();

  // if ( wifi_connected && mqtt_client.begin() ) {
  if ( mqtt_client.begin() ) {

    Serial.println(__LINE__);
  
    mqtt_connected = mqtt_client.active_MQTT();
  }

  // Loop infinito obrigatorio para manter a Task rodando
  while(true) {

    if ( mqtt_connected ) {

      // Executa o client.loop()
      mqtt_client.run();

      // Verifica se existe alguma mensagem na lista de recv FALTA IMPLEMENTAR
      if ( mqtt_client.is_recv_list_empty() ) {

      }

      // Verifica se existe alguma mensagem na lista de envio
      if ( mqtt_client.is_send_list_empty() ) {
        mqtt_client.send_message(mqtt_client.get_next_send_message());
      }

      // Enviar uma mensagem para o broker a cada 60 segundos
      if ( ( millis() - active_client_delay) > MQTT_ACTIVE_DELAY_MS ) {
        active_client_delay = millis();
        mqtt_client.send_message("Gateway " + String(device_id) + " ON and with " + String(number_nodes) + " nodes connected");
      }

    } else {

      delay(1000);
      mqtt_connected = mqtt_client.active_MQTT();

    }    

    esp_task_wdt_reset();                               // Reseta o contador do watchdog
    vTaskDelay(pdMS_TO_TICKS(MQTT_TASK_DELAY_MS));      // Pausa a Tesk e libera o nucleo para a proximo Tesk na fila de prioridade
  }
}

// Task for the LoRa radio module
void TaskLoRa( void * pvParameters ) {
  esp_task_wdt_add(NULL);

  // Serial.println(__LINE__);

  //===========================================
  // LoRa
  lora_enable = lora.begin(device_id);
  // ( !lora_enable ) ? Serial.println("Error initializing the LoRa module") : Serial.println("LoRa module OK!");
  long last_time = 0;
  long last_time_request = 0;       // Network
  // uint8_t node_index = 0;           // Network
  // uint8_t number_nodes = 0;        // Network

  // Mandatory infinite loop to keep the Task running
  while( true ) {

    lora.run();

    // Gera uma mensagem na Serial a cada 2 segundo indicando que a Taks LoRa estar ativa
    if ( millis() > ( last_time + 2000 ) ) {
      String message_number_node_connected = "TASK_GATEWAY_LORA: ";
      message_number_node_connected += lora.get_size_list_node();
      message_number_node_connected += " node connected";
      Serial.println(message_number_node_connected);
      wifi_client.send_message_client_WS(message_number_node_connected);
      last_time = millis();

      // // colocado para depuracao, retirar depois 
      // Serial.print("lora.is_recv_list_empty(): ");
      // Serial.println(lora.is_recv_list_empty());

    }

    // vefica se a lista de mensagens recebidas estar vazia
    if ( lora.is_recv_list_empty() ) {
      lora_recv_t message_recv = lora.get_next_recv_message();

      mqtt_t new_message;

      // Verifica se existe clientes wifi conectador no WS
      // if ( wifi_client.number_connected_clients_WS() > 0 ) {
      if ( wifi_client.number_connected_clients_WS() > 0 && message_recv.payload != "" ) {
        // enviar todas as mensagem recebidas pelo radio LoRa
        String wifi_payload = message_recv.payload;
        wifi_payload.replace("]",",");
        wifi_payload += String(message_recv.RSSI) + "," + String(message_recv.SNR, 2) + "]";
        wifi_client.send_message_client_WS(wifi_payload);
      }

      // // colocado para depuracao, retirar depois 
      // Serial.print("message_recv.message_type: ");
      // Serial.println(message_recv.message_type);

      switch (message_recv.message_type)
      {
      case JOIN:
        
        if ( lora.add_node(message_recv) ) {
          lora_send_t message_new_device;

          message_new_device.target_device_id = message_recv.sourcer_device_id;
          message_new_device.sourcer_device_id = device_id;
          message_new_device.message_id = module.hash(device_id);
          message_new_device.message_type = JOIN_ACK;

          lora.add_send_message(message_new_device);

          last_time_request = millis();
        }

        break;

      case REPLAY:

        new_message.id = message_recv.sourcer_device_id;
        new_message.type = 1;
        // new_message.type = message_recv.message_type;
        // new_message.payload = message_recv.payload;
        message_recv.payload.replace("\"","");

        Serial.println(message_recv.payload);
        new_message.payload = message_recv.payload;

        // Reseta o contador de falha REQUEST
        lora.reset_number_no_ack(new_message.id);

        mqtt_client.add_send_message(new_message);

        break;
      
      default:
        break;
      }
    } else {
      // Serial.println(__LINE__);
      // Serial.println(__LINE__);
    }

    // vefica se a lista de mensagens a ser enviadas estar vazia
    if ( lora.is_send_list_empty() ) {
      String message_send = lora.get_next_send_message();
      lora.send_message(message_send);
    }

    // Enviar pedidos de novo registro de dado para os nodes
    // pega o numero de nodes conectado ao gateway
    number_nodes = lora.get_size_list_node();
    // se o numero de node for maior que zero e se já estourou o tempo, enviar um REQUEST ao node
    if ( ( number_nodes > 0 ) && ( millis() > ( last_time_request + 5000 ) ) ) {

      for (uint8_t i = 0; i < number_nodes; i++) {
        if ( lora.request_new_record(i) ) {

          // debug informacao do numero de solicitacao REQUEST sem resposta
          String node_no_ack ="{" + String(i) + ":" + String(lora.get_number_no_ack(i)) + "}";

          Serial.println(node_no_ack);

          // enviar a mensage se existe cliente wifi no WS
          if ( wifi_client.number_connected_clients_WS() > 0 ) {
            wifi_client.send_message_client_WS(node_no_ack);
          }
          
          break;
        }
      }

      last_time_request = millis();
    }

    esp_task_wdt_reset();                                   // Reset watchdog counter 
    vTaskDelay(pdMS_TO_TICKS(LORA_TASK_DELAY_MS));      // Pause Tesk and release the nucleus for the next Tesk in the priority queue
  }
}

void loop() {
  // put your main code here, to run repeatedly:
}