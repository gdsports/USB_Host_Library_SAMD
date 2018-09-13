#include <hid.h>
#include <hiduniversal.h>
#include <usbhub.h>

#include "hidjoystickrptparser.h"

USBHost UsbH;
USBHub Hub(&UsbH);
HIDUniversal Hid(&UsbH);
JoystickEvents JoyEvents;
JoystickReportParser Joy(&JoyEvents);

void setup() {
        Serial.begin(115200);
#if !defined(__MIPSEL__)
        while (!Serial); // Wait for serial port to connect - used on Leonardo, Teensy and other boards with built-in USB CDC serial connection
#endif
        Serial.println("Start");

        if (UsbH.Init())
                Serial.println("USB host did not start.");

        delay(200);

        if (!Hid.SetReportParser(0, &Joy))
                ErrorMessage<uint8_t > (PSTR("SetReportParser"), 1);
}

void loop() {
        UsbH.Task();
}

