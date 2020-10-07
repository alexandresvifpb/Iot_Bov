#include "GT_WiFi_Lib_V2.h"

/**
 * Class WiFi_GateWay
 **/

// WEBSERVER
const uint server_port = WIFI_WEB_SERVER_PORT;
WiFiServer gt_webServer(server_port);
WiFiClient gt_wifi_client;
WiFiClient gt_remote_client;
String wifi_device_id;
bool is_active_service_WS = false;      // verificar se realmente eh necessario

//  OTA
void start_OTA(void);
void end_OTA(void);
void progress_OTA(unsigned int progress, unsigned int total);
void error_OTA(ota_error_t error);
String get_value(String data, char separator, int index);

// WiFi_GateWay class constructor
WiFi_GateWay::WiFi_GateWay() {}

// WiFi
uint8_t const number_wifi_routers = 4;
String ssid[number_wifi_routers] = {"XTEC - ACARSV", "brisa-594111", "AMD", "AndroidAP"};   // {"brisa-594111", "XTEC - ACARSV", "AMD"};
String pass[number_wifi_routers] = {"mtek2003", "gbalklxz", "amd12345678", "rkzr1138"};     // {"gbalklxz", "mtek2003", "amd12345678"};

// Initializes the class
bool WiFi_GateWay::begin(String device_id) {
  Serial.println("Begin WiFi ...");
  wifi_device_id = device_id;
  return true;
}

//
bool WiFi_GateWay::connect_wifi_STD_mode(void) {

  /*
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_STA_SSID, WIFI_STA_PASS);

  Serial.println("Wait connecting to WiFi...");

  uint8_t _contWhile = 30;
  while ( (WiFi.status() != WL_CONNECTED) && (_contWhile) ) {
    Serial.print("WIFI_TRY #");
    Serial.println(_contWhile);
    WiFi.reconnect();
    _contWhile--;
    delay(1000);
  }
  */

  /*
  for (uint8_t i = 0; i < number_wifi_routers; i++) {
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid[i].c_str(),pass[i].c_str());

    Serial.print("Wait connecting to WiFi ");
    Serial.println(ssid[i]);

    uint8_t _contWhile = 30;
    while ( (WiFi.status() != WL_CONNECTED) && (_contWhile) ) {
      Serial.print("WIFI_TRY #");
      Serial.println(_contWhile);
      WiFi.reconnect();
      _contWhile--;
      delay(1000);
    }

    if (active_webserver())
      return true;
      
    WiFi.mode(WIFI_OFF);
  }
  */
  
  bool _connected = false;

  for (uint8_t i = 0; i < 30; i++) {

    if (_connected) {
      Serial.println("CONNECTED.... ");
      return true;
    }

    Serial.print("WIFI_TRY #");
    Serial.println(i);

    for (uint8_t j = 0; j < number_wifi_routers; j++) {

      WiFi.mode(WIFI_STA);
      WiFi.begin(ssid[j].c_str(),pass[j].c_str());

      Serial.print("Wait connecting to WiFi ");
      Serial.println(ssid[j]);
      delay(5000);

      if (active_webserver()) {
        _connected = true;
        break;
      } else {
        WiFi.reconnect();
        delay(2000);
        WiFi.mode(WIFI_OFF);
      }
      
    }
    
  }
   

  return active_webserver();
}

//
bool WiFi_GateWay::active_webserver(void) {
  if ( is_wifi_connected() ) {
    gt_webServer.begin(WIFI_WEB_SERVER_PORT);
    return true;
  }
  return false;
}

//
bool WiFi_GateWay::is_wifi_connected(void) {
  if ( WiFi.status() == WL_CONNECTED ) {
    return true;
  }
  return false;
}

// OTA

// Informs if the connection to the wifi network has been established
bool WiFi_GateWay::active_OTA(void) {
  if ( is_wifi_connected() ) {
    // The port defaults to 3232
    // ArduinoOTA.setPort(3232);

    // Defines the hostname (optional)
    String _hostname = WIFI_HOSTNAME + wifi_device_id;
    ArduinoOTA.setHostname(_hostname.c_str());

    // Sets the password (optional)
    //   ArduinoOTA.setPassword("amd1234");

    // It is possible to set a hash md5 encryption for the password using the "setPasswordHash" function
    // MD5 example for password "admin" = 21232f297a57a5a743894a0e4a801fc3
    // ArduinoOTA.setPasswordHash("21232f297a57a5a743894a0e4a801fc3");

    // Defines what will be executed when ArduinoOTA starts
    ArduinoOTA.onStart(start_OTA); // startOTA is a function created to simplify the code

    // Defines what will be executed when the ArduinoOTA is finished
    ArduinoOTA.onEnd(end_OTA); // endOTA is a function designed to simplify the code 

    // Defines what will be performed when ArduinoOTA is recording
    ArduinoOTA.onProgress(progress_OTA); // progressOTA is a function created to simplify the code 

    // Defines what will be executed when ArduinoOTA encounters an error
    ArduinoOTA.onError(error_OTA);// errorOTA is a function created to simplify the code
  
    // Initializes ArduinoOTA
    ArduinoOTA.begin();

    // Send IP and Hostname to serial port
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    Serial.print("Hostname: ");
    Serial.println(_hostname);

    return true;
  } else {
    return false;
  }
}

// ArduinoOTA upload stages display functions (start, progress, end and error)
void start_OTA(void) {
  String type;
   
  // If the update is being saved to the external flash memory, then inform "flash"
  if (ArduinoOTA.getCommand() == U_FLASH)
    type = "flash";
  else  // If the update is done via the internal memory (file system), then inform "filesystem"
    type = "filesystem"; // U_SPIFFS

  // Displays message next to the recording type
  Serial.println("Start updating " + type);
}

// Send the outgoing message to serial
void end_OTA(void) {
  Serial.println("\nEnd");
}

// Serialize progress in percentage
void progress_OTA(unsigned int progress, unsigned int total) {
  Serial.printf("Progress: %u%%\r", (progress / (total / 100))); 
}

// Sends serial messages if an error occurs, specifically displays the type of error
void error_OTA(ota_error_t error) {
  Serial.printf("Error[%u]: ", error);
      
  if (error == OTA_AUTH_ERROR) 
    Serial.println("Auth Failed");
  else
    if (error == OTA_BEGIN_ERROR)
      Serial.println("Begin Failed");
    else 
    if (error == OTA_CONNECT_ERROR)
      Serial.println("Connect Failed");
    else
    if (error == OTA_RECEIVE_ERROR) 
      Serial.println("Receive Failed");
    else 
    if (error == OTA_END_ERROR)
      Serial.println("End Failed");
}

//
// https://stackoverflow.com/questions/9072320/split-string-into-string-array
String get_Value(String data, char separator, int index)
{
  int found = 0;
  int strIndex[] = {0, -1};
  int maxIndex = data.length()-1;

  for(int i=0; i<=maxIndex && found<=index; i++){
    if(data.charAt(i)==separator || i==maxIndex){
        found++;
        strIndex[0] = strIndex[1]+1;
        strIndex[1] = (i == maxIndex) ? i+1 : i;
    }
  }

  return found>index ? data.substring(strIndex[0], strIndex[1]) : "";
}

// WEBSERVER

// 
void WiFi_GateWay::listen_for_clients_WS(void) {
  // Serial.print(__FUNCTION__);
  // Serial.println(__LINE__);
  if ( gt_webServer.hasClient() ) {
    // If we are already connected to another computer, 
    // then reject the new connection. Otherwise accept
    // the connection. 
    if ( gt_remote_client.connected() ) {
      Serial.println("Connection rejected");
      gt_webServer.available().stop();
    } else {
      Serial.println("Connection accepted");
      gt_remote_client = gt_webServer.available();

      is_active_service_WS = true;
    }
  }
}

// Retorna o numero de clientes conectado no WebServer
uint8_t WiFi_GateWay::number_connected_clients_WS(void) {
  return gt_remote_client.connected();
}

// Retorna tamanho dos dados recebido do cliente remoto
int WiFi_GateWay::available_recv_data_client_WS(void) {
  if ( gt_remote_client.connected() )     {
    return gt_remote_client.available();
  } else {
    gt_remote_client.stop();
    return 0;
  }
}

// Retorna uma string com os dados enviado pelo cliente remoto
String WiFi_GateWay::recv_message_client_WS(void) {
  // if ( gt_remote_client ) {
  if ( number_connected_clients_WS() ) {
    return gt_remote_client.readStringUntil('\n');
  }
  return "";
}

// Enviua uma string para o cliente remoto
void WiFi_GateWay::send_message_client_WS(String message) {
  if ( number_connected_clients_WS() ) {
    gt_remote_client.println(message.c_str());
  }
}








/**
 * Class MQTT_Client
 **/

// 
LinkedList<mqtt_t> list_send_send = LinkedList<mqtt_t>();
LinkedList<mqtt_t> list_recv_send = LinkedList<mqtt_t>();

// WiFiClient espWiFiClient;
PubSubClient gt_mqtt_client(gt_wifi_client);
String mqtt_topic_subscribe = "";
bool mqtt_broker_connected = false;

// Funcoes privadas
void callback(char* topic, byte* payload, unsigned int length);
mqtt_t encoder_Str_to_JSON(String strJson);
String decode_JSON_to_Str(mqtt_t msg_mqtt);

// Funcao construtora da classe
MQTT_Client::MQTT_Client() {}

//
boolean MQTT_Client::begin(void) {

  Serial.print(__FUNCTION__);
  Serial.println(__LINE__);
  
  mqtt_topic_subscribe = MQTT_TOPIC_SUBSCRIBE + wifi_device_id;

  Serial.print("mqtt_topic_subscribe");
  Serial.println(mqtt_topic_subscribe);

  active_MQTT();
  return true;
}

//
void MQTT_Client::run(void) {
  gt_mqtt_client.loop();
}

//
bool MQTT_Client::active_MQTT(void) {
  if ( WiFi.status() == WL_CONNECTED ) {
        
    Serial.print("MQTT Server: ");
    Serial.print(MQTT_SERVER);
    Serial.print(":");
    Serial.println(MQTT_PORT);

    gt_mqtt_client.setServer( MQTT_SERVER, MQTT_PORT);
    gt_mqtt_client.setCallback(callback);

    send_message("Client MQTT device " + wifi_device_id + " actived!");

    mqtt_broker_connected = true;
    return mqtt_broker_connected;
  }

  return mqtt_broker_connected;
}

//
bool MQTT_Client::reconnect_MQTT(void) {

  // Loop until we're reconnected
  bool _gt_connected = false;
  uint8_t _cont = 12;
  
  while ( !gt_mqtt_client.connected() && _cont ) {
    Serial.print("Attempting MQTT connection...");
        
    // Attempt to connect
    if (gt_mqtt_client.connect("ESP32Client")) {
            
      Serial.println("connected");
            
      // Subscribe
      gt_mqtt_client.subscribe("esp32/output");
      _gt_connected = true;

    } else {

      Serial.print("failed, rc=");
      Serial.print(gt_mqtt_client.state());
      Serial.println(" try again in 5 seconds");
            
      // Wait 5 seconds before retrying
      _cont--;
      delay(5000);
    }
  }
    
  return _gt_connected;
}

//
void MQTT_Client::add_send_message(mqtt_t message) {
  list_send_send.add(message);
}

//
String MQTT_Client::get_next_send_message(void) {
  mqtt_t _send_message = list_send_send.remove(0);

  return decode_JSON_to_Str(_send_message);
}

//
mqtt_t MQTT_Client::get_next_recv_message(void) {
  return list_recv_send.remove(0);
}

//
bool MQTT_Client::is_send_list_empty(void) {
  return list_send_send.size();
}

//
bool MQTT_Client::is_recv_list_empty(void) {
  return list_recv_send.size();
}

//
void MQTT_Client::send_message(String message) {
  if ( !mqtt_broker_connected ) {
    mqtt_broker_connected = reconnect_MQTT();
    if ( !mqtt_broker_connected ) return;
  }

  Serial.print("MQTT_SEND: ");
  Serial.print(MQTT_TOPIC_PUBLISH);
  Serial.print(" ");
  Serial.println(message);

  // message.replace("\"[","[");
  // message.replace("]\"","]");
  // Serial.println(message);

  gt_mqtt_client.publish(MQTT_TOPIC_PUBLISH, message.c_str());

}

void send_message(mqtt_t message) {

  String _strJSON;

  StaticJsonDocument<256> _doc;

  JsonObject _root = _doc.to<JsonObject>();

  _root["id"] = message.id;
  _root["type"] = 9;
  // _root["payload"] = message.payload;

  JsonArray _payload  = _root.createNestedArray("payload");
  _payload.add(1);
  _payload.add(2);
  _payload.add(3);

  if (serializeJson(_doc, _strJSON) == 0) {
    Serial.println(F("Failed to write to file"));
  }

  gt_mqtt_client.publish(MQTT_TOPIC_PUBLISH, _strJSON.c_str());
}

//Private
void callback(char* topic, byte* message, unsigned int length) {

  Serial.print("RCEV_MQTT: ");
  String messageTemp;
  
  for (int i = 0; i < length; i++) {
    messageTemp += (char)message[i];
  }

  if (messageTemp != "") {
    Serial.println(messageTemp);
  }

}

// Funcao para criar uma string codificada em JSON
// Inputs:  String id - endereco do dispositivo
//          uint8_t type - tipo do dados a ser salvo
//          String payload - dados a ser salvo
// Return: String retorna uma string com os dados codificado em JSON
mqtt_t encoder_Str_to_JSON(String strJson) {

  mqtt_t _message;

  StaticJsonDocument<250> doc;
    
  DeserializationError error = deserializeJson(doc, strJson);

  if (error) {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.c_str());
    return _message;
  }

  JsonObject root = doc.as<JsonObject>();

  String id = root["id"];
  uint8_t type = root["type"];
  String payload = root["payload"];

  _message.id = id;
  _message.type = type;
  _message.payload = payload;

  return _message;

}

String decode_JSON_to_Str(mqtt_t msg_mqtt) {

  String _strJSON;

  StaticJsonDocument<256> _doc;

  JsonObject _root = _doc.to<JsonObject>();

  _root["id"] = msg_mqtt.id;
  _root["type"] = msg_mqtt.type;
  _root["payload"] = msg_mqtt.payload;

  if (serializeJson(_doc, _strJSON) == 0) {
    Serial.println(F("Failed to write to file"));
  }

  return _strJSON;
}