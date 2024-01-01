#include "Arduino.h"
#include "M5Dial.h"
#include <esp_now.h>
#include <WiFi.h>
#include <esp_wifi.h> 
#define CHANNEL 1
// Global copy of slave
uint8_t broadcastAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
esp_now_peer_info_t peerInfo;
uint8_t data[3];
int16_t oldPosition = 2000;

void InitESPNow();
void deletePeer();
void ScanForSlave(); 
bool manageSlave();
void sendData();
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status);

// Init ESP Now with fallback
void InitESPNow() {
  WiFi.disconnect();
  if (esp_now_init() == ESP_OK) {
    Serial.println("ESPNow Init Success");
  }
  else {
    Serial.println("ESPNow Init Failed");
    // Retry InitESPNow, add a counte and then restart?
    // InitESPNow();
    // or Simply Restart
    ESP.restart();
  }
}

// callback when data is sent from Master to Slave
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  char macStr[18];
  snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
           mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
  Serial.print("Last Packet Sent to: "); Serial.println(macStr);
  Serial.print("Last Packet Send Status: "); Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

void setup() {
    auto cfg = M5.config();
    M5Dial.begin(cfg, true, false);
    M5Dial.Display.setTextColor(GREEN);
    M5Dial.Display.setTextDatum(middle_center);
    M5Dial.Display.setTextFont(&fonts::Orbitron_Light_32);
    M5Dial.Display.setTextSize(2);
      //Set device in STA mode to begin with
    WiFi.mode(WIFI_STA);
    esp_wifi_set_channel(CHANNEL, WIFI_SECOND_CHAN_NONE);
    Serial.println("ESPNow Master");
    // This is the mac address of the Master in Station Mode
    Serial.print("STA MAC: "); Serial.println(WiFi.macAddress());
    Serial.print("STA CHANNEL "); Serial.println(WiFi.channel());
    // Init ESPNow with a fallback logic
    InitESPNow();
        // Register peer
    memcpy(peerInfo.peer_addr, broadcastAddress, 6);
    peerInfo.channel = CHANNEL;  
    peerInfo.encrypt = false;
    
    // Add peer        
    if (esp_now_add_peer(&peerInfo) != ESP_OK){
      Serial.println("Failed to add peer");
      return;
    }
    // Once ESPNow is successfully Init, we will register for Send CB to
    // get the status of Trasnmitted packet
    esp_now_register_send_cb(OnDataSent);
}



void loop() {
    M5Dial.update();
    int16_t newPosition = M5Dial.Encoder.read();
    if (newPosition != oldPosition) {
        M5Dial.Speaker.tone(8000, 20);
        M5Dial.Display.clear();
        oldPosition = newPosition;
        Serial.println(newPosition);
        M5Dial.Display.drawString(String(newPosition),
                                  M5Dial.Display.width() / 2,
                                  M5Dial.Display.height() / 2);
        data[0] =  (uint8_t)(newPosition&0xff);
        data[1] = (uint8_t)((newPosition >> 8) & 0xFF);
        data[2] = 0x00;
  
        const uint8_t *peer_addr = peerInfo.peer_addr;
        Serial.print("Sending: "); Serial.printf("%02x %02x  \n",data[0],data[1]);
        esp_err_t result = esp_now_send(peer_addr, data, sizeof(&data));
    }
    if (M5Dial.BtnA.wasClicked()) {
        M5Dial.Encoder.readAndReset();
    }
    if (M5Dial.BtnA.pressedFor(2000)) {
        // M5Dial.Encoder.readAndReset();
        data[2] = 0xff;
        const uint8_t *peer_addr = peerInfo.peer_addr;
        Serial.print("Sending: "); Serial.printf("%02x %02x  \n",data[0],data[1]);
        esp_err_t result = esp_now_send(peer_addr, data, sizeof(&data));
    }
}