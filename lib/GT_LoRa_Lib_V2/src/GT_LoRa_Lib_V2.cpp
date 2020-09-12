#include "GT_LoRa_Lib_V2.h"

LinkedList<lora_send_t> list_lora_send = LinkedList<lora_send_t>();
LinkedList<lora_recv_t> list_lora_recv = LinkedList<lora_recv_t>();

LinkedList<device_node_t> list_nodes = LinkedList<device_node_t>();

String lora_device_id;
lora_temp_t message_temp;
boolean new_message_temp = false;
int16_t rssi;
float snr;

int8_t search_node(String device_id);

LoRaGatewayLib::LoRaGatewayLib() {}

// LoRa module initialization function
boolean LoRaGatewayLib::begin(String id) {

  SPI.begin(LORA_PIN_SCK, LORA_PIN_MISO, LORA_PIN_MOSI, LORA_PIN_SS);
  // LoRa.setPins(SS, LORA_PIN_RST, LORA_PIN_DIO0);
  LoRa.setPins(LORA_PIN_SS, LORA_PIN_RST, LORA_PIN_DIO0);

  if (!LoRa.begin(LORA_BAND)) {
    Serial.println("LoRa init failed. Check your connections.");
    return false;
  }

  if (synWord) {

    // Serial.println(__LINE__);

    LoRa.setSyncWord(synWord);                                          // ranges from 0-0xFF, default 0x34, see API docs
  }

  LoRa.enableCrc();
  LoRa.onReceive(onReceive);
  SetRxMode();

  lora_device_id = id;

  return true;
}

//
void LoRaGatewayLib::run(void) {

  if ( new_message_temp ) {

    new_message_temp = false;

    // Serial.println(__LINE__);

    lora_recv_t _message_recv = decodeJSON_recv(message_temp.payload);
    _message_recv.RSSI = message_temp.RSSI;
    _message_recv.SNR = message_temp.SNR;

    // verifica se a mensagem recebida é para este device ou uma broadcast
    if ( _message_recv.target_device_id == lora_device_id || ( _message_recv.target_device_id == "ffffff") || ( _message_recv.target_device_id == "FFFFFF") ) {

      // Serial.println(__LINE__);

      list_lora_recv.add(_message_recv);
    }

  }

  /*

  for (uint8_t i = 0; i < list_nodes.size(); i++) {
    device_node_t _device = list_nodes.get(i);
    if ( _device.timeout <= ( millis() - REQUEST_INTERVAL ) ) {
      lora_send_t _message_REQUEST;

      _message_REQUEST.sourcer_device_id = lora_device_id;
      _message_REQUEST.target_device_id = _device.device_id;
      _message_REQUEST.message_id = 255;
      _message_REQUEST.message_type = REQUEST;

      add_send_message(_message_REQUEST);

      _device.timeout = millis();
      list_nodes.set(i, _device);
    }
  }

*/
  


    // para implementar uma rede mesh, completar o codigo para salvar mensagem recebidas que não seja para o node
    // em uma lista de mensagem a ser enviada para o gatway
}

//
void LoRaGatewayLib::add_send_message(lora_send_t msg) {

  Serial.println("ADD_LORA_SEND_MSG");

  list_lora_send.add(msg);
}

//
void LoRaGatewayLib::add_recv_message(lora_recv_t msg) {

  Serial.println("ADD_LORA_RECV_MSG");
  
  list_lora_recv.add(msg);
}

//
String LoRaGatewayLib::get_next_send_message(void) {
  lora_send_t _send_msg = list_lora_send.remove(0);

  return encodeJSON_send(_send_msg);
}

//
lora_recv_t LoRaGatewayLib::get_next_recv_message(void) {
  return list_lora_recv.remove(0);
}

// is the receiving list empty
boolean LoRaGatewayLib::is_send_list_empty(void) {
  return list_lora_send.size();
}

// is the receiving list empty
boolean LoRaGatewayLib::is_recv_list_empty(void) {
  return list_lora_recv.size();
}

// is the receiving list empty
boolean LoRaGatewayLib::is_node_list_empty(void) {
  return list_nodes.size();
}

// 
boolean LoRaGatewayLib::send_message(String strSend) {

  bool result = false;

  Serial.print("LORA_SEND: ");
  Serial.println(strSend);

  SetTxMode();                        // set tx mode
  LoRa.beginPacket();                 // start packet
  LoRa.print(strSend);                // add payload
  LoRa.endPacket();                   // finish packet and send it
  SetRxMode();                        // set rx mode

  result = true;
  return result;
}

//
boolean LoRaGatewayLib::add_node(lora_recv_t msg_recv) {

  Serial.print("Number of the devices: ");
  Serial.println(list_nodes.size());

  for (uint8_t i = 0; i < list_nodes.size(); i++) {
    if ( msg_recv.sourcer_device_id == list_nodes.get(i).device_id ) {
      return false;
    }
  }
  
  device_node_t _new_device;
  _new_device.device_id = msg_recv.sourcer_device_id;
  _new_device.RSSI = msg_recv.RSSI;
  _new_device.SNR = msg_recv.SNR;
  _new_device.timeout = millis();
  _new_device.counter_no_ack = 0;

  return list_nodes.add(_new_device);
}

// is the receiving list empty
uint8_t LoRaGatewayLib::get_size_list_node(void) {
  return list_nodes.size();
}

// retorna o numero de tentativas de REQUEST sem resposta do node
uint8_t LoRaGatewayLib::get_number_no_ack(uint8_t index) {
  return list_nodes.get(index).counter_no_ack;
}

// reseta o contador de falha de REQUEST
void LoRaGatewayLib::reset_number_no_ack(String device_id) {
  int8_t index_node = search_node(device_id);
  device_node_t _node_rst = list_nodes.get(index_node);
  _node_rst.counter_no_ack = 0;
  list_nodes.set(index_node, _node_rst);
}

//
boolean LoRaGatewayLib::request_new_record(uint8_t index) {

  device_node_t _device = list_nodes.get(index);

  if ( _device.timeout <= ( millis() - REQUEST_INTERVAL ) ) {

    // verifica se o node execedeu o numero de tentativa de REQUEST
    if ( _device.counter_no_ack > MAX_NUMBER_NO_ACK ) {
      list_nodes.remove(index);
      return false;
    }

    lora_send_t _message_REQUEST;

    _message_REQUEST.sourcer_device_id = lora_device_id;
    _message_REQUEST.target_device_id = _device.device_id;
    _message_REQUEST.message_id = 255;
    _message_REQUEST.message_type = REQUEST;

    add_send_message(_message_REQUEST);

    _device.timeout = millis();
    _device.counter_no_ack = _device.counter_no_ack + 1;

    list_nodes.set(index, _device);

    return true;
  }

  return false;
}

// Automatically executed every time the LoRa module receives a new message
void LoRaGatewayLib::onReceive(int packetSize) {
  if (packetSize == 0) return;

  String _packetRecv = "";
  // float _snr = LoRa.packetSnr();
  // int _rssi = LoRa.packetRssi();
  // message_recv.SNR = LoRa.packetSnr();
  // message_recv.RSSI = LoRa.packetRssi();

  while (LoRa.available()) {          // ler a string recebida pelo modulo LoRa

    _packetRecv += (char)LoRa.read();

  }

  if (_packetRecv != "") {

    // Serial.println(__LINE__);
    Serial.print("LORA_RECV: ");
    Serial.println(_packetRecv);

    message_temp.SNR = LoRa.packetSnr();
    message_temp.RSSI = LoRa.packetRssi();
    message_temp.payload = _packetRecv;
    new_message_temp = true;

  }

}

// 
void LoRaGatewayLib::SetRxMode(void) {
    LoRa.disableInvertIQ();               // normal mode
    LoRa.receive();                       // set receive mode
}

// 
void LoRaGatewayLib::SetTxMode(void) {
    LoRa.idle();                          // set standby mode
    LoRa.enableInvertIQ();                // active invert I and Q signals
}

// Convert a messageLoRa_t to a String structure
String LoRaGatewayLib::encodeJSON_recv(lora_recv_t msg) {

  String _strJSON;

  StaticJsonDocument<256> _doc;

  JsonObject _root = _doc.to<JsonObject>();

  _root["sd"] = msg.sourcer_device_id;
  _root["td"] = msg.target_device_id;
  _root["md"] = msg.message_id;
  _root["tp"] = msg.message_type;
  _root["nb"] = msg.neighbor_id_list;
  _root["hp"] = msg.hops;
  _root["sn"] = msg.SNR;
  _root["rs"] = msg.RSSI;
  _root["pl"] = msg.payload;

  if (serializeJson(_doc, _strJSON) == 0) {
    Serial.println(F("Failed to write to file"));
  }

  return _strJSON;
}

// Convert a messageLoRa_t to a String structure
String LoRaGatewayLib::encodeJSON_send(lora_send_t msg) {

  String _strJSON;

  StaticJsonDocument<256> _doc;

  JsonObject _root = _doc.to<JsonObject>();

  _root["sd"] = msg.sourcer_device_id;
  _root["td"] = msg.target_device_id;
  _root["md"] = msg.message_id;
  _root["tp"] = msg.message_type;
  _root["nb"] = msg.neighbor_id_list;
  _root["hp"] = msg.hops;
  _root["pl"] = msg.payload;

  if (serializeJson(_doc, _strJSON) == 0) {
    Serial.println(F("Failed to write to file"));
  }

  return _strJSON;
}

//
lora_recv_t LoRaGatewayLib::decodeJSON_recv(String strJSON) {
  lora_recv_t _message;

  StaticJsonDocument<250> _doc;

  DeserializationError error = deserializeJson(_doc, strJSON);

  if (error) {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.c_str());
    return _message;
  }

  JsonObject _root = _doc.as<JsonObject>();

  String _sourcer_device_id = _root["sd"];
  String _target_device_id = _root["td"];
  String _message_id = _root["md"];
  uint8_t _message_type = _root["tp"];
  String _neighbor_id_list = _root["nb"];
  uint8_t _hops = _root["hp"];
  // float _snr = _root["sn"];
  // float _rssi = _root["rs"];
  String _payload = _root["pl"];

  _message.sourcer_device_id = _sourcer_device_id;
  _message.target_device_id = _target_device_id;
  _message.message_id = _message_id;
  _message.message_type = _message_type;
  _message.neighbor_id_list = _neighbor_id_list;
  _message.hops = _hops;
  // _message.SNR = _snr;
  // _message.RSSI = _rssi;
  _message.payload = _payload;

  return _message;
}

//
lora_send_t LoRaGatewayLib::decodeJSON_send(String strJSON) {
  lora_send_t _message;

  StaticJsonDocument<200> _doc;

  DeserializationError error = deserializeJson(_doc, strJSON);

  if (error) {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.c_str());
    return _message;
  }

  JsonObject _root = _doc.as<JsonObject>();

  String _sourcer_device_id = _root["sd"];
  String _target_device_id = _root["td"];
  String _message_id = _root["md"];
  uint8_t _message_type = _root["tp"];
  String _neighbor_id_list = _root["nb"];
  uint8_t _hops = _root["hp"];
  String _payload = _root["pl"];

  _message.sourcer_device_id = _sourcer_device_id;
  _message.target_device_id = _target_device_id;
  _message.message_id = _message_id;
  _message.message_type = _message_type;
  _message.neighbor_id_list = _neighbor_id_list;
  _message.hops = _hops;
  _message.payload = _payload;

  return _message;
}

int8_t search_node(String device_id) {
  for (uint8_t i = 0; i < list_nodes.size(); i++) {
    if ( list_nodes.get(i).device_id == device_id ) {
      return i;
    }
  }
  return -1; 
}