#include <hid.h>
#include <hiduniversal.h>
#include <hidescriptorparser.h>
#include <usbhub.h>
#include "pgmstrings.h"

// Satisfy the IDE, which needs to see the include statment in the ino too.
#ifdef dobogusinclude
#include <spi4teensy3.h>
#endif
#include <SPI.h>

class HIDUniversal2 : public HIDUniversal
{
public:
    HIDUniversal2(USBHost *usb) : HIDUniversal(usb) {};

protected:
    uint32_t OnInitSuccessful();
};

uint32_t HIDUniversal2::OnInitSuccessful()
{
    uint8_t    rcode;

    Serial.println("HIDUniversal2::OnInitSuccessful");
    HexDumper<USBReadParser, uint32_t, uint32_t>    Hex;
    ReportDescParser                                Rpt;

    if ((rcode = GetReportDescr(0, &Hex)))
        goto FailGetReportDescr1;

    if ((rcode = GetReportDescr(0, &Rpt)))
	goto FailGetReportDescr2;

    return 0;

FailGetReportDescr1:
    Serial.println("HIDUniversal2::OnInitSuccessful fail1");
    USBTRACE("GetReportDescr1:");
    goto Fail;

FailGetReportDescr2:
    Serial.println("HIDUniversal2::OnInitSuccessful fail2");
    USBTRACE("GetReportDescr2:");
    goto Fail;

Fail:
    Serial.println(rcode, HEX);
    Release();
    return rcode;
}

USBHost UsbH;
//USBHub Hub(&UsbH);
HIDUniversal2 Hid(&UsbH);
UniversalReportParser Uni;

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

  if (!Hid.SetReportParser(0, &Uni))
      ErrorMessage<uint8_t>(PSTR("SetReportParser"), 1  );
}

void loop()
{
    UsbH.Task();
}

