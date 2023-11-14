#include <Arduino.h>
#include "Wire.h"

#define I2C_DEV_ADDR 0x55
#include "M5Dial.h"
// #include "USB.h"
// #include "USBHIDKeyboard.h"
// USBHIDKeyboard Keyboard;


long oldPosition = 0;
unsigned long lastEncoderUpdateTime = 0;
int16_t oldEncoderValue = 0;
int16_t newEncoderValue = 0;
int16_t newPosition = 0;
int16_t encoderDelta = 0;
int16_t encoderChangeThreshold = 5; // 変化量のしきい値
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
    M5Dial.update();
    // エンコーダーの値を読み取り
    newEncoderValue = int16_t(M5Dial.Encoder.read());

    // エンコーダーの変化を時間で追跡
    unsigned long currentTime = millis();
    unsigned long deltaTime = currentTime - lastEncoderUpdateTime;

    if (deltaTime >= 50) { // 50ミリ秒ごとに計算
        // エンコーダーの変化量を計算
        encoderDelta =     newEncoderValue - oldEncoderValue;

        // 変化量がしきい値を超えていれば値を変更
        if (abs(encoderDelta) > 0) {
            newPosition = oldPosition + (encoderDelta*abs(encoderDelta));
            M5Dial.Speaker.tone(8000, 20);
            M5Dial.Display.clear();
            oldPosition = newPosition;
            // Serial.println(newPosition);
            M5Dial.Display.drawString(String(newPosition),
                                  M5Dial.Display.width() / 2,
                                  M5Dial.Display.height() / 2);
            oldEncoderValue = newEncoderValue;
            oldPosition = newPosition;
            lastEncoderUpdateTime = currentTime;
        }
        lastEncoderUpdateTime = currentTime;
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
        oldEncoderValue = 0;
        oldPosition =0;
        lastEncoderUpdateTime = currentTime;
        delay(100);
    }
}