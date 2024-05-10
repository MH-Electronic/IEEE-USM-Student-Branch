#include <MD_Parola.h>

#include <MD_MAX72xx.h>
#include <SPI.h>
#include <string.h>


#include <BluetoothSerial.h>

// Bluetooth Connection
#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif

// Hardware Configuration
// #define HARDWARE_TYPE MD_MAX72XX::FC16_HW
#define HARDWARE_TYPE MD_MAX72XX::GENERIC_HW

#define MAX_DEVICES 4
#define CS_PIN 5
#define PIN_RED    2 
#define PIN_GREEN  0 
#define PIN_BLUE   4 

MD_Parola Display = MD_Parola(HARDWARE_TYPE, CS_PIN, MAX_DEVICES);

// C-string
char word_display[50];

// Bluetooth Class Define
BluetoothSerial SerialBT;

void setup() {
  pinMode(PIN_RED,OUTPUT);
  pinMode(PIN_GREEN,OUTPUT);
  pinMode(PIN_BLUE,OUTPUT);

  Serial.begin(115200);
  SerialBT.begin("ESP32test"); 
  Serial.println("The device started, now you can pair it with bluetooth!");

  Display.begin();
  Display.setIntensity(0);
  Display.displayClear();
}

void loop() {
  // When Bluetooth is connected, the led will emit white light
  if (SerialBT.connected()) {
    analogWrite(PIN_RED, 0);
    analogWrite(PIN_GREEN, 64);
    analogWrite(PIN_BLUE, 255);
  } else{
    analogWrite(PIN_RED, 255);
    analogWrite(PIN_GREEN, 191);
    analogWrite(PIN_BLUE, 0);
  }

  if (Display.displayAnimate()) {
    Display.displayReset();
  }

  // Run the code when Bluetooth device is sending messages
  if (SerialBT.available()) {
    for(int i = 0; i < 10; i++){
      int val_1 = random(200,255);
      int val_2 = random(0,80);
      int val_3 = random(0,55);

      analogWrite(PIN_RED, val_1);
      analogWrite(PIN_GREEN, val_2);
      analogWrite(PIN_BLUE, val_3);
      delay(300);
    }
    
    String receivedWord = SerialBT.readStringUntil('\n');

    receivedWord += " ";

    receivedWord.toCharArray(word_display, sizeof(word_display));

    Display.displayClear();

     Display.displayScroll(word_display, PA_RIGHT, PA_SCROLL_LEFT, 255);
  }
  delay(20);
}
