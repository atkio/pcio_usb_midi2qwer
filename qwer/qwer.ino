/*********************************************************************
 Adafruit invests time and resources providing this open source code,
 please support Adafruit and open-source hardware by purchasing
 products from Adafruit!

 MIT license, check LICENSE for more information
 Copyright (c) 2019 Ha Thach for Adafruit Industries
 All text above, and the splash screen below must be included in
 any redistribution
*********************************************************************/

#include "Adafruit_TinyUSB.h"
#include <Arduino.h>
#include "pico/multicore.h"
#include "pico/util/queue.h"

queue_t queue;

/* This sketch demonstrates USB HID keyboard.
 * - PIN A0-A3 is used to send digit '0' to '3' respectively
 *   (On the RP2040, pins D0-D5 used)
 * - LED and/or Neopixels will be used as Capslock indicator
 */

// HID report descriptor using TinyUSB's template
// Single Report (no ID) descriptor
uint8_t const desc_hid_report[] = {
    TUD_HID_REPORT_DESC_KEYBOARD()
};

// USB HID object. For ESP32 these values cannot be changed after this declaration
// desc report, desc len, protocol, interval, use out endpoint
Adafruit_USBD_HID usb_hid;

//------------- Input Pins -------------//
// Array of pins and its keycode.
// Notes: these pins can be replaced by PIN_BUTTONn if defined in setup()
#ifdef ARDUINO_ARCH_RP2040
uint8_t pins[] = { D0, D1, D2, D3 };
#else
uint8_t pins[] = {A0, A1, A2, A3};
#endif

// number of pins
uint8_t pincount = sizeof(pins) / sizeof(pins[0]);

// For keycode definition check out https://github.com/hathach/tinyusb/blob/master/src/class/hid/hid.h
uint8_t hidcode[] = {HID_KEY_0, HID_KEY_1, HID_KEY_2, HID_KEY_3};

#if defined(ARDUINO_SAMD_CIRCUITPLAYGROUND_EXPRESS) || defined(ARDUINO_NRF52840_CIRCUITPLAY) || defined(ARDUINO_FUNHOUSE_ESP32S2)
bool activeState = true;
#else
bool activeState = false;
#endif

void uart_task()
{
  char buf[4];
  while(true)
  {
    if(Serial2.readBytes(buf,4))
    {
      queue_try_add(&queue,(void*)buf);
    }
  }
}

// the setup function runs once when you press reset or power the board
void setup() {
  Serial1.begin(115200);
  Serial2.begin(115200);
  Serial2.setTimeout(10);

  queue_init(&queue,sizeof(int32_t),10);
  multicore_launch_core1(uart_task);  

  // Manual begin() is required on core without built-in support e.g. mbed rp2040
  if (!TinyUSBDevice.isInitialized()) {
    TinyUSBDevice.begin(0);
  }

  // Setup HID
  usb_hid.setBootProtocol(HID_ITF_PROTOCOL_KEYBOARD);
  usb_hid.setPollInterval(2);
  usb_hid.setReportDescriptor(desc_hid_report, sizeof(desc_hid_report));
  usb_hid.setStringDescriptor("TinyUSB Keyboard");

  // Set up output report (on control endpoint) for Capslock indicator
  usb_hid.setReportCallback(NULL, hid_report_callback);

  usb_hid.begin();

  // If already enumerated, additional class driverr begin() e.g msc, hid, midi won't take effect until re-enumeration
  if (TinyUSBDevice.mounted()) {
    TinyUSBDevice.detach();
    delay(10);
    TinyUSBDevice.attach();
  }

  Serial2.println("end of main");

  
}

void process_hid() {
  // used to avoid send multiple consecutive zero report for keyboard
  static bool keyPressedPreviously = false;

  uint8_t count = 0;
  uint8_t keycode[6] = {0};

  // scan normal key and send report
  for (uint8_t i = 0; i < pincount; i++) {
    if (activeState == digitalRead(pins[i])) {
      // if pin is active (low), add its hid code to key report
      keycode[count++] = hidcode[i];

      // 6 is max keycode per report
      if (count == 6) break;
    }
  }

  if (TinyUSBDevice.suspended() && count) {
    // Wake up host if we are in suspend mode
    // and REMOTE_WAKEUP feature is enabled by host
    TinyUSBDevice.remoteWakeup();
  }

  // skip if hid is not ready e.g still transferring previous report
  if (!usb_hid.ready()) return;

  if (count) {
    // Send report if there is key pressed
    uint8_t const report_id = 0;
    uint8_t const modifier = 0;

    keyPressedPreviously = true;
    usb_hid.keyboardReport(report_id, modifier, keycode);
  } else {
    // Send All-zero report to indicate there is no keys pressed
    // Most of the time, it is, though we don't need to send zero report
    // every loop(), only a key is pressed in previous loop()
    if (keyPressedPreviously) {
      keyPressedPreviously = false;
      usb_hid.keyboardRelease(0);
    }
  }
}

char buf[4];
uint8_t mappingkey(uint8_t input)
{
    switch(input)
    {
      //
      case 0x30 - 0xc: return HID_KEY_Z;
      case 0x32 - 0xc: return HID_KEY_X;
      case 0x34 - 0xc: return HID_KEY_C;
      case 0x35 - 0xc: return HID_KEY_V;
      case 0x37 - 0xc: return HID_KEY_B;
      case 0x39 - 0xc: return HID_KEY_N;
      case 0x3b - 0xc: return HID_KEY_M;

      //
      case 0x30: return HID_KEY_A;
      case 0x32: return HID_KEY_S;
      case 0x34: return HID_KEY_D;
      case 0x35: return HID_KEY_F;
      case 0x37: return HID_KEY_G;
      case 0x39: return HID_KEY_H;
      case 0x3b: return HID_KEY_J;
      //
      case 0x3c: return HID_KEY_Q;
      case 0x3e: return HID_KEY_W;
      case 0x40: return HID_KEY_E;
      case 0x41: return HID_KEY_R;
      case 0x43: return HID_KEY_T;
      case 0x45: return HID_KEY_Y;
      case 0x47: return HID_KEY_U;

      //
      case 0x3c + 0xc: return HID_KEY_1;
      case 0x3e + 0xc: return HID_KEY_2;
      case 0x40 + 0xc: return HID_KEY_3;
      case 0x41 + 0xc: return HID_KEY_4;
      case 0x43 + 0xc: return HID_KEY_5;
      case 0x45 + 0xc: return HID_KEY_6;
      case 0x47 + 0xc: return HID_KEY_7;
    }
    return 0;
}
void loop() {  
  #ifdef TINYUSB_NEED_POLLING_TASK
  // Manual call tud_task since it isn't called by Core's background
  TinyUSBDevice.task();
  #endif
  
  // not enumerated()/mounted() yet: nothing to do
  if (!TinyUSBDevice.mounted()) {
    return;
  }

  if (TinyUSBDevice.suspended()) {
    // Wake up host if we are in suspend mode
    // and REMOTE_WAKEUP feature is enabled by host
    TinyUSBDevice.remoteWakeup();
  }

  // skip if hid is not ready e.g still transferring previous report
  if (!usb_hid.ready()) return;  


  if(queue_try_remove(&queue,buf))
  {
    for (uint32_t idx = 0; idx < 4; idx++) {
      Serial1.printf("%02x ", buf[idx]);
    }
    Serial1.printf("\r\n");

    if(buf[0]==0x80)
    {
        usb_hid.keyboardReport(0, 0, 0);
    }
    else
    {
       uint8_t keycode[6] = {mappingkey(buf[1]),0,0,0,0,0};
       usb_hid.keyboardReport(0, 0, keycode);
    }

  }  
}

// Output report callback for LED indicator such as Caplocks
void hid_report_callback(uint8_t report_id, hid_report_type_t report_type, uint8_t const* buffer, uint16_t bufsize) {
  (void) report_id;
  (void) bufsize;

  // LED indicator is output report with only 1 byte length
  if (report_type != HID_REPORT_TYPE_OUTPUT) return;

  // The LED bit map is as follows: (also defined by KEYBOARD_LED_* )
  // Kana (4) | Compose (3) | ScrollLock (2) | CapsLock (1) | Numlock (0)
  uint8_t ledIndicator = buffer[0];

  // turn on LED if capslock is set
  // digitalWrite(LED_BUILTIN, ledIndicator & KEYBOARD_LED_CAPSLOCK);
}
