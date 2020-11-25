#include "Arduino.h"
#include <usbhub.h>
#include <PS5USB.h>

#define SerialDebug Serial1

USBHost     usb;
PS5USB PS5(&usb);

bool printAngle, printTouch;
uint8_t oldL2Value, oldR2Value;


void setup()
{
  delay(1000);
  SerialDebug.begin(115200);
  SerialDebug.println("Starting PS5 USB Test");

  SerialDebug.println("Initializing USB");
  if (usb.Init() == -1)
    SerialDebug.println("USBhost did not start.");
  SerialDebug.println("USB Started");

  delay( 20 );
}

uint32_t lastUSBstate;
uint16_t lastMessageCounter = -1;

void loop()
{
  usb.Task();

  if (PS5.connected() && lastMessageCounter!=PS5.getMessageCounter()) {
      SerialDebug.print(PS5.getMessageCounter());
      SerialDebug.print(F("\tLeftHatX: "));
      SerialDebug.print(PS5.getAnalogHat(LeftHatX));
      SerialDebug.print(F("\tLeftHatY: "));
      SerialDebug.print(PS5.getAnalogHat(LeftHatY));
      SerialDebug.print(F("\tRightHatX: "));
      SerialDebug.print(PS5.getAnalogHat(RightHatX));
      SerialDebug.print(F("\tRightHatY: "));
      SerialDebug.print(PS5.getAnalogHat(RightHatY));
      SerialDebug.print(F("\tL2: "));
      SerialDebug.print(PS5.getAnalogButton(L2));
      SerialDebug.print(F("\tR2: "));
      SerialDebug.print(PS5.getAnalogButton(R2));
      SerialDebug.println();

      // set the left trigger to resist at the right trigger's level
      PS5.leftTrigger.setTriggerForce(0, PS5.getAnalogButton(R2), 255);

      if (PS5.getButtonClick(CROSS)) {
        PS5.setLed(255,0,0);
      }
      if (PS5.getButtonClick(SQUARE)) {
        PS5.setLed(0,255,0);
      }
      if (PS5.getButtonClick(CIRCLE)) {
        PS5.setLed(0,0,255);
      }
  }
}