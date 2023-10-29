#include <Arduino.h>
#include "Wire.h"

#define I2C_DEV_ADDR 0x55
#include "M5Dial.h"
// #include "USB.h"
// #include "USBHIDKeyboard.h"
// USBHIDKeyboard Keyboard;

long oldPosition = -999;

void setup() {
    auto cfg = M5.config();
    M5Dial.begin(cfg, true, false);
    M5Dial.Display.setTextColor(GREEN);
    M5Dial.Display.setTextDatum(middle_center);
    M5Dial.Display.setTextFont(&fonts::Orbitron_Light_32);
    M5Dial.Display.setTextSize(2);
    Wire.begin(13,15,100000);
      //Write message to the slave

    // Keyboard.begin();
    // USB.begin();
}


void loop() {
    long mills = millis();
    M5Dial.update();
    int16_t newPosition =int16_t(M5Dial.Encoder.read());
    if (newPosition != oldPosition) {
        M5Dial.Speaker.tone(8000, 20);
        M5Dial.Display.clear();
        oldPosition = newPosition;
        // Serial.println(newPosition);
        M5Dial.Display.drawString(String(newPosition),
                                  M5Dial.Display.width() / 2,
                                  M5Dial.Display.height() / 2);
    }
    if (M5Dial.BtnA.wasPressed()) {
        Serial.println(newPosition);
        Wire.beginTransmission(I2C_DEV_ADDR);
        delay(100);
        Wire.write(newPosition>>8);
        Wire.write(newPosition&0xff);
        uint8_t error = Wire.endTransmission(true);
        Serial.printf("endTransmission: %u\n", error);
    }
    if (M5Dial.BtnA.pressedFor(5000)) {
        M5Dial.Encoder.readAndReset(); 
    }
}