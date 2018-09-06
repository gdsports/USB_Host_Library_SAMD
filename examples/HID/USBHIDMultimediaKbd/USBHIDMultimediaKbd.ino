#include <hidcomposite.h>
#include <usbhub.h>

// Satisfy the IDE, which needs to see the include statment in the ino too.
#ifdef dobogusinclude
#include <spi4teensy3.h>
#endif
#include <SPI.h>

// Override HIDComposite to be able to select which interface we want to hook into
class HIDSelector : public HIDComposite
{
public:
    HIDSelector(USBHost *p) : HIDComposite(p) {};

protected:
    void ParseHIDData(HID *hid, uint8_t ep, bool is_rpt_id, uint8_t len, uint8_t *buf); // Called by the HIDComposite library
    bool SelectInterface(uint8_t iface, uint8_t proto);
};

// Return true for the interface we want to hook into
bool HIDSelector::SelectInterface(uint8_t iface, uint8_t proto)
{
  Serial.print("HIDSelector::SelectInterface("); Serial.print(iface);
  Serial.print(','); Serial.print(proto); Serial.println(')');
  if (proto != 0)
    return true;

  return false;
}

// Will be called for all HID data received from the USB interface
void HIDSelector::ParseHIDData(HID *hid, uint8_t ep, bool is_rpt_id, uint8_t len, uint8_t *buf) {
  Serial.print("HIDSelector::ParseHIDData(,"); Serial.print(ep);
  Serial.print(','); Serial.print(is_rpt_id);
  Serial.print(','); Serial.print(len); 
  Serial.print(','); Serial.print((uint32_t)buf, HEX); Serial.println(')');
  if (len && buf)  {
    for (uint8_t i = 0; i < len; i++) {
      Serial.print(buf[i], HEX); Serial.print(' ');
    }
    Serial.println();
  }
}

USBHost     UsbH;
//USBHub     Hub(&UsbH);
HIDSelector    hidSelector(&UsbH);

void setup()
{
  Serial.begin( 115200 );
#if !defined(__MIPSEL__)
  while (!Serial); // Wait for serial port to connect - used on Leonardo, Teensy and other boards with built-in USB CDC serial connection
#endif
  Serial.println("Start");

  if (UsbH.Init())
    Serial.println("USB host did not start.");

  delay( 200 );
}

void loop()
{
  UsbH.Task();
}
