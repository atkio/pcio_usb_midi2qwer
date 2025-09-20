#include "Adafruit_TinyUSB.h"
#include <Arduino.h>
#include "pico/multicore.h"
#include "pico/util/queue.h"

// --- 新增: 用于管理按键状态 ---
// 最多同时支持6个非修饰键
#define MAX_PRESSED_KEYS 6 
// 全局数组，存储当前被按下的所有键码
uint8_t pressed_keycodes[MAX_PRESSED_KEYS] = {0}; 
// ------------------------------

queue_t queue;

uint8_t const desc_hid_report[] = {
    TUD_HID_REPORT_DESC_KEYBOARD()
};

Adafruit_USBD_HID usb_hid;

// --- 此处省略了 pins[], pincount, hidcode[] 的定义，因为它们在你的新逻辑中没有被使用 ---

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

void setup() {
  Serial1.begin(115200);
  Serial2.begin(115200);
  Serial2.setTimeout(10);

  queue_init(&queue,sizeof(int32_t),10);
  multicore_launch_core1(uart_task);  

  if (!TinyUSBDevice.isInitialized()) {
    TinyUSBDevice.begin(0);
  }

  usb_hid.setBootProtocol(HID_ITF_PROTOCOL_KEYBOARD);
  usb_hid.setPollInterval(2);
  usb_hid.setReportDescriptor(desc_hid_report, sizeof(desc_hid_report));
  usb_hid.setStringDescriptor("TinyUSB Keyboard");
  usb_hid.setReportCallback(NULL, hid_report_callback);
  usb_hid.begin();

  if (TinyUSBDevice.mounted()) {
    TinyUSBDevice.detach();
    delay(10);
    TinyUSBDevice.attach();
  }

  Serial.println("RP2040 Keyboard Ready.");
}

// --- 此处省略了 process_hid() 函数，因为它在你的新逻辑中没有被使用 ---

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

// --- 新增: 辅助函数，用于管理 pressed_keycodes 数组 ---

// 添加一个键码到数组的第一个空位
void add_keycode(uint8_t keycode) {
  if (keycode == 0) return;
  for (int i = 0; i < MAX_PRESSED_KEYS; i++) {
    if (pressed_keycodes[i] == 0) {
      pressed_keycodes[i] = keycode;
      return;
    }
  }
}

// 从数组中移除一个键码
void remove_keycode(uint8_t keycode) {
  if (keycode == 0) return;
  for (int i = 0; i < MAX_PRESSED_KEYS; i++) {
    if (pressed_keycodes[i] == keycode) {
      pressed_keycodes[i] = 0; // 将其位置清零
      // 注意：一个更完善的实现会整理数组以防空洞，但对于多数场景这已足够
    }
  }
}
// --------------------------------------------------------


void loop() {  
  // not enumerated()/mounted() yet: nothing to do
  if (!TinyUSBDevice.mounted()) {
    return;
  }

  // skip if hid is not ready e.g still transferring previous report
  if (!usb_hid.ready()) return;  

  if(queue_try_remove(&queue, buf))
  {
    Serial1.printf("RX: %02x %02x %02x %02x\r\n", buf[0], buf[1], buf[2], buf[3]);

    uint8_t keycode = mappingkey(buf[1]);
    
    // 判断是按下还是松开
    if(buf[0] == 0x80) // 松开按键
    {
      remove_keycode(keycode);
    }
    else // 按下按键
    {
      add_keycode(keycode);
    }
    
    // 无论按下还是松开，都根据 pressed_keycodes 数组的当前状态发送报告
    usb_hid.keyboardReport(0, 0, pressed_keycodes);
    Serial1.print("Sent report for keys: ");
    for(int i=0; i<MAX_PRESSED_KEYS; i++){
      if(pressed_keycodes[i] != 0){
        Serial1.print(pressed_keycodes[i], HEX);
        Serial1.print(" ");
      }
    }
    Serial1.println();
  }    
}

// Output report callback for LED indicator such as Caplocks
void hid_report_callback(uint8_t report_id, hid_report_type_t report_type, uint8_t const* buffer, uint16_t bufsize) {
  (void) report_id;
  (void) bufsize;

  if (report_type != HID_REPORT_TYPE_OUTPUT) return;

  uint8_t ledIndicator = buffer[0];
  // digitalWrite(LED_BUILTIN, ledIndicator & KEYBOARD_LED_CAPSLOCK);
}
