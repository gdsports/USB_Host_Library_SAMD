/*
 Example sketch for the Playstation Buzz library - developed by Kristian Lauszus
 For more information visit my blog: http://blog.tkjelectronics.dk/ or
 send me an e-mail:  kristianl@tkjelectronics.com
 */

#include <PSBuzz.h>

USBHost UsbH;
PSBuzz Buzz(&UsbH);

void setup() {
  Serial.begin(115200);
#if !defined(__MIPSEL__)
  while (!Serial); // Wait for serial port to connect - used on Leonardo, Teensy and other boards with built-in USB CDC serial connection
#endif
  if (UsbH.Init()) {
    Serial.print(F("\r\nUSB host did not start"));
    while (1); // Halt
  }
  Serial.println(F("\r\nPS Buzz Library Started"));
}

void loop() {
  UsbH.Task();

  if (Buzz.connected()) {
    for (uint8_t i = 0; i < 4; i++) {
      if (Buzz.getButtonClick(RED, i)) {
        Buzz.setLedToggle(i); // Toggle the LED
        Serial.println(F("RED"));
      }
      if (Buzz.getButtonClick(YELLOW, i))
        Serial.println(F("YELLOW"));
      if (Buzz.getButtonClick(GREEN, i))
        Serial.println(F("GREEN"));
      if (Buzz.getButtonClick(ORANGE, i))
        Serial.println(F("ORANGE"));
      if (Buzz.getButtonClick(BLUE, i))
        Serial.println(F("BLUE"));
    }
  }
}
