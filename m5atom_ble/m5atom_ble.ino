/**
 * This example turns the ESP32 into a Bluetooth LE keyboard that writes the words, presses Enter, presses a media key and then Ctrl+Alt+Delete
 */
#include <BleKeyboard.h>
#include "M5Atom.h"

BleKeyboard bleKeyboard;

void setup() {
  Serial.begin(115200);
  Serial.println("Starting BLE work!");
  Serial2.begin(115200, SERIAL_8N1, 33, 23);
  
  bleKeyboard.begin();
}

char buf[5];

char mappingkey(char input)
{
    switch(input)
    {
      case 0x30: return 'a';
      case 0x32: return 's';
      case 0x34: return 'd';
      case 0x35: return 'f';
      case 0x37: return 'g';
      case 0x39: return 'h';
      case 0x3b: return 'j';
      case 0x3c: return 'q';
      case 0x3e: return 'w';
      case 0x40: return 'e';
      case 0x41: return 'r';
      case 0x43: return 't';
      case 0x45: return 'y';
      case 0x47: return 'u';
    }
    return 0;
}

void loop() {

  
  if(bleKeyboard.isConnected() && Serial2.readBytes(buf,3)) {

//    for (uint32_t idx = 0; idx < 4; idx++) {
//      Serial.printf("%02x ", buf[idx]);
//    }
//    Serial.printf("\r\n");
    
    if(buf[0]==0x90)
    {
       char button = mappingkey(buf[1]);
       if(button != 0)
       {
//          Serial.printf("push");
          bleKeyboard.press(button);
       }
       else
       {
//          Serial.printf("out");
       }
    }
    else
    {
//       Serial.printf("release");
       bleKeyboard.releaseAll();
    }
    /*
    Serial.println("Sending Ctrl+Alt+Delete...");
    bleKeyboard.press(KEY_LEFT_CTRL);
    bleKeyboard.press(KEY_LEFT_ALT);
    bleKeyboard.press(KEY_DELETE);
    delay(100);
    bleKeyboard.releaseAll();
    */
  }
  
}
